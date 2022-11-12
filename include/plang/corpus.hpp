//
// Created by Johannes on 17.08.2022.
//

#ifndef PLANG_CORPUS_HPP
#define PLANG_CORPUS_HPP

#include <any>
#include <chrono>
#include <filesystem>
#include <istream>
#include <variant>
#include <vector>
#include <plang/base.hpp>
#include <plang/corpus/detail.hpp>
#include <plang/corpus/path.hpp>
#include <plang/corpus/plot.hpp>
#include <plang/plot.hpp>

namespace std {
    namespace {

        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     http://stackoverflow.com/questions/4948780
        template<class T>
        inline constexpr void hash_combine(std::size_t &seed, T const &v) {
            seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // Recursive template code derived from Matthieu M.
        template<class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct hash_value_impl {
            static constexpr void apply(size_t &seed, Tuple const &tuple) {
                hash_value_impl<Tuple, Index - 1>::apply(seed, tuple);
                hash_combine(seed, std::get<Index>(tuple));
            }
        };

        template<class Tuple>
        struct hash_value_impl<Tuple, 0> {
            static constexpr void apply(size_t &seed, Tuple const &tuple) {
                hash_combine(seed, std::get<0>(tuple));
            }
        };
    }// namespace

    template<typename T>
    struct hash<plang::detail::corpus::tag<T>> {
        constexpr size_t operator()(plang::detail::corpus::tag<T> const &) const {
            return typeid(T).hash_code();
        }
    };

    template<typename... Tps>
    struct hash<std::tuple<Tps...>> {
        constexpr size_t operator()(std::tuple<Tps...> const &tt) const {
            size_t seed = 0;
            hash_value_impl<std::tuple<Tps...>>::apply(seed, tt);
            return seed;
        }
    };
}// namespace std

namespace plang {

    namespace detail {

        /// \brief Defines a new template by applying a template to its type parameters
        /// \tparam T Type to transform
        template<typename T>
        struct template_transform;

        /// \brief Defines a new template by applying a template to its type parameters
        /// \tparam T Template type to transform
        /// \tparam Args Template type parameters
        template<template<typename...> typename T, typename... Args>
        struct template_transform<T<Args...>> {

            /// \brief Applies a template to a classes' type parameters
            /// \tparam M Template to apply
            template<template<typename> typename M>
            using type = T<M<Args>...>;
        };

        /// \brief Common derived class template for distributed overloading
        /// \tparam Tps Distributed overloading types
        /// \extends plang::detail::path_manager
        /// \extends plang::detail::symbol_class_manager
        /// \extends plang::detail::symbol_manager
        /// \extends plang::detail::point_class_manager
        /// \extends plang::detail::object_class_manager
        template<typename... Tps>
        struct mixin_base : virtual public Tps... {
        public:
            using Tps::fetch...;
            using Tps::fetch_n...;
            using Tps::fetch_all...;
            using Tps::insert...;
            using Tps::resolve...;
            using Tps::update...;
            using Tps::print...;
            using Tps::remove...;
        };

        /// \brief Base class for the corpus
        struct corpus_mixins : virtual public mixin_base<plang::detail::path_manager,
                                                         plang::detail::symbol_class_manager,
                                                         plang::detail::symbol_manager,
                                                         plang::detail::point_class_manager,
                                                         plang::detail::object_class_manager> {};

    }// namespace detail

    /// \brief Variant of all persist-able classes
    using entry_t = std::variant<std::monostate,
                                 plang::source,
                                 plang::path,
                                 plang::plot::symbol::clazz,
                                 plang::plot::symbol::clazz::hint,
                                 plang::plot::symbol,
                                 plang::plot::symbol::compound,
                                 plang::plot::point::clazz,
                                 plang::plot::point::clazz::hint,
                                 plang::plot::point::subject::sym,
                                 plang::plot::point::subject::cls,
                                 plang::plot::point,
                                 plang::plot::object::clazz,
                                 plang::plot::object::clazz::hint::lit,
                                 plang::plot::object::clazz::hint::sym,
                                 plang::plot::object::clazz::hint::pnt,
                                 plang::plot::object::lit,
                                 plang::plot::object::sym,
                                 plang::plot::object::cls,
                                 plang::plot::object::pnt>;

    /// \brief Any single persist-able entry
    /// \sa plang::entry_t
    using entry = entry_t;

