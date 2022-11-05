//
// Created by Johannes on 04.10.2022.
//

#include <sqlite3.h>
#include <plang/corpus/plot/symbol.hpp>

using namespace plang;
using namespace plang::op;
using namespace plang::plot;
using namespace plang::detail;

symbol symbol_manager::select_symbol(stmt &stmt, bool_t dynamic) const {
    symbol sym;
    sym.id       = sqlite3_column_int64(&*stmt, 0);
    sym.name     = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1)));
    sym.class_id = sqlite3_column_int64(&*stmt, 2);
    if (sqlite3_column_type(&*stmt, 3) != SQLITE_NULL) { sym.ordinal = sqlite3_column_double(&*stmt, 3); }
    if (dynamic && sqlite3_column_type(&*stmt, 4) != SQLITE_NULL) {
        sym.description = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 4)));
    }
    sym.source_id = sqlite3_column_int64(&*stmt, 5);
    return sym;
}

std::vector<std::tuple<pkey<path>, pkey<plot::symbol::clazz>, pkey<plot::symbol>, uint_t>>
symbol_manager::partially_resolve(const std::vector<string_t> &path, bool_t fully) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH RECURSIVE ar AS (SELECT ?1 as ray),
               cte(path_id, leaf_id, parent_id, class_id, symbol_id, depth)
                   AS (SELECT p.id, p.id, p.id, c.id, s.id, iif(not ?2, 0, -1)
                       FROM plot_symbol s,
                            ar
                                LEFT JOIN plot_symbol_class c on s.class_id = c.id
                                LEFT JOIN path p on c.path_id = p.id
                       WHERE s.name = json_extract(ar.ray, '$[' || 0 || ']')
                       UNION ALL
                       SELECT c.id, cte.leaf_id, c.parent_id, cte.class_id, cte.symbol_id, cte.depth + 1
                       FROM cte,
                            ar
                                LEFT JOIN path c on cte.parent_id = c.id
                       WHERE depth < json_array_length(ar.ray)
                         and c.name == json_extract(ar.ray, '$[' || (cte.depth + 1) || ']')),
               max as (SELECT leaf_id, max(depth) as depth
                       FROM cte
                       GROUP BY leaf_id)
SELECT path_id, class_id, symbol_id, cte.depth + 1 as depth
FROM max
         LEFT JOIN cte on max.leaf_id = cte.leaf_id and max.depth = cte.depth;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    auto json = vector_to_json(path.crbegin(), path.crend());

    std::vector<std::tuple<pkey<class path>, pkey<plot::symbol::clazz>, pkey<plot::symbol>, uint_t>> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_text(&*stmt, 1, json.c_str(), json.size(), nullptr);
        sqlite3_bind_int(&*stmt, 2, fully ? 1 : 0);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            result.emplace_back(sqlite3_column_int64(&*stmt, 0),
                                sqlite3_column_int64(&*stmt, 1),
                                sqlite3_column_int64(&*stmt, 2),
                                sqlite3_column_int(&*stmt, 3));
        }
    });

    return result;
}

ostream_t &symbol_manager::print_helper(ostream_t &os, pkey<plot::symbol> id, format format) const {
    auto inner_format = format;
    inner_format.set_detail(format::detail::EXPLICIT_REF);
    auto sym = fetch(id, format.get_detail() >= format::detail::DEFINITION).value();
    auto clazz =
            symbol_class_manager::fetch(sym.get_class_id(), format.get_detail() >= format::detail::DEFINITION).value();

    os << path_manager::print(clazz.get_path_id(), inner_format) << format("[", format::style::for_op(lang::op::SYM_L))
       << sym.get_name();

    switch (format.get_detail()) {

        case format::detail::ID_REF:
        case format::detail::EXPLICIT_REF:
            break;
        case format::detail::DEFINITION:
        case format::detail::FULL:
            print_decoration(os, sym.get_id(), format);
            break;
    }
    os << format("]", format::style::for_op(lang::op::SYM_R));

    if (format.get_detail() >= format::detail::DEFINITION) {
        os << format(";", format::style::for_op(lang::op::DECL));
    }

    return os;
}

