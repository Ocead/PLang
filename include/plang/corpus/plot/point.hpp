//
// Created by Johannes on 16.08.2022.
//

/** \page lang_plot_point Points
 *
 * [TOC]
 *
 * # Point classes
 *
 * Point classes structure the kinds of statements you can make about symbols. The work as tags on or relations between
 * points. For example
 *
 *  + the point classes under `.character.relationship?:...` contain the kinds of relationships character may have
 *  + the point class `.world.geography.leads?:` may be used for building maps by making nets between adjacent places
 *  + the point class `.object.optical.color?:` defines the color of an object
 *
 *  As with symbol classes, each point class has a unique path and always denote the same kind or statement.
 *
 *  ## Declaration
 *
 *  Within declarations, point class dont have an identifying operator. Rather they already mimic the structure of
 *  sentences that can be formed with them. In the default configuration point classes (and later points) are declared
 *  in a subject-verb-object order. Point classes dont have concrete subjects, so the shortest possible point class
 *  declarations like this:
 *
 *  ```
 *  .character.lives _;
 *  ```
 *
 *  Here `.character.lives` is the unique path of the point class and the following `_` is it's default object class.
 *  This point class may be used to form statements about which characters are alive. This statement does not require
 *  additional objects. This syntax, using `_` as the name for a points class obsolete default object class, is
 *  convention used in the standard corpus.
 *
 *  Decorations may be applied to point classes like with symbol classes:
 *
 *  ```
 *  .character.lives (-4, "Denotes that a character lives") _;
 *  ```
 *
 *  To specify which symbols may be the subject of a point of a point class, hints may be applied to the declaration:
 *
 *  ```
 *  (.character[]) .character.lives _;
 *  ```
 *
 *  If a point class is hinted, the parser will favor symbols of matching classes when looking up matching candidates
 *  in point declarations. Thus, the class path of the subject symbol may be shortened or omitted.
 *  Multiple subject hints may be concatenated with the comma (`,`) operator. Recursive hints are denoted with an
 *  ellipsis (`...`).
 *
 *  Point classes may be marked as singletons. This means, that at any time, only one point of this class may exist for
 *  each symbol. This is done by adding an exclamation mark (`!`) before the point classes path:
 *
 *  ```
 *  (.character[]) ! .character.lives _;
 *  ```
 *
 *  Within the story context, declaring a new singleton point will override earlier points instead of adding to them.
 *
 *  A point declaration also includes declarations for a points object classes. Their syntax is explained
 *  [here](@ref lang_plot_object).
 *
 *  ## Reference
 *
 *  Referencing a point class on its own is done by appending the path of the point class with a question mark and a
 *  colon (`?:`). The same rules for abbreviation as with symbol classes apply.
 *
 *  ```
 *  .world.geography.natural.climate.weather?:
 *  world.geography.natural.climate.weather?:
 *  geography.natural.climate.weather?:
 *  natural.climate.weather?:
 *  climate.weather?:
 *  weather?:
 *  ```
 */

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
