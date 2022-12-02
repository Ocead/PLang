//
// Created by Johannes on 02.12.2022.
//

#ifndef PLANG_OBJECT_HPP
#define PLANG_OBJECT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/corpus/plot/point.hpp>
#include <plang/corpus/plot/symbol.hpp>
#include <plang/plot.hpp>

namespace plang::detail {

    class object_manager : virtual protected plang::detail::corpus,
                           virtual protected plang::detail::symbol_manager,
                           virtual protected plang::detail::point_manager {
    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::object> id, format format) const;

    public:
        std::optional<plot::object> fetch(pkey<plot::object> id, bool_t dynamic = false, tag<plot::object> = {}) const;

        std::vector<plot::object> fetch_n(std::vector<pkey<plot::object>> const &ids,
                                          bool_t dynamic    = false,
                                          tag<plot::object> = {}) const;

        std::vector<plot::object>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::object> = {}) const;

        resolve_result<plot::object> resolve(std::vector<string_t> const &path,
                                             class path const &ctx,
                                             bool_t insert             = false,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::object> = {});

        resolve_result<plot::object> resolve(std::vector<string_t> const &path,
                                             class path const &ctx,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::object> = {}) const;

        resolve_ref_result<plot::object> resolve(std::vector<string_t> const &path,
                                                 plot::object &ent,
                                                 class path const &ctx,
                                                 bool_t insert  = false,
                                                 bool_t dynamic = false);

        resolve_ref_result<plot::object> resolve(std::vector<string_t> const &path,
                                                 plot::object &ent,
                                                 class path const &ctx,
                                                 bool_t dynamic = false) const;

        action insert(plot::object &clazz, bool_t replace = false, corpus::tag<plot::object> = {});

        action update(plot::object &clazz, bool_t dynamic = false, corpus::tag<plot::object> = {});

        stream_helper print(pkey<plot::object> id, format format, corpus::tag<plot::object> = {}) const;

        std::tuple<string_t, action> remove(plot::object &clazz, bool_t cascade, corpus::tag<plot::object> = {});

        ~object_manager();
    };
}// namespace plang::detail

#endif//PLANG_OBJECT_HPP
