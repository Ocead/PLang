//
// Created by Johannes on 04.08.2022.
//

#ifndef LIBPLANG_PLOT_HPP
#define LIBPLANG_PLOT_HPP

#include <vector>
#include <plang/base.hpp>

namespace plang::detail {
    class symbol_manager;
    class symbol_class_manager;
}// namespace plang::detail

/// \brief Contains the data model for the plot
namespace plang::plot {

    class symbol : public plang::persisted<symbol>, public sourced, public plang::decorated {
    public:
        class clazz : public plang::persisted<clazz>, public sourced {
        public:
            class hint : public plang::persisted<hint>, public sourced {
            protected:
                col<hint, pkey<clazz>> hint_id;///< \brief Symbol class hinted to
                col<hint, bool_t> recursive;   ///< \brief <code>true</code>, if contained symbol classes are also valid

                hint();

            public:
                hint(symbol::clazz const &clazz, bool_t recursive = false);

                pkey<clazz> get_hint_id() const;

                void set_hint(symbol::clazz const &clazz);

                bool_t get_recursive() const;

                void set_recursive(bool_t recursive);

                friend class plang::detail::symbol_class_manager;
            };

        private:
            /// \brief Default constructor for internal use
            clazz();

        protected:
            col<clazz, pkey<path>> path_id;///< \brief Path of the symbol class
            std::vector<hint> hints;       ///< \brief Hints for compound symbols

        public:
            /// \brief Constructs an unpersisted symbol class under a path
            /// \param path Path of the symbol class
            /// \param hints Hints of the symbol class
            clazz(path const &path, std::vector<hint> &&hints = {});

            /// \brief Returns the path id of the node
            /// \return The path id of the node
            pkey<path> get_path_id() const;

            /// \brief Sets the path of the node
            /// \param parent The new path of the node
            void set_path(path const &path);

            std::vector<hint> &get_hints();

            std::vector<hint> const &get_hints() const;

            friend class plang::detail::symbol_class_manager;
        };

        class compound : public plang::persisted<compound> {
        protected:
            col<compound, pkey<symbol>> symbol_id;
            col<compound, pkey<symbol>> compound_id;
            col<compound, int_t> distance;
        };

    protected:
        col<symbol, string_t> name;
        col<symbol, pkey<clazz>> class_id;

        friend class plang::detail::symbol_manager;
    };

    class point : public plang::persisted<point> {
    public:
        class clazz : public plang::persisted<clazz> {
        public:
            class hint : public plang::persisted<hint> {
            protected:
                col<hint, pkey<clazz>> class_id;
                col<hint, pkey<symbol::clazz>> hint_id;
                col<hint, bool_t> recursive;
            };

        protected:
            col<clazz, pkey<path>> path_id;
            col<clazz, bool_t> singleton;
        };

        class subject {
        public:
            class sym : public plang::persisted<sym> {
            protected:
                col<sym, pkey<point>> point_id;
                col<sym, pkey<symbol>> symbol_id;
            };

            class cls : public plang::persisted<cls> {
            protected:
                col<cls, pkey<point>> point_id;
                col<cls, pkey<symbol::clazz>> symbol_class_id;
            };
        };

    protected:
        col<point, pkey<clazz>> class_id;
        col<point, bool_t> truth;
    };

    class object {
    public:
        class clazz : public plang::persisted<clazz>, public plang::decorated {
        public:
            class hint {
            public:
                class lit : public plang::persisted<lit> {
                public:
                    enum class type : int_t {
                        COMMENT = 0,
                        LIKE    = 1,
                        GLOB    = 2,
                        REGEX   = 3,
                        MATCH   = 4
                    };

                protected:
                    col<lit, pkey<clazz>> class_id;
                    col<lit, string_t> hint;
                    type type;

                    int_t _get_type() const {
                        return static_cast<int>(type);
                    }

                    void _set_type(int _type) {
                        lit::type = static_cast<enum type>(_type);
                    }
                };

                class sym : public plang::persisted<sym> {
                protected:
                    col<sym, pkey<clazz>> class_id;
                    col<sym, pkey<symbol>> hint_id;
                    col<sym, bool_t> recursive;
                };

                class pnt : public plang::persisted<pnt> {
                protected:
                    col<pnt, pkey<clazz>> class_id;
                    col<pnt, pkey<point>> hint_id;
                    col<pnt, bool_t> recursive;
                };
            };

        protected:
            col<clazz, pkey<point::clazz>> point_class_id;
            col<clazz, string_t> name;
            col<clazz, bool_t> singleton;
        };

        class lit : public plang::persisted<lit> {
        protected:
            col<lit, pkey<clazz>> class_id;
            col<lit, pkey<point>> point_id;
            col<lit, string_t> object;
        };

        class sym : public plang::persisted<sym> {
        protected:
            col<sym, pkey<clazz>> class_id;
            col<sym, pkey<point>> point_id;
            col<sym, pkey<symbol>> object_id;
        };

        class cls : public plang::persisted<cls> {
        protected:
            col<cls, pkey<clazz>> class_id;
            col<cls, pkey<point>> point_id;
            col<cls, pkey<symbol::clazz>> object_id;
        };

        class pnt : public plang::persisted<pnt> {
        protected:
            col<pnt, pkey<clazz>> class_id;
            col<pnt, pkey<point>> point_id;
            col<pnt, pkey<point>> object_id;
        };

        object() = delete;
    };// namespace object

}// namespace plang::plot

#endif//LIBPLANG_PLOT_HPP
