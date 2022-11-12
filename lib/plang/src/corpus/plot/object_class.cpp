//
// Created by Johannes on 06.11.2022.
//

#include <sqlite3.h>
#include <plang/corpus/plot/object.hpp>

using namespace plang;
using namespace plang::op;
using namespace plang::plot;
using namespace plang::detail;

plot::object::clazz object_class_manager::select_object_class(plang::detail::stmt &stmt, bool_t dynamic) const {
    object::clazz clazz;
    clazz.id             = sqlite3_column_int64(&*stmt, 0);
    clazz.name           = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1)));
    clazz.point_class_id = sqlite3_column_int64(&*stmt, 2);
    clazz._default       = sqlite3_column_int(&*stmt, 3);
    clazz.singleton      = sqlite3_column_int(&*stmt, 4);
    if (sqlite3_column_type(&*stmt, 5) != SQLITE_NULL) { clazz.ordinal = sqlite3_column_double(&*stmt, 5); }
    if (dynamic && sqlite3_column_type(&*stmt, 6) != SQLITE_NULL) {
        clazz.description = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 6)));
    }
    clazz.source_id = sqlite3_column_int(&*stmt, 7);
    return clazz;
}
std::vector<std::tuple<pkey<path>, pkey<plot::point::clazz>, pkey<plot::object::clazz>, uint_t>>
object_class_manager::partially_resolve(const std::vector<string_t> &path, bool_t fully) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH RECURSIVE ar AS (SELECT ?1 as ray),
               cte(path_id, leaf_id, parent_id, class_id, symbol_id, depth)
                   AS (SELECT p.id, p.id, p.id, c.id, o.id, iif(not ?2, 0, -1)
                       FROM plot_object_class o,
                            ar
                                LEFT JOIN plot_point_class c on o.point_class_id = c.id
                                LEFT JOIN path p on c.path_id = p.id
                       WHERE o.name = json_extract(ar.ray, '$[' || 0 || ']')
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

    std::vector<std::tuple<pkey<class path>, pkey<plot::point::clazz>, pkey<plot::object::clazz>, uint_t>> result;

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

ostream_t &object_class_manager::print_helper(ostream_t &os, pkey<plot::object::clazz> id, plang::format format) const {
    //TODO: Implement
}

std::vector<plot::object::clazz::hint::variant> object_class_manager::_fetch_hints(pkey<plot::object::clazz> id) const {
    //language=sqlite
    static const string_t query_l{R"__SQL__(
SELECT id, hint, type, source_id FROM plot_object_class_hint_lit
WHERE class_id = ?1;
)__SQL__"};

    //language=sqlite
    static const string_t query_s{R"__SQL__(
SELECT id, hint_id, "recursive", source_id FROM plot_object_class_hint_sym
WHERE class_id = ?1;
)__SQL__"};

    //language=sqlite
    static const string_t query_p{R"__SQL__(
SELECT id, hint_id, "recursive", source_id FROM plot_object_class_hint_pnt
WHERE class_id = ?1;
)__SQL__"};

    /*static thread_local*/ stmt stmt_l;
    /*static thread_local*/ stmt stmt_s;
    /*static thread_local*/ stmt stmt_p;

    std::vector<plot::object::clazz::hint::variant> result;

    _reuse(stmt_l, query_l, [&]() {
        sqlite3_bind_int64(&*stmt_l, 1, id);

        while (_check(sqlite3_step(&*stmt_l)) == SQLITE_ROW) {
            plot::object::clazz::hint::lit hint{
                    string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt_l, 1))),
                    static_cast<plot::object::clazz::hint::lit::type>(sqlite3_column_int(&*stmt_l, 2))};
            hint.id        = sqlite3_column_int64(&*stmt_l, 0);
            hint.source_id = sqlite3_column_int64(&*stmt_l, 3);
            result.emplace_back(std::move(hint));
        }
    });

    _reuse(stmt_s, query_s, [&]() {
        sqlite3_bind_int64(&*stmt_s, 1, id);

        while (_check(sqlite3_step(&*stmt_s)) == SQLITE_ROW) {
            plot::object::clazz::hint::sym hint;
            hint.id        = sqlite3_column_int64(&*stmt_s, 0);
            hint.hint_id   = sqlite3_column_int64(&*stmt_s, 1);
            hint.recursive = sqlite3_column_int(&*stmt_s, 2);
            hint.source_id = sqlite3_column_int64(&*stmt_s, 3);
            result.emplace_back(std::move(hint));
        }
    });

    _reuse(stmt_p, query_p, [&]() {
        sqlite3_bind_int64(&*stmt_p, 1, id);

        while (_check(sqlite3_step(&*stmt_p)) == SQLITE_ROW) {
            plot::object::clazz::hint::pnt hint;
            hint.id        = sqlite3_column_int64(&*stmt_p, 0);
            hint.hint_id   = sqlite3_column_int64(&*stmt_p, 1);
            hint.recursive = sqlite3_column_int(&*stmt_p, 2);
            hint.source_id = sqlite3_column_int64(&*stmt_p, 3);
            result.emplace_back(std::move(hint));
        }
    });

    return result;
}

