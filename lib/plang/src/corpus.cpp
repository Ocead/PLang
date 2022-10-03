//
// Created by Johannes on 16.08.2022.
//

#include <antlr4-runtime.h>
#include <plang/corpus.hpp>
#include "lang/GENERATED/PlangLexer.h"
#include "lang/GENERATED/PlangParser.h"
#include "lang/visitor.hpp"

namespace plang::detail {
    template<typename T, typename = void>
    struct is_resolve_defined : std::false_type {};


    template<template<typename...> typename T, typename... Args>
    struct is_resolve_defined<T<Args...>,
                              std::void_t<decltype(std::declval<plang::corpus>().detail::corpus_mixins::resolve(
                                      std::declval<Args>()...))>> : std::true_type {};
}// namespace plang::detail

using namespace plang;

const decltype(format::value) format::OUTPUT_MASK        = 0b000000011;
const decltype(format::value) format::ENRICH_MASK        = 0b000001100;
const decltype(format::value) format::DETAIL_MASK        = 0b000110000;
const decltype(format::value) format::QUALIFICATION_MASK = 0b001000000;
const decltype(format::value) format::INDENT_MASK        = 0b110000000;

const format format::PLAIN =
        format(output::ANSI, enrich::PLAIN, detail::DEFINITION, qualification::FULL, indent::CENTER);

corpus::report::report() = default;

corpus::report::key corpus::report::get_key(entry const &entry) {
    return std::visit(
            [](auto const &e) -> corpus::report::key {
                using T = std::remove_const_t<std::remove_reference_t<decltype(e)>>;
                if constexpr (!std::is_same_v<T, std::monostate>) {
                    return {plang::entry_type(detail::corpus::tag<T>()), e.get_id()};
                } else {
                    return {entry_type(), -1};
                }
            },
            entry);
}

bool_t corpus::report::valid(corpus::report::key const &key) {
    return std::get<0>(key).index() != 0 && std::get<1>(key) >= 0;
}

void corpus::report::mention(plang::entry &&entry) {
    auto key = get_key(entry);
    if (valid(key)) {
        if (_inserted.find(key) == _inserted.end() && _updated.find(key) == _updated.end()) {
            _mentioned[key] = std::move(entry);
        }
    }
}

void corpus::report::insert(plang::entry &&entry) {
    auto key = get_key(entry);
    if (valid(key)) {
        _mentioned.erase(key);

        if (_updated.find(key) == _updated.end()) {
            _inserted[key] = std::move(entry);
        } else {
            _inserted.insert(_updated.extract(key));
        }
    }
}

void corpus::report::update(plang::entry &&entry) {
    auto key = get_key(entry);
    if (valid(key)) {
        _mentioned.erase(key);

        if (_inserted.find(key) == _inserted.end()) { _updated[key] = std::move(entry); }
    }
}

void corpus::report::remove(corpus::report::key &&key, string_t &&repr) {
    if (valid(key)) {
        _mentioned.erase(key);
        _inserted.erase(key);
        _updated.erase(key);

        _removed[std::move(key)] = std::move(repr);
    }
}

void corpus::report::fail(string_t &&repr, std::vector<entry> &&candidates) {
    _failed.emplace(std::move(repr), std::move(candidates));
}

decltype(corpus::report::_mentioned) const &corpus::report::mentioned() const {
    return _mentioned;
}

decltype(corpus::report::_inserted) const &corpus::report::inserted() const {
    return _inserted;
}

decltype(corpus::report::_updated) const &corpus::report::updated() const {
    return _updated;
}

decltype(corpus::report::_removed) const &corpus::report::removed() const {
    return _removed;
}

decltype(corpus::report::_failed) const &corpus::report::failed() const {
    return _failed;
}

decltype(corpus::report::_scope) &corpus::report::scope() {
    return _scope;
}

std::size_t corpus::report::size() const {
    return _mentioned.size() + _inserted.size() + _updated.size() + _removed.size() + _failed.size();
}

std::size_t corpus::report::diff_size() const {
    return _inserted.size() + _updated.size() + _removed.size();
}

