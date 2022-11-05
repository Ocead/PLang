//
// Created by Johannes on 16.08.2022.
//

#include <algorithm>
#include <plang/corpus/path.hpp>
#include <plang/error.hpp>
#include "sqlite.hpp"

using namespace plang;
using namespace plang::detail;

std::vector<std::tuple<pkey<path>, uint_t>> path_manager::partially_resolve(const std::vector<string_t> &path,
                                                                            bool_t fully) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH RECURSIVE ar AS (SELECT ?1 as ray),
               cte(head_id, leaf_id, depth) AS (SELECT id, id, iif(not ?2, 0, -1)
                                                FROM path p,
                                                     ar
                                                WHERE iif(not ?2, p.name == json_extract(ar.ray, '$[' || 0 || ']'),
                                                          p.id = p.parent_id)
                                                UNION ALL
                                                SELECT head_id, c.id, depth + 1
                                                FROM cte,
                                                     ar
                                                         LEFT JOIN path c on cte.leaf_id = c.parent_id
                                                WHERE depth < json_array_length(ar.ray)
                                                  and c.name == json_extract(ar.ray, '$[' || (cte.depth + 1) || ']')),
               max as (SELECT head_id, max(depth) as depth
                       FROM cte
                       GROUP BY head_id)
SELECT cte.head_id, leaf_id, cte.depth + 1 as depth
FROM max
         LEFT JOIN cte on max.head_id = cte.head_id and max.depth = cte.depth;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    string_t json_array = vector_to_json(path);

    std::vector<std::tuple<pkey<class path>, uint_t>> map{};

    _reuse(stmt, query, [&] {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.length(), nullptr);
        sqlite3_bind_int(&*stmt, 2, fully);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            map.emplace_back(sqlite3_column_int(&*stmt, 1), sqlite3_column_int(&*stmt, 2));
        }
    });

    return map;
}

std::tuple<pkey<path>, uint_t> path_manager::_resolve(std::vector<std::tuple<pkey<path>, uint_t>> const &candidates_,
                                                      pkey<path> scope) const {
    auto candidates = candidates_;
    auto size       = candidates.size();
    if (size == 1) {
        return *candidates.begin();
    } else if (size == 0) {
        return std::make_tuple(or_root_id(scope), 0);
    } else {
        auto depth = std::get<1>(*candidates.begin());
        std::for_each(candidates.begin(), candidates.end(), [this, &scope](auto &t) {
            auto &[k, v] = t;
            v            = is_parent_of(k, scope) || is_parent_of(scope, k);
        });
        auto count = std::count_if(candidates.begin(), candidates.end(), [](auto const &t) { return std::get<1>(t); });
        if (count == 1) {
            auto it = std::find_if(candidates.begin(), candidates.end(), [](auto const &t) { return std::get<1>(t); });
            return std::make_tuple(std::get<0>(*it), depth);
        } else {
            return std::make_tuple(-1, std::numeric_limits<uint_t>::min());
        }
    }
}

std::vector<pkey<path>> path_manager::get_children(pkey<path> id) const {

    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id
FROM path
WHERE parent_id = $1
ORDER BY coalesce((SELECT max(ordinal) FROM path WHERE parent_id = $1), ordinal);
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<pkey<path>> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, id);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) { result.push_back(sqlite3_column_int64(&*stmt, 0)); }
    });

    return result;
}

bool_t path_manager::is_parent_of(pkey<path> parent_id, pkey<path> node_id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH RECURSIVE ids AS (SELECT ?1 as parent_id, ?2 as node_id),
               cte(parent_id, node_id) AS (
                   SELECT *
                   FROM ids
                   UNION
                   SELECT cte.parent_id, p.parent_id
                   FROM cte
                            LEFT JOIN path p on cte.node_id = p.id
                   WHERE cte.parent_id != p.id
                     and p.id != p.parent_id
               )
SELECT exists(SELECT * FROM cte WHERE parent_id == node_id and node_id is not null);
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, parent_id);
        sqlite3_bind_int64(&*stmt, 2, node_id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return !!sqlite3_column_int(&*stmt, 0);
        } else {
            return false;
        }
    });
}

pkey<path> path_manager::or_root_id(plang::detail::base_types::pkey<path> id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT coalesce(max(p.id), (SELECT id FROM path p2 WHERE p2.id == p2.parent_id)) FROM path p
WHERE p.id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return sqlite3_column_int(&*stmt, 0);
        } else {
            throw std::logic_error("");
        }
    });
}

