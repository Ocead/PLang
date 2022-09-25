//
// Created by Johannes on 04.08.2022.
//

#ifndef LIBPLANG_STORAGE_HPP
#define LIBPLANG_STORAGE_HPP

#include "sqlite.hpp"
#include <filesystem>
#include <plang/base.hpp>
#include <plang/plot.hpp>
// #include <sqlite_orm/sqlite_orm.h>

/*
namespace sqlite_orm {

    template<typename TableTp, typename ColTp, int I>
    struct type_printer<plang::detail::cls_col<TableTp, ColTp, I>, void> : public type_printer<ColTp, void> {};

    template<typename TableTp, typename ColTp, int I>
    struct type_printer<plang::detail::fun_col<TableTp, ColTp, I>, void> : public type_printer<ColTp, void> {};

    template<typename TableTp, typename ColTp, int I>
    struct statement_binder<plang::detail::cls_col<TableTp, ColTp, I>, void> : public statement_binder<ColTp, void> {};

    template<typename TableTp, typename ColTp, int I>
    struct statement_binder<plang::detail::fun_col<TableTp, ColTp, I>, void> : public statement_binder<ColTp, void> {};

}// namespace sqlite_orm
*/

namespace plang {

    class db;

    /*
    namespace detail {
        class storage;

        /// \brief Contains helper functions for creating the database connector
        class db {
        private:
            static auto table_source() {
                using namespace sqlite_orm;
                return make_table<source>("source",
                                          make_column("id", &persisted::id, primary_key(), autoincrement()),
                                          make_column("file", &source::file),
                                          make_column("time", &source::_get_time, &source::_set_time));
            }

            static auto table_path() {
                using namespace sqlite_orm;
                return make_table<path>(
                        "path",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("name", &path::name),
                        make_column("parent_id", &path::parent_id),
                        make_column("ordinal", &decorated::ordinal),
                        make_column("description", &decorated::description),
                        make_column("source", &sourced::source_id),
                        foreign_key(&path::parent_id).references(column<path>(&persisted::id)),
                        foreign_key(column<path>(&sourced::source_id)).references(column<source>(&persisted::id)));
            }

            static auto table_symbol_class() {
                using namespace sqlite_orm;
                return make_table<plot::symbol::clazz>(
                        "plot_symbol_class",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("path_id", &plot::symbol::clazz::path_id),
                        foreign_key(&plot::symbol::clazz::path_id).references(column<path>(&persisted::id)));
            }

            static auto table_symbol_class_hint() {
                using namespace sqlite_orm;
                return make_table<plot::symbol::clazz::hint>(
                        "plot_symbol_class_hint",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::symbol::clazz::hint::class_id),
                        make_column("hint_id", &plot::symbol::clazz::hint::hint_id),
                        make_column("recursive", &plot::symbol::clazz::hint::recursive),
                        foreign_key(&plot::symbol::clazz::hint::class_id)
                                .references(column<plot::symbol::clazz>(&persisted::id)),
                        foreign_key(&plot::symbol::clazz::hint::hint_id)
                                .references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_symbol() {
                using namespace sqlite_orm;
                return make_table<plot::symbol>(
                        "plot_symbol",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::symbol::class_id),
                        make_column("ordinal", &decorated::ordinal),
                        make_column("description", &decorated::description),
                        foreign_key(&plot::symbol::class_id).references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_symbol_compound() {
                using namespace sqlite_orm;
                return make_table<plot::symbol::compound>(
                        "plot_symbol_compound",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("symbol_id", &plot::symbol::compound::symbol_id),
                        make_column("compound_id", &plot::symbol::compound::compound_id),
                        make_column("distance", &plot::symbol::compound::distance),
                        foreign_key(&plot::symbol::compound::symbol_id)
                                .references(column<plot::symbol>(&persisted::id)),
                        foreign_key(&plot::symbol::compound::compound_id)
                                .references(column<plot::symbol>(&persisted::id)));
            }

            static auto table_point_class() {
                using namespace sqlite_orm;
                return make_table<plot::point::clazz>(
                        "plot_point_class",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("path_id", &plot::point::clazz::path_id),
                        make_column("singleton", &plot::point::clazz::singleton),
                        foreign_key(&plot::point::clazz::path_id).references(column<path>(&persisted::id)));
            }

            static auto table_point_class_hint() {
                using namespace sqlite_orm;
                return make_table<plot::point::clazz::hint>(
                        "plot_point_class_hint",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::point::clazz::hint::class_id),
                        make_column("hint_id", &plot::point::clazz::hint::hint_id),
                        make_column("recursive", &plot::point::clazz::hint::recursive),
                        foreign_key(&plot::point::clazz::hint::class_id)
                                .references(column<plot::point::clazz>(&persisted::id)),
                        foreign_key(&plot::point::clazz::hint::hint_id)
                                .references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_point() {
                using namespace sqlite_orm;
                return make_table<plot::point>(
                        "plot_point",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::point::class_id),
                        make_column("truth", &plot::point::truth),
                        foreign_key(&plot::point::class_id).references(column<plot::point::clazz>(&persisted::id)));
            }

            static auto table_point_subject_sym() {
                using namespace sqlite_orm;
                return make_table<plot::point::subject::sym>(
                        "plot_point_subject_sym",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("point_id", &plot::point::subject::sym::point_id),
                        make_column("symbol_id", &plot::point::subject::sym::symbol_id),
                        foreign_key(&plot::point::subject::sym::point_id)
                                .references(column<plot::point>(&persisted::id)),
                        foreign_key(&plot::point::subject::sym::symbol_id)
                                .references(column<plot::symbol>(&persisted::id)));
            }

            static auto table_point_subject_cls() {
                using namespace sqlite_orm;
                return make_table<plot::point::subject::cls>(
                        "plot_point_subject_cls",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("point_id", &plot::point::subject::cls::point_id),
                        make_column("symbol_class_id", &plot::point::subject::cls::symbol_class_id),
                        foreign_key(&plot::point::subject::cls::point_id)
                                .references(column<plot::point>(&persisted::id)),
                        foreign_key(&plot::point::subject::cls::symbol_class_id)
                                .references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_object_class() {
                using namespace sqlite_orm;
                return make_table<plot::object::clazz>(
                        "plot_object_class",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("point_class_id", &plot::object::clazz::point_class_id),
                        make_column("name", &plot::object::clazz::name),
                        make_column("singleton", &plot::object::clazz::singleton),
                        make_column("ordinal", &decorated::ordinal),
                        make_column("description", &decorated::description),
                        foreign_key(&plot::object::clazz::point_class_id)
                                .references(column<plot::point::clazz>(&persisted::id)));
            }

            static auto table_object_class_hint_lit() {
                using namespace sqlite_orm;
                return make_table<plot::object::clazz::hint::lit>(
                        "plot_object_class_hint_lit",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::clazz::hint::lit::class_id),
                        make_column("hint", &plot::object::clazz::hint::lit::hint),
                        make_column("type",
                                    &plot::object::clazz::hint::lit::_get_type,
                                    &plot::object::clazz::hint::lit::_set_type),
                        foreign_key(column<plot::object::clazz::hint::lit>(&plot::object::clazz::hint::lit::class_id))
                                .references(column<plot::object::clazz>(&persisted::id)));
            }

            static auto table_object_class_hint_sym() {
                using namespace sqlite_orm;
                return make_table<plot::object::clazz::hint::sym>(
                        "plot_object_class_hint_sym",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::clazz::hint::sym::class_id),
                        make_column("hint_id", &plot::object::clazz::hint::sym::hint_id),
                        make_column("recursive", &plot::object::clazz::hint::sym::recursive),
                        foreign_key(column<plot::object::clazz::hint::sym>(&plot::object::clazz::hint::sym::class_id))
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(column<plot::object::clazz::hint::sym>(&plot::object::clazz::hint::sym::hint_id))
                                .references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_object_class_hint_pnt() {
                using namespace sqlite_orm;
                return make_table<plot::object::clazz::hint::pnt>(
                        "plot_object_class_hint_pnt",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::clazz::hint::pnt::class_id),
                        make_column("hint_id", &plot::object::clazz::hint::pnt::hint_id),
                        make_column("recursive", &plot::object::clazz::hint::pnt::recursive),
                        foreign_key(column<plot::object::clazz::hint::pnt>(&plot::object::clazz::hint::pnt::class_id))
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(column<plot::object::clazz::hint::pnt>(&plot::object::clazz::hint::pnt::hint_id))
                                .references(column<plot::point::clazz>(&persisted::id)));
            }

            static auto table_object_lit() {
                using namespace sqlite_orm;
                return make_table<plot::object::lit>(
                        "plot_object_lit",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::lit::class_id),
                        make_column("point_id", &plot::object::lit::point_id),
                        make_column("object", &plot::object::lit::object),
                        foreign_key(&plot::object::lit::class_id)
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(&plot::object::sym::point_id).references(column<plot::point>(&persisted::id)));
            }

            static auto table_object_sym() {
                using namespace sqlite_orm;
                return make_table<plot::object::sym>(
                        "plot_object_sym",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::sym::class_id),
                        make_column("point_id", &plot::object::sym::point_id),
                        make_column("object_id", &plot::object::sym::object_id),
                        foreign_key(&plot::object::sym::class_id)
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(&plot::object::sym::point_id).references(column<plot::point>(&persisted::id)),
                        foreign_key(&plot::object::sym::object_id).references(column<plot::symbol>(&persisted::id)));
            }

            static auto table_object_cls() {
                using namespace sqlite_orm;
                return make_table<plot::object::cls>(
                        "plot_object_cls",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::cls::class_id),
                        make_column("point_id", &plot::object::cls::point_id),
                        make_column("object_id", &plot::object::cls::object_id),
                        foreign_key(column<plot::object::cls>(&plot::object::cls::class_id))
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(column<plot::object::cls>(&plot::object::cls::point_id))
                                .references(column<plot::point>(&persisted::id)),
                        foreign_key(column<plot::object::cls>(&plot::object::cls::object_id))
                                .references(column<plot::symbol::clazz>(&persisted::id)));
            }

            static auto table_object_pnt() {
                using namespace sqlite_orm;
                return make_table<plot::object::pnt>(
                        "plot_object_pnt",
                        make_column("id", &persisted::id, primary_key(), autoincrement()),
                        make_column("class_id", &plot::object::pnt::class_id),
                        make_column("point_id", &plot::object::pnt::point_id),
                        make_column("object_id", &plot::object::pnt::object_id),
                        foreign_key(&plot::object::pnt::class_id)
                                .references(column<plot::object::clazz>(&persisted::id)),
                        foreign_key(&plot::object::pnt::point_id).references(column<plot::point>(&persisted::id)),
                        foreign_key(&plot::object::pnt::object_id).references(column<plot::point>(&persisted::id)));
            }

            static auto make_storage(char const *filename) {
                using namespace sqlite_orm;
                return sqlite_orm::make_storage(
                        filename,
                        db::table_source(),
                        make_unique_index("source_id_pkey_uindex", column<source>(&persisted::id)),
                        db::table_source(),

                        db::table_path(),
                        make_unique_index("path_id_pkey_uindex", column<path>(&persisted::id)),
                        //make_unique_index("path_name_parent_id_uindex", column<path>(&path::name), column<path>(&path::parent_id)),
                        db::table_path(),

                        db::table_symbol_class(),
                        make_unique_index("plot_symbol_class_id_pkey_uindex",
                                          column<plot::symbol::clazz>(&persisted::id)),
                        //make_unique_index("plot_symbol_class_path_id_uindex", column<plot::symbol::clazz>(&plot::symbol::clazz::path_id)),
                        db::table_symbol_class(),
                        db::table_symbol_class_hint(),
                        make_unique_index("plot_symbol_class_hint_id_pkey_uindex",
                                          column<plot::symbol::clazz::hint>(&persisted::id)),
                        db::table_symbol_class_hint(),
                        db::table_symbol(),
                        make_unique_index("plot_symbol_id_pkey_uindex", column<plot::symbol>(&persisted::id)),
                        //make_unique_index("plot_symbol_name_class_id_uindex", column<plot::symbol>(&plot::symbol::name), column<plot::symbol>(&plot::symbol::class_id)),
                        db::table_symbol(),
                        db::table_symbol_compound(),
                        make_unique_index("plot_symbol_compound_id_pkey_uindex",
                                          column<plot::symbol::compound>(&persisted::id)),
                        db::table_symbol_compound(),

                        db::table_point_class(),
                        make_unique_index("plot_point_class_id_pkey_uindex",
                                          column<plot::point::clazz>(&persisted::id)),
                        //make_unique_index("plot_point_class_path_id_uindex", column<plot::symbol::clazz>(&plot::point::clazz::path_id)),
                        db::table_point_class(),
                        db::table_point_class_hint(),
                        make_unique_index("plot_point_class_hint_id_pkey_uindex",
                                          column<plot::point::clazz::hint>(&persisted::id)),
                        db::table_point_class_hint(),
                        db::table_point(),
                        make_unique_index("plot_poin_id_pkey_uindex", column<plot::point>(&persisted::id)),
                        db::table_point(),
                        db::table_point_subject_sym(),
                        make_unique_index("plot_point_subject_sym_id_pkey_uindex",
                                          column<plot::point::subject::sym>(&persisted::id)),
                        db::table_point_subject_sym(),
                        db::table_point_subject_cls(),
                        make_unique_index("plot_point_subject_class_id_pkey_uindex",
                                          column<plot::point::subject::cls>(&persisted::id)),
                        db::table_point_subject_cls(),

                        db::table_object_class(),
                        make_unique_index("plot_object_class_id_pkey_uindex",
                                          column<plot::object::clazz>(&persisted::id)),
                        //make_unique_index("plot_object_class_name_point_class_id_uindex", column<plot::object::clazz>(&plot::object::clazz::name), column<plot::object::clazz>(&plot::object::clazz::point_class_id)),
                        db::table_object_class(),
                        db::table_object_class_hint_lit(),
                        make_unique_index("plot_object_class_hint_lit_id_pkey_uindex",
                                          column<plot::object::clazz::hint::lit>(&persisted::id)),
                        db::table_object_class_hint_lit(),
                        db::table_object_class_hint_sym(),
                        make_unique_index("plot_object_class_hint_sym_id_pkey_uindex",
                                          column<plot::object::clazz::hint::sym>(&persisted::id)),
                        db::table_object_class_hint_sym(),
                        db::table_object_class_hint_pnt(),
                        make_unique_index("plot_object_class_hint_pnt_id_pkey_uindex",
                                          column<plot::object::clazz::hint::pnt>(&persisted::id)),
                        db::table_object_class_hint_pnt(),
                        db::table_object_lit(),
                        make_unique_index("plot_object_lit_id_pkey_uindex", column<plot::object::lit>(&persisted::id)),
                        db::table_object_lit(),
                        db::table_object_sym(),
                        make_unique_index("plot_object_sym_id_pkey_uindex", column<plot::object::sym>(&persisted::id)),
                        db::table_object_sym(),
                        db::table_object_cls(),
                        make_unique_index("plot_object_cls_id_pkey_uindex", column<plot::object::cls>(&persisted::id)),
                        db::table_object_cls(),
                        db::table_object_pnt(),
                        make_unique_index("plot_object_pnt_id_pkey_uindex", column<plot::object::pnt>(&persisted::id)),
                        db::table_object_pnt());
            }

            friend class plang::db;
            friend class plang::detail::storage;
        };

        class storage : public std::invoke_result_t<decltype(detail::db::make_storage), char const *> {
        public:
            using base_storage_t = typename std::invoke_result_t<decltype(detail::db::make_storage), char const *>;

            storage(char const *filename);

            virtual ~storage();
        };
    }// namespace detail

    */