void object_class_manager::_update_hints(pkey<plot::object::clazz> id,
                                         std::vector<plot::object::clazz::hint::variant> &hints) {
    //language=sqlite
    static const string_t query_l{R"__SQL__(
INSERT INTO plot_object_class_hint_lit (id, class_id, hint, type, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_object_class_hint_lit), ?1, ?2, ?3, ?4)
RETURNING plot_object_class_hint_lit.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_s{R"__SQL__(
INSERT INTO plot_object_class_hint_sym (id, class_id, hint_id, "recursive", source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_object_class_hint_sym), ?1, ?2, ?3, ?4)
RETURNING plot_object_class_hint_sym.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_p{R"__SQL__(
INSERT INTO plot_object_class_hint_pnt (id, class_id, hint_id, "recursive", source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_object_class_hint_pnt), ?1, ?2, ?3, ?4)
RETURNING plot_object_class_hint_pnt.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_l;
    /*static thread_local*/ stmt stmt_s;
    /*static thread_local*/ stmt stmt_p;

    for (auto &v : hints) {
        std::visit(
                [&](auto &h) {
                    using T = std::decay_t<decltype(h)>;

                    if constexpr (std::is_same_v<T, plot::object::clazz::hint::lit>) {
                        h.id = _reuse(stmt_l, query_l, [&]() -> pkey<plot::object::clazz::hint::lit> {
                            sqlite3_bind_int64(&*stmt_l, 1, id);
                            sqlite3_bind_text(&*stmt_l, 2, h.hint.c_str(), h.hint.length(), nullptr);
                            sqlite3_bind_int(&*stmt_l, 3, h._get_type());
                            sqlite3_bind_int64(&*stmt_l, 4, _get_source_id());

                            if (_check(sqlite3_step(&*stmt_l)) == SQLITE_ROW) {
                                return sqlite3_column_int64(&*stmt_l, 0);
                            } else {
                                return -1;
                            }
                        });
                    } else if constexpr (std::is_same_v<T, plot::object::clazz::hint::sym>) {
                        h.id = _reuse(stmt_s, query_s, [&]() -> pkey<plot::object::clazz::hint::sym> {
                            sqlite3_bind_int64(&*stmt_s, 1, id);
                            sqlite3_bind_int64(&*stmt_s, 2, h.hint_id);
                            sqlite3_bind_int(&*stmt_s, 3, h.recursive ? 1 : 0);
                            sqlite3_bind_int64(&*stmt_s, 4, _get_source_id());

                            if (_check(sqlite3_step(&*stmt_s)) == SQLITE_ROW) {
                                return sqlite3_column_int64(&*stmt_s, 0);
                            } else {
                                return -1;
                            }
                        });
                    } else if constexpr (std::is_same_v<T, plot::object::clazz::hint::pnt>) {
                        h.id = _reuse(stmt_p, query_p, [&]() -> pkey<plot::object::clazz::hint::pnt> {
                            sqlite3_bind_int64(&*stmt_p, 1, id);
                            sqlite3_bind_int64(&*stmt_p, 2, h.hint_id);
                            sqlite3_bind_int(&*stmt_p, 3, h.recursive ? 1 : 0);
                            sqlite3_bind_int64(&*stmt_p, 4, _get_source_id());

                            if (_check(sqlite3_step(&*stmt_p)) == SQLITE_ROW) {
                                return sqlite3_column_int64(&*stmt_p, 0);
                            } else {
                                return -1;
                            }
                        });
                    }
                },
                v);
    }
}