ostream_t &symbol_manager::print_decoration(ostream_t &os, pkey<plot::symbol> id, format format) const {
    auto p = fetch(id, true).value();
    if (p.ordinal.has_value() || p.description.has_value()) {
        os << ' ' << format("(", format::style::for_op(lang::op::HINT_L));

        if (p.ordinal.has_value()) {
            auto ord = std::to_string(p.get_ordinal().value());
            ord.erase(ord.find_last_not_of('0') + 1, std::string::npos);
            ord.erase(ord.find_last_not_of('.') + 1, std::string::npos);
            os << format(ord, format::style{.text = format::style::color::BRIGHT_BLUE});
            if (p.description.has_value()) { os << format(",", format::style::for_op(lang::op::LIST)) << ' '; }
        }

        if (p.description.has_value()) {
            auto desc    = p.get_description().value();
            char_t delim = desc.find('"') != string_t::npos ? '\'' : '"';
            sstream_t ss;
            ss << std::quoted(desc, delim);
            os << format(ss.str(), format::style::for_op(lang::op::STR_DELIM_DOUBLE));
        }

        os << format(")", format::style::for_op(lang::op::HINT_R));
    }

    return os;
}

std::optional<plot::symbol> symbol_manager::fetch(pkey<plot::symbol> id, bool_t dynamic, tag<plot::symbol>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
WHERE id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    auto sym = _reuse(stmt, query, [&]() -> std::optional<plot::symbol> {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return select_symbol(stmt, dynamic);
        } else {
            return std::nullopt;
        }
    });

    return sym;
}

std::vector<plot::symbol> symbol_manager::fetch_n(const std::vector<pkey<plot::symbol>> &ids,
                                                  bool_t dynamic,
                                                  tag<plot::symbol>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
WHERE id in (SELECT value
             FROM json_each(?1, '$'));
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<symbol> result;
    result.reserve(ids.size());

    string_t json_array = vector_to_json(ids);

    _reuse(stmt, query, [&] {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.size(), nullptr);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol sym = select_symbol(stmt, dynamic);
            result.push_back(std::move(sym));
        }
    });

    return result;
}

std::vector<plot::symbol>
symbol_manager::fetch_all(bool_t dynamic, int_t limit, int_t offset, tag<plot::symbol>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<symbol> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int(&*stmt, 1, limit);
        sqlite3_bind_int(&*stmt, 2, offset);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            symbol sym = select_symbol(stmt, dynamic);
            result.push_back(std::move(sym));
        }
    });

    return result;
}

resolve_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                     const class path &ctx,
                                                     bool_t insert,
                                                     bool_t dynamic,
                                                     corpus::tag<plot::symbol>) {
    symbol sym;
    auto result          = resolve(path, sym, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                     const class path &ctx,
                                                     bool_t dynamic,
                                                     corpus::tag<plot::symbol> tag) const {
    return const_cast<symbol_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                         plot::symbol &ent,
                                                         const class path &ctx,
                                                         bool_t insert,
                                                         bool_t dynamic) {
    if (path.empty()) { return {ent, {}, action::FAIL}; }

    if (insert) {
        std::vector<string_t> class_path = path;
        class_path.pop_back();
        auto clazz = symbol_class_manager::resolve(class_path, ctx, insert, dynamic);
        if (clazz.has_result()) {
            auto result = resolve({*path.rbegin()}, ent, clazz.entry(), insert, dynamic);
            return result;
        } else {
            return {ent, {}, action::FAIL};
        }
    }

    auto result = partially_resolve(path, false);

    if (result.size() == 1) { //Case: overall single candidate
        if (std::get<uint_t>(result[0]) == path.size()) {
            ent = fetch(std::get<pkey<symbol>>(result[0]), dynamic).value();
            return {ent, {}, action::NONE};
        } else {
            return {ent, {fetch(std::get<pkey<symbol>>(result[0]), dynamic).value()}, action::FAIL};
        }
    } else {  //Case: none or multiple candidates
        decltype(result) related;
        for (auto const &e : result) {
            if (is_parent_of(ctx.get_id(), std::get<pkey<class path>>(e)) or
                is_parent_of(std::get<pkey<class path>>(e), ctx.get_id())) {
                related.push_back(e);
            }
        }

        if (related.size() == 1) { //Case: single candidate along current path
            if (std::get<uint_t>(related[0]) == path.size()) {
                ent = fetch(std::get<pkey<symbol>>(related[0]), dynamic).value();
                return {ent, {}, action::NONE};
            } else {
                return {ent, {fetch(std::get<pkey<symbol>>(result[0]), dynamic).value()}, action::FAIL};
            }
        } else { //Case: none or multiple candidates along current path
            std::vector<pkey<symbol>> ids;
            ids.reserve(result.size());
            uint_t min = 0;
            uint_t max = 0;
            if (result.size() > 0) {
                auto [mit,
                      xit] = std::minmax_element(result.begin(), result.end(), [](auto const &rhs, auto const &lhs) {
                    return std::get<uint_t>(rhs) < std::get<uint_t>(lhs);
                });
                min        = std::get<uint_t>(*mit);
                max        = std::get<uint_t>(*xit);
            }

            for (auto const &e : result) {
                if (std::get<uint_t>(e) >= (max != 1 ? max : min)) { ids.push_back(std::get<pkey<symbol>>(e)); }
            }
            if (ids.size() == 1) { //Case: single most viable candidate along path
                ent = fetch(ids[0]).value();
                return {ent, {}, action::NONE};
            } else { //Case: none or most viable candidates along path
                auto candidates = fetch_n(ids, dynamic);
                return {ent, std::move(candidates), action::FAIL};
            }

        }
    }
}

resolve_ref_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                         plot::symbol &ent,
                                                         const class path &ctx,
                                                         bool_t dynamic) const {
    return const_cast<symbol_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

resolve_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                     const class plot::symbol::clazz &ctx,
                                                     bool_t insert,
                                                     bool_t dynamic,
                                                     corpus::tag<plot::symbol>) {
    symbol sym;
    auto result          = resolve(path, sym, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                     const class plot::symbol::clazz &ctx,
                                                     bool_t dynamic,
                                                     corpus::tag<plot::symbol> tag) const {
    return const_cast<symbol_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                         plot::symbol &ent,
                                                         const class plot::symbol::clazz &ctx,
                                                         bool_t insert,
                                                         bool_t dynamic) {
    if (path.empty()) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
WHERE class_id = ?;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        std::vector<symbol> result;

        _reuse(stmt, query, [&] {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());
            while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                symbol sym = select_symbol(stmt, dynamic);
                result.push_back(std::move(sym));
            }
        });

        return {ent, result, action::FAIL};
    } else if (path.size() == 1) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
