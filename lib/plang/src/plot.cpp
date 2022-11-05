//
// Created by Johannes on 25.09.2022.
//

#include <plang/plot.hpp>

using namespace plang;
using namespace plang::plot;

symbol::clazz::hint::hint() = default;

symbol::clazz::hint::hint(const symbol::clazz &clazz, bool_t recursive)
    : hint_id(clazz.get_id()),
      recursive(recursive) {
    if (!clazz.is_persisted()) {
        // TODO: Throw exception
    }
}

pkey<symbol::clazz> symbol::clazz::hint::get_hint_id() const {
    return hint_id;
}

void symbol::clazz::hint::set_hint(const symbol::clazz &clazz) {
    if (clazz.is_persisted()) { hint_id = clazz.get_id(); }
}

bool_t symbol::clazz::hint::get_recursive() const {
    return recursive;
}

void symbol::clazz::hint::set_recursive(bool_t _recursive) {
    recursive = _recursive;
}

symbol::clazz::clazz() = default;

symbol::clazz::clazz(const root::path &path, std::vector<hint> &&hints)
    : path_id(path.get_id()),
      hints(hints) {
    if (!path.is_persisted()) {
        // TODO: Throw exception
    }
}

pkey<path> symbol::clazz::get_path_id() const {
    return path_id;
}

void symbol::clazz::set_path(const root::path &path) {
    path_id = path.get_id();
}

std::vector<symbol::clazz::hint> &symbol::clazz::get_hints() {
    return hints;
}

std::vector<symbol::clazz::hint> const &symbol::clazz::get_hints() const {
    return hints;
}

symbol::symbol() = default;

symbol::symbol(string_t name, const symbol::clazz &clazz)
    : name(name),
      class_id(clazz.get_id()) {
    if (!clazz.is_persisted()) {
        // TODO: Throw exception
    }
}

const string_t &symbol::get_name() const {
    return name;
}

void symbol::set_name(string_t &&_name) {
    name = std::move(_name);
}

pkey<symbol::clazz> symbol::get_class_id() const {
    return class_id;
}

void symbol::set_class(const symbol::clazz &clazz) {
    if (clazz.is_persisted()) {
        class_id = clazz.get_id();
    } else {
        // TODO: Throw exception
    }
}
