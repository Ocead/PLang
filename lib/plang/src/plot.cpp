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

point::clazz::hint::hint() = default;

point::clazz::hint::hint(const symbol::clazz &hint, plang::column_types::bool_t recursive)
    : hint_id(hint.get_id()),
      recursive(recursive) {
    if (!hint.is_persisted()) {
        // TODO: Throw exception
    }
}

pkey<symbol::clazz> point::clazz::hint::get_hint_id() const {
    return hint_id;
}

void point::clazz::hint::set_hint(const plang::plot::symbol::clazz &hint) {
    if (hint.is_persisted()) {
        hint_id = hint.get_id();
    } else {
        // TODO: Throw exception
    }
}

bool_t point::clazz::hint::get_recursive() const {
    return recursive;
}

void point::clazz::hint::set_recursive(bool_t _recursive) {
    recursive = _recursive;
}

point::clazz::clazz() = default;

point::clazz::clazz(const plang::root::path &path, plang::column_types::bool_t singleton, std::vector<hint> &&hints)
    : path_id(path.get_id()),
      singleton(singleton),
      hints(std::move(hints)) {
    if (!path.is_persisted()) {
        // TODO: Throw exception
    }
}

pkey<path> point::clazz::get_path_id() const {
    return path_id;
}

void point::clazz::set_path(const plang::root::path &path) {
    if (path.is_persisted()) {
        path_id = path.get_id();
    } else {
        // TODO: Throw exception
    }
}

bool_t point::clazz::get_singleton() const {
    return singleton;
}

void point::clazz::set_singleton(plang::column_types::bool_t _singleton) {
    singleton = _singleton;
}

std::vector<point::clazz::hint> &point::clazz::get_hints() {
    return hints;
}

const std::vector<point::clazz::hint> &point::clazz::get_hints() const {
    return hints;
}

object::clazz::hint::lit::lit(string_t hint, type type)
    : hint(hint),
      _type(type) {}

const string_t &object::clazz::hint::lit::get_hint() const {
    return hint;
}

void object::clazz::hint::lit::set_hint(plang::column_types::string_t _hint) {
    hint = std::move(_hint);
}

object::clazz::hint::lit::type object::clazz::hint::lit::get_type() const {
    return _type;
}

void object::clazz::hint::lit::set_type(plang::plot::object::clazz::hint::lit::type type) {
    _type = type;
}

object::clazz::hint::sym::sym()
    : hint_id(-1),
      recursive() {}

object::clazz::hint::sym::sym(const symbol::clazz &clazz, bool_t recursive)
    : hint_id(clazz.get_id()),
      recursive(recursive) {
    if (!clazz.is_persisted()) {
        // TODO: Throw excaption
    }
}

pkey<symbol::clazz> object::clazz::hint::sym::get_hint_id() const {
    return hint_id;
}

void object::clazz::hint::sym::set_hint(const symbol::clazz &hint) {
    if (hint.is_persisted()) {
        hint_id = hint.get_id();
    } else {
        // TODO: Throw exception
    }
}

bool_t object::clazz::hint::sym::get_recursive() const {
    return recursive;
}

void object::clazz::hint::sym::set_recursive(bool_t _recursive) {
    recursive = _recursive;
}

object::clazz::hint::pnt::pnt()
    : hint_id(-1),
      recursive() {}

object::clazz::hint::pnt::pnt(const point::clazz &clazz, bool_t recursive)
    : hint_id(clazz.get_id()),
      recursive(recursive) {
    if (!clazz.is_persisted()) {
        // TODO: Throw excaption
    }
}

pkey<point::clazz> object::clazz::hint::pnt::get_hint_id() const {
    return hint_id;
}

void object::clazz::hint::pnt::set_hint(const point::clazz &hint) {
    if (hint.is_persisted()) {
        hint_id = hint.get_id();
    } else {
        // TODO: Throw exception
    }
}

bool_t object::clazz::hint::pnt::get_recursive() const {
    return recursive;
}

void object::clazz::hint::pnt::set_recursive(bool_t _recursive) {
    recursive = _recursive;
}

object::clazz::hint::variant::variant(string_t &&string, lit::type type)
    : Base(lit(std::forward<string_t>(string), type)) {}

object::clazz::hint::variant::variant(const symbol::clazz &clazz, plang::column_types::bool_t recursive)
    : Base(sym(clazz, recursive)) {}

object::clazz::hint::variant::variant(const point::clazz &clazz, plang::column_types::bool_t recursive)
    : Base(pnt(clazz, recursive)) {}

plang::plot::object::clazz::hint::variant::~variant() = default;

object::clazz::clazz() = default;

object::clazz::clazz(const point::clazz &clazz, string_t name, bool_t _default, bool_t singleton)
    : point_class_id(clazz.get_id()),
      name(std::move(name)),
      _default(_default),
      singleton(singleton) {}

pkey<point::clazz> object::clazz::get_point_class_id() const {
    return point_class_id;
}

void object::clazz::set_point_class(const point::clazz &clazz) {
    if (clazz.is_persisted()) {
        point_class_id = clazz.get_id();
    } else {
        // TODO: Throw exception
    }
}

const string_t &object::clazz::get_name() const {
    return name;
}

void object::clazz::set_name(const string_t &_name) {
    name = _name;
}

bool_t object::clazz::get_default() const {
    return _default;
}

void object::clazz::set_default(plang::column_types::bool_t _default_) {
    _default = _default_;
}

bool_t object::clazz::get_singleton() const {
    return singleton;
}

void object::clazz::set_singleton(plang::column_types::bool_t _singleton) {
    singleton = _singleton;
}

std::vector<object::clazz::hint::variant> &object::clazz::get_hints() {
    return hints;
}

std::vector<object::clazz::hint::variant> const &object::clazz::get_hints() const {
    return hints;
}
