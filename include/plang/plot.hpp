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

    class symbol : public plang::persisted, public sourced, public plang::decorated {
    public:
        class clazz : public plang::persisted, public sourced {
        public:
            class hint : public plang::persisted, public sourced {
            protected:
                col<hint, int_t> hint_id;   ///< \brief Symbol class hinted to
                col<hint, bool_t> recursive;///< \brief <code>true</code>, if contained symbol classes are also valid

                hint();

            public:
                hint(symbol::clazz const &clazz, bool_t recursive = false);

                int_t get_hint_id() const;

                void set_hint(symbol::clazz const &clazz);

                bool_t get_recursive() const;

                void set_recursive(bool_t recursive);

                friend class plang::detail::symbol_class_manager;
            };

        private:
            /// \brief Default constructor for internal use
            clazz();

        protected:
            col<clazz, int_t> path_id;///< \brief Path of the symbol class
            std::vector<hint> hints;  ///< \brief Hints for compound symbols

        public:
            /// \brief Constructs an unpersisted symbol class under a path
            /// \param path Path of the symbol class
            /// \param hints Hints of the symbol class
            clazz(path const &path, std::vector<hint> && hints = {});

            /// \brief Returns the path id of the node
            /// \return The path id of the node
            int_t get_path_id() const;

            /// \brief Sets the path of the node
            /// \param parent The new path of the node
            void set_path(path const &path);

            std::vector<hint> &get_hints();

            std::vector<hint> const &get_hints() const;

            friend class plang::detail::symbol_class_manager;
        };

        class compound : public plang::persisted {
        protected:
            col<compound, int_t> symbol_id;
            col<compound, int_t> compound_id;
            col<compound, int_t> distance;
        };

    protected:
        col<symbol, string_t> name;
        col<symbol, int_t> class_id;

        friend class plang::detail::symbol_manager;
    };

    class point : public plang::persisted {
    public:
        class clazz : public plang::persisted {
        public:
            class hint : public plang::persisted {
            protected:
                col<hint, int_t> class_id;
                col<hint, int_t> hint_id;
                col<hint, bool_t> recursive;
            };

        protected:
            col<clazz, int_t> path_id;
            col<clazz, bool_t> singleton;
        };

        class subject {
        public:
            class sym : public plang::persisted {
            protected:
                col<sym, int_t> point_id;
                col<sym, int_t> symbol_id;
            };

            class cls : public plang::persisted {
            protected:
                col<cls, int_t> point_id;
                col<cls, int_t> symbol_class_id;
            };
        };

    protected:
        col<point, int_t> class_id;
        col<point, bool_t> truth;
    };

    class object {
    public:
        class clazz : public plang::persisted, public plang::decorated {
        public:
            class hint {
            public:
                class lit : public plang::persisted {
                public:
                    enum class type : int_t {
                        COMMENT = 0,
                        LIKE    = 1,
                        GLOB    = 2,
                        REGEX   = 3,
                        MATCH   = 4
                    };

                protected:
                    col<lit, int_t> class_id;
                    col<lit, string_t> hint;
                    type type;

                    int_t _get_type() const {
                        return static_cast<int>(type);
                    }

                    void _set_type(int type) {
                        lit::type = static_cast<enum type>(type);
                    }
                };

                class sym : public plang::persisted {
                protected:
                    col<sym, int_t> class_id;
                    col<sym, int_t> hint_id;
                    col<sym, bool_t> recursive;
                };

                class pnt : public plang::persisted {
                protected:
                    col<pnt, int_t> class_id;
                    col<pnt, int_t> hint_id;
                    col<pnt, bool_t> recursive;
                };
            };

        protected:
            col<clazz, int_t> point_class_id;
            col<clazz, string_t> name;
            col<clazz, bool_t> singleton;
        };

        class lit : public plang::persisted {
        protected:
            col<lit, int_t> class_id;
            col<lit, int_t> point_id;
            col<lit, string_t> object;
        };

        class sym : public plang::persisted {
        protected:
            col<sym, int_t> class_id;
            col<sym, int_t> point_id;
            col<sym, int_t> object_id;
        };

        class cls : public plang::persisted {
        protected:
            col<cls, int_t> class_id;
            col<cls, int_t> point_id;
            col<cls, int_t> object_id;
        };

        class pnt : public plang::persisted {
        protected:
            col<pnt, int_t> class_id;
            col<pnt, int_t> point_id;
            col<pnt, int_t> object_id;
        };

        object() = delete;
    };// namespace object

}// namespace plang::plot

#endif//LIBPLANG_PLOT_HPP
