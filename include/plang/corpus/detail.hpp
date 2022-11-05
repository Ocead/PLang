//
// Created by Johannes on 16.08.2022.
//

#ifndef PLANG_DETAIL_HPP
#define PLANG_DETAIL_HPP

#include <filesystem>
#include <functional>
#include <utility>
#include <variant>
#include <plang/base.hpp>
#include <plang/lang.hpp>

extern "C" {
struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_context;
struct sqlite3_value;
}

namespace plang {

    /// \brief Sets options for text output
    /// \details Objects of this type may be passed to corpus' <code>print*</code> functions
    /// to include various formatting.
    class format {
    private:
        std::uint16_t value;

        static const decltype(value) OUTPUT_MASK;
        static const decltype(value) ENRICH_MASK;
        static const decltype(value) DETAIL_MASK;
        static const decltype(value) QUALIFICATION_MASK;
        static const decltype(value) INDENT_MASK;

    public:
        static const format PLAIN;

        /// Stylesheet
        struct style {
            /// \brief Colors corresponding to 4-bit ANSI escape color codes
            enum class color {
                BLACK          = 0, ///< \brief 【<span style="color: Black">⬤</span>】 Black color.
                RED            = 1, ///< \brief 【<span style="color: Crimson">⬤</span>】 Red color.
                GREEN          = 2, ///< \brief 【<span style="color: Green">⬤</span>】 Green color.
                YELLOW         = 3, ///< \brief 【<span style="color: Gold">⬤</span>】 Yellow color.
                BLUE           = 4, ///< \brief 【<span style="color: MediumBlue">⬤</span>】 Blue color.
                MAGENTA        = 5, ///< \brief 【<span style="color: DarkMagenta">⬤</span>】 Magenta color.
                CYAN           = 6, ///< \brief 【<span style="color: DarkTurquoise">⬤</span>】 Cyan color.
                LIGHT_GRAY     = 7, ///< \brief 【<span style="color: LightGray">⬤</span>】 Light gray color.
                GRAY           = 8, ///< \brief 【<span style="color: Gray">⬤</span>】 Gray color.
                BRIGHT_RED     = 9, ///< \brief 【<span style="color: Red">⬤</span>】 Bright red color.
                BRIGHT_GREEN   = 10,///< \brief 【<span style="color: LimeGreen">⬤</span>】 Bright green color.
                BRIGHT_YELLOW  = 11,///< \brief 【<span style="color: Yellow">⬤</span>】 Bright yellow color.
                BRIGHT_BLUE    = 12,///< \brief 【<span style="color: Blue">⬤</span>】 Bright blue color.
                BRIGHT_MAGENTA = 13,///< \brief 【<span style="color: Magenta">⬤</span>】 Bright magenta color.
                BRIGHT_CYAN    = 14,///< \brief 【<span style="color: Cyan">⬤</span>】 Bright cyan color.
                WHITE          = 15,///< \brief 【<span style="color: White">⬤</span>】 White color.
                DEFAULT        = 16 ///< \brief Default color for the given context
            };

            /// \brief Font styles available in common backends
            /// \details Styles may be combined using the bitwise OR (<code>|</code>) operator
            /// \see op::operator|(format::style::font_t, format::style::font_t)
            enum class font : std::uint8_t {
                NORMAL        = 0b0,      ///< \brief &nbsp;Normal&nbsp; text.
                BOLD          = 0b1 << 1u,///< \brief &nbsp;<b>Bold</b>&nbsp; text.
                ITALIC        = 0b1 << 2u,///< \brief &nbsp;<i>Italic</i>&nbsp; text.
                UNDERLINE     = 0b1 << 3u,///< \brief &nbsp;<u>Underlined</u>&nbsp; text.
                CODE          = 0b1 << 4u,///< \brief &nbsp;<code>Monospaced</code>&nbsp; text.
                STRIKETHROUGH = 0b1 << 6u,///< \brief &nbsp;<strike>Struck</strike>&nbsp; text.
                INVERTED      = 0b1 << 7u ///< \brief <span style="background-color: var(--page-foreground-color);
                                          /// color: var(--page-background-color);">&nbsp;Inverted&nbsp;</span> text.
            };

            using color_t = std::variant<color, std::uint8_t, char[22]>;///< ANSI 4/8-bit or 24-bit RGB string
            using font_t  = font;                                       ///< \see plang::format::style::font

