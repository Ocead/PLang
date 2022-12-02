//
// Created by Johannes on 16.08.2022.
//

/** \page lang_plot_object Objects
 *
 * [TOC]
 *
 * # Object classes
 *
 * Object classes structure the meaning of objects in points (sentences). Each point class possesses its own set of
 * object classes. While most point classes in the standard corpus get by with a single or even no required object,
 * some statements are more complex than that, for example:
 *
 * ```
 * (.character[]) .character.behaviour.speech
 *           what
 *           ?to : (.character[]);
 * ```
 *
 * This point class may be used for for a line of verbal dialogue. Its object classes are what is said (`what`) and to
 * whom (`?to`).
 *
 * ## Declaration
 *
 * Object classes are declared together with point classes. Their declaration follows the path and optional decoration
 * of the point class.
 *
 * The simplest form of declaring object classes is simply stating their names in sequence:
 *
 * ```
 * .character.behaviour.speech ?what ?to;
 * ```
 *
 * Each object class begins with a question mark (`?`) followed by a name unique to the scope of the point class,
 * meaning multiple point classes may have an object class named `?what`.
 * Even if there are multiple object classes, they are not connected with commas (`,`) in this case. This is done to
 * mimic the appearance of a sentence in natural language.
 *
 * The first object class in the declaration may be marked as the point classes default object class by omitting the
 * leading question mark (`?`), like so:
 *
 * ```
 * .character.behaviour.speech what ?to;
 * ```
 *
 * The name of the default object class may be omitted in point declarations, again mimicking natural language as well
 * as shortening the syntax.
 *
 * Each object class may also be appended with a definition. These can be used to either explain the usage of the
 * object class, or to hint to the kinds of entries that should be used with it.
 *
 * To define an object class, append it with a colon (`:`) followed by any of the following in parentheses (`()`):
 *
 * <ul>
 * <li>
 * a string literal (e.g.: `"This is a comment"`)<br/>
 * This acts like a comment without any impact on the parser logic
 * </li>
 * <li>
 * a like hint (e.g.: `l""`)<br/>
 * Meaning a text matching the hint as in an SQLite `LIKE` expression should be used as object.<br/>
 * See [this link](https://www.w3schools.com/sql/sql_like.asp) for further instructions.
 * </li>
 * <li>
 * a glob hint (e.g.: `g""`)<br/>
 * Meaning a text matching the hint as a glob pattern should be used as object.<br/>
 * See [this link](https://man7.org/linux/man-pages/man7/glob.7.html) for further instructions.
 * </li>
 * <li>
 * a regex hint (e.g.: `r"^(yes|no)$"`)<br/>
 * Meaning a text matching the hint as a regular expression should be used as object
 * </li>
 * <li>
 * a match hint (e.g.: `m"yes OR no"`)<br/>
 * Meaning a text matching the hint as aSQLite FTS match filter should be used as object
 * </li>
 * <li>
 * a symbol class reference (e.g.: `.symbol.class[]`)<br/>
 * Meaning a symbol of this class or the class itself should be used as object
 * </li>
 * <li>
 * a recursive symbol class reference (e.g.: `.symbol.class[]...`)<br/>
 * Meaning a symbol of this class or its children or the classes themselves should be used as object
 * </li>
 * <li>
 * a point class reference (e.g.: `.point.class?:`)<br/>
 * Meaning a point of this class should be used as object
 * </li>
 * <li>
 * a recursive point class reference (e.g.: `.point.class?:...`)<br/>
 * Meaning a point of this class or its children should be used as object
 * </li>
 * </ul>
 *
 * Multiple literals and references may be concatenated with the comma (`,`) operator.
 * An object only has to satisfy one of the definitions (comments excluded).
 *
 * Normally, a point can have multiple entries as one of its objects simultaneously, like addressing two characters
 * in `.character.behaviour.speech`. To enforce that only a single entry may be the object, object classes can be to
 * marked as singleton be prepending them with an exclamation mark (`!`), like so:
 *
 * ```
 * (.character[]) ! .character.identity.birthday ! is;
 * ```
 *
 * ## Reference
 *
 * An object class may be referenced by its point classes path followed by a question mark (`?`) and the name of the
 * object class. The same rules for abbreviation as with symbol classes apply.
 *
 * ```
 * .object.optical.color?is
 * object.optical.color?is
 * optical.color?is
 * color?is
 * ```
 */
#ifndef PLANG_OBJECT_CLASS_HPP
#define PLANG_OBJECT_CLASS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/corpus/plot/symbol_class.hpp>
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

#endif//PLANG_OBJECT_CLASS_HPP
