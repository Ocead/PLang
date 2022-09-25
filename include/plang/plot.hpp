//
// Created by Johannes on 04.08.2022.
//

#ifndef LIBPLANG_PLOT_HPP
#define LIBPLANG_PLOT_HPP

#include <plang/base.hpp>

/// \brief Contains the data model for the plot
namespace plang::plot {

    class symbol : public plang::persisted, public plang::decorated {
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