corpus::report::~report() noexcept = default;

corpus::corpus()
    : detail::corpus_mixins() {
    detail::corpus::_open(":memory:");
    detail::corpus::_create_schema();
    detail::path_manager::ensure_root_path();
    detail::corpus::_set_pragmas();
}

corpus::corpus(const std::filesystem::path &file)
    : detail::corpus() {
    detail::corpus::_open(file.string().c_str());
    detail::corpus::_create_schema();
    detail::path_manager::ensure_root_path();
    detail::corpus::_set_pragmas();
}

corpus::report corpus::execute(istream_t &stream, path const &scope) {
    using namespace antlr4;
    using namespace lang::generated;

    ANTLRInputStream input(stream);
    PlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    PlangParser parser(&tokens);
    parser.setBuildParseTree(true);
    PlangParser::StartSVOContext *tree = parser.startSVO();

    if (parser.getNumberOfSyntaxErrors() == 0) {
        detail::corpus::_begin();

        auto visitor = lang::make_visitor(*this, scope);
        auto result  = visitor->visitStartSVO(tree);

        auto report = visitor->get_report();

        if (report.failed().empty()) {
            detail::corpus::_commit();
        } else {
            report._mentioned.clear();
            report._inserted.clear();
            report._updated.clear();
            report._removed.clear();
            detail::corpus::_rollback();
        }

        return report;
    } else {
        return report();
    }
}

corpus::report corpus::execute(string_t const &expr, path const &scope) {
    std::istringstream is{expr};
    return execute(is, scope);
}

corpus::report corpus::decl(istream_t &stream, path const &scope) {
    using namespace antlr4;
    using namespace lang::generated;

    ANTLRInputStream input(stream);
    PlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    PlangParser parser(&tokens);
    parser.setBuildParseTree(true);
    PlangParser::DeclSVOContext *tree = parser.declSVO();

    if (parser.getNumberOfSyntaxErrors() == 0) {
        detail::corpus::_begin();

        auto visitor = lang::make_visitor(*this, scope);
        auto result  = visitor->visitDeclSVO(tree);

        auto report = visitor->get_report();

        if (report.failed().empty()) {
            detail::corpus::_commit();
        } else {
            report._mentioned.clear();
            report._inserted.clear();
            report._updated.clear();
            report._removed.clear();
            detail::corpus::_rollback();
        }

        return report;
    } else {
        return report();
    }
}

corpus::report corpus::decl(string_t const &expr, path const &scope) {
    std::istringstream is{expr};
    return decl(is, scope);
}

resolve_entry_result corpus::ref(istream_t &stream, path const &scope) const {
    using namespace antlr4;
    using namespace lang::generated;

    ANTLRInputStream input(stream);
    PlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    PlangParser parser(&tokens);
    parser.setBuildParseTree(true);
    PlangParser::RefContext *tree = parser.ref();

    if (parser.getNumberOfSyntaxErrors() == 0) {
        auto visitor = lang::make_visitor(*this, scope);
        auto result  = visitor->visitRef(tree);

        return std::any_cast<resolve_entry_result>(std::move(result));
    } else {
        return {};
    }
}

resolve_entry_result corpus::ref(string_t const &expr, path const &scope) const {
    std::istringstream is{expr};
    return ref(is, scope);
}

plang::entry corpus::fetch(entry_type type, pkey_t id, bool_t dynamic) const {
    return std::visit(
            [&](auto const &type) -> plang::entry {
                using TI = std::remove_reference_t<std::remove_const_t<decltype(type)>>;
                using T  = typename TI::type;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    auto result = detail::corpus_mixins::fetch(id, dynamic, type);
                    if (result.has_value()) {
                        return {result.value()};
                    } else {
                        return {};
                    }
                } else {
                    throw new std::logic_error(string_t("Fetch for ") + typeid(T).name() + " is not implemented.");
                }
            },
            type);
}

