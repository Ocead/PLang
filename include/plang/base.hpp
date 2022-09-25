//
// Created by Johannes on 03.08.2022.
//

#ifndef LIBPLANG_BASE_HPP
#define LIBPLANG_BASE_HPP

#include <chrono>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>

/// \brief Root namespace for <b>PLang</b>
namespace plang {

    /// \brief Contains the basic column types for persistence
    namespace column_types {
        /// \brief Boolean type
        ///
        /// Maps to <code>BOOLEAN</code> in SQLite
        using bool_t   = bool;
        /// \brief Integer type
        ///
        /// Maps to <code>INTEGER</code> in SQLite
        using uint_t   = std::uint32_t;
        /// \brief Integer type
        ///
        /// Maps to <code>INTEGER</code> in SQLite
        using int_t    = std::int32_t;
        /// \brief Long integer type
        ///
        /// Maps to <code>BIGINT</code> in SQLite
        using long_t   = std::int64_t;
        /// \brief Floating point type
        ///
        /// Maps to <code>DOUBLE PRECISION</code> in SQLite
        using float_t  = std::double_t;
        /// \brief Character type
        using char_t = char;
        /// \brief Text type
        ///
        /// Maps to <code>Text</code> in SQLite
        using string_t = std::basic_string<char_t>;

        using string_view_t = std::string_view;

        using istream_t = std::basic_istream<string_t::value_type>;

        using ostream_t = std::basic_ostream<string_t::value_type>;

        using sstream_t = std::basic_stringstream<string_t::value_type>;

    }// namespace column_types

    using namespace column_types;

    /// \brief Contains structures not to be used directly
    namespace detail {
        /// \brief Wrapper type for persisted non-fundamental class members
        /// \tparam TableTp Class of the class member
        /// \tparam ColTp Type of the class member
        /// \tparam I Optional index
        template<typename TableTp, typename ColTp, int I = 0>
        class cls_col : public ColTp {
        public:
            /// \brief Constructs wrapped type with any pack of parameters legal for constructing the wrapped type
            /// \tparam Args Parameter pack passed to the wrapped type's constructor
            /// \param args Parameters passed to the wrapped type's constructor
            template<typename... Args, typename = decltype(ColTp(std::declval<Args>()...))>
            constexpr inline cls_col(Args &&...args)
                : ColTp(std::forward<Args>(args)...) {}

            /// \brief Implicit cast to non-const reference to wrapped type
            constexpr inline operator ColTp &() noexcept {
                return cls_col::ColTp;
            }

            /// \brief Implicit cast to const reference to wrapped type
            constexpr inline operator ColTp const &() const noexcept {
                return cls_col::ColTp;
            }

            /// \brief Default destructor
            inline ~cls_col() = default;
        };

        /// \brief Wrapper type for persisted fundamental class members
        /// \tparam TableTp Class of the class member
        /// \tparam ColTp Type of the class member
        /// \tparam I Optional index
        template<typename TableTp, typename ColTp, int I = 0>
        class fun_col {
        private:
            ColTp value;///< \brief Value of wrapped type

        public:
            /// \brief Default constructor
            constexpr inline fun_col() noexcept
                : value() {}

            /// \brief Explicit constructor
            /// \param val Initialization value
            constexpr inline fun_col(ColTp &&val) noexcept
                : value(val) {}

            /// \brief Copy assignment
            /// \param ref Source object
            constexpr inline fun_col operator=(ColTp const &ref) noexcept {
                value = ref;
                return *this;
            }

            /// \brief Move assignment
            /// \param fref Source object
            constexpr inline fun_col operator=(ColTp &&fref) noexcept {
                value = std::move(fref);
                return *this;
            }

            /// \brief Implicit cast to non-const reference to wrapped type
            constexpr inline operator ColTp &() noexcept {
                return value;
            }

            /// \brief Implicit cast to const reference to wrapped type
            constexpr inline operator ColTp const &() const noexcept {
                return value;
            }

            /// \brief Default destructor
            inline ~fun_col() = default;
        };