WHERE name = ?1 and class_id = ?2;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        auto result = _reuse(stmt, query, [&]() -> std::optional<symbol> {
            sqlite3_bind_text(&*stmt, 1, path[0].c_str(), path[0].size(), nullptr);
            sqlite3_bind_int64(&*stmt, 2, ctx.get_id());
            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                return select_symbol(stmt, dynamic);
            } else {
                return std::nullopt;
            }
        });

        if (result.has_value()) {
            ent = std::move(result.value());
            return {ent, {}, action::NONE};
        } else if (insert) {
            symbol sym{path[0], ctx};
            auto a = symbol_manager::insert(sym, false);
            if (a != action::FAIL) { ent = std::move(sym); }
            return {ent, {}, a};
        } else {
            //language=sqlite
            static const string_t query_n{R"__SQL__(
SELECT id, name, class_id, ordinal, description, source_id FROM plot_symbol
WHERE name like ('%' || ? || '%') and class_id = ?2;
)__SQL__"};

            /*static thread_local*/ class stmt stmt_n;

            std::vector<symbol> candidates;

            _reuse(stmt, query_n, [&] {
                sqlite3_bind_text(&*stmt, 1, path[0].c_str(), path[0].size(), nullptr);
                sqlite3_bind_int64(&*stmt, 2, ctx.get_id());
                while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                    symbol sym = select_symbol(stmt, dynamic);
                    candidates.emplace_back(std::move(sym));
                }
            });

            return {ent, candidates, action::FAIL};
        }
    } else {
        if (insert) {

        } else {
        }
        return {ent, {}, action::FAIL};
    }
}

