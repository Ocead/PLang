//
// Created by Johannes on 15.08.2022.
//

#ifndef LIBPLANG_ERROR_HPP
#define LIBPLANG_ERROR_HPP

#include <cstdint>
#include <plang/base.hpp>
#include <stdexcept>

/// \brief Contains exceptions
namespace plang::exception {

    class lang_error : public std::runtime_error {
    private:
        std::uint8_t id;

    protected:
        lang_error(std::uint8_t id, std::string const &what_arg) noexcept;

    public:
        lang_error(std::uint8_t id, std::runtime_error const &other) noexcept;

        constexpr std::uint8_t get_id() const noexcept;

        virtual ~lang_error() noexcept;
    };

    class malformed_expression_error : public lang_error {};

    class invalid_reference_error : public lang_error {
    public:
        invalid_reference_error(string_t ref) noexcept;

        virtual ~invalid_reference_error() noexcept;
    };

    class ambiguous_reference_error : public lang_error {
    public:
        ambiguous_reference_error(string_t ref) noexcept;

        virtual ~ambiguous_reference_error() noexcept;
    };

    class redefinition_error : public lang_error {};

    class hint_match_error : public lang_error {};

    class duplicate_path_error : public lang_error {};

    class duplicate_symbol_error : public lang_error {};

    class duplicate_object_class_error : public lang_error {};

}// namespace plang::exception

#endif//LIBPLANG_ERROR_HPP
