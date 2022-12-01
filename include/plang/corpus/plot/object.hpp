//
// Created by Johannes on 16.08.2022.
//

/// \page lang_plot_object Objects

#ifndef PLANG_OBJECT_HPP
#define PLANG_OBJECT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/corpus/plot/symbol.hpp>
#include <plang/plot.hpp>

namespace plang::detail {

    class object_class_manager : virtual protected plang::detail::corpus,
                                 virtual protected plang::detail::path_manager,
                                 virtual protected plang::detail::symbol_class_manager {
    private:
        plot::object::clazz select_object_class(stmt &stmt, bool_t dynamic) const;

        std::vector<std::tuple<pkey<path>, pkey<plot::point::clazz>, pkey<plot::object::clazz>, uint_t>>
        partially_resolve(const std::vector<string_t> &path, bool_t fully) const;

    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::object::clazz> id, format format) const;

        std::vector<plot::object::clazz::hint::variant> _fetch_hints(pkey<plot::object::clazz> id) const;

        void _update_hints(pkey<plot::object::clazz> id, std::vector<plot::object::clazz::hint::variant> &hints);

    public:
        std::optional<plot::object::clazz> fetch(pkey<plot::object::clazz> id,
                                                 bool_t dynamic           = false,
                                                 tag<plot::object::clazz> = {}) const;

        std::vector<plot::object::clazz> fetch_n(std::vector<pkey<plot::object::clazz>> const &ids,
                                                 bool_t dynamic           = false,
                                                 tag<plot::object::clazz> = {}) const;

        std::vector<plot::object::clazz>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::object::clazz> = {}) const;

        resolve_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                    class path const &ctx,
                                                    bool_t insert                    = false,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::object::clazz> = {});

        resolve_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                    class path const &ctx,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::object::clazz> = {}) const;

        resolve_ref_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::object::clazz &ent,
                                                        class path const &ctx,
                                                        bool_t insert  = false,
                                                        bool_t dynamic = false);

        resolve_ref_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::object::clazz &ent,
                                                        class path const &ctx,
                                                        bool_t dynamic = false) const;

        resolve_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                    class plot::point::clazz const &ctx,
                                                    bool_t insert                    = false,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::object::clazz> = {});

        resolve_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                    class plot::point::clazz const &ctx,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::object::clazz> = {}) const;

        resolve_ref_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::object::clazz &ent,
                                                        class plot::point::clazz const &ctx,
                                                        bool_t insert  = false,
                                                        bool_t dynamic = false);

        resolve_ref_result<plot::object::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::object::clazz &ent,
                                                        class plot::point::clazz const &ctx,
                                                        bool_t dynamic = false) const;

        action insert(plot::object::clazz &clazz, bool_t replace = false, corpus::tag<plot::object::clazz> = {});

        action update(plot::object::clazz &clazz, bool_t dynamic = false, corpus::tag<plot::object::clazz> = {});

        stream_helper print(pkey<plot::object::clazz> id, format format, corpus::tag<plot::object::clazz> = {}) const;

        std::tuple<string_t, action> remove(plot::object::clazz &clazz,
                                            bool_t cascade,
                                            corpus::tag<plot::object::clazz> = {});

        ~object_class_manager();
    };

}// namespace plang::detail

#endif//PLANG_OBJECT_HPP