std::vector<string_t> path_manager::get_full_path(pkey<path> id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH RECURSIVE cte(n, id, parent_id, name) AS (SELECT 0, id, parent_id, name
                                               FROM path
                                               WHERE id = ?1
                                               UNION ALL
                                               SELECT c.n + 1, p.id, p.parent_id, p.name
                                               FROM cte c
                                                        LEFT JOIN path p on c.parent_id = p.id
                                               WHERE n == 0 or c.id != c.parent_id)
SELECT name
FROM cte
ORDER BY n DESC;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<string_t> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, id);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            result.push_back(string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 0))));
        }
    });

    return result;
}

std::vector<string_t> path_manager::get_unique_path(pkey<path> id) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, parent_id, ordinal, description, source_id FROM path
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<string_t> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, id);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            result.push_back(string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1))));
        }
    });

    return result;
}

ostream_t &path_manager::print_helper(ostream_t &os, pkey<path> id, format format) const {
    const auto nodes = (format.get_qualification() == format::qualification::FULL) ? get_full_path(id)
                                                                                   : get_unique_path(id);

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        os << nodes[i];
        if (i < nodes.size() - 1) { os << format(".", format::style::for_op(lang::op::PATH)); }
    }

    switch (format.get_detail()) {

        case format::detail::ID_REF:
        case format::detail::EXPLICIT_REF: {
            break;
        }
        case format::detail::DEFINITION:
        case format::detail::FULL: {
            print_decoration(os, id, format);
            os << format(";", format::style::for_op(lang::op::DECL));
            break;
        }
    }

    return os;
}

ostream_t &path_manager::print_decoration(ostream_t &os, pkey<path> id, format format) const {
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

std::optional<path> path_manager::fetch(pkey<path> id, bool_t dynamic, corpus::tag<class path>) const {
    //TODO: Switch for texts
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, parent_id, ordinal, description, source_id FROM path
WHERE id = ?;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::optional<path> {
        sqlite3_bind_int64(&*stmt, 1, id);
        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            path path;
            path.id        = sqlite3_column_int(&*stmt, 0);
            path.name      = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1)));
            path.parent_id = sqlite3_column_int(&*stmt, 2);
            if (sqlite3_column_type(&*stmt, 3) != SQLITE_NULL) {
                path.ordinal = sqlite3_column_int(&*stmt, 3);
            } else {
                path.ordinal.reset();
            }
            if (dynamic && sqlite3_column_type(&*stmt, 4) != SQLITE_NULL) {
                if (auto *desc = reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 4))) {
                    path.description = string_t(desc);
                } else {
                    path.description.reset();
                }
            }
            path.source_id = sqlite3_column_int(&*stmt, 5);
            return path;
        } else {
            return std::nullopt;
        }
    });
}

std::vector<path> path_manager::fetch_n(const std::vector<pkey<path>> &ids,
                                        bool_t dynamic,
                                        corpus::tag<class path>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT *
FROM path
WHERE id in (SELECT value
             FROM json_each(?1, '$'));
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<path> result;
    result.reserve(ids.size());

    string_t json_array = vector_to_json(ids);

    _reuse(stmt, query, [&] {
        sqlite3_bind_text(&*stmt, 1, json_array.c_str(), json_array.size(), nullptr);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            path path;
            path.id        = sqlite3_column_int(&*stmt, 0);
            path.name      = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1)));
            path.parent_id = sqlite3_column_int(&*stmt, 2);
            path.ordinal   = sqlite3_column_int(&*stmt, 3);
            if (dynamic) {
                if (auto *desc = reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 4))) {
                    path.description = string_t(desc);
                } else {
                    path.description.reset();
                }
            }
            path.source_id = sqlite3_column_int(&*stmt, 5);
            result.push_back(std::move(path));
        }
    });

    return result;
}

std::vector<path> path_manager::fetch_all(bool_t dynamic, int_t limit, int_t offset, corpus::tag<class path>) const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id, name, parent_id, ordinal, description, source_id FROM path
LIMIT ?1 OFFSET ?1 * ?2;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    std::vector<path> result;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int(&*stmt, 1, limit);
        sqlite3_bind_int(&*stmt, 2, offset);
        while (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            path path;
            path.id        = sqlite3_column_int(&*stmt, 0);
            path.name      = string_t(reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 1)));
            path.parent_id = sqlite3_column_int(&*stmt, 2);
            if (sqlite3_column_type(&*stmt, 3) != SQLITE_NULL) {
                path.ordinal = sqlite3_column_int(&*stmt, 3);
            } else {
                path.ordinal.reset();
            }
            if (dynamic && sqlite3_column_type(&*stmt, 4) != SQLITE_NULL) {
                if (auto *desc = reinterpret_cast<const char *>(sqlite3_column_text(&*stmt, 4))) {
                    path.description = string_t(desc);
                } else {
                    path.description.reset();
                }
            }
            path.source_id = sqlite3_column_int(&*stmt, 5);
            result.push_back(std::move(path));
        }
    });

    return result;
}

