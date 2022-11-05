//
// Created by Johannes on 16.08.2022.
//

/** \page lang_plot_symbol Symbols
 *
 * [TOC]
 *
 * # Symbol classes
 *
 * Symbol classes structure everything you can make statements about, the symbols.
 * The symbol classes serve here as a primary type or kind of symbol. For example:
 *
 * + the symbol class `.character[]` contains each person, character, speaking role, NPC, etc.
 * + every food item would be a symbol of the class `.world.culture.artifact.food[]`
 * + every color is shelved under `.object.optical.color[]` and
 * + each place of interest under `.world.geography.natural[]` or `.world.geography.human.building[]`.
 *
 * Each symbol class has a unique path and will provide you with a reference
 * of what kinds of statement can be made about a symbol.
 * This is important, since in PLang each statement (named a point) has the same structure.
 *
 * ## Declaration
 *
 * The identifier for symbol classes and symbols is a pair of square brackets (`[]`).
 * A symbol class is declared by declaring a path with a pair of square brackets immediately behind it like so:
 *
 *
 * ```
 * .object.optical.color[];
 * ```
 *
 * Here `.object.optical.color` is the path and `[]` makes the path declaration a symbol class declaration.
 *
 * Decorations for paths may be also applied to symbol classes:
 *
 * ```
 * .object.optical.color[] (6, "A visible color");
 * ```
 *
 * Symbol class declarations may include hints on how to form meaningful compound symbols:
 *
 * ```
 * (.object.optical.color[]...) .object.optical.color[] (6, "A visible color");
 * ```
 *
 * The symbol classes whose symbols may precede the declared symbol class are written in parentheses (`()`)
 * before the path of the symbol class, separated with a comma (`,`).
 * If the hinted symbol class is followed with an ellipsis (`...`) the hint includes all symbol classes
 * contained within the path of the hinted symbol class. For example the hint above is a shorter notation equal to:
 *
 * ```
 * (.object.optical.color.basic[],
 *  .object.optical.color.saturation[],
 *  .object.optical.color.luminance[]  ) .object.optical.color[] (6, "A visible color");
 * ```
 *
 * In the default `implicit` mode,
 * reference of symbol classes in hint lists may precede the declaration of the symbol class.
 *
 * To override the hint list of a symbol class, re-declare it with the new hint list.
 * To delete the hint list of a symbol class, re-declare it an empty hint list (`()`);
 *
 * ## Reference
 * To reference a symbol class, type the fully-qualified or any unique path of the class followed by a pair of
 * square brackets (`[]`). Given no conflicting declarations exist,
 * these lines would all reference the same symbol class:
 *
 * ```
 * .world.culture.artifact.food[]
 * world.culture.artifact.food[]
 * culture.artifact.food[]
 * artifact.food[]
 * food[]
 * ```
 *
 * What counts as a unique path may broaden depending on the context the reference is made in.
 *
 * # Symbols
 *
 * Symbols act as the subjects and objects to make statements about/with.
 * Extending the examples from  the symbol classes:
 *
 * + `.character[protagonist]` and `.character[narrator]` are instances of characters
 * + `.world.culture.artifact.food[pizza]` is an instance of food
 * + `.object.optical.color[red]` and `.object.optical.color[green]` are colors
 * + `.world.geography.human[protagonists_home]`
 * and `.world.geography.natural[a_random_tree]` might be places of interest
 *
 * Within a single class, all symbol names again have to be unique. There may be multiple symbols with the same name,
 * as long as they all have different classes. In the latter case, the paths to the symbol classes may be used to
 * identify a specific symbol.
 *
 * ## Declaration
 *
 * Again, square brackets (`[]`) signify a symbol, with the name of the symbol placed in between the brackets, like so:
 *
 * ```
 * .character[protagonist];
 * ```
 * This would declare a symbol named "protagonist" from the class `.character`.
 *
 * Symbols may be decorated in the same way as paths and symbol classes. The decoration is applied within the symbol
 * brackets:
 *
 * ```
 * .character[protagonist (0, "This is the protagonist")];
 * ```
 *
 * ## Reference
 *
 * To reference a symbol, write any unique combination of the symbol classes path and the symbol name
 * in square brackets.
 *
 * Given no conflicting declarations exists,
 * these lines would all reference the same symbol:
 *
 * ```
 * .world.geography.natural[a_random_tree]
 * world.geography.natural[a_random_tree]
 * geography.natural[a_random_tree]
 * natural[a_random_tree]
 * [a_random_tree]
 * ```
 *
 * What counts as a unique reference may broaden depending on the context the reference is made in.
 */

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

    class symbol_class_manager : virtual protected plang::detail::corpus,
                                 virtual protected plang::detail::path_manager {
    private:
        std::vector<plot::symbol> _all_symbols_for_path(pkey<plot::symbol> id, bool_t dynamic);

        std::vector<plot::symbol> _all_symbols_for_compound(pkey<plot::symbol> id, bool_t dynamic);

        std::vector<plot::symbol> _all_symbols_for_subject(pkey<plot::symbol> id, bool_t dynamic);

    protected:
        ostream_t &print_helper(ostream_t &os, pkey<plot::symbol::clazz> id, format format) const;

        std::vector<plot::symbol::clazz::hint> _fetch_hints(pkey<plot::symbol::clazz> id) const;

        void _update_hints(pkey<plot::symbol::clazz> id, std::vector<plot::symbol::clazz::hint> &hints);

    public:
        std::optional<plot::symbol::clazz> fetch(pkey<plot::symbol::clazz> id,
                                                 bool_t dynamic           = false,
                                                 tag<plot::symbol::clazz> = {}) const;

        std::vector<plot::symbol::clazz> fetch_n(std::vector<pkey<plot::symbol::clazz>> const &ids,
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

        stream_helper print(pkey<plot::symbol::clazz> id, format format, corpus::tag<plot::symbol::clazz> = {}) const;

        std::tuple<string_t, action> remove(plot::symbol::clazz &clazz,
                                            bool_t cascade,
                                            corpus::tag<plot::symbol::clazz> = {});

        ~symbol_class_manager();
    };

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


}// namespace plang::detail

#endif//PLANG_SYMBOL_HPP