        /// \brief Wrapper type for persisted class members
        ///
        /// Organizes type parameters for object-relational mapping and implicitly converts to the wrapped type
        template<typename TableTp, typename ColTp, int I = 0>
        using col = typename std::conditional_t<std::is_fundamental_v<ColTp> || std::is_enum_v<ColTp>,
                                                detail::fun_col<TableTp, ColTp, I>,
                                                detail::cls_col<TableTp, ColTp, I>>;

    }// namespace detail

    using detail::col;

    namespace detail {
        class path_manager;
    }

    namespace root {
        class source;
    }

    namespace detail {

        class corpus;

        /// \brief Contains abstract classes contained in multiple tables
        namespace base_types {

            class persisted;

            using pkey_t = int_t;

            /// \brief Base type for persisted classes
            class persisted {
            protected:
                pkey_t id;///< \brief Primary key for all persisted objects

                /// \brief Default constructor
                ///
                /// Sets the primary key to a value invalid for persistence
                constexpr persisted() noexcept;

            public:
                /// \brief Returns the primary key
                /// \return The primary key
                pkey_t get_id() const noexcept;

                /// \brief Returns <code>true</code>, if the object is persisted
                /// \return <code>true</code>,if the object is persisted
                bool_t is_persisted() const noexcept;

                /// \brief Default destructor
                ~persisted() noexcept;

                friend class detail::corpus;
            };

            /// \brief Base type for traceable classes
            class sourced {
            protected:
                std::optional<col<sourced, int_t>> source_id;///< \brief Id of the source of the object


                /// \brief Default constructor
                ///
                /// Sets the source id to a value invalid for persistence
                constexpr sourced() noexcept;

                /// \brief Constructs this object with a source
                /// \param source Source of this object
                explicit sourced(root::source const &source) noexcept;

            public:
                /// \brief Returns the source id
                /// \return The source id

                std::optional<int_t> get_source_id() const noexcept;

                /// \brief Sets the source of the object
                /// \param source Source of this object
                void set_source_id(root::source const &source) noexcept;

                /// \brief Default destructor
                ~sourced() noexcept;
            };

            /// \brief Base type for decorated classes
            class decorated {
            protected:
                col<decorated, std::optional<float_t>> ordinal;     ///< \brief Number to sort the object by in lists
                col<decorated, std::optional<string_t>> description;///< \brief A textual description of the object


                /// \brief Default constructor
                decorated() noexcept;

                /// \brief Explicit constructor
                ///
                /// \param ordinal The ordinal of the object
                /// \param description The description of the object
                decorated(std::optional<float_t> ordinal, std::optional<string_t> description) noexcept;

            public:
                /// \brief Returns the ordinal of the object
                /// \return The ordinal of the object
                const std::optional<float_t> &get_ordinal() const noexcept;

                /// \brief Sets or unsets the ordinal of the object
                /// \param ordinal New ordinal of the object
                void set_ordinal(const std::optional<float_t> &ordinal = std::nullopt) noexcept;

                /// \brief Returns the description of the object
                /// \return The description of the object
                const std::optional<string_t> &get_description() const noexcept;

                /// \brief Sets or unsets the description of the object
                /// \param description New description of the object
                void set_description(const std::optional<string_t> &description = std::nullopt) noexcept;

                /// \brief Default destructor
                ~decorated() noexcept;
            };

        }// namespace base_types

    }// namespace detail

    using namespace detail::base_types;

    /// \brief Contains the data model's top-level classes
    namespace root {
        /// \brief A source of data inside a corpus
        /// \details The corpus holds entries of this class to track the changes and their origin to it.
        /// There are no interfaces to directly interact with this class' table.
        /// Instead the \ref plang::corpus  manages it automatically.
        class source : public persisted {
        public:
            using clock_t      = std::chrono::system_clock;///< \brief Clock type used for generating timestamps
            using time_point_t = clock_t::time_point;      ///< \brief Timestamp type

