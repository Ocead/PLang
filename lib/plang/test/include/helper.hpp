//
// Created by Johannes on 20.08.2022.
//

#ifndef PLANG_HELPER_HPP
#define PLANG_HELPER_HPP

#ifndef PLANG_TEST_DB
#define PLANG_TEST_DB
#endif

#include <plang/corpus.hpp>

namespace {
    std::optional<plang::corpus> corpus;
}

inline plang::corpus &make_corpus() {
    using namespace plang;

    ::corpus = plang::corpus{PLANG_TEST_DB};
    auto format = format::PLAIN;
    format.set_detail(format::detail::EXPLICIT_REF);
    ::corpus->set_format(format);
    return *::corpus;
}

#ifdef PLANG_TEST_FAIL_NOT_IMPLEMENTED
#define NOT_IMPLEMENTED() FAIL("Not implemented")
#else
#define NOT_IMPLEMENTED() static_cast<void>(nullptr)
#endif

#endif//PLANG_HELPER_HPP