            color_t text       = color::DEFAULT;///< \brief Text color
            color_t background = color::DEFAULT;///< \brief Background color
            font_t font        = font::NORMAL;  ///< \brief Font format

            /// \brief Returns the style for a PLang operator
            /// \param op Operator to format
            /// \return Style of the operator
            static style const &for_op(lang::op op);
        };

        /// \brief Supported display backends
        enum class output : decltype(value) {
            /// \brief For ANSI terminals
            /// \details Output will contain ANSI escape sequences like <code>\033[1;31m</code>.
            ANSI     = 0b00 << 0,
            /// \brief For use within GNU Readline
            READLINE = 0b01 << 0,
            /// \brief For usage in HTML pages
            /// \details Output will contain ANSI escape sequences like
            /// <code>&lt;span style="color: red;"&gt;&lt;/span&gt;</code>
            HTML     = 0b10 << 0,
            /// \brief Like \ref output::HTML, but with different tags
            /// \details This output mode may override styles
            DOXYGEN  = 0b11 << 0
        };

        /// \brief Types for formatting
        enum class enrich : decltype(value) {
            PLAIN = 0b00 << 2,///< \brief No formatting
            COLOR = 0b01 << 2,///< \brief Only colors
            FONT  = 0b10 << 2,///< \brief Only font style
            RICH  = 0b11 << 2 ///< \brief Full formatting
        };

        /// \brief Detail of the printed objects
        enum class detail : decltype(value) {
            ID_REF       = 0b00 << 4,///< \brief For use as an id reference in PLang
            EXPLICIT_REF = 0b01 << 4,///< \brief For use as an explicit reference in PLang
            DEFINITION   = 0b10 << 4,///< \brief For use as a definition in PLang
            FULL         = 0b11 << 4 ///< \brief For displaying detailed information (not valid PLang)
        };

        /// \brief Qualification of the printed objects
        enum class qualification : decltype(value) {
            UNIQUE = 0b0 << 6,///< \brief Use the shortest unique representation
            FULL   = 0b1 << 6 ///< \brief Use the fully-qualified representation
        };

        /// \brief Rules for multiline printing
        enum class indent : decltype(value) {
            COMPACT  = 0b00 << 7,///< \brief No line breaks or extra spaces
            ONE_LINE = 0b01 << 7,///< \brief One expression per line
            LEFT     = 0b10 << 7,///< \brief Indent following lines of an expression from the left
            CENTER   = 0b11 << 7 ///< \brief Indent following lines of an expression to center
        };

    private:
        /// Returns the opening format string
        /// \return The opening format string
        string_t const &get_open() const;

        /// Returns the text color format string
        /// \param style Stylesheet
        /// \return The text color format string
        string_t get_text_color(style const &style) const;

        /// \brief Returns the background color format string
        /// \param style Stylesheet
        /// \return Returns the background color format string
        string_t get_background_color(style const &style) const;

        /// \brief Returns the opening font format string
        /// \param style Stylesheet
        /// \return The opening font format string
        string_t get_font_begin(style const &style) const;

        /// \brief \brief Returns the closing font format string
        /// \param style Stylesheet
        /// \return The closing font format string
        string_t get_font_end(style const &style) const;

        /// Returns the closing format string
        /// \return The closing format string
        string_t const &get_close() const;

    public:
        /// \brief Constructs a new format
        /// \param output The output mode
        /// \param enrich The enrich mode
        /// \param detail The detail level
        /// \param qualification The qualification level
        /// \param indent The line break mode
        inline constexpr format(output output, enrich enrich, detail detail, qualification qualification, indent indent)
            : value(static_cast<decltype(value)>(output) | static_cast<decltype(value)>(enrich) |
                    static_cast<decltype(value)>(detail) | static_cast<decltype(value)>(qualification) |
                    static_cast<decltype(value)>(indent)) {}

        /// \brief Returns the output mode
        /// \return The output mode
        inline constexpr output get_output() const {
            return static_cast<enum output>(value & OUTPUT_MASK);
        }

        /// \brief Sets the output mode
        /// \param output The new output mode
        inline constexpr void set_output(output output) {
            value = (value & ~OUTPUT_MASK) | static_cast<decltype(value)>(output);
        }

        /// \brief Returns the enrich mode
        /// \return The enrich mode
        inline constexpr enrich get_enrich() const {
            return static_cast<enum enrich>(value & ENRICH_MASK);
        }