std::vector<plang::entry> corpus::fetch_n(entry_type type, std::vector<pkey_t> const &ids, bool_t dynamic) const {
    return std::visit(
            [&](auto const &type) -> std::vector<plang::entry> {
                using TI = std::remove_reference_t<std::remove_const_t<decltype(type)>>;
                using T  = typename TI::type;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    auto vec = detail::corpus_mixins::fetch_n(ids, dynamic, type);
                    if (!vec.empty()) {
                        std::vector<plang::entry> ent_vec{};
                        ent_vec.reserve(vec.size());
                        std::transform(std::make_move_iterator(vec.begin()),
                                       std::make_move_iterator(vec.end()),
                                       std::back_inserter(ent_vec),
                                       [](T &&t) -> plang::entry { return plang::entry(std::move(t)); });
                        return ent_vec;
                    } else {
                        return {};
                    }
                } else {
                    throw new std::logic_error(string_t("Fetch_n for ") + typeid(T).name() + " is not implemented.");
                }
            },
            type);
}

std::vector<plang::entry> corpus::fetch_all(entry_type type, bool_t dynamic, int_t limit, int_t offset) const {
    return std::visit(
            [&](auto const &type) -> std::vector<plang::entry> {
                using TI = std::remove_reference_t<std::remove_const_t<decltype(type)>>;
                using T  = typename TI::type;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    auto vec = detail::corpus_mixins::fetch_all(dynamic, limit, offset, type);
                    if (!vec.empty()) {
                        std::vector<plang::entry> ent_vec{};
                        ent_vec.reserve(vec.size());
                        std::transform(std::make_move_iterator(vec.begin()),
                                       std::make_move_iterator(vec.end()),
                                       std::back_inserter(ent_vec),
                                       [](T &&t) -> plang::entry { return plang::entry(std::move(t)); });
                        return ent_vec;
                    } else {
                        return {};
                    }
                } else {
                    throw new std::logic_error(string_t("Fetch_all for ") + typeid(T).name() + " is not implemented.");
                }
            },
            type);
}

resolve_entry_result corpus::resolve(const std::vector<string_t> &path,
                                     plang::entry_type type,
                                     const plang::entry &context,
                                     bool_t insert,
                                     bool_t dynamic) {
    auto result = std::visit(
            [&](auto &type, auto &ctx) -> resolve_entry_result {
                using TI = std::remove_reference_t<decltype(type)>;
                using T  = typename TI::type;
                using C  = typename std::remove_reference_t<std::remove_const_t<decltype(ctx)>>;

                if constexpr (std::is_same_v<std::monostate, T>) {
                    return {};
                } else {
                    auto [val, vec, ac] = [&](TI &type,
                                              C const &ctx) -> std::tuple<plang::entry, std::vector<T>, action> {
                        if constexpr (detail::is_resolve_defined<std::tuple<decltype(path),
                                                                            decltype(ctx),
                                                                            decltype(insert),
                                                                            decltype(dynamic),
                                                                            decltype(type)>>::value) {
                            auto [val_, vec_, ac_] = detail::corpus_mixins::resolve(path, ctx, insert, dynamic, type);
                            if (val_.has_value()) {
                                return {std::move(val_.value()), std::move(vec_), ac_};
                            } else {
                                return {std::monostate(), std::move(vec_), action::FAIL};
                            }
                        } else {
                            return {};
                        }
                    }(type, ctx);

                    if (vec.empty()) {
                        return {std::move(val), {}, ac};
                    } else {
                        std::vector<plang::entry> ent_vec{};
                        ent_vec.reserve(vec.size());
                        std::transform(std::make_move_iterator(vec.begin()),
                                       std::make_move_iterator(vec.end()),
                                       std::back_inserter(ent_vec),
                                       [](T &&t) -> plang::entry { return plang::entry(std::move(t)); });
                        return {std::move(val), std::move(ent_vec), ac};
                    }
                }
            },
            type,
            context);

    return result;
}

resolve_entry_result
corpus::resolve(const std::vector<string_t> &path, entry_type type, const entry &context, bool_t dynamic) const {
    return const_cast<corpus *>(this)->resolve(path, type, context, false, dynamic);
}