    /// \brief Variant of references to persist-able entries
    using entry_ref_t = typename detail::template_transform<entry_t>::template type<std::reference_wrapper>;

    /// \brief Reference to any persist-able entry
    /// \sa plang::entry_ref_t
    using entry_ref = entry_ref_t;

    /// \brief Variant of tags for persist-able types
    using entry_type_t = typename detail::template_transform<entry_t>::template type<detail::corpus::tag>;

    /// \brief Any persist-able type
    /// \sa plang::entry_type_t
    using entry_type = entry_type_t;


    /// \brief Result type of variant-typed resolve methods
    class resolve_entry_result : public std::tuple<entry, std::vector<entry>, action> {
    public:
        using Base = std::tuple<entry, std::vector<entry>, action>;
        using Base::Base;

        template<typename T>
        resolve_entry_result(resolve_result<T> &&result)
            : Base(result.has_result() ? entry(result.entry()) : entry(), {}, result.action()) {
            auto &can = candidates();
            can.reserve(result.candidates().size());
            std::transform(result.candidates().begin(),
                           result.candidates().end(),
                           std::make_move_iterator(can.begin()),
                           [](T &&r) -> plang::entry { return std::forward<T>(r); });
        }

        inline bool_t has_result() const {
            return std::get<0>(*this).index() != 0;
        }

        inline plang::entry &entry() {
            return std::get<0>(*this);
        }

        inline plang::entry const &entry() const {
            return std::get<0>(*this);
        }

        inline std::vector<plang::entry> &candidates() {
            return std::get<1>(*this);
        }

        inline std::vector<plang::entry> const &candidates() const {
            return std::get<1>(*this);
        }

        inline action action() const {
            return std::get<2>(*this);
        }

        ~resolve_entry_result() = default;
    };

    /// \brief Result type of variant-typed resolve methods
    using resolve_entry_ref_result = std::tuple<entry &, std::vector<entry>, action>;

    /// \brief The main interface for interacting with a corpus
    /// \details This class contains multiple mixins for various parts of the corpus named like
    /// <code>&lt;PART&gt;_manager</code>.<br/>
    /// See the <b>"Public Member Functions inherited from plang::detail::<PART>_manager"</b>
    /// sections of the documentation for the methods each mixin provides.
    /// \note Check the friends of a class to see which mixin in responsible for handling it
    class corpus final : virtual public detail::corpus, virtual public detail::corpus_mixins {
    public:
        /// \brief Provides a report of the effective operations from parsing PLang code
        class report {
        public:
            using key = std::tuple<entry_type, pkey<void>>;

        private:
            using clock_t = std::chrono::high_resolution_clock;

            clock_t::time_point _start;
            clock_t::time_point _post_lex;
            clock_t::time_point _post_parse;
            clock_t::time_point _post_visit;
            clock_t::time_point _post_commit;

            std::unordered_map<key, entry> _mentioned;                    ///< \brief Mentioned entries
            std::unordered_map<key, entry> _inserted;                     ///< \brief Inserted entries
            std::unordered_map<key, entry> _updated;                      ///< \brief Updated entries
            std::unordered_map<key, string_t> _removed;                   ///< \brief Removed entries
            std::unordered_multimap<string_t, std::vector<entry>> _failed;///< \brief Failed entries

            std::optional<path> _scope;

            static key get_key(entry const &entry);

            static bool_t valid(key const &key);

        public:
            /// \brief Default constructor
            report();

            /// \brief Report an entry as mentioned
            /// \param entry Mentioned entry
            /// \details Calling this function with an entry already contained in \ref _inserted or \ref _updated
            /// is a no-op.
            void mention(entry &&entry);

            /// \brief Report an entry as inserted
            /// \param entry Inserted entry
            /// \details Calling this function with an entry already contained in \ref _updated
            /// is a no-op.
            void insert(entry &&entry);

            /// \brief Report an entry as updated
            /// \param entry Updated entry
            /// \details Calling this function with an entry already contained in \ref _inserted
            /// is a no-op.
            void update(entry &&entry);

            /// \brief Report an entry as deleted
            /// \param key Key of the deleted entry
            /// \param repr Representation of the deleted entry
            void remove(key &&key, string_t &&repr);

            /// \brief Report an entry as failed
            /// \param repr Representation of the entry
            /// \param candidates Candidates for the entry
            void fail(string_t &&repr, std::vector<entry> &&candidates);

            decltype(_start) const &start() const;