        /// \brief Sets the enrich mode
        /// \param enrich The new enrich mode
        inline constexpr void set_enrich(enrich enrich) {
            value = (value & ~ENRICH_MASK) | static_cast<decltype(value)>(enrich);
        }

        /// \brief Returns the detail level
        /// \return The detail level
        inline constexpr detail get_detail() const {
            return static_cast<enum detail>(value & DETAIL_MASK);
        }

        /// \brief Sets the detail level
        /// \param detail The new detail level
        inline constexpr void set_detail(detail detail) {
            value = (value & ~DETAIL_MASK) | static_cast<decltype(value)>(detail);
        }

        /// \brief Returns the qualification level
        /// \return The qualification level
        inline constexpr qualification get_qualification() const {
            return static_cast<enum qualification>(value & QUALIFICATION_MASK);
        }

        /// \brief Sets the qualification level
        /// \param qualification The new qualification level
        inline constexpr void set_qualification(qualification qualification) {
            value = (value & ~QUALIFICATION_MASK) | static_cast<decltype(value)>(qualification);
        }

        // \brief Returns the line break rule
        /// \return The line break rule
        inline constexpr indent get_indent() const {
            return static_cast<enum indent>(value & INDENT_MASK);
        }

        /// \brief Sets the line break rule
        /// \param indent The line break rule
        inline constexpr void set_indent(indent indent) {
            value = (value & ~INDENT_MASK) | static_cast<decltype(value)>(indent);
        }

        template<typename T>
        inline constexpr bool_t operator&(T rhs) const {
            return value & static_cast<decltype(value)>(rhs);
        };

        template<typename T>
        inline constexpr bool_t operator|(T rhs) {
            return value & static_cast<decltype(value)>(rhs);
        };

        /// Applies a style to a string
        /// \param str String to format
        /// \param style Style to apply
        /// \return The formatted string
        string_t operator()(string_t const &str, style const &style) const;

        /// Applies a style to a character
        /// \param chr Character to format
        /// \param style Style to apply
        /// \return The formatted string
        string_t operator()(char_t str, style const &style) const;

        /// \brief Default destructor
        inline ~format() noexcept = default;
    };

    /// \brief The action applied to a persist-able entry
    enum class action {
        NONE,  ///< \brief No action performed
        INSERT,///< \brief Entry was inserted
        UPDATE,///< \brief Entry was updated
        REMOVE,///< \brief Entry was deleted
        FAIL   ///< \brief Action failed
    };

    /// \brief Result type of direct resolve methods
    /// \tparam T Entry type of the resolve method
    template<typename T>
    class resolve_result : public std::tuple<std::optional<T>, std::vector<T>, action> {
    public:
        using Base = std::tuple<std::optional<T>, std::vector<T>, action>;
        using Base::Base;

        inline resolve_result(Base &&fref) noexcept
            : Base(std::forward<Base>(fref)) {}

        inline bool_t has_result() const {
            return std::get<0>(*this).has_value();
        }

        inline T &entry() {
            return std::get<0>(*this).value();
        }

        inline T const &entry() const {
            return std::get<0>(*this).value();
        }

        inline std::vector<T> &candidates() {
            return std::get<1>(*this);
        }

        inline std::vector<T> const &candidates() const {
            return std::get<1>(*this);
        }

        inline enum action action() const {
            return std::get<2>(*this);
        }

        ~resolve_result() = default;
    };

    /// \brief Result type of variant-typed resolve methods
    template<typename T>
    using resolve_ref_result = std::tuple<T &, std::vector<T>, action>;

    namespace detail {

        /// \brief Helper struct for destroying prepared statements
        struct stmt_finalizer {
            /// \brief Destroys a prepared statement
            /// \param stmt Pointer to statement to destroy
            void operator()(sqlite3_stmt *stmt);
        };

        /// \brief Reusable prepared statement
        class stmt : public std::unique_ptr<sqlite3_stmt, stmt_finalizer> {
            using base_type = std::unique_ptr<sqlite3_stmt, stmt_finalizer>;
        };

        /// \brief Wraps a <a href="https://en.cppreference.com/w/cpp/named_req/Callable"><i>Callable</i></a> writing to a stream so it can be used with the `<<` operator
        class stream_helper {
        private:
            std::function<ostream_t &(ostream_t &)> fun;///< \brief Wrapped function

        public:
            /// \brief Wraps a no-op on an output stream
            inline stream_helper()
                : fun([](ostream_t &os) { return std::ref(os); }) {}

