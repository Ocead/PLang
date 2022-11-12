//
// Created by Johannes on 05.11.2022.
//

#include <sqlite3.h>
#include <plang/corpus/plot/point.hpp>

using namespace plang;
using namespace plang::op;
using namespace plang::plot;
using namespace plang::detail;

ostream_t &point_class_manager::print_helper(plang::column_types::ostream_t &os,
                                             pkey<plot::point::clazz> id,
                                             plang::format format) const {
    auto inner_format = format;
    inner_format.set_detail(format::detail::EXPLICIT_REF);
    auto clazz = fetch(id, format.get_detail() >= format::detail::DEFINITION).value();

    if (!clazz.hints.empty()) {
        os << format("(", format::style::for_op(lang::op::HINT_L));
        for (size_t i = 0; i < clazz.hints.size(); ++i) {
            auto const &h = clazz.hints[i];
            symbol_class_manager::print_helper(os, h.get_hint_id(), inner_format);
            if (h.recursive) { os << format("...", format::style::for_op(lang::op::RECUR)); }
            if (i != clazz.hints.size() - 1) { os << format(",", format::style::for_op(lang::op::LIST)) << ' '; }
        }
        os << format(")", format::style::for_op(lang::op::HINT_R)) << ' ';
    }

    switch (format.get_detail()) {

        case format::detail::ID_REF:
        case format::detail::EXPLICIT_REF:
            os << path_manager::print(clazz.get_path_id(), inner_format)
               << format("?:", format::style::for_op(lang::op::OBJ_NAME));
            break;
        case format::detail::DEFINITION:
        case format::detail::FULL:
            path_manager::print_decoration(os, clazz.get_path_id(), format);
            os << format(";", format::style::for_op(lang::op::DECL));
            break;
    }

    return os;
}

std::vector<plot::point::clazz::hint> point_class_manager::_fetch_hints(pkey<plot::point::clazz> id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, hint_id, recursive, source_id FROM plot_point_class_hint
WHERE class_id = ?;
)__SQL__"};

    class stmt stmt;

    return _reuse(stmt, query, [&]() {
        std::vector<point::clazz::hint> hints;

        sqlite3_bind_int64(&*stmt, 1, id);

        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            point::clazz::hint hint;
            hint.id        = sqlite3_column_int(&*stmt, 1);
            hint.hint_id   = sqlite3_column_int(&*stmt, 2);
            hint.recursive = sqlite3_column_int(&*stmt, 3);
            hint.source_id = sqlite3_column_int(&*stmt, 4);

            hints.push_back(hint);
        }

        return hints;
    });
}