            decltype(_post_lex) const &post_lex() const;

            decltype(_post_parse) const &post_parse() const;

            decltype(_post_visit) const &post_visit() const;

            decltype(_post_commit) const &post_commit() const;

            /// \brief Returns the mentioned entries
            /// \return The mentioned entries
            decltype(_mentioned) const &mentioned() const;

            /// \brief Returns the inserted entries
            /// \return The inserted entries
            decltype(_inserted) const &inserted() const;

            /// \brief Returns the updated entries
            /// \return The updated entries
            decltype(_updated) const &updated() const;

            /// \brief Returns the removed entries
            /// \return The removed entries
            decltype(_removed) const &removed() const;

            /// \brief Returns the failed entries
            /// \return The failed entries
            decltype(_failed) const &failed() const;

            decltype(_scope) &scope();

            /// \brief Returns the combined size of mentioned, inserted, updated, removed and failed entries
            /// \return The combined size of mentioned, inserted, updated, removed and failed entries
            std::size_t size() const;

            /// \brief Returns the combined size of inserted, update and removed entries
            /// \return The combined size of inserted, update and removed entries
            std::size_t diff_size() const;

            /// \brief Default destructor
            ~report() noexcept;

            friend class corpus;
        };

        /// \brief Constructs a corpus in memory
        corpus();

        /// \brief Constructs a corpus from a PLang database file
        /// \param [in] file Path to the database file
        corpus(std::filesystem::path const &file);

        /// \brief Parses PLang code from a stream
        /// \param [in] stream Stream to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return A report of the executed declarations
        report execute(istream_t &stream, path const &scope);

        /// \brief Parses PLang code from a string
        /// \param [in] expr String to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return A report of the executed declarations
        report execute(string_t const &expr, path const &scope);

        /// \brief Parses a single declaration from a stream
        /// \param [in] stream Stream to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return A report of the executed declarations
        report decl(istream_t &stream, path const &scope);

        /// \brief Parses a single declaration from a string
        /// \param [in] expr String to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return A report of the executed declarations
        report decl(string_t const &expr, path const &scope);

        /// \brief Parses references from a stream
        /// \param [in] stream Stream to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return The result of the appropriate resolve method
        resolve_entry_result ref(istream_t &stream, path const &scope) const;

        /// \brief Parses references from a string
        /// \param [in] expr String to parse from
        /// \param [in] scope Scope to resolve declaration in
        /// \return The result of the appropriate resolve method
        resolve_entry_result ref(string_t const &expr, path const &scope) const;

        /// \brief Returns a persisted entry
        /// \tparam T Type to fetch
        /// \param [in] id Id of the entry
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The persisted entry, if it exists
        template<typename T, typename = std::enable_if_t<std::is_base_of_v<plang::persisted<T>, T>>>
        std::optional<T> fetch(pkey<T> id, bool_t dynamic = false) const {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::fetch(id, dynamic, detail::corpus::tag<T>());
        }

        /// \brief Returns a persisted entry
        /// \param [in] type Id of the type
        /// \param [in] id Id of the entry
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The persisted entry, if it exists
        entry fetch(entry_type type, pkey<void> id, bool_t dynamic = false) const;

        /// \brief Returns multiple persisted entries
        /// \tparam T Type to fetch
        /// \param [in] ids Ids of the entries
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The persisted entries, if existent
        template<typename T, typename = std::enable_if_t<std::is_base_of_v<plang::persisted<T>, T>>>
        std::vector<T> fetch_n(std::vector<pkey<T>> const &ids, bool_t dynamic = false) const {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins(fetch_n(ids, dynamic, detail::corpus::tag<T>()));
        }

        /// \brief Returns multiple persisted entries
        /// \param type Type to fetch
        /// \param ids Ids of the entries
        /// \param texts `true`, if texts should also be returned
        /// \return The persisted entries, if existent
        std::vector<entry> fetch_n(entry_type type, std::vector<pkey<void>> const &ids, bool_t dynamic = false) const;

        /// \brief Returns all persisted entries of a type
        /// \tparam T Type to fetch
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \param [in] limit Limits the output to up to <code>\p limit</code> entries
        /// \param [in] offset Skips the first <code>\p offset</code> entries
        /// \return The list of persisted entries
        template<typename T>
        std::vector<T> fetch_all(bool_t dynamic = false, int_t limit = -1, int_t offset = 0) const {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::fetch_all(dynamic, limit, offset, detail::corpus::tag<T>());
        }