            /// \brief Wraps a <a href="https://en.cppreference.com/w/cpp/named_req/Callable"><i>Callable</i></a> writing to a stream
            /// \tparam Args Argument types
            /// \param args Takes the same arguments as the constructor for
            /// <code>std::function&lt;ostream_t &(ostream_t &)&gt;</code>
            template<typename... Args>
            inline stream_helper(Args &&...args)
                : fun(std::forward<Args>(args)...) {}


            /// \brief Calls the wrapped function to write to a stream
            /// \param os Stream to write to
            /// \return The stream
            inline ostream_t &operator()(ostream_t &os) const {
                return fun(os);
            }

            /// \brief Calls the wrapped function to write to a stream
            /// \param os Stream to write to
            /// \return The stream
            inline string_t operator()() const {
                sstream_t ss{};
                fun(ss);
                return ss.str();
            }

            /// \brief Casts the function call to a string
            /// \return The string returned by the function
            inline operator string_t() const {
                sstream_t ss{};
                fun(ss);
                return ss.str();
            }

            /// \brief Default destructor
            ~stream_helper() noexcept = default;
        };

        /// \brief Provides a basic interface to a database
        class corpus {
        private:
            source source;///< \brief The source object all entries created or modified
                          ///< through this object are linked to

            format outer_format;///< \brief Default format used for printing
            format inner_format;///< \brief Default format used for nested printing

            /// \brief Implements regex capabilities for the database
            /// \param ctx Database context
            /// \param argc Number of passed arguments
            /// \param argv Argument values
            /// \details Current implementation is a functional wrapper of `std::regex_match`
            static void regexp(sqlite3_context *ctx, int argc, sqlite3_value **argv);

        protected:
            std::shared_ptr<sqlite3> db;///< \brief Pointer to the database connection

            /// \brief Converts a format for use in nested printing
            /// \param format Format to convert
            /// \return Same format for use in nested printing
            static format _make_inner_format(format format);

            template<template<typename ...> typename V, typename T>
            static string_t vector_to_json(V<T> const& vec) {
                return vector_to_json(vec.cbegin(), vec.cend());
            }

            template<typename I>
            static string_t vector_to_json(I from, I to) {
                using T = typename I::value_type;
                if (from >= to) {
                    return "[]";
                } else {
                    sstream_t ss{};
                    ss << "[";
                    for (auto it = from; it < to; ++it) {
                        if constexpr (std::is_same_v<std::decay_t<T>, string_t>) {
                            ss << std::quoted(*it) << ",";
                        } else {
                            ss << *it << ",";
                        }
                    }
                    ss.seekp(-1, std::ios_base::end);
                    ss << "]";
                    return ss.str();
                }
            }

            /// \brief Default constructor
            corpus();

            /// \brief Opens a database
            /// \param file Path to the database file
            void _open(char const *file);

            int _begin();

            int _rollback();

            int _commit();

            /// \brief Reads SQLite return codes and throws appropriate exceptions
            /// \param code Return code
            /// \return Return code
            int _check(int code) const;

            /// \brief Prepares a SQLite query
            /// \param stmt Target statement
            /// \param query SQL query
            void _prepare(stmt &stmt, string_t const &query) const;

            /// \brief Resets a prepared statement
            /// \param stmt Statement to reset
            void _reset(stmt &stmt) const;

            /// \brief Executes one or multiple SQL statements
            /// \param query SQL string
            /// \param fun Callback function for each result row
            /// \return SQLite return code
            int _exec(string_t const &query, int (*fun)(int, char *[], char *[]) = nullptr);

            /// \brief Prepares and executes a reusable prepared statement
            /// \tparam F Callback function type
            /// \param stmt Prepared statement object
            /// \param query SQL string
            /// \param fun Callback function called between preparation and reset of the prepared statement
            /// \return The result of the callback function
            template<typename F = void (*)()>
            auto _reuse(
                    stmt &stmt,
                    string_t const &query,
                    F fun = []() -> void {}) const -> std::invoke_result_t<decltype(fun)> {
                using R = std::invoke_result_t<decltype(fun)>;
                if (!stmt) {
                    _prepare(stmt, query);
                } else {
                    _reset(stmt);
                }

                if constexpr (std::is_void_v<R>) {
                    fun();
                    _reset(stmt);
                    return;
                } else {
                    R result = fun();
                    _reset(stmt);
                    return result;
                }
            }

            /// \brief Sets the pragmas for the SQL session
            void _set_pragmas();

