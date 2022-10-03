//
// Created by Johannes on 16.08.2022.
//

/// \page lang_plot_symbol Symbols

#ifndef PLANG_SYMBOL_HPP
#define PLANG_SYMBOL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/plot.hpp>

namespace plang::detail {

    class symbol_class_manager : virtual protected plang::detail::corpus, virtual private plang::detail::path_manager {
    protected:
        ostream_t &print_helper(ostream_t &os, pkey_t id, format format) const;

        std::vector<plot::symbol::clazz::hint> _fetch_hints(pkey_t id) const;

        void _update_hints(pkey_t id, std::vector<plot::symbol::clazz::hint> &hints);

    public:
        std::optional<plot::symbol::clazz> fetch(pkey_t id,
                                                 bool_t dynamic           = false,
                                                 tag<plot::symbol::clazz> = {}) const;

        std::vector<plot::symbol::clazz> fetch_n(std::vector<pkey_t> const &ids,
                                                 bool_t dynamic           = false,
                                                 tag<plot::symbol::clazz> = {}) const;

        std::vector<plot::symbol::clazz>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::symbol::clazz> = {}) const;

        resolve_result<plot::symbol::clazz> resolve(std::vector<string_t> const &path,
                                                    class path const &ctx,
                                                    bool_t insert                    = false,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::symbol::clazz> = {});

        resolve_result<plot::symbol::clazz> resolve(std::vector<string_t> const &path,
                                                    class path const &ctx,
                                                    bool_t dynamic                   = false,
                                                    corpus::tag<plot::symbol::clazz> = {}) const;

        resolve_ref_result<plot::symbol::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::symbol::clazz &ent,
                                                        class path const &ctx,
                                                        bool_t insert  = false,
                                                        bool_t dynamic = false);

        resolve_ref_result<plot::symbol::clazz> resolve(std::vector<string_t> const &path,
                                                        plot::symbol::clazz &ent,
                                                        class path const &ctx,
                                                        bool_t dynamic = false) const;

        action insert(plot::symbol::clazz &clazz, bool_t replace = false, corpus::tag<plot::symbol::clazz> = {});

        action update(plot::symbol::clazz &clazz, bool_t dynamic = false, corpus::tag<plot::symbol::clazz> = {});

        stream_helper print(pkey_t id, format format, corpus::tag<plot::symbol::clazz> = {}) const;

        std::tuple<string_t, action> remove(plot::symbol::clazz &clazz,
                                            bool_t cascade,
                                            corpus::tag<plot::symbol::clazz> = {});

        ~symbol_class_manager();
    };

    class symbol_manager : virtual protected plang::detail::corpus {
    public:
    };

}// namespace plang::detail

#endif//PLANG_SYMBOL_HPP
