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

inline plang::corpus make_corpus() {
    using namespace plang;

    plang::corpus corpus{PLANG_TEST_DB};
    auto format = format::PLAIN;
    format.set_detail(format::detail::EXPLICIT_REF);
    corpus.set_format(format);
    return corpus;
}

#endif//PLANG_HELPER_HPP
