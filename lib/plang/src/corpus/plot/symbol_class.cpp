//
// Created by Johannes on 25.09.2022.
//

#include <sqlite3.h>
#include <plang/corpus/plot/symbol.hpp>

using namespace plang;
using namespace plang::op;
using namespace plang::plot;
using namespace plang::detail;

ostream_t &symbol_class_manager::print_helper(ostream_t &os, pkey<plot::symbol::clazz> id, format format) const {
    auto inner_format = format;
    inner_format.set_detail(format::detail::EXPLICIT_REF);
    auto clazz = fetch(id, format.get_detail() >= format::detail::DEFINITION).value();

    if (!clazz.hints.empty()) {
        os << format("(", format::style::for_op(lang::op::HINT_L));
        for (size_t i = 0; i < clazz.hints.size(); ++i) {
            auto const &h = clazz.hints[i];
            print_helper(os, h.get_hint_id(), inner_format);
            if (h.recursive) { os << format("...", format::style::for_op(lang::op::RECUR)); }
            if (i != clazz.hints.size() - 1) { os << format(",", format::style::for_op(lang::op::LIST)) << ' '; }
        }
        os << format(")", format::style::for_op(lang::op::HINT_R)) << ' ';
    }

    os << path_manager::print(clazz.get_path_id(), inner_format) << format("[]", format::style::for_op(lang::op::SYM));

    switch (format.get_detail()) {

        case format::detail::ID_REF:
        case format::detail::EXPLICIT_REF:
            break;
        case format::detail::DEFINITION:
        case format::detail::FULL:
            path_manager::print_decoration(os, clazz.get_path_id(), format);
            os << format(";", format::style::for_op(lang::op::DECL));
            break;
    }

    return os;
}

std::vector<symbol::clazz::hint> symbol_class_manager::_fetch_hints(pkey<plot::symbol::clazz> id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, hint_id, recursive, source_id FROM plot_symbol_class_hint
WHERE class_id = ?;
)__SQL__"};

    class stmt stmt;

    return _reuse(stmt, query, [&]() {
        std::vector<symbol::clazz::hint> hints;

        sqlite3_bind_int64(&*stmt, 1, id);

        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol::clazz::hint hint;
            hint.id        = sqlite3_column_int(&*stmt, 0);
            hint.hint_id   = sqlite3_column_int(&*stmt, 1);
            hint.recursive = sqlite3_column_int(&*stmt, 2);
            hint.source_id = sqlite3_column_int(&*stmt, 3);

            hints.push_back(hint);
        }

        return hints;
    });
}

void symbol_class_manager::_update_hints(pkey<plot::symbol::clazz> id, std::vector<symbol::clazz::hint> &hints) {
    //language=sqlite
    static const string_t query_h{R"__SQL__(
INSERT INTO plot_symbol_class_hint (id, class_id, hint_id, recursive, source_id)
VALUES((SELECT coalesce(max(id) + 1, 0) FROM plot_symbol_class_hint), ?1, ?2, ?3, ?4)
RETURNING plot_symbol_class_hint.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_r{R"__SQL__(
DELETE FROM plot_symbol_class_hint
WHERE class_id = $1;
)__SQL__"};

    stmt stmt;

    class stmt stmt_r;

    _reuse(stmt_r, query_r, [&]() {
        sqlite3_bind_int64(&*stmt_r, 1, id);
        _check(sqlite3_step(&*stmt_r));
    });

    for (auto &h : hints) {
        h.id = _reuse(stmt, query_h, [&]() -> pkey<plot::symbol::clazz::hint> {
            sqlite3_bind_int64(&*stmt, 1, id);
            sqlite3_bind_int64(&*stmt, 2, h.hint_id);
            sqlite3_bind_int64(&*stmt, 3, h.recursive);
            sqlite3_bind_int64(&*stmt, 4, _get_source_id());

            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                return sqlite3_column_int(&*stmt, 0);
            } else {
                return -1;
            }
        });
    }
}