std::optional<plot::object::clazz> object_class_manager::fetch(pkey<plot::object::clazz> id,
                                                               bool_t dynamic,
                                                               tag<plot::object::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
WHERE id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    auto clazz = _reuse(stmt, query, [&]() -> std::optional<object::clazz> {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return select_object_class(stmt, dynamic);
        } else {
            return std::nullopt;
        }
    });

    if (clazz.has_value() && dynamic) { clazz->hints = _fetch_hints(id); }

    return clazz;
}

std::vector<plot::object::clazz> object_class_manager::fetch_n(const std::vector<pkey<plot::object::clazz>> &ids,
                                                               bool_t dynamic,
                                                               tag<plot::object::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
WHERE id in (SELECT value
             FROM json_each(?1, '$'));
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<object::clazz> result;
    result.reserve(ids.size());

    string_t json_array = vector_to_json(ids);

    _reuse(stmt, query, [&]() {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.size(), nullptr);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            object::clazz clazz = select_object_class(stmt, dynamic);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

std::vector<plot::object::clazz>
object_class_manager::fetch_all(bool_t dynamic, int_t limit, int_t offset, tag<plot::object::clazz>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<object::clazz> result;

    _reuse(stmt, query, [&]() {
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            object::clazz clazz = select_object_class(stmt, dynamic);

            if (dynamic) { clazz.hints = _fetch_hints(clazz.id); }

            result.push_back(std::move(clazz));
        }
    });

    return result;
}

resolve_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class path &ctx,
                                                                  bool_t insert,
                                                                  bool_t dynamic,
                                                                  corpus::tag<plot::object::clazz>) {
    object::clazz clazz;
    auto result          = resolve(path, clazz, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class path &ctx,
                                                                  bool_t dynamic,
                                                                  corpus::tag<plot::object::clazz> tag) const {
    return const_cast<object_class_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::object::clazz &ent,
                                                                      const class path &ctx,
                                                                      bool_t insert,
                                                                      bool_t dynamic) {
    if (path.empty()) { return {ent, {}, action::FAIL}; }

    if (insert) {
        std::vector<string_t> class_path = path;
        class_path.pop_back();
        auto clazz = point_class_manager::resolve(class_path, ctx, insert, dynamic);
        if (clazz.has_result()) {
            auto result = resolve({*path.rbegin()}, ent, clazz.entry(), insert, dynamic);
            return result;
        } else {
            return {ent, {}, action::FAIL};
        }
    }

    auto result = partially_resolve(path, false);

    if (result.size() == 1) {//Case: overall single candidate
        if (std::get<uint_t>(result[0]) == path.size()) {
            ent = fetch(std::get<pkey<plot::object::clazz>>(result[0]), dynamic).value();
            return {ent, {}, action::NONE};
        } else {
            return {ent, {fetch(std::get<pkey<plot::object::clazz>>(result[0]), dynamic).value()}, action::FAIL};
        }
    } else {//Case: none or multiple candidates
        decltype(result) related;
        for (auto const &e : result) {
            if (is_parent_of(ctx.get_id(), std::get<pkey<class path>>(e)) or
                is_parent_of(std::get<pkey<class path>>(e), ctx.get_id())) {
                related.push_back(e);
            }
        }

        if (related.size() == 1) {//Case: single candidate along current path
            if (std::get<uint_t>(related[0]) == path.size()) {
                ent = fetch(std::get<pkey<plot::object::clazz>>(related[0]), dynamic).value();
                return {ent, {}, action::NONE};
            } else {
                return {ent, {fetch(std::get<pkey<plot::object::clazz>>(result[0]), dynamic).value()}, action::FAIL};
            }
        } else {//Case: none or multiple candidates along current path
            std::vector<pkey<plot::object::clazz>> ids;
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
                if (std::get<uint_t>(e) >= (max != 1 ? max : min)) {
                    ids.push_back(std::get<pkey<plot::object::clazz>>(e));
                }
            }
            if (ids.size() == 1) {//Case: single most viable candidate along path
                ent = fetch(ids[0]).value();
                return {ent, {}, action::NONE};
            } else {//Case: none or most viable candidates along path
                auto candidates = fetch_n(ids, dynamic);
                return {ent, std::move(candidates), action::FAIL};
            }
        }
    }
}

