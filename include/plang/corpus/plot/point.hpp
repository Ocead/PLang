//
// Created by Johannes on 16.08.2022.
//

/// \page lang_plot_point Points

#ifndef PLANG_POINT_HPP
#define PLANG_POINT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/corpus/plot/symbol.hpp>
#include <plang/corpus/plot/object.hpp>
#include <plang/plot.hpp>

namespace plang::detail {

    class point_class_manager : virtual protected plang::detail::corpus,
                                virtual protected plang::detail::path_manager,
                                virtual protected plang::detail::symbol_class_manager,
                                virtual protected plang::detail::object_class_manager {
    private:
        std::vector<std::tuple<pkey<path>, pkey<plot::point::clazz>, uint_t>>
        partially_resolve(const std::vector<string_t> &path, bool_t fully) const;

    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::point::clazz> id, format format) const;

        std::vector<plot::point::clazz::hint> _fetch_hints(pkey<plot::point::clazz> id) const;

        void _update_hints(pkey<plot::point::clazz> id, std::vector<plot::point::clazz::hint> &hints);

    public:
        std::optional<plot::point::clazz> fetch(pkey<plot::point::clazz> id,
                                                bool_t dynamic          = false,
                                                tag<plot::point::clazz> = {}) const;

        std::vector<plot::point::clazz> fetch_n(std::vector<pkey<plot::point::clazz>> const &ids,
                                                bool_t dynamic          = false,
                                                tag<plot::point::clazz> = {}) const;

        std::vector<plot::point::clazz>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::point::clazz> = {}) const;

        resolve_result<plot::point::clazz> resolve(std::vector<string_t> const &path,
                                                   class path const &ctx,
                                                   bool_t insert                   = false,
                                                   bool_t dynamic                  = false,
                                                   corpus::tag<plot::point::clazz> = {});

        resolve_result<plot::point::clazz> resolve(std::vector<string_t> const &path,
                                                   class path const &ctx,
                                                   bool_t dynamic                  = false,
                                                   corpus::tag<plot::point::clazz> = {}) const;

        resolve_ref_result<plot::point::clazz> resolve(std::vector<string_t> const &path,
                                                       plot::point::clazz &ent,
                                                       class path const &ctx,
                                                       bool_t insert  = false,
                                                       bool_t dynamic = false);

        resolve_ref_result<plot::point::clazz> resolve(std::vector<string_t> const &path,
                                                       plot::point::clazz &ent,
                                                       class path const &ctx,
                                                       bool_t dynamic = false) const;

        action insert(plot::point::clazz &clazz, bool_t replace = false, corpus::tag<plot::point::clazz> = {});

        action update(plot::point::clazz &clazz, bool_t dynamic = false, corpus::tag<plot::point::clazz> = {});

        stream_helper print(pkey<plot::point::clazz> id, format format, corpus::tag<plot::point::clazz> = {}) const;

        std::tuple<string_t, action> remove(plot::point::clazz &clazz,
                                            bool_t cascade,
                                            corpus::tag<plot::point::clazz> = {});

        ~point_class_manager();
    };

}// namespace plang::detail

#endif//PLANG_POINT_HPP