std::optional<symbol::clazz> symbol_class_manager::fetch(pkey<plot::symbol::clazz> id,
                                                         bool_t dynamic,
                                                         tag<symbol::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, source_id FROM plot_symbol_class
WHERE id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    auto clazz = _reuse(stmt, query, [&]() -> std::optional<symbol::clazz> {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.source_id = sqlite3_column_int(&*stmt, 2);
            return clazz;
        } else {
            return std::nullopt;
        }
    });

    if (clazz.has_value() && dynamic) { clazz->hints = _fetch_hints(id); }

    return clazz;
}

std::vector<symbol::clazz> symbol_class_manager::fetch_n(const std::vector<pkey<plot::symbol::clazz>> &ids,
                                                         bool_t dynamic,
                                                         tag<symbol::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, source_id FROM plot_symbol_class
WHERE id in (SELECT key
             FROM json_each(?1, '$'));
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<symbol::clazz> result;
    result.reserve(ids.size());

    string_t json_array{[&ids] {
        if (ids.empty()) {
            return string_t("[]");
        } else {
            sstream_t ss{};
            ss << "[";
            for (auto &n : ids) ss << std::to_string(n) << ",";
            ss.seekp(-1, std::ios_base::end);
            ss << "]";
            return ss.str();
        }
    }()};

    _reuse(stmt, query, [&] {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.size(), nullptr);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.source_id = sqlite3_column_int(&*stmt, 2);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

std::vector<symbol::clazz>
symbol_class_manager::fetch_all(bool_t dynamic, int_t limit, int_t offset, tag<symbol::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, source_id FROM plot_symbol_class
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<symbol::clazz> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int(&*stmt, 1, limit);
        sqlite3_bind_int(&*stmt, 2, offset);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.source_id = sqlite3_column_int(&*stmt, 2);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

resolve_result<plot::symbol::clazz> symbol_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class path &ctx,
                                                                  bool_t insert,
                                                                  bool_t dynamic,
                                                                  corpus::tag<plot::symbol::clazz>) {
    symbol::clazz clazz;
    auto result          = resolve(path, clazz, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::symbol::clazz> symbol_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class path &ctx,
                                                                  plang::column_types::bool_t dynamic,
                                                                  corpus::tag<plot::symbol::clazz> tag) const {
    return const_cast<symbol_class_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::symbol::clazz> symbol_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::symbol::clazz &ent,
                                                                      const class path &ctx,
                                                                      bool_t insert,
                                                                      bool_t dynamic) {
    if (path.empty()) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, path_id, source_id FROM plot_symbol_class
WHERE path_id = ?;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        auto result = _reuse(stmt, query, [&]() -> std::optional<symbol::clazz> {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());
            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                symbol::clazz clazz;
                clazz.id        = sqlite3_column_int(&*stmt, 0);
                clazz.path_id   = sqlite3_column_int(&*stmt, 1);
                clazz.source_id = sqlite3_column_int(&*stmt, 2);
                return clazz;
            } else {
                return std::nullopt;
            }
        });

        if (result.has_value()) {
            ent = result.value();

            if (dynamic) { ent.hints = _fetch_hints(ent.id); }

            return {ent, {}, action::NONE};
        } else if (insert) {
            ent.set_path(ctx);
            symbol_class_manager::insert(ent);
            return {ent, {}, action::INSERT};
        } else {
            return {ent, {}, action::FAIL};
        }
    } else {
        auto path_result = path_manager::resolve(path, ctx, insert, dynamic);
        if (path_result.has_result()) {
            return resolve({}, ent, path_result.entry(), insert, dynamic);
        } else {
            return {ent, {}, action::FAIL};
        }
    }
}