resolve_result<path> path_manager::resolve(const std::vector<string_t> &path,
                                           const class path &ctx,
                                           column_types::bool_t insert,
                                           column_types::bool_t dynamic,
                                           corpus::tag<class path>) {
    class path node;
    auto result          = resolve(path, node, ctx, insert, dynamic);
    auto &[res, can, ac] = result;
    if (res.is_persisted()) {
        return std::move(result);
    } else {
        return {std::nullopt, std::move(can), ac};
    }
}

resolve_result<path> path_manager::resolve(const std::vector<string_t> &path,
                                           const class path &ctx,
                                           column_types::bool_t dynamic,
                                           corpus::tag<class path> tag) const {
    return const_cast<path_manager *>(this)->resolve(path, ctx, false, dynamic, tag);
}

resolve_ref_result<path> path_manager::resolve(const std::vector<string_t> &path,
                                               class path &ent,
                                               const class path &ctx,
                                               bool_t insert,
                                               bool_t dynamic) {
    if (!path.empty()) {
        auto candidates  = partially_resolve(path, false);
        auto [id, depth] = _resolve(candidates, ctx.get_id());
        auto parent_id   = id;

        if (id >= 0) {
            if (insert) {
                class path new_path;
                action action = action::NONE;
                for (auto it = path.begin() + depth; it != path.end(); ++it) {
                    new (&new_path) class path();
                    new_path.name      = *it;
                    new_path.parent_id = parent_id;
                    action             = path_manager::insert(new_path);
                    parent_id          = new_path.get_id();
                }

                if (new_path.is_persisted()) {
                    ent = new_path;
                } else {
                    ent = fetch(id).value();
                }
                return {ent, {}, action};
            } else {
                if (path.size() == depth) {
                    ent = fetch(id).value();
                    return {ent, {}, action::NONE};
                } else {
                    return {ent, {}, action::FAIL};
                }
            }
        } else {
            std::vector<class path> can_vec;
            std::transform(candidates.begin(), candidates.end(), std::back_inserter(can_vec), [&](auto const &arg) {
                return fetch(std::get<0>(arg), dynamic).value();
            });

            return {ent, std::move(can_vec), action::FAIL};
        }
    } else {
        ent = ctx;
        return {ent, fetch_n(get_children(ctx.get_id()), dynamic), action::NONE};
    }
}

resolve_ref_result<path> path_manager::resolve(const std::vector<string_t> &path,
                                               class path &ent,
                                               const class path &ctx,
                                               column_types::bool_t dynamic) const {
    return const_cast<path_manager *>(this)->resolve(path, ent, ctx, false, dynamic);
}

action path_manager::insert(path &path, bool_t replace, corpus::tag<class path>) {
    //language=sqlite
    static const string_t query_r{R"__SQL__(
INSERT INTO path (id, name, parent_id, ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM path), ?1, ?2, ?3, ?4, ?5)
ON CONFLICT DO UPDATE SET ordinal = excluded.ordinal,
                          description = excluded.description,
                          source_id = excluded.source_id
WHERE name = excluded.name
  and parent_id = excluded.parent_id
RETURNING path.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
INSERT INTO path (id, name, parent_id, ordinal, description, source_id)
VALUES ((SELECT coalesce(max(id) + 1, 0) FROM path), ?1, ?2, ?3, ?4, ?5)
RETURNING path.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_r;
    /*static thread_local*/ stmt stmt_n;

    auto &query = replace ? query_r : query_n;
    auto &stmt  = replace ? stmt_r : stmt_n;

    action action = action::FAIL;

    path.id = _reuse(stmt, query, [&]() -> pkey<class path> {
        sqlite3_bind_text(&*stmt, 1, path.name.c_str(), path.name.length(), nullptr);
        sqlite3_bind_int64(&*stmt, 2, path.parent_id);
        if (path.ordinal) {
            sqlite3_bind_double(&*stmt, 3, path.ordinal.value());
        } else {
            sqlite3_bind_null(&*stmt, 3);
        }
        if (path.description) {
            sqlite3_bind_text(&*stmt, 4, path.description.value().c_str(), path.description.value().length(), nullptr);
        } else {
            sqlite3_bind_null(&*stmt, 4);
        }
        sqlite3_bind_int64(&*stmt, 5, _get_source_id());
        int id = -1;
        if (sqlite3_step(&*stmt) == SQLITE_ROW) {
            id     = sqlite3_column_int(&*stmt, 0);
            action = action::INSERT;
        }
        if (sqlite3_step(&*stmt) == SQLITE_DONE) {
            if (action == action::FAIL) action = action::NONE;
            return id;
        } else {
            return path.id;
        }
    });

    return action;
}

