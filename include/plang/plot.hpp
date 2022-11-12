//
// Created by Johannes on 04.08.2022.
//

#ifndef LIBPLANG_PLOT_HPP
#define LIBPLANG_PLOT_HPP

#include <variant>
#include <vector>
#include <plang/base.hpp>

namespace plang::detail {
    class symbol_manager;
    class symbol_class_manager;
    class point_class_manager;
    class point_manager;
    class object_class_manager;
    class object_manager;
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

    private:
        symbol();

    protected:
        col<symbol, string_t> name;
        col<symbol, pkey<clazz>> class_id;

    public:
        symbol(string_t name, symbol::clazz const &clazz);

        string_t const &get_name() const;

        void set_name(string_t &&name);

        pkey<symbol::clazz> get_class_id() const;

        void set_class(symbol::clazz const &clazz);

        friend class plang::detail::symbol_manager;
    };

    class point : public plang::persisted<point>, public sourced {
    public:
        class clazz : public plang::persisted<clazz>, public sourced {
        public:
            class hint : public plang::persisted<hint>, public sourced {
            protected:
                col<hint, pkey<symbol::clazz>> hint_id;
                col<hint, bool_t> recursive;

                hint();

            public:
                hint(symbol::clazz const &hint, bool_t recursive = false);

                pkey<symbol::clazz> get_hint_id() const;

                void set_hint(symbol::clazz const &hint);

                bool_t get_recursive() const;

                void set_recursive(bool_t recursive);

                friend detail::point_class_manager;
            };

        protected:
            col<clazz, pkey<path>> path_id;///< \brief Path of the point class
            col<clazz, bool_t> singleton;  ///< \brief Whether the point class produces singletons
            std::vector<hint> hints;       ///< \brief Hints for point subjects

            clazz();

        public:
            clazz(path const &path, bool_t singleton = false);

            pkey<path> get_path_id() const;

            void set_path(path const &path);

            bool_t get_singleton() const;

            void set_singleton(bool_t singleton);

            std::vector<hint> &get_hints();

            std::vector<hint> const &get_hints() const;

            friend class plang::detail::point_class_manager;
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
        class clazz : public plang::persisted<clazz>, public sourced, public plang::decorated {
        public:
            class hint {
            public:
                class lit : public plang::persisted<lit>, public sourced {
                public:
                    enum class type : int_t {
                        COMMENT = 0,
                        LIKE    = 1,
                        GLOB    = 2,
                        REGEX   = 3,
                        MATCH   = 4
                    };

                protected:
                    col<lit, string_t> hint;
                    type _type;

                    constexpr int_t _get_type() const {
                        return static_cast<int>(_type);
                    }

                    constexpr void _set_type(int type) {
                        lit::_type = static_cast<enum type>(type);
                    }

                public:
                    lit(string_t hint, type type = type::COMMENT);

                    string_t const &get_hint() const;

                    void set_hint(string_t hint);

                    type get_type() const;

                    void set_type(type type);

                    friend class detail::object_class_manager;
                };

                class sym : public plang::persisted<sym>, public sourced {
                protected:
                    col<sym, pkey<symbol::clazz>> hint_id;
                    col<sym, bool_t> recursive;

                    sym();

                public:
                    sym(symbol::clazz const &clazz, bool_t recursive = false);

                    pkey<symbol::clazz> get_hint_id() const;

                    void set_hint(symbol::clazz const &hint);

                    bool_t get_recurisve() const;

                    void set_recursive(bool_t recursive);

                    friend class detail::object_class_manager;
                };

                class pnt : public plang::persisted<pnt>, public sourced {
                protected:
                    col<pnt, pkey<point::clazz>> hint_id;
                    col<pnt, bool_t> recursive;

                    pnt();

                public:
                    pnt(point::clazz const &clazz, bool_t recursive = false);

                    pkey<point::clazz> get_hint_id() const;

                    void set_hint(point::clazz const &hint);

                    bool_t get_recurisve() const;

                    void set_recursive(bool_t recursive);

                    friend class detail::object_class_manager;
                };

                using variant = std::variant<hint::lit, hint::sym, hint::pnt>;
            };

        protected:
            col<clazz, pkey<point::clazz>> point_class_id;
            col<clazz, string_t> name;
            col<clazz, bool_t> _default;
            col<clazz, bool_t> singleton;
            std::vector<hint::variant> hints;

            clazz();

        public:
            clazz(point::clazz const &clazz, string_t name, bool_t _default = false);

            pkey<point::clazz> get_point_class_id() const;

            void set_point_class(point::clazz const &clazz);

            string_t const &get_name() const;

            void set_name(string_t const &name);

            bool_t get_default() const;

            void set_default(bool_t _default);

            bool_t get_singleton() const;

            void set_singleton(bool_t singleton);

            std::vector<hint::variant> &get_hints();

            std::vector<hint::variant> const &get_hints() const;

            friend class detail::object_class_manager;
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
