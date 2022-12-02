//
// Created by Johannes on 02.12.2022.
//

#ifndef PLANG_SYMBOL_HPP
#define PLANG_SYMBOL_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/plot/symbol_class.hpp>
#include <plang/plot.hpp>


namespace plang::detail {

    class symbol_manager : virtual protected plang::detail::corpus,
                           virtual protected plang::detail::symbol_class_manager {
    private:
        plot::symbol select_symbol(stmt &stmt, bool_t dynamic) const;

        std::vector<std::tuple<pkey<path>, pkey<plot::symbol::clazz>, pkey<plot::symbol>, uint_t>>
        partially_resolve(std::vector<string_t> const &path, bool_t fully) const;

    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::symbol> id, format format) const;

        ostream_t &print_decoration(ostream_t &os, pkey<plot::symbol> id, format format) const;

    public:
        std::optional<plot::symbol> fetch(pkey<plot::symbol> id, bool_t dynamic = false, tag<plot::symbol> = {}) const;

        std::vector<plot::symbol> fetch_n(std::vector<pkey<plot::symbol>> const &ids,
                                          bool_t dynamic    = false,
                                          tag<plot::symbol> = {}) const;

        std::vector<plot::symbol>
        fetch_all(bool_t dynamic = true, int_t limit = -1, int_t offset = 0, tag<plot::symbol> = {}) const;

        resolve_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                             class path const &ctx,
                                             bool_t insert             = false,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::symbol> = {});

        resolve_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                             class path const &ctx,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::symbol> = {}) const;

        resolve_ref_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                                 plot::symbol &ent,
                                                 class path const &ctx,
                                                 bool_t insert  = false,
                                                 bool_t dynamic = false);

        resolve_ref_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                                 plot::symbol &ent,
                                                 class path const &ctx,
                                                 bool_t dynamic = false) const;

        resolve_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                             class plot::symbol::clazz const &ctx,
                                             bool_t insert             = false,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::symbol> = {});

        resolve_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                             class plot::symbol::clazz const &ctx,
                                             bool_t dynamic            = false,
                                             corpus::tag<plot::symbol> = {}) const;

        resolve_ref_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                                 plot::symbol &ent,
                                                 class plot::symbol::clazz const &ctx,
                                                 bool_t insert  = false,
                                                 bool_t dynamic = false);

        resolve_ref_result<plot::symbol> resolve(std::vector<string_t> const &path,
                                                 plot::symbol &ent,
                                                 class plot::symbol::clazz const &ctx,
                                                 bool_t dynamic = false) const;

        action insert(plot::symbol &sym, bool_t replace = false, corpus::tag<plot::symbol> = {});

        action update(plot::symbol &sym, bool_t dynamic = false, corpus::tag<plot::symbol> = {});

        stream_helper print(pkey<plot::symbol> id, format format, corpus::tag<plot::symbol> = {}) const;

        std::tuple<string_t, action> remove(plot::symbol &sym, bool_t cascade, corpus::tag<plot::symbol> = {});

        ~symbol_manager();
    };

}

#endif//PLANG_SYMBOL_HPP