resolve_ref_result<plot::symbol::clazz> symbol_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::symbol::clazz &ent,
                                                                      const class path &ctx,
                                                                      bool_t dynamic) const {
    return const_cast<symbol_class_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

action symbol_class_manager::insert(plot::symbol::clazz &clazz, bool_t replace, tag<plot::symbol::clazz>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
INSERT OR REPLACE INTO plot_symbol_class (id, path_id, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_symbol_class), ?1, ?2)
RETURNING plot_symbol_class.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
INSERT INTO plot_symbol_class (id, path_id, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_symbol_class), ?1, ?2)
RETURNING plot_symbol_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &stmt  = replace ? stmt_r : stmt_n;
    auto &query = replace ? query_r : query_n;

    action action = action::FAIL;

    clazz.id = _reuse(stmt, query, [&]() -> pkey<plot::symbol::clazz> {
        sqlite3_bind_int64(&*stmt, 1, clazz.path_id);
        sqlite3_bind_int64(&*stmt, 2, _get_source_id());
        int id = -1;
        if (auto ret = sqlite3_step(&*stmt); ret == SQLITE_ROW) {
            clazz.id = id   = sqlite3_column_int(&*stmt, 0);
            clazz.source_id = _get_source_id();
            action          = action::INSERT;
        } else if (ret != SQLITE_DONE && replace) {
            return id;
        }
        if (sqlite3_step(&*stmt) == SQLITE_DONE) {
            if (action == action::FAIL) action = action::NONE;
            return id;
        } else {
            return clazz.id;
        }
    });

    if (action != action::FAIL) { _update_hints(clazz.id, clazz.hints); }

    return action;
}

action symbol_class_manager::update(plot::symbol::clazz &clazz, bool_t dynamic, corpus::tag<plot::symbol::clazz>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
UPDATE plot_symbol_class
SET path_id   = ?2,
    source_id = ?3
WHERE id = ?1
RETURNING plot_symbol_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    action action = action::FAIL;

    if (!clazz.is_persisted()) { return action::FAIL; }

    _reuse(stmt, query, [&]() -> class symbol::clazz & {
        sqlite3_bind_int64(&*stmt, 1, clazz.get_id());
        sqlite3_bind_int64(&*stmt, 2, clazz.get_path_id());
        sqlite3_bind_int64(&*stmt, 3, _get_source_id());

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            action = action::UPDATE;
        } else {
            action = action::NONE;
        }

        return clazz;
    });

    if (action != action::FAIL && dynamic) { _update_hints(clazz.id, clazz.hints); }

    return action;
}

stream_helper symbol_class_manager::print(pkey<plot::symbol::clazz> id,
                                          format format,
                                          corpus::tag<class symbol::clazz>) const {
    return [this, id, format](auto &os) -> ostream_t & { return print_helper(os, id, format); };
}

std::tuple<string_t, action> symbol_class_manager::remove(plot::symbol::clazz &clazz,
                                                          bool_t cascade,
                                                          corpus::tag<plot::symbol::clazz>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE
FROM plot_symbol_class
WHERE id = ?1
  and (?2 or (not exists(SELECT * FROM plot_symbol s WHERE s.class_id = ?1)) and
             not exists(SELECT * FROM plot_symbol_class_hint h WHERE h.hint_id = ?1 and h.class_id != ?1) and
             not exists(SELECT * FROM plot_point_class_hint h WHERE h.hint_id = ?1) and
             not exists(SELECT * FROM plot_point_subject_cls c WHERE c.subject_id = ?1))
RETURNING id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    if (clazz.is_persisted()) {
        return _reuse(stmt, query, [&]() -> std::tuple<string_t, action> {
            sqlite3_bind_int64(&*stmt, 1, clazz.get_id());
            sqlite3_bind_int(&*stmt, 2, cascade ? 1 : 0);
            string_t repr = print(clazz.get_id(), get_format())();

            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                auto id  = sqlite3_column_int64(&*stmt, 0);
                clazz.id = -1;
                return {repr, action::REMOVE};
            } else {
                return {repr, action::FAIL};
            }
        });
    } else {
        return {"", action::FAIL};
    }
}

symbol_class_manager::~symbol_class_manager() = default;