action path_manager::update(path &path, bool_t dynamic, corpus::tag<class path>) {
    //language=sqlite
    static const string_t query_t{R"__SQL__(
UPDATE path
SET name        = ?2,
    parent_id   = ?3,
    ordinal     = ?4,
    description = ?5,
    source_id   = ?6
WHERE id = ?1
RETURNING path.id;
)__SQL__"};

    //language=sqlite
    static const string_t query_n{R"__SQL__(
UPDATE path
SET name        = ?2,
    parent_id   = ?3,
    ordinal     = ?4,
    source_id   = ?6
WHERE id = ?1
RETURNING path.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt_t;
    /*static thread_local*/ stmt stmt_n;

    auto &query   = dynamic ? query_t : query_n;
    auto &stmt    = dynamic ? stmt_t : stmt_n;
    action action = action::FAIL;

    if (path.get_id() != get_root_path_id() && is_parent_of(path.get_id(), path.get_parent_id())) {
        throw std::logic_error("Cyclic path");
    }

    _reuse(stmt, query, [&]() -> class path & {
        if (!path.is_persisted()) { throw std::logic_error("Cannot update unpersisted path"); }

        sqlite3_bind_int64(&*stmt, 1, path.get_id());
        sqlite3_bind_text(&*stmt, 2, path.get_name().c_str(), path.get_name().length(), nullptr);
        sqlite3_bind_int64(&*stmt, 3, path.get_parent_id());
        if (path.ordinal) {
            sqlite3_bind_double(&*stmt, 4, path.get_ordinal().value());
        } else {
            sqlite3_bind_null(&*stmt, 4);
        }
        if (dynamic) {
            if (path.description) {
                auto const &description = path.get_description().value();
                sqlite3_bind_text(&*stmt, 5, description.c_str(), description.length(), nullptr);
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

        return path;
    });

    return action;
}

stream_helper path_manager::print(pkey<path> id, format format, corpus::tag<class path>) const {
    return [this, id, format](auto &os) -> ostream_t & { return print_helper(os, id, format); };
}

std::tuple<string_t, action> path_manager::remove(path &path, bool_t cascade, corpus::tag<class path>) {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE
FROM path
WHERE id = ?1
  and (?2 or (not exists(SELECT * FROM path WHERE parent_id = ?1) and
             not exists(SELECT * FROM plot_symbol_class WHERE path_id = ?1) and
             not exists(SELECT * FROM plot_point_class WHERE path_id = ?1)))
RETURNING id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::tuple<string_t, action> {
        if (path.is_root()) { throw std::logic_error("Cannot delete root path"); }

        sqlite3_bind_int64(&*stmt, 1, path.get_id());
        sqlite3_bind_int(&*stmt, 2, cascade);
        string_t repr = path.is_persisted() ? print(path.get_id(), get_format())() : "";

        if (_check(sqlite3_step(&*stmt)) == SQLITE_ROW) {
            return {repr, action::REMOVE};
        } else {
            return {repr, action::FAIL};
        }
    });
}

void path_manager::wipe() {
    //language=sqlite
    static const string_t query{R"__SQL__(
DELETE FROM path;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    _exec(query);
    ensure_root_path();
}

void path_manager::ensure_root_path() {
    //language=sqlite
    static const string_t query{R"__SQL__(
INSERT INTO path (id, name, parent_id, ordinal, description, source_id)
WITH sub AS (SELECT coalesce(max(id), 0) as id, '', coalesce(max(id), 0), null, null, ?1
             FROM path
             WHERE not exists(SELECT * FROM path WHERE id == path.parent_id))
SELECT *
FROM sub
WHERE sub.id is not null
ON CONFLICT DO UPDATE SET id = id
RETURNING path.id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    _reuse(stmt, query, [&] {
        sqlite3_bind_int64(&*stmt, 1, _get_source_id());
        if (_check(sqlite3_step(&*stmt)) != SQLITE_ROW) { throw std::runtime_error("Could not create root path."); }
    });
}

pkey<path> path_manager::get_root_path_id() const {
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT id FROM path
WHERE id == parent_id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&] {
        int ret = _check(sqlite3_step(&*stmt));
        if (ret == SQLITE_ROW) {
            return sqlite3_column_int(&*stmt, 0);
        } else {
            throw std::logic_error{sqlite3_errmsg(db.get())};
        }
    });
}

path_manager::~path_manager() = default;