        /// \brief Returns all persisted entries of a type
        /// \param [in] type ID of the type to fetch
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \param [in] limit Limits the output to up to \p limit entries
        /// \param [in] offset Skips the first \p offset entries
        /// \return The list of persisted entries
        std::vector<entry> fetch_all(entry_type type, bool_t dynamic = false, int_t limit = -1, int_t offset = 0) const;

        /// \brief Resolves an entry for a given context
        /// \tparam T Type of the entry to resolve
        /// \tparam C Type of the context to resolve in
        /// \param [in] path Path to the entry
        /// \param [in] context Context for resolving the entry
        /// \param [in] insert `true`, if the entry should be created if non-existent
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        template<typename T, typename C>
        inline resolve_result<T> resolve(std::vector<string_t> const &path,
                                         C const &context = std::monostate(),
                                         bool_t insert    = false,
                                         bool_t dynamic   = false) {
            return corpus_mixins::resolve(path, context, insert, dynamic, corpus::tag<T>());
        }

        /// \brief Resolves an unknown entry for a given context
        /// \param [in] path Path to the entry
        /// \param [in] type Type of the entry
        /// \param [in] context Context to resolve in
        /// \param [in] insert `true`, if the entry should be created if non-existent
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        resolve_entry_result resolve(std::vector<string_t> const &path,
                                     entry_type type,
                                     entry const &context = entry(),
                                     bool_t insert        = false,
                                     bool_t dynamic       = false);

        /// \brief Resolves an entry for a given context
        /// \tparam T Type of the entry to resolve
        /// \tparam C Type of the context to resolve in
        /// \param [in] path Path to the entry
        /// \param [in] context Context for resolving the entry
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        template<typename T, typename C>
        inline resolve_result<T> resolve(std::vector<string_t> const &path,
                                         C const &context = std::monostate(),
                                         bool_t dynamic   = false) const {
            return corpus_mixins::resolve(path, context, dynamic, corpus::tag<T>());
        }

        /// \brief Resolves an unknown entry for a given context without inserts
        /// \param [in] path Path to the entry
        /// \param [in] type Type of the entry
        /// \param [in] context Context to resolve in
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        resolve_entry_result resolve(std::vector<string_t> const &path,
                                     entry_type type,
                                     entry const &context = entry(),
                                     bool_t dynamic       = false) const;

        /// \brief Resolves an entry for a given context
        /// \param [in] path Path to the entry
        /// \param [in,out] entry Entry to resolve.
        /// If the resolution succeeds, \p entry is updated to represent the resolved entry.
        /// \param [in] context Context to resolve in
        /// \param [in] insert `true`, if the entry should be created if non-existent
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        resolve_entry_ref_result resolve(std::vector<string_t> const &path,
                                         entry &entry,
                                         plang::entry const &context = plang::entry(),
                                         bool_t insert               = false,
                                         bool_t dynamic              = false);

        /// \brief Resolves an entry for a given context without inserts
        /// \param [in] path Path to the entry
        /// \param [in,out] entry Entry to resolve.
        /// If the resolution succeeds, \p entry is updated to represent the resolved entry.
        /// \param [in] context Context to resolve in
        /// \param [in] dynamic `true`, if texts should also be returned
        /// \return The resolved entry and other possible candidates
        resolve_entry_ref_result resolve(std::vector<string_t> const &path,
                                         plang::entry &entry,
                                         plang::entry const &context = plang::entry(),
                                         bool_t dynamic              = false) const;

        /// \brief Inserts a persist-able entry into the corpus
        /// \tparam T Type of the entry
        /// \param [in,out] t Object to be inserted
        /// \param [in] replace `true`, if existent entries should be overridden
        /// \return The effective action
        template<typename T>
        action insert(T &t, bool_t replace = false) {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::insert(static_cast<T &>(t), replace, detail::corpus::tag<T>());
        };

        /// \brief Inserts a persist-able entry into the corpus
        /// \param [in,out] entry Object to be inserted
        /// \param [in] replace `true`, if existent entries should be overridden
        /// \return The effective action
        /// \details Calling this function with an empty entry is a no-op.
        action insert(entry &entry, bool_t replace = false);

        /// \brief Updates a persist-able entry in the corpus
        /// \tparam T Type of the entry
        /// \param [in,out] t Object to update
        /// \param [in] dynamic `true`, if texts should also be updated
        /// \return The effective action
        template<typename T>
        action update(T &t, bool_t dynamic = false) {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::update(t, dynamic, detail::corpus::tag<T>());
        };