resolve_ref_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::object::clazz &ent,
                                                                      const class path &ctx,
                                                                      bool_t dynamic) const {
    return const_cast<object_class_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

resolve_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class plot::point::clazz &ctx,
                                                                  bool_t insert,
                                                                  bool_t dynamic,
                                                                  corpus::tag<plot::object::clazz>) {
    object::clazz clazz;
    auto result          = resolve(path, clazz, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                  const class plot::point::clazz &ctx,
                                                                  bool_t dynamic,
                                                                  corpus::tag<plot::object::clazz> tag) const {
    return const_cast<object_class_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::object::clazz &ent,
                                                                      const class plot::point::clazz &ctx,
                                                                      bool_t insert,
                                                                      bool_t dynamic) {
    if (path.empty()) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
WHERE point_class_id = ?1;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        std::vector<object::clazz> result;

        _reuse(stmt, query, [&]() {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());

            while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) { result.push_back(select_object_class(stmt, dynamic)); }
        });

        return {ent, std::move(result), action::FAIL};
    } else if (path.size() == 1) {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
WHERE point_class_id = ?1 and name = ?2;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        auto result = _reuse(stmt, query, [&]() -> std::optional<object::clazz> {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());
            if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
                return select_object_class(stmt, dynamic);
            } else {
                return std::nullopt;
            }
        });

        if (result.has_value()) {
            ent = std::move(result.value());

            if (dynamic) { ent.hints = _fetch_hints(ent.id); }

            return {ent, {}, action::NONE};
        } else if (insert) {
            ent.set_point_class(ctx);
            auto action = object_class_manager::insert(ent);
            return {ent, {}, action};
        } else {
            return {ent, {}, action::FAIL};
        }
    } else {
        //language=sqlite
        static const string_t query{R"__SQL__(
SELECT id, name, point_class_id, "default", singleton, ordinal, description, source_id FROM plot_object_class
WHERE point_class_id = ?1 and name in (SELECT value
                                       FROM json_each(?2, '$'));
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        auto json = vector_to_json(path);

        std::vector<object::clazz> result;

        _reuse(stmt, query, [&]() {
            sqlite3_bind_int64(&*stmt, 1, ctx.get_id());
            sqlite3_bind_text(&*stmt, 2, json.c_str(), json.length(), nullptr);

            while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) { result.push_back(select_object_class(stmt, dynamic)); }
        });

        return {ent, std::move(result), action::FAIL};
    }
}

resolve_ref_result<plot::object::clazz> object_class_manager::resolve(const std::vector<string_t> &path,
                                                                      plot::object::clazz &ent,
                                                                      const class plot::point::clazz &ctx,
                                                                      bool_t dynamic) const {
    return const_cast<object_class_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

action object_class_manager::insert(plot::object::clazz &clazz, bool_t replace, corpus::tag<plot::object::clazz>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
INSERT OR REPLACE INTO plot_object_class (id, name, point_class_id, "default", singleton,
                                          ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_object_class), ?1, ?2, ?3, ?4, ?5, ?6, ?7)
RETURNING plot_object_class.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
INSERT INTO plot_object_class (id, name, point_class_id, "default", singleton,
                                          ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM plot_object_class), ?1, ?2, ?3, ?4, ?5, ?6, ?7)