            /// \brief Creates the database schema on an opened database
            void _create_schema();

            /// \brief Adds undo/redo for the current session
            void _make_undoable();

            /// \brief Creates the current source
            void _create_source();

            /// \brief Returns the current sessions source id
            /// \return The current sessions source id
            pkey<class source> _get_source_id();

            /// \brief Closes a database
            void _close();

        public:
            /// \brief Tag type for dispatching overloaded functions
            /// \tparam T Tagged type
            template<typename T>
            struct tag {
                using type = T;

                template<class R>
                constexpr bool_t operator==(tag<R> const &) const {
                    return std::is_same_v<T, R>;
                }
            };

            /// \brief Undoes the last step on the database
            /// \return Number of remaining undos and name of the undone step
            std::tuple<uint_t, string_t> undo();

            /// \brief Redoes the last step on the database
            /// \return Number of remaining undos and name of the redone step
            std::tuple<uint_t, string_t> redo();

            /// \brief Flushes database changes to disk
            /// \return `true`, if no error occurs
            bool_t flush();

            /// \brief Returns the default format used for printing
            /// \return The default format used for printing
            format const &get_format() const;

            /// \brief Sets the default format used for printing
            /// \param format New default format
            void set_format(format const &format);

            /// \brief Returns the default format used for nested printing
            /// \return The default format used for nested printing
            format const &get_inner_format() const;

            /// \brief Creates a backup of this corpus in another corpus
            /// \param target Corpus to backup into
            /// \return `true`, if the backup was successful
            bool_t backup(corpus &target);

            /// \brief Default destructor
            ~corpus() noexcept;
        };

    }// namespace detail

    /// \brief Contains operator overloads
    /// \details You can enable operator overloads via <code>using namespace plang::op;</code>.
    namespace op {
        /// \brief Checks for overlap between styles
        /// \param lhs First font style
        /// \param rhs Second font style
        /// \return <code>true</code>, if the styles overlap
        inline constexpr bool_t operator&(format::style::font_t lhs, format::style::font_t rhs) {
            return static_cast<std::underlying_type_t<format::style::font_t>>(lhs) &
                   static_cast<std::underlying_type_t<format::style::font_t>>(rhs);
        }

        /// \brief Combines two font styles
        /// \param lhs First font style
        /// \param rhs Second font style
        /// \return The combined font style
        inline constexpr format::style::font_t operator|(format::style::font_t lhs, format::style::font_t rhs) {
            return static_cast<format::style::font_t>(static_cast<std::underlying_type_t<format::style::font_t>>(lhs) |
                                                      static_cast<std::underlying_type_t<format::style::font_t>>(rhs));
        }

        /// \brief Streams out a function call
        /// \param os Stream to write to
        /// \param fun Function to call
        /// \return The stream
        inline ostream_t &operator<<(ostream_t &os, detail::stream_helper const &fun) {
            return fun(os);
        }
    }// namespace op

}// namespace plang

namespace std {

    /// \internal
    /// \brief Specialization of std::tuple_size for mixin resolve results
    /// \tparam T Resolve result type
    template<typename T>
    struct tuple_size<plang::resolve_result<T>> : std::integral_constant<std::size_t, 3> {};

    /// \internal
    /// \brief Specialization of std::tuple_element for mixin resolve result entries
    /// \tparam T Resolve result type
    template<typename T>
    struct tuple_element<0, plang::resolve_result<T>> {
        using type = std::optional<T>;
    };

    /// \internal
    /// \brief Specialization of std::tuple_element for mixin resolve result candidates
    /// \tparam T Resolve result type
    template<typename T>
    struct tuple_element<1, plang::resolve_result<T>> {
        using type = std::vector<T>;
    };

    /// \internal
    /// \brief Specialization of std::tuple_element for mixin resolve result candidates
    /// \tparam T Resolve result type
    template<typename T>
    struct tuple_element<2, plang::resolve_result<T>> {
        using type = plang::action;
    };

    /// \internal
    /// \brief Specialization of std::get for mixin resolve results
    /// \tparam I Tuple index
    /// \tparam T Resolve result type
    /// \param r Resolve result to access
    /// \return The {\ref I}th element
    template<size_t I, typename T>
    std::tuple_element_t<I, plang::resolve_result<T>> &get(plang::resolve_result<T> &r) {
        return std::get<I>(static_cast<typename plang::resolve_result<T>::Base &>(r));
    }

}// namespace std

#endif//PLANG_DETAIL_HPP
