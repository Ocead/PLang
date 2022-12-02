//
// Created by Johannes on 02.12.2022.
//

#ifndef PLANG_POINT_HPP
#define PLANG_POINT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/plot/symbol.hpp>
#include <plang/corpus/plot/point_class.hpp>
#include <plang/corpus/plot/object_class.hpp>
#include <plang/plot.hpp>

namespace plang::detail {

    class point_manager : virtual protected plang::detail::corpus,
                          virtual protected plang::detail::symbol_manager,
                          virtual protected plang::detail::point_class_manager,
                          virtual protected plang::detail::object_class_manager {
    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::point> id, format format) const;

    public:
        std::optional<plot::point> fetch(pkey<plot::point> id, bool_t dynamic = false, tag<plot::point> = {}) const;

        std::vector<plot::point> fetch_n(std::vector<pkey<plot::point>> const &ids,
                                         bool_t dynamic   = false,
                                         tag<plot::point> = {}) const;

        std::vector<plot::point>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::point> = {}) const;

        resolve_result<plot::point> resolve(std::vector<string_t> const &path,
                                            class path const &ctx,
                                            bool_t insert            = false,
                                            bool_t dynamic           = false,
                                            corpus::tag<plot::point> = {});

        resolve_result<plot::point> resolve(std::vector<string_t> const &path,
                                            class path const &ctx,
                                            bool_t dynamic           = false,
                                            corpus::tag<plot::point> = {}) const;

        resolve_ref_result<plot::point> resolve(std::vector<string_t> const &path,
                                                plot::point &ent,
                                                class path const &ctx,
                                                bool_t insert  = false,
                                                bool_t dynamic = false);

        resolve_ref_result<plot::point> resolve(std::vector<string_t> const &path,
                                                plot::point &ent,
                                                class path const &ctx,
                                                bool_t dynamic = false) const;

        action insert(plot::point &clazz, bool_t replace = false, corpus::tag<plot::point> = {});

        action update(plot::point &clazz, bool_t dynamic = false, corpus::tag<plot::point> = {});

        stream_helper print(pkey<plot::point> id, format format, corpus::tag<plot::point> = {}) const;

        std::tuple<string_t, action> remove(plot::point &clazz, bool_t cascade, corpus::tag<plot::point> = {});

        ~point_manager();
    };
}// namespace plang::detail

#endif//PLANG_POINT_HPP