        protected:
            col<source, std::optional<string_t>> name;   ///< \brief Name of the source
            col<source, std::optional<string_t>> version;///< \brief Version of the source
            col<source, std::optional<string_t>> url;    ///< \brief URL to the source
            col<source, time_point_t> start;             ///< \brief Start time of reading
            col<source, time_point_t> end;               ///< \brief End time of reading

        public:
            /// \brief Constructs a new unpersisted source
            /// \param name Name of the source file
            /// \param version Version of the source
            /// \param url URL to the source file
            /// \param start Start time of reading
            source(std::optional<string_t> &&name,
                   std::optional<string_t> &&version,
                   std::optional<string_t> &&url,
                   time_point_t start = clock_t::now());

            /// \brief Returns the name of the source
            /// \return The name of the source
            std::optional<string_t> const &get_name() const noexcept;

            /// \brief Returns the version of the source
            /// \return The version of the source
            std::optional<string_t> const &get_version() const noexcept;

            /// \brief Returns the URL of the source
            /// \return The URL of the source
            std::optional<string_t> const &get_url() const noexcept;

            /// \brief Returns the start time of reading the source
            /// \return The start time of reading the source
            root::source::time_point_t get_start() const noexcept;

            /// \brief Returns the end time of reading the source
            /// \return The end time of reading the source
            root::source::time_point_t get_an_end() const noexcept;

            /// \brief Default destructor
            ~source() noexcept;
        };

        /// \brief A node in the corpus' path tree
        class path : public persisted, public sourced, public decorated {
        private:
            /// \brief Constructor for internal use
            path();

        protected:
            col<path, string_t> name;  ///< \brief The name of the node
            col<path, int_t> parent_id;///< \brief The id of the parent node

        public:
            /// \brief Constructs an unpersisted path as child of another
            /// \param name Name of the node
            /// \param parent Parent of the node
            path(string_t name, path const &parent);

            /// \brief Returns the name of the node
            /// \return The name of the node
            const string_t &get_name() const;

            /// \brief Sets the name of the node
            /// \param name New name of the node
            /// \return <code>true</code>, if the name is valid
            bool set_name(const string_t &name);

            /// \brief Returns the parent id of the node
            /// \return The parent id of the node
            int_t get_parent_id() const;

            /// \brief Sets the parent of the node
            /// \param parent The new parent of the node
            void set_parent(path const &parent);

            /// \brief Returns <code>true</code>, if the node is the root node
            /// \return <code>true</code>, if the node is the root node
            bool_t is_root() const;

            /// \brief Default destructor
            ~path() noexcept;

            friend class plang::detail::path_manager;
        };

    }// namespace root

    using namespace root;

}// namespace plang

/// \brief Contains specializations of standard library templates
namespace std {

    /// \internal
    /// \brief Specialization of std::hash for fundamental column types
    /// \tparam TableTp Table type
    /// \tparam ColTp Column type
    /// \tparam I Column index
    template<typename TableTp, typename ColTp, int I>
    struct hash<plang::detail::fun_col<TableTp, ColTp, I>> {
        /// \brief Computes the hash for a table columns
        /// \param ref Table cell
        /// \return Calculated hash value
        constexpr std::size_t operator()(plang::detail::fun_col<TableTp, ColTp, I> const &ref) const noexcept {
            return std::hash<ColTp>()(ref);
        }
    };

    /// \internal
    /// \brief Specialization of std::hash for non-fundamental column types
    /// \tparam TableTp Table type
    /// \tparam ColTp Column type
    /// \tparam I Column index
    template<typename TableTp, typename ColTp, int I>
    struct hash<plang::detail::cls_col<TableTp, ColTp, I>> : public hash<ColTp> {
        /// \brief Computes the hash for a table columns
        /// \param ref Table cell
        /// \return Calculated hash value
        constexpr std::size_t operator()(plang::detail::cls_col<TableTp, ColTp, I> const &ref) const noexcept {
            return std::hash<ColTp>()(ref);
        }
    };

}// namespace std

#endif//LIBPLANG_BASE_HPP