        /// \brief Updates a persist-able entry in the corpus
        /// \param [in,out] entry Object to update
        /// \param [in] dynamic `true`, if texts should also be updated
        /// \return The effective action
        action update(entry &entry, bool_t dynamic = false);

        /// \brief Prints a persisted entry with default format
        /// \tparam T Type of the entry
        /// \param [in] t Object to print
        /// \return The representation of the entry
        template<typename T>
        inline detail::stream_helper print(T const &t) const {
            return print<T>(t, get_format());
        };

        /// \brief Prints a persisted entry
        /// \tparam T Type of the entry
        /// \param [in] t Object to print
        /// \param [in] format Format options for printing
        /// \return The representation of the entry
        template<typename T>
        detail::stream_helper print(T const &t, format format) const {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::print(t.get_id(), format, detail::corpus::tag<T>());
        };

        /// \brief Prints a persisted entry with default format
        /// \param [in] entry Object to print
        /// \return The representation of the entry
        /// \details Calling this function with an empty entry returns an empty string.
        inline detail::stream_helper print(entry const &entry) const {
            return print(entry, get_format());
        }

        /// \brief Prints a persisted entry
        /// \param [in] entry Object to print
        /// \param [in] format Format options for printing
        /// \return The representation of the entry
        /// \details Calling this function with an empty entry returns an empty string.
        detail::stream_helper print(entry const &entry, format format) const;


        /// \brief Removes a persisted entry from the corpus
        /// \tparam T Type of the entry
        /// \param [in,out] t Object to be removed
        /// \param [in] cascade <code>true</code>, if dependant entries should be removed to
        /// \return The representation of the removed entry
        template<typename T>
        std::tuple<string_t, action> remove(T &t, bool_t cascade = false) {
            static_assert(std::is_base_of_v<plang::persisted<T>, T>);
            return detail::corpus_mixins::remove(t, cascade, detail::corpus::tag<T>());
        };

        /// \brief Removes a persisted entry from the corpus
        /// \param [in,out] entry Object to be removed
        /// \param [in] cascade <code>true</code>, if dependant entries should be removed to
        /// \return The representation of the removed entry
        /// \details Calling this function with an empty entry returns an empty string.
        std::tuple<string_t, action> remove(entry &entry, bool_t cascade = false);

        using detail::corpus::get_format;
        using detail::corpus::get_inner_format;
        using detail::corpus::set_format;

        /// \brief Creates a backup of this corpus in another corpus
        /// \param target Corpus to backup into
        /// \return `true`, if the backup was successful
        bool_t backup(corpus &target);

        /// \brief Default destructor
        /// \details Destroying a corpus closes any open connections with underlying databases.
        ~corpus() noexcept;
    };

}// namespace plang

namespace std {

    /// \internal
    /// \brief Specialization of std::tuple_size for resolve results
    /// \tparam T Resolve result type
    template<>
    struct tuple_size<plang::resolve_entry_result> : std::integral_constant<std::size_t, 3> {};

    /// \internal
    /// \brief Specialization of std::tuple_element for resolve result entries
    /// \tparam T Resolve result type
    template<>
    struct tuple_element<0, plang::resolve_entry_result> {
        using type = plang::entry;
    };

    /// \internal
    /// \brief Specialization of std::tuple_element for resolve result candidates
    /// \tparam T Resolve result type
    template<>
    struct tuple_element<1, plang::resolve_entry_result> {
        using type = std::vector<plang::entry>;
    };

    /// \internal
    /// \brief Specialization of std::tuple_element for resolve result actions
    /// \tparam T Effective action
    template<>
    struct tuple_element<2, plang::resolve_entry_result> {
        using type = plang::action;
    };

    /// \internal
    /// \brief Specialization of std::get for resolve results
    /// \tparam I Tuple index
    /// \param r Resolve result to access
    /// \return The {\ref I}th element
    template<size_t I, typename T>
    std::tuple_element_t<I, plang::resolve_entry_result> &get(plang::resolve_entry_result &r) {
        return std::get<I>(static_cast<typename plang::resolve_entry_result::Base &>(r));
    }

    //template<>
    //struct hash<plang::entry_type> : public hash<plang::entry_type_t> {};

}// namespace std

#endif//PLANG_CORPUS_HPP