void point_class_manager::_update_hints(pkey<plot::point::clazz> id, std::vector<plot::point::clazz::hint> &hints) {
    //language=sqlite
    static const string_t query_h{R"__SQL__(
INSERT INTO plot_point_class_hint (id, class_id, hint_id, recursive, source_id)
VALUES((SELECT coalesce(max(id) + 1, 0) FROM plot_point_class_hint), ?1, ?2, ?3, ?4)
RETURNING plot_point_class_hint.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_r{R"__SQL__(
DELETE FROM plot_point_class_hint
WHERE class_id = $1;
)__SQL__"};

    stmt stmt;

    class stmt stmt_r;

    _reuse(stmt_r, query_r, [&]() {
        sqlite3_bind_int64(&*stmt_r, 1, id);
        _check(sqlite3_step(&*stmt_r));
    });

    for (auto &h : hints) {
        h.id = _reuse(stmt, query_h, [&]() -> pkey<plot::point::clazz::hint> {
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

std::optional<plot::point::clazz> point_class_manager::fetch(pkey<plot::point::clazz> id,
                                                             bool_t dynamic,
                                                             tag<plot::point::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, singleton, source_id FROM plot_point_class
WHERE id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    auto clazz = _reuse(stmt, query, [&]() -> std::optional<point::clazz> {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            point::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.singleton = sqlite3_column_int(&*stmt, 2);
            clazz.source_id = sqlite3_column_int(&*stmt, 3);
            return clazz;
        } else {
            return std::nullopt;
        }
    });

    if (clazz.has_value() && dynamic) { clazz->hints = _fetch_hints(id); }

    return clazz;
}

std::vector<plot::point::clazz> point_class_manager::fetch_n(const std::vector<pkey<plot::point::clazz>> &ids,
                                                             bool_t dynamic,
                                                             tag<plot::point::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, singleton, source_id FROM plot_point_class
WHERE id in (SELECT value
             FROM json_each(?1, '$'));
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<point::clazz> result;
    result.reserve(ids.size());

    string_t json_array = vector_to_json(ids);

    _reuse(stmt, query, [&]() {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.size(), nullptr);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            point::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.singleton = sqlite3_column_int(&*stmt, 2);
            clazz.source_id = sqlite3_column_int(&*stmt, 3);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

std::vector<plot::point::clazz>
point_class_manager::fetch_all(bool_t dynamic, int_t limit, int_t offset, tag<plot::point::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, path_id, singleton, source_id FROM plot_point_class
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<point::clazz> result;

    _reuse(stmt, query, [&]() {
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            point::clazz clazz;
            clazz.id        = sqlite3_column_int(&*stmt, 0);
            clazz.path_id   = sqlite3_column_int(&*stmt, 1);
            clazz.singleton = sqlite3_column_int(&*stmt, 2);
            clazz.source_id = sqlite3_column_int(&*stmt, 3);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

resolve_result<plot::point::clazz> point_class_manager::resolve(const std::vector<string_t> &path,
                                                                const class path &ctx,
                                                                bool_t insert,
                                                                bool_t dynamic,
                                                                corpus::tag<plot::point::clazz>) {
    point::clazz clazz;
    auto result          = resolve(path, clazz, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::point::clazz> point_class_manager::resolve(const std::vector<string_t> &path,
                                                                const class path &ctx,
                                                                bool_t dynamic,
                                                                corpus::tag<plot::point::clazz> tag) const {
    return const_cast<point_class_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::point::clazz> point_class_manager::resolve(const std::vector<string_t> &path,
                                                                    plot::point::clazz &ent,
                                                                    const class path &ctx,
                                                                    bool_t insert,
                                                                    bool_t dynamic) {
    if (path.empty()) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, path_id, singleton, source_id FROM plot_point_class
WHERE path_id = ?1;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        auto result = _reuse(stmt, query, [&]() -> std::optional<point::clazz> {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());
            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                point::clazz clazz;
                clazz.id        = sqlite3_column_int64(&*stmt, 0);
                clazz.path_id   = sqlite3_column_int64(&*stmt, 1);
                clazz.singleton = sqlite3_column_int(&*stmt, 2);
                clazz.source_id = sqlite3_column_int64(&*stmt, 3);
                return clazz;
            } else {
                return std::nullopt;
            }
        });

        if (result.has_value()) {
            ent = std::move(result.value());

            if (dynamic) { ent.hints = _fetch_hints(ent.id); }

            return {ent, {}, action::NONE};
        } else if (insert) {
            ent.set_path(ctx);
            auto action = point_class_manager::insert(ent);
            return {ent, {}, action};
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

resolve_ref_result<plot::point::clazz> point_class_manager::resolve(const std::vector<string_t> &path,
                                                                    plot::point::clazz &ent,
                                                                    const class path &ctx,
                                                                    plang::column_types::bool_t dynamic) const {
    return const_cast<point_class_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

action point_class_manager::insert(plot::point::clazz &clazz, bool_t replace, corpus::tag<plot::point::clazz>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
INSERT OR REPLACE INTO plot_point_class (id, path_id, singleton, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_point_class), ?1, ?2, ?3)
RETURNING plot_point_class.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
INSERT INTO plot_point_class (id, path_id, singleton, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_point_class), ?1, ?2, ?3)
RETURNING plot_point_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &stmt  = replace ? stmt_r : stmt_n;
    auto &query = replace ? query_r : query_n;

    action action = action::FAIL;

    if (clazz.path_id < 0) { return action; }

    clazz.id = _reuse(stmt, query, [&]() -> pkey<plot::point::clazz> {
        sqlite3_bind_int64(&*stmt, 1, clazz.get_path_id());
        sqlite3_bind_int(&*stmt, 2, clazz.get_singleton() ? 1 : 0);
        sqlite3_bind_int64(&*stmt, 3, _get_source_id());
        pkey<plot::point::clazz> id;
        if (auto ret = sqlite3_step(&*stmt); ret == SQLITE_ROW) {
            clazz.id = id   = sqlite3_column_int64(&*stmt, 0);
            clazz.source_id = _get_source_id();
            action          = action::INSERT;
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

action point_class_manager::update(plot::point::clazz &clazz, bool_t dynamic, corpus::tag<plot::point::clazz>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
UPDATE plot_point_class
SET path_id = ?2,
    singleton = ?3,
    source_id = ?4
WHERE id = ?1
RETURNING plot_point_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    action action = action::FAIL;

    _reuse(stmt, query, [&]() -> plot::point::clazz & {
        sqlite3_bind_int64(&*stmt, 1, clazz.get_id());
        sqlite3_bind_int64(&*stmt, 2, clazz.get_path_id());
        sqlite3_bind_int(&*stmt, 3, clazz.get_singleton() ? 1 : 0);
        sqlite3_bind_int64(&*stmt, 4, _get_source_id());

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

stream_helper point_class_manager::print(pkey<plot::point::clazz> id,
                                         plang::format format,
                                         corpus::tag<plot::point::clazz>) const {
    return [this, id, format](auto &os) -> ostream_t & { return print_helper(os, id, format); };
}

std::tuple<string_t, action> point_class_manager::remove(plot::point::clazz &clazz,
                                                         bool_t cascade,
                                                         corpus::tag<plot::point::clazz>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE FROM plot_point_class
WHERE id = ?1
  and (?2 or (not exists(SELECT * FROM plot_point_subject_sym WHERE subject_id = ?1) and
             not exists(SELECT * FROM plot_object_class_hint_pnt WHERE hint_id = ?1) and
             not exists(SELECT * FROM plot_causal_pnt WHERE class_id = ?1)))
RETURNING id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::tuple<string_t, action> {
        sqlite3_bind_int(&*stmt, 1, clazz.get_id());
        sqlite3_bind_int(&*stmt, 2, cascade);
        string_t repr = clazz.is_persisted() ? print(clazz.get_id(), get_format())() : "";

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return {repr, action::REMOVE};
        } else {
            return {repr, action::FAIL};
        }
    });
}

point_class_manager::~point_class_manager() = default;
