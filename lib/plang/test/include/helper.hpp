//
// Created by Johannes on 20.08.2022.
//

#ifndef PLANG_HELPER_HPP
#define PLANG_HELPER_HPP

#ifndef PLANG_TEST_DB
#define PLANG_TEST_DB
#endif

#include <plang/corpus.hpp>

CATCH_REGISTER_ENUM(plang::action,
                    plang::action::NONE,
                    plang::action::INSERT,
                    plang::action::UPDATE,
                    plang::action::REMOVE,
                    plang::action::FAIL);

namespace {
    std::optional<plang::corpus> corpus;
}

inline plang::corpus &make_corpus() {
    using namespace plang;

    ::corpus    = plang::corpus{PLANG_TEST_DB};
    auto format = format::PLAIN;
    format.set_detail(format::detail::EXPLICIT_REF);
    ::corpus->set_format(format);
    return *::corpus;
}

inline std::set<plang::string_t> print_to_set(plang::corpus const &_corpus, plang::corpus::report::map const &map) {
    std::set<plang::string_t> result;

    std::transform(map.begin(),
                   map.end(),
                   std::inserter(result, result.begin()),
                   [&_corpus](auto const &e) -> plang::string_t { return _corpus.print(std::get<plang::entry>(e)); });

    return result;
}

#ifdef PLANG_TEST_FAIL_NOT_IMPLEMENTED
#define NOT_IMPLEMENTED() FAIL("Not implemented")
#else
#define NOT_IMPLEMENTED() static_cast<void>(nullptr)
#endif

#endif//PLANG_HELPER_HPP