RETURNING plot_object_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &stmt  = replace ? stmt_r : stmt_n;
    auto &query = replace ? query_r : query_n;

    action action = action::FAIL;

    clazz.id = _reuse(stmt, query, [&]() -> pkey<plot::object::clazz> {
        sqlite3_bind_text(&*stmt, 1, clazz.name.c_str(), clazz.name.length(), nullptr);
        sqlite3_bind_int64(&*stmt, 2, clazz.point_class_id);
        sqlite3_bind_int(&*stmt, 3, clazz._default ? 1 : 0);
        sqlite3_bind_int(&*stmt, 4, clazz.singleton ? 1 : 0);
        if (clazz.ordinal) { sqlite3_bind_double(&*stmt, 5, *clazz.ordinal); }
        if (clazz.description) {
            sqlite3_bind_text(&*stmt, 6, clazz.description->c_str(), clazz.description->length(), nullptr);
        }
        sqlite3_bind_int64(&*stmt, 7, _get_source_id());

        pkey<plot::object::clazz> id;
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

    if (action != action::FAIL) {
        _update_hints(clazz.id, clazz.hints);
    }

    return action;
}

action object_class_manager::update(plot::object::clazz &clazz, bool_t dynamic, corpus::tag<plot::object::clazz>) {
    //language=sqlite
    static const string_t query_d{R"__SQL__(
UPDATE plot_object_class
SET name = ?2,
    point_class_id = ?3,
    "default" = ?4,
    singleton = ?5,
    ordinal = ?6,
    description = ?7,
    source_id = ?8
WHERE id = ?1
RETURNING plot_object_class.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
UPDATE plot_object_class
SET name = ?2,
    point_class_id = ?3,
    "default" = ?4,
    singleton = ?5,
    ordinal = ?6,
    source_id = ?8
WHERE id = ?1
RETURNING plot_object_class.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_d;
    /*static thread_local*/ stmt stmt_n;

    auto &query = dynamic ? query_d : query_n;
    auto &stmt  = dynamic ? stmt_d : stmt_n;

    action action = action::FAIL;

    if (!clazz.is_persisted()) { return action; }

    _reuse(stmt, query, [&]() -> plot::object::clazz & {
        sqlite3_bind_int64(&*stmt, 1, clazz.id);
        sqlite3_bind_text(&*stmt, 2, clazz.name.c_str(), clazz.name.length(), nullptr);
        sqlite3_bind_int64(&*stmt, 3, clazz.point_class_id);
        sqlite3_bind_int(&*stmt, 4, clazz._default ? 1 : 0);
        sqlite3_bind_int(&*stmt, 5, clazz.singleton ? 1 : 0);
        if (clazz.ordinal) { sqlite3_bind_double(&*stmt, 6, *clazz.ordinal); }
        if (clazz.description) {
            sqlite3_bind_text(&*stmt, 7, clazz.description->c_str(), clazz.description->length(), nullptr);
        }
        sqlite3_bind_int64(&*stmt, 8, _get_source_id());

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            action = action::UPDATE;
        } else {
            action = action::NONE;
        }

        return clazz;
    });

    if (action != action::FAIL && dynamic) {
        _update_hints(clazz.id, clazz.hints);
    }

    return action;
}

stream_helper object_class_manager::print(pkey<plot::object::clazz> id,
                                          plang::format format,
                                          corpus::tag<plot::object::clazz>) const {
    return [this, id, format](auto &os) -> ostream_t & { return print_helper(os, id, format); };
}

std::tuple<string_t, action> object_class_manager::remove(plot::object::clazz &clazz,
                                                          bool_t cascade,
                                                          corpus::tag<plot::object::clazz>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE FROM plot_object_class
WHERE id = ?1
  and (?2 or (not exists(SELECT * FROM plot_object_lit WHERE class_id = ?1) and
             not exists(SELECT * FROM plot_object_sym WHERE class_id = ?1) and
             not exists(SELECT * FROM plot_object_cls WHERE class_id = ?1) and
             not exists(SELECT * FROM plot_object_pnt WHERE class_id = ?1) and
             not exists(SELECT * FROM plot_causal_element_obj WHERE object_class_id = ?1)))
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

object_class_manager::~object_class_manager() = default;