resolve_ref_result<plot::symbol> symbol_manager::resolve(const std::vector<string_t> &path,
                                                         plot::symbol &ent,
                                                         const class plot::symbol::clazz &ctx,
                                                         bool_t dynamic) const {
    return const_cast<symbol_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

action symbol_manager::insert(plot::symbol &sym, bool_t replace, corpus::tag<plot::symbol>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
INSERT INTO plot_symbol(id, name, class_id, ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_symbol), ?1, ?2, ?3, ?4, ?5)
ON CONFLICT DO UPDATE SET ordinal     = excluded.ordinal,
                          description = excluded.description,
                          source_id   = excluded.source_id
WHERE name = excluded.name
  and class_id = excluded.class_id
RETURNING plot_symbol.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
INSERT INTO plot_symbol(id, name, class_id, ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_symbol), ?1, ?2, ?3, ?4, ?5)
RETURNING plot_symbol.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &stmt  = replace ? stmt_r : stmt_n;
    auto &query = replace ? query_r : query_n;

    action action = action::FAIL;

    if (sym.name.empty() || sym.class_id < 0) {
        return action;
    }

    sym.id = _reuse(stmt, query, [&]() -> pkey<plot::symbol> {
        sqlite3_bind_text(&*stmt, 1, sym.name.c_str(), sym.name.size(), nullptr);
        sqlite3_bind_int64(&*stmt, 2, sym.class_id);
        if (sym.ordinal) {
            sqlite3_bind_double(&*stmt, 3, sym.ordinal.value());
        } else {
            sqlite3_bind_null(&*stmt, 3);
        }
        if (sym.description) {
            sqlite3_bind_text(&*stmt, 4, sym.description->c_str(), sym.description->size(), nullptr);
        } else {
            sqlite3_bind_null(&*stmt, 4);
        }
        sqlite3_bind_int64(&*stmt, 5, _get_source_id());
        int id = -1;
        if (auto ret = sqlite3_step(&*stmt); ret == SQLITE_ROW) {
            sym.id = id   = sqlite3_column_int(&*stmt, 0);
            sym.source_id = _get_source_id();
            action        = action::INSERT;
        }
        if (sqlite3_step(&*stmt) == SQLITE_DONE) {
            if (action == action::FAIL) action = action::NONE;
            return id;
        } else {
            return sym.id;
        }
    });

    return action;
}

action symbol_manager::update(plot::symbol &sym, bool_t dynamic, corpus::tag<plot::symbol>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
UPDATE plot_symbol
SET name = ?2,
    class_id = ?3,
    ordinal = ?4,
    description = ?5,
    source_id = ?6
WHERE id = ?1
RETURNING plot_symbol.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
UPDATE plot_symbol
SET name = ?2,
    class_id = ?3,
    ordinal = ?4,
    source_id = ?6
WHERE id = ?1
RETURNING plot_symbol.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &query   = dynamic ? query_r : query_n;
    auto &stmt    = dynamic ? stmt_r : stmt_n;
    action action = action::FAIL;

    _reuse(stmt, query, [&]() -> plot::symbol & {
        sqlite3_bind_int64(&*stmt, 1, sym.id);
        sqlite3_bind_text(&*stmt, 2, sym.name.c_str(), sym.name.size(), nullptr);
        sqlite3_bind_int64(&*stmt, 3, sym.class_id);
        if (sym.ordinal.has_value()) {
            sqlite3_bind_double(&*stmt, 4, sym.ordinal.value());
        } else {
            sqlite3_bind_null(&*stmt, 4);
        }
        if (dynamic) {
            if (sym.description.has_value()) {
                sqlite3_bind_text(&*stmt, 5, sym.description->c_str(), sym.description->size(), nullptr);
            } else {
                sqlite3_bind_null(&*stmt, 5);
            }
        }
        sqlite3_bind_int64(&*stmt, 6, _get_source_id());

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            action = action::UPDATE;
        } else {
            action = action::NONE;
        }

        return sym;
    });

    return action;
}

stream_helper symbol_manager::print(pkey<plot::symbol> id, format format, corpus::tag<plot::symbol>) const {
    return [this, id, format](auto &os) -> ostream_t & { return print_helper(os, id, format); };
}

std::tuple<string_t, action> symbol_manager::remove(plot::symbol &sym, bool_t cascade, corpus::tag<plot::symbol>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE FROM plot_symbol
WHERE id = ?1
  and (?2 or (not exists(SELECT * FROM plot_point_subject_sym WHERE subject_id = ?1) and
             not exists(SELECT * FROM plot_causal_element_sym WHERE symbol_id = ?1) and
             not exists(SELECT * FROM plot_object_sym WHERE object_id = ?1) and
             not exists(SELECT * FROM plot_symbol_compound WHERE compound_id = ?1)))
RETURNING id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::tuple<string_t, action> {
        sqlite3_bind_int(&*stmt, 1, sym.get_id());
        sqlite3_bind_int(&*stmt, 2, cascade);
        string_t repr = sym.is_persisted() ? print(sym.get_id(), get_format())() : "";

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return {repr, action::REMOVE};
        } else {
            return {repr, action::FAIL};
        }
    });
}

symbol_manager::~symbol_manager() = default;
