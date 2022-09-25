//
// Created by Johannes on 04.08.2022.
//

#include "db.hpp"
#include <regex>

EXTERN(ddl_base_sql);
EXTERN(ddl_plot_symbol_sql);
EXTERN(ddl_plot_point_sql);
EXTERN(ddl_plot_object_sql);
EXTERN(ddl_plot_causal_sql);

extern "C" {
extern int sqlite3_regexp_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);
}

using namespace plang;

/*
detail::storage::storage(char const *filename)
    : detail::storage::base_storage_t(plang::detail::db::make_storage(filename)) {}

detail::storage::~storage() = default;
*/

db::db()
/*: storage(":memory:")*/ {}

db::db(std::filesystem::path const &path)
    : /*storage(path.string().c_str()),*/
      connection(nullptr) {
    /*using namespace sqlite_orm;

    auto on_open = storage.on_open;
    storage.open_forever();
    storage.on_open = [&](sqlite3 *db) {
        db::connection = db;
    };

    storage.transaction([&]() mutable {
        for (auto &res : storage.sync_schema(true)) { (void) res; }
        return true;
    });

    storage.on_open = on_open;*/

    sqlite3_auto_extension(reinterpret_cast<void (*)(void)>(sqlite3_regexp_init));
    sqlite3_open(path.string().c_str(), &connection);

    _exec("PRAGMA compile_options;", [](int b, char **c, char **d) {
        for (int i = 0; i < b; ++i) { printf("%s ", *(c++)); }
        printf("\n");
        return SQLITE_OK;
    });
    for (auto &[q, l] : {std::make_tuple(ddl_base_sql, ddl_base_sql_len),
                         std::make_tuple(ddl_plot_symbol_sql, ddl_plot_symbol_sql_len),
                         std::make_tuple(ddl_plot_point_sql, ddl_plot_point_sql_len),
                         std::make_tuple(ddl_plot_object_sql, ddl_plot_object_sql_len),
                         std::make_tuple(ddl_plot_causal_sql, ddl_plot_causal_sql_len)}) {
        _exec({reinterpret_cast<char const *>(q), l});
    }
}

db::~db() {
    sqlite3_close(connection);
}