resolve_entry_ref_result corpus::resolve(const std::vector<string_t> &path,
                                         plang::entry &entry,
                                         plang::entry const &context,
                                         bool_t insert,
                                         bool_t dynamic) {
    auto [vector, ac] = std::visit(
            [&](auto &ent, auto const &ctx) -> std::tuple<std::vector<plang::entry>, action> {
                using T = std::remove_reference_t<decltype(ent)>;
                using C = typename std::remove_reference_t<std::remove_const_t<decltype(ctx)>>;

                auto [ref, vec, act] =
                        [&](T &ent, C const &ctx) -> std::tuple<std::reference_wrapper<T>, std::vector<T>, action> {
                    if constexpr (detail::is_resolve_defined<std::tuple<decltype(path),
                                                                        decltype(ent),
                                                                        decltype(ctx),
                                                                        decltype(insert),
                                                                        decltype(dynamic)>>::value) {
                        return detail::corpus_mixins::resolve(path, ent, ctx, insert, dynamic);
                    } else
                        return std::make_tuple(std::ref(ent), std::vector<T>(), action::FAIL);
                }(ent, ctx);

                if (vec.empty()) {
                    return {};
                } else {
                    std::vector<plang::entry> ent_vec{};
                    ent_vec.reserve(vec.size());
                    std::transform(std::make_move_iterator(vec.begin()),
                                   std::make_move_iterator(vec.end()),
                                   std::back_inserter(ent_vec),
                                   [](T &&t) -> plang::entry { return plang::entry(std::move(t)); });
                    return {ent_vec, act};
                }
            },
            entry,
            context);
    return {std::ref(entry), vector, ac};
}

resolve_entry_ref_result corpus::resolve(const std::vector<string_t> &path,
                                         plang::entry &entry,
                                         plang::entry const &context,
                                         bool_t dynamic) const {
    return const_cast<corpus *>(this)->resolve(path, entry, context, false, dynamic);
}

action corpus::insert(plang::entry &entry, bool_t replace) {
    return std::visit(
            [&](auto const &entry) -> action {
                using T = std::remove_reference_t<std::remove_const_t<decltype(entry)>>;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    return detail::corpus_mixins::insert(entry, replace, corpus::tag<T>());
                } else {
                    throw new std::logic_error(string_t("Insert for ") + typeid(T).name() + " is not implemented.");
                }
            },
            entry);
}

action corpus::update(plang::entry &entry, bool_t dynamic) {
    return std::visit(
            [&](auto const &entry) -> action {
                using T = std::remove_reference_t<std::remove_const_t<decltype(entry)>>;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    return detail::corpus_mixins::insert(entry, dynamic, corpus::tag<T>());
                } else {
                    throw new std::logic_error(string_t("Update for ") + typeid(T).name() + " is not implemented.");
                }
            },
            entry);
}

detail::stream_helper corpus::print(plang::entry const &entry, format format) const {
    return std::visit(
            [this, format](auto const &entry) -> detail::stream_helper {
                using T = std::decay_t<decltype(entry)>;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    return detail::corpus_mixins::print(entry.get_id(), format, corpus::tag<T>());
                } else {
                    throw std::logic_error(string_t("Print for ") + typeid(T).name() + " is not implemented.");
                }
            },
            const_cast<plang::entry &>(entry));
}

std::tuple<string_t, action> corpus::remove(plang::entry &entry, bool_t cascade) {
    return std::visit(
            [&](auto &entry) -> std::tuple<string_t, action> {
                using T = std::decay_t<decltype(entry)>;

                if constexpr (std::is_same_v<path, T> || std::is_same_v<plot::symbol::clazz, T>) {
                    return detail::corpus_mixins::remove(entry, cascade, corpus::tag<T>());
                } else {
                    throw new std::logic_error(string_t("Remove for ") + typeid(T).name() + " is not implemented.");
                }
            },
            entry);
}

bool_t corpus::backup(corpus &target) {
    return detail::corpus::backup(target);
}


corpus::~corpus() noexcept = default;
