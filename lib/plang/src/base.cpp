//
// Created by Johannes on 05.08.2022.
//

#include "plang/base.hpp"

using namespace plang;

//region sourced

detail::base_types::sourced::sourced() noexcept
    : source_id() {}

detail::base_types::sourced::sourced(const root::source &source) noexcept
    : source_id(source.get_id()) {}

std::optional<pkey<class root::source>> detail::base_types::sourced::get_source_id() const noexcept {
    return source_id;
}
void detail::base_types::sourced::set_source_id(root::source const &source) noexcept {
    detail::base_types::sourced::source_id = source.get_id();
}

detail::base_types::sourced::~sourced() noexcept {}

//endregion sourced

//region decorated

detail::base_types::decorated::decorated() noexcept
    : decorated({}, {}) {}

detail::base_types::decorated::decorated(std::optional<float_t> ordinal, std::optional<string_t> description) noexcept
    : ordinal(ordinal),
      description(description) {}

const std::optional<plang::float_t> &detail::base_types::decorated::get_ordinal() const noexcept {
    return ordinal;
}

void detail::base_types::decorated::set_ordinal(const std::optional<float_t> &_ordinal) noexcept {
    detail::base_types::decorated::ordinal = _ordinal;
}

const std::optional<string_t> &detail::base_types::decorated::get_description() const noexcept {
    return description;
}

void detail::base_types::decorated::set_description(const std::optional<string_t> &_description) noexcept {
    detail::base_types::decorated::description = _description;
}

detail::base_types::decorated::~decorated() noexcept = default;

//endregion decorated

//region source

root::source::source(std::optional<string_t> &&name,
                     std::optional<string_t> &&version,
                     std::optional<string_t> &&url,
                     plang::root::source::time_point_t start)
    : name(std::forward<std::optional<string_t>>(name)),
      version(std::forward<std::optional<string_t>>(version)),
      url(std::forward<std::optional<string_t>>(url)),
      start(start),
      end(start) {}

std::optional<string_t> const &root::source::get_name() const noexcept {
    return name;
}
std::optional<string_t> const &root::source::get_version() const noexcept {
    return version;
}
std::optional<string_t> const &root::source::get_url() const noexcept {
    return url;
}
root::source::time_point_t root::source::get_start() const noexcept {
    return start;
}
root::source::time_point_t root::source::get_an_end() const noexcept {
    return end;
}

root::source::~source() noexcept = default;

//endregion source

//region path

root::path::path()
    : persisted(),
      decorated(),
      name(),
      parent_id(-1) {
    if (name.empty()) {
        // TODO: Throw exception
    }
}

root::path::path(string_t name, path const &parent)
    : persisted(),
      decorated(),
      name(name),
      parent_id(parent.get_id()) {
    if (name.empty()) {
        // TODO: Throw exception
    }

    if (parent.is_persisted()) {
        // TODO: Throw exception
    }
}

const string_t &root::path::get_name() const {
    return name;
}

bool root::path::set_name(const string_t &_name) {
    if (!_name.empty()) {
        root::path::name = _name;
        return true;
    } else {
        return false;
    }
}

pkey<path> root::path::get_parent_id() const {
    return parent_id;
}

void root::path::set_parent(path const &parent) {
    root::path::parent_id = parent.get_id();
}

bool_t root::path::is_root() const {
    return is_persisted() && (get_id() == parent_id);
}

root::path::~path() noexcept = default;

//endregion path
