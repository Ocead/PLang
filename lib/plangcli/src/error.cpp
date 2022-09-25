//
// Created by Johannes on 12.09.2022.
//

#include <plangcli/error.hpp>

using namespace plang::exception;


// region cli_error

cli_error::cli_error(std::uint8_t id, std::string const &what_arg) noexcept
    : std::runtime_error(what_arg),
      id(id) {}

cli_error::cli_error(std::uint8_t id, std::runtime_error const &other) noexcept
    : std::runtime_error(other),
      id(id) {}

constexpr std::uint8_t cli_error::get_id() const noexcept {
    return id;
}

cli_error::~cli_error() noexcept = default;


// endregion cli_error

// region no_corpus_error

no_corpus_error::no_corpus_error() noexcept
    : cli_error(0, "No corpus opened") {}

no_corpus_error::~no_corpus_error() noexcept = default;

// endregion no_corpus_error