    class corpus;

    /// \brief Database manager for plang corpuses
    class db {
    public:
        //using storage_t = detail::storage;

        /*
        template<typename T>
        static void make_indexes(T &storage) {
            using namespace sqlite_orm;
            storage.execute();

            storage.execute(make_unique_index("path_pkey_uindex_id", column<path>(&persisted::id)));

            storage.execute(make_unique_index("id", column<plot::symbol::clazz>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::symbol::clazz::hint>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::symbol>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::symbol::compound>(&persisted::id)));

            storage.execute(make_unique_index("id", column<plot::point::clazz>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::point::clazz::hint>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::point>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::point::subject::sym>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::point::subject::cls>(&persisted::id)));

            storage.execute(make_unique_index("id", column<plot::object::clazz>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::clazz::hint::lit>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::clazz::hint::sym>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::clazz::hint::pnt>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::lit>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::sym>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::cls>(&persisted::id)));
            storage.execute(make_unique_index("id", column<plot::object::pnt>(&persisted::id)));
        }
        */

        //storage_t storage;  ///< \brief Underlying sqlite_orm storage object
        sqlite3 *connection;///< \brief Underlying sqlite3 connection object

        /// \brief Default constructor
        db();

        /// \brief Constructs a database object for a given file
        /// \param path Path to the database file
        explicit db(std::filesystem::path const &path);

        /// \brief Default destructor
        ~db();

        friend class corpus;
    };

}// namespace plang

#endif//LIBPLANG_STORAGE_HPP
