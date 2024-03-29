//
// Created by Johannes on 22.08.2022.
//

#ifndef LIBPLANG_FORMS_HPP
#define LIBPLANG_FORMS_HPP

#include <any>
#include <optional>
#include <vector>
#include <plang/base.hpp>
#include "lang/GENERATED/PlangBaseVisitor.h"

namespace plang::lang {

    using any_vector_t = std::vector<std::any>;

    class any_vector : public any_vector_t {
    public:
        using any_vector_t::any_vector_t;
    };

    struct decoration_form {
        antlr4::tree::TerminalNode *ordinal;
        antlr4::tree::TerminalNode *description;
    };

    struct literal_form {
        antlr4::tree::TerminalNode *string;
        char_t modifier;
    };

    struct path_form {
        any_vector nodes;
        bool_t qualified{false};
        std::optional<decoration_form> decoration;
    };

    struct symbol_class_form {
        path_form path;
        any_vector list;
        bool_t recursive;
    };

    struct single_symbol_form {
        antlr4::tree::TerminalNode *name;
        std::optional<decoration_form> decoration;
    };

    struct symbol_form {
        path_form path;
        any_vector list;
    };

    struct point_class_form {
        path_form path;
        bool_t singleton;
        bool_t recursive;

        any_vector hints;
        any_vector object_classes;
    };

    struct object_class_form {
        path_form path;
        antlr4::tree::TerminalNode *name;
        bool_t _default;
        bool_t singleton;
        bool_t local;
        bool_t hinted;

        any_vector hints;
    };

    /// \brief Helper struct for casting into variants
    /// \tparam T Return type template parameter
    template<typename T>
    struct variant_cast;

    /// \brief Helper struct for casting into variants
    /// \tparam V Variant template type
    /// \tparam Args Union types
    template<template<typename...> typename V, typename... Args>
    struct variant_cast<V<Args...>> {

        /// Casts any value into a variant
        /// \param a Any value
        /// \return The value in a variant
        std::optional<V<Args...>> operator()(std::any a) {
            if (!a.has_value()) throw std::bad_any_cast();

            std::optional<V<Args...>> v = std::nullopt;

            bool found = ((a.type() == typeid(Args) && (v = std::any_cast<Args>(std::move(a)), true)) || ...);

            if (!found) throw std::bad_any_cast{};

            return std::move(*v);
        }
    };

}// namespace plang::lang

#endif//LIBPLANG_FORMS_HPP
