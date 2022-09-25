//
// Created by Johannes on 12.09.2022.
//

#ifndef PLANGCLI_ERROR_HPP
#define PLANGCLI_ERROR_HPP

#include <exception>
#include <plang/base.hpp>

namespace plang::exception {

    class cli_error : public std::runtime_error {
    private:
        std::uint8_t id;

    protected:
        cli_error(std::uint8_t id, std::string const &what_arg) noexcept;

    public:
        cli_error(std::uint8_t id, std::runtime_error const &other) noexcept;

        constexpr std::uint8_t get_id() const noexcept;

        virtual ~cli_error() noexcept;
    };

    class unknown_command_error : public cli_error {
    public:
        unknown_command_error(string_t ref) noexcept;

        virtual ~unknown_command_error() noexcept;
    };

    class malformed_command_error : public cli_error {
    public:
        malformed_command_error(string_t ref) noexcept;

        virtual ~malformed_command_error() noexcept;
    };

    class no_corpus_error : public cli_error {
    public:
        no_corpus_error() noexcept;

        virtual ~no_corpus_error() noexcept;
    };

    class io_error : public cli_error {
    public:
        io_error(string_t ref) noexcept;

        virtual ~io_error() noexcept;
    };

}// namespace plang::exception

#endif//PLANGCLI_ERROR_HPP
