//
// Created by Johannes on 16.08.2022.
//

/** \page lang_plot_symbol Symbols
 * # Symbol classes
 *
 * Symbol classes structure everything you can make statements about, the symbols. The symbol classes serve here as a primary type or kind of symbol. For example:
 *
 * + the symbol class `.charcter[]` contains each person, character, speaking role, NPC, etc.
 * + every food item would be a symbol of the class `.world.culture.artifact.food[]`
 * + every color is shelved under `.object.optical.color[]` and
 * + each place of interest under `.world.geography.natural[]` or `.world.geography.human.building[]`.
 *
 * Each symbol class has a unique path and will provide you with a reference of what kinds of statement can be made about a symbol. This is important, since in PLang each statement (named a point) has the same structure.
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
 * The symbol classes whose symbols may precede the declared symbol class are written in parentheses (`()`) before the path of the symbol class, separated with a comma (`,`). If the hinted symbol class is followed with an ellipsis (`...`) the hint includes all symbol classes contained within the path of the hinted symbol class. For example the hint above is a shorter notation equal to:
 *
 * ```
 * (.object.optical.color.basic[],
 *  .object.optical.color.saturation[],
 *  .object.optical.color.luminance[]  ) .object.optical.color[] (6, "A visible color");
 * ```
 *
 * In the default `implicit` mode, reference of symbol classes in hint lists may precede the declaration of the symbol class.
 *
 * To override the hint list of a symbol class, re-declare it with the new hint list.
 * To delete the hint list of a symbol class, re-declare it an empty hint list (`()`);
 *
 * ## Reference
 * To reference a symbol class, type the fully-qualified or any unique path of the class followed by a pair or square brackets (`[]`). Given no conflicting declarations exist, these lines would all reference the same symbol class:
 *
 * ```
 *                                                                                                                                                             .world.culture.artifact.food[]
 * world.culture.artifact.food[]
 * culture.artifact.food[]
 * artifact.food[]
 * food[]
 * ```
 *
 * What counts as a unique path may broaden depending on the context the reference is made in.
 *
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
