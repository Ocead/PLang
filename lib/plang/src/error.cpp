//
// Created by Johannes on 15.08.2022.
//

#include <plang/error.hpp>

using namespace plang::exception;

// region lang_error

lang_error::lang_error(std::uint8_t id, std::string const &what_arg) noexcept
    : std::runtime_error(what_arg),
      id(id) {}

lang_error::lang_error(std::uint8_t id, std::runtime_error const &other) noexcept
    : std::runtime_error(other),
      id(id) {}

constexpr std::uint8_t lang_error::get_id() const noexcept {
    return id;
}

lang_error::~lang_error() noexcept = default;

// endregion lang_error

// region malformed_expression_error

// endregion malformed_expression_error

// region invalid_reference_error

invalid_reference_error::invalid_reference_error(column_types::string_t ref) noexcept
    : lang_error(2, "Cannot resolve \"" + ref + "\"") {}

invalid_reference_error::~invalid_reference_error() noexcept = default;

// endregion invalid_reference_error

// region ambiguous_reference_error

ambiguous_reference_error::ambiguous_reference_error(column_types::string_t ref) noexcept
    : lang_error(3, "\"" + ref + "\" is ambiguous") {}

ambiguous_reference_error::~ambiguous_reference_error() noexcept = default;

// endregion ambiguous_reference_error

// region redefinition_error

// endregion redefinition_error

// region hint_match_error

// endregion hint_match_error

// region duplicate_path_error

// endregion duplicate_path_error

// region duplicate_symbol_error

// endregion duplicate_symbol_error

// region duplicate_object_class_error

// endregion duplicate_object_class_error
