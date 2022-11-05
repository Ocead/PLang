//
// Created by Johannes on 09.10.2022.
//

#include <set>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <plang/corpus.hpp>
#include "helper.hpp"

using namespace plang;
using namespace plang::plot;
using namespace plang::op;

TEST_CASE("2.2.1. Symbol API", "[api]") {
    auto &corpus = make_corpus();
    auto scope   = corpus.fetch<path>(0).value();

    SECTION("Resolve") {
        SECTION("via path") {
            SECTION("Direct existent symbol") {
                corpus.resolve<symbol>({"symbol"}, scope, true);

                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, scope, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() == corpus.resolve<symbol::clazz>({}, scope, false).entry().get_id());
            }

            SECTION("Indirect existent symbol") {
                corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, true);

                auto [r, c, a] = corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() ==
                        corpus.resolve<symbol::clazz>({"a", "b", "c"}, scope, false).entry().get_id());
            }

            SECTION("Direct inexistent symbol") {
                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Indirect inexistent symbol") {
                auto [r, c, a] = corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Direct ambiguous symbol") {
                corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, true);
                corpus.resolve<symbol>({"d", "e", "c", "symbol"}, scope, true);

                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> expected{".a.b.c[symbol]", ".d.e.c[symbol]"};
                std::set<string_t> actual;
                for (auto const &e : c) { actual.insert(corpus.print(e)); }
                REQUIRE(expected == actual);
            }

            SECTION("Indirect ambiguous symbol") {
                corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, true);
                corpus.resolve<symbol>({"d", "e", "c", "symbol"}, scope, true);
                corpus.resolve<symbol>({"d", "e", "f", "symbol"}, scope, true);

                auto [r, c, a] = corpus.resolve<symbol>({"c", "symbol"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> expected{".a.b.c[symbol]", ".d.e.c[symbol]"};
                std::set<string_t> actual;
                for (auto const &e : c) { actual.insert(corpus.print(e)); }
                REQUIRE(expected == actual);
            }

            SECTION("Direct inserting symbol") {
                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, scope, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() == corpus.resolve<symbol::clazz>({}, scope, false).entry().get_id());
            }

            SECTION("Indirect inserting symbol") {
                auto [r, c, a] = corpus.resolve<symbol>({"a", "b", "c", "symbol"}, scope, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() ==
                        corpus.resolve<symbol::clazz>({"a", "b", "c"}, scope, false).entry().get_id());
            }
        }

        SECTION("via symbol class") {
            SECTION("Existent symbol") {
                auto clazz = corpus.resolve<symbol::clazz>({}, scope, true).entry();
                corpus.resolve<symbol>({"symbol"}, scope, true);

                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, clazz, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() == clazz.get_id());
            }

            SECTION("Inexistent symbol") {
                auto clazz = corpus.resolve<symbol::clazz>({}, scope, true).entry();

                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, clazz, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Inserting symbol") {
                auto clazz = corpus.resolve<symbol::clazz>({}, scope, true).entry();

                auto [r, c, a] = corpus.resolve<symbol>({"symbol"}, clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "symbol");
                REQUIRE(r->get_class_id() == clazz.get_id());
            }
        }
    }

    SECTION("Fetch") {
        SECTION("Undecorated") {
            auto undecorated = corpus.resolve<symbol>({"symbol"}, scope, true).entry();
            REQUIRE(undecorated.is_persisted());

            SECTION("Undecorated without dynamic") {
                auto r = corpus.fetch<symbol>(undecorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_class_id() == undecorated.get_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_ordinal() == undecorated.get_ordinal());
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_description() == undecorated.get_description());
                REQUIRE(r->get_source_id() == undecorated.get_source_id());
            }

            SECTION("Undecorated with dynamic") {
                auto r = corpus.fetch<symbol>(undecorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_class_id() == undecorated.get_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_source_id() == undecorated.get_source_id());
            }
        }

        SECTION("Decorated") {
            auto decorated = corpus.resolve<symbol>({"symbol"}, scope, true).entry();
            REQUIRE(decorated.is_persisted());
            decorated.set_ordinal(12.);
            decorated.set_description("Gnampf");
            corpus.update(decorated, true);

            SECTION("Decorated without dynamic") {
                auto r = corpus.fetch<symbol>(decorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_class_id() == decorated.get_class_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal());
                REQUIRE(r->get_ordinal() == 12.);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_source_id() == decorated.get_source_id());
            }

            SECTION("Decorated with dynamic") {
                auto r = corpus.fetch<symbol>(decorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_class_id() == decorated.get_class_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal());
                REQUIRE(r->get_ordinal() == 12.);
                REQUIRE(r->get_description() == decorated.get_description());
                REQUIRE(r->get_description() == "Gnampf");
                REQUIRE(r->get_source_id() == decorated.get_source_id());
            }
        }

        SECTION("Unpersisted") {
            auto r = corpus.fetch<symbol>(255, true);

            REQUIRE(!r.has_value());
        }
    }

    SECTION("Insert") {
        auto clazz = corpus.resolve<symbol::clazz>({"a", "b", "c"}, scope, true).entry();
        REQUIRE(clazz.is_persisted());

        SECTION("Non-replacing") {
            SECTION("Undecorated unique symbol") {
                symbol sym{"d", clazz};

                auto a = corpus.insert(sym);

                REQUIRE(a == action::INSERT);
                REQUIRE(sym.is_persisted());

                auto f = corpus.fetch(sym.get_id(), true).value();

                REQUIRE(f.get_id() == sym.get_id());
                REQUIRE(f.get_name() == sym.get_name());
                REQUIRE(f.get_class_id() == sym.get_class_id());
                REQUIRE(f.get_ordinal() == sym.get_ordinal());
                REQUIRE(f.get_description() == sym.get_description());
                REQUIRE(f.get_source_id() == sym.get_source_id());
            }

            SECTION("Decorated unique symbol") {
                symbol sym{"d", clazz};
                sym.set_ordinal(7);
                sym.set_description("Test");

                auto a = corpus.insert(sym);

                REQUIRE(a == action::INSERT);
                REQUIRE(sym.is_persisted());

                auto f = corpus.fetch(sym.get_id(), true).value();

                REQUIRE(f.get_id() == sym.get_id());
                REQUIRE(f.get_name() == sym.get_name());
                REQUIRE(f.get_class_id() == sym.get_class_id());
                REQUIRE(f.get_ordinal() == sym.get_ordinal());
                REQUIRE(f.get_description() == sym.get_description());
                REQUIRE(f.get_source_id() == sym.get_source_id());
            }

            SECTION("Duplicate symbol") {
                symbol sym1{"d", clazz};
                symbol sym2{"d", clazz};

                auto a1 = corpus.insert(sym1);
                auto a2 = corpus.insert(sym2);

                REQUIRE(a1 == action::INSERT);
                REQUIRE(sym1.is_persisted());
                REQUIRE(a2 == action::FAIL);
                REQUIRE(!sym2.is_persisted());
            }

            SECTION("Unpersisted class") {
                symbol::clazz clazz2{scope};
                symbol sym{"d", clazz2};

                auto a = corpus.insert(sym);

                REQUIRE(a == action::FAIL);
                REQUIRE(!sym.is_persisted());

                auto [r2, c2, a2] = corpus.resolve<symbol>({"d"}, scope);
                REQUIRE(!r2.has_value());
                REQUIRE(c2.empty());
                REQUIRE(a2 == action::FAIL);
            }

            SECTION("Nameless symbol") {
                symbol sym{"", clazz};

                auto a = corpus.insert(sym);

                REQUIRE(a == action::FAIL);
                REQUIRE(!sym.is_persisted());

                auto [r2, c2, a2] = corpus.resolve<symbol>({""}, scope);
                REQUIRE(!r2.has_value());
                REQUIRE(c2.empty());
                REQUIRE(a2 == action::FAIL);
            }
        }

        SECTION("Replacing") {
            symbol to_be_replaced{"d", clazz};
            to_be_replaced.set_ordinal(8);
            REQUIRE(corpus.insert(to_be_replaced) == action::INSERT);

            SECTION("Undecorated unique symbol") {
                symbol sym{"d", clazz};

                auto a = corpus.insert(sym, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(sym.is_persisted());

                auto f = corpus.fetch(sym.get_id(), true).value();

                REQUIRE(f.get_id() == sym.get_id());
                REQUIRE(f.get_name() == sym.get_name());
                REQUIRE(f.get_class_id() == sym.get_class_id());
                REQUIRE(f.get_ordinal() == sym.get_ordinal());
                REQUIRE(f.get_description() == sym.get_description());
                REQUIRE(f.get_source_id() == sym.get_source_id());
            }

            SECTION("Decorated unique symbol") {
                symbol sym{"d", clazz};
                sym.set_ordinal(7);
                sym.set_description("Test");

                auto a = corpus.insert(sym, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(sym.is_persisted());

                auto f = corpus.fetch(sym.get_id(), true).value();

                REQUIRE(f.get_id() == sym.get_id());
                REQUIRE(f.get_name() == sym.get_name());
                REQUIRE(f.get_class_id() == sym.get_class_id());
                REQUIRE(f.get_ordinal() == sym.get_ordinal());
                REQUIRE(f.get_description() == sym.get_description());
                REQUIRE(f.get_source_id() == sym.get_source_id());
            }

            SECTION("Duplicate symbol") {
                symbol sym1{"d", clazz};
                symbol sym2{"d", clazz};

                auto a1 = corpus.insert(sym1, true);
                auto a2 = corpus.insert(sym2, true);

                REQUIRE(a1 == action::INSERT);
                REQUIRE(sym1.is_persisted());
                REQUIRE(a2 == action::INSERT);
                REQUIRE(sym2.is_persisted());

                auto f = corpus.fetch(sym1.get_id(), true);

                REQUIRE(f.has_value());
                REQUIRE(f->get_id() == sym1.get_id());
                REQUIRE(f->get_name() == sym1.get_name());
                REQUIRE(f->get_class_id() == sym1.get_class_id());
                REQUIRE(f->get_ordinal() == sym1.get_ordinal());
                REQUIRE(f->get_description() == sym1.get_description());
                REQUIRE(f->get_source_id() == sym1.get_source_id());

                REQUIRE(sym1.get_id() == sym2.get_id());
                REQUIRE(sym1.get_name() == sym2.get_name());
                REQUIRE(sym1.get_class_id() == sym2.get_class_id());
                REQUIRE(sym1.get_ordinal() == sym2.get_ordinal());
                REQUIRE(sym1.get_description() == sym2.get_description());
                REQUIRE(sym1.get_source_id() == sym2.get_source_id());
            }

            SECTION("Unpersisted class") {
                symbol::clazz clazz2{scope};
                symbol sym{"f", clazz2};

                auto a = corpus.insert(sym, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!sym.is_persisted());

                auto [r2, c2, a2] = corpus.resolve<symbol>({"f"}, scope);
                REQUIRE(!r2.has_value());
                REQUIRE(c2.empty());
                REQUIRE(a2 == action::FAIL);
            }

            SECTION("Nameless symbol") {
                symbol sym{"", clazz};

                auto a = corpus.insert(sym, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!sym.is_persisted());

                auto [r2, c2, a2] = corpus.resolve<symbol>({""}, scope);
                REQUIRE(!r2.has_value());
                REQUIRE(c2.empty());
                REQUIRE(a2 == action::FAIL);
            }
        }
    }

    SECTION("Update") {
        auto [_r, _c, _a] = corpus.resolve<symbol>({"a", "b", "c", "d"}, scope, true);
        REQUIRE(_r.has_value());
        auto &expected = _r.value();

        SECTION("Persisted symbol without texts") {
            SECTION("Add decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_class_id() == expected.get_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == 7.);
                REQUIRE(actual.get_description() == std::nullopt);
            }

            SECTION("Remove decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();
                REQUIRE(actual.get_ordinal() == 7.);

                expected.set_ordinal();
                expected.set_description();
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_class_id() == expected.get_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == std::nullopt);
                REQUIRE(actual.get_description() == std::nullopt);
            }
        }

        SECTION("Persisted symbol with texts") {
            SECTION("Add decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, true) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_class_id() == expected.get_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == 7.);
                REQUIRE(actual.get_description() == expected.get_description());
                REQUIRE(actual.get_description() == "Test");
            }

            SECTION("Remove decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();
                REQUIRE(actual.get_ordinal() == 7.);

                expected.set_ordinal();
                expected.set_description();
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_class_id() == expected.get_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == std::nullopt);
                REQUIRE(actual.get_description() == std::nullopt);
            }
        }

        SECTION("Rename") {
            SECTION("Without collision") {
                expected.set_name("e");

                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto [actual, c, a] = corpus.resolve<symbol>({"a", "b", "c", "e"}, scope, false);
                REQUIRE(actual.has_value());
                REQUIRE(a == action::NONE);

                REQUIRE(actual->get_id() == expected.get_id());
            }

            SECTION("With collision") {
                auto [r, _c2, _a2] = corpus.resolve<symbol>({"a", "b", "c", "e"}, scope, true);
                expected.set_name("e");

                REQUIRE_THROWS(corpus.update(expected, false));

                auto [actual, c, a] = corpus.resolve<symbol>({"a", "b", "c", "e"}, scope, false);
                REQUIRE(a == action::NONE);
                REQUIRE(r->get_id() == actual->get_id());
                REQUIRE(actual->get_id() != expected.get_id());
            }
        }

        SECTION("Move") {
            SECTION("Without collision") {
                auto clazz = corpus.resolve<symbol::clazz>({"a", "b", "e"}, scope, true).entry();
                expected.set_class(clazz);

                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto [actual, c, a] = corpus.resolve<symbol>({"a", "b", "e", "d"}, scope, false);
                REQUIRE(a == action::NONE);
                REQUIRE(actual->get_id() == expected.get_id());
                REQUIRE(actual->get_class_id() == expected.get_class_id());
                REQUIRE(actual->get_class_id() == clazz.get_id());
            }

            SECTION("With collision") {
                auto clazz = corpus.resolve<symbol::clazz>({"a", "b", "e"}, scope, true).entry();
                REQUIRE(corpus.resolve<symbol>({"a", "b", "e", "d"}, scope, true).action() == action::INSERT);
                expected.set_class(clazz);

                REQUIRE_THROWS(corpus.update(expected, false));

                auto [actual, c, a] = corpus.resolve<symbol>({"a", "b", "e", "d"}, scope, false);
                REQUIRE(a == action::NONE);
                REQUIRE(actual->get_id() != expected.get_id());
                REQUIRE(actual->get_class_id() == expected.get_class_id());
                REQUIRE(actual->get_class_id() == clazz.get_id());
            }
        }
    }

    SECTION("Print") {
        NOT_IMPLEMENTED();
    }

    SECTION("Remove") {
        auto sym = corpus.resolve<symbol>({"a", "b", "c", "d"}, scope, true).entry();
        REQUIRE(sym.is_persisted());

        SECTION("Non-cascading") {
            SECTION("Remove persisted symbol without compounds") {
                auto id = sym.get_id();

                auto [repr, a] = corpus.remove(sym, true);
                REQUIRE(repr == ".a.b.c[d]");
                REQUIRE(a == action::REMOVE);

                REQUIRE(!corpus.fetch(id).has_value());
            }

            SECTION("Remove persisted symbol with compounds") {
                NOT_IMPLEMENTED();
            }
        }

        SECTION("Cascading") {
            SECTION("Remove persisted symbol without compounds") {
                auto id = sym.get_id();

                auto [repr, a] = corpus.remove(sym, true);
                REQUIRE(repr == ".a.b.c[d]");
                REQUIRE(a == action::REMOVE);

                REQUIRE(!corpus.fetch(id).has_value());
            }

            SECTION("Remove persisted symbol with compounds") {
                NOT_IMPLEMENTED();
            }
        }
    }

    corpus.wipe();
}

TEST_CASE("2.2.2. Symbol Parser", "[parser]") {
    auto &corpus = make_corpus();
    auto scope   = corpus.fetch<path>(0).value();

    SECTION("Declaration") {
        SECTION("Undecorated local symbol") {
            auto report = corpus.execute(R"([symbol];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = report.inserted().begin()->second;
            REQUIRE(corpus.print(sym)() == ".[symbol]");
        }

        SECTION("Undecorated root symbol") {
            auto report = corpus.execute(R"(.[symbol];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = report.inserted().begin()->second;
            REQUIRE(corpus.print(sym)() == ".[symbol]");
        }

        SECTION("Undecorated unqualified symbol") {
            auto report = corpus.execute(R"(unqualified[symbol];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = report.inserted().begin()->second;
            REQUIRE(corpus.print(sym)() == ".unqualified[symbol]");
        }

        SECTION("Undecorated qualified symbol") {
            auto report = corpus.execute(R"(.qualified[symbol];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = report.inserted().begin()->second;
            REQUIRE(corpus.print(sym)() == ".qualified[symbol]");
        }

        SECTION("Decorated local symbol") {
            auto report = corpus.execute(R"([decorated (7, "Test")];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = std::get<symbol>(report.inserted().begin()->second);
            REQUIRE(corpus.print(sym)() == ".[decorated]");
            REQUIRE(*sym.get_ordinal() == 7.);
            REQUIRE(*sym.get_description() == "Test");
        }

        SECTION("Decorated root symbol") {
            auto report = corpus.execute(R"(.[decorated (7, "Test")];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = std::get<symbol>(report.inserted().begin()->second);
            REQUIRE(corpus.print(sym)() == ".[decorated]");
            REQUIRE(*sym.get_ordinal() == 7.);
            REQUIRE(*sym.get_description() == "Test");
        }

        SECTION("Decorated unqualified symbol") {
            auto report = corpus.execute(R"(unqualified.decorated[symbol (7, "Test")];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = std::get<symbol>(report.inserted().begin()->second);
            REQUIRE(corpus.print(sym)() == ".unqualified.decorated[symbol]");
            REQUIRE(*sym.get_ordinal() == 7.);
            REQUIRE(*sym.get_description() == "Test");
        }

        SECTION("Decorated qualified symbol") {
            auto report = corpus.execute(R"(.qualified.decorated[symbol (7, "Test")];)", scope);

            REQUIRE(report.inserted().size() == 1);

            auto &sym = std::get<symbol>(report.inserted().begin()->second);
            REQUIRE(corpus.print(sym)() == ".qualified.decorated[symbol]");
            REQUIRE(*sym.get_ordinal() == 7.);
            REQUIRE(*sym.get_description() == "Test");
        }

        SECTION("Undecorated local symbol list") {
            auto report = corpus.execute(R"([symbol, list];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".[symbol]", ".[list]"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) { actual.insert(corpus.print(e.second)); }
            REQUIRE(expected == actual);
        }

        SECTION("Undecorated root symbol list") {
            auto report = corpus.execute(R"(.[symbol, list];)", scope);

            REQUIRE(report.inserted().size() == 2);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".[symbol]", ".[list]"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) { actual.insert(corpus.print(e.second)); }
            REQUIRE(expected == actual);
        }

        SECTION("Undecorated unqualified symbol list") {
            auto report = corpus.execute(R"(unqualified[symbol, list];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".unqualified[symbol]", ".unqualified[list]"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) { actual.insert(corpus.print(e.second)); }
            REQUIRE(expected == actual);
        }

        SECTION("Undecorated qualified symbol list") {
            auto report = corpus.execute(R"(.qualified[symbol, list];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".qualified[symbol]", ".qualified[list]"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) { actual.insert(corpus.print(e.second)); }
            REQUIRE(expected == actual);
        }

        SECTION("Decorated local symbol list") {
            auto report = corpus.execute(R"([symbol (7), list ("Test")];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".[symbol]7.000000", ".[list]Test"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) {
                auto &sym = std::get<symbol>(e.second);
                auto str  = corpus.print(sym)();
                if (sym.get_ordinal()) { str += std::to_string(*sym.get_ordinal()); }
                if (sym.get_description()) { str += *sym.get_description(); }
                actual.insert(str);
            }
            REQUIRE(expected == actual);
        }

        SECTION("Decorated root symbol list") {
            auto report = corpus.execute(R"(.[symbol (7), list ("Test")];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".[symbol]7.000000", ".[list]Test"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) {
                auto &sym = std::get<symbol>(e.second);
                auto str  = corpus.print(sym)();
                if (sym.get_ordinal()) { str += std::to_string(*sym.get_ordinal()); }
                if (sym.get_description()) { str += *sym.get_description(); }
                actual.insert(str);
            }
            REQUIRE(expected == actual);
        }

        SECTION("Decorated unqualified symbol list") {
            auto report = corpus.execute(R"(unqualified[symbol (7), list ("Test")];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".unqualified[symbol]7.000000", ".unqualified[list]Test"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) {
                auto &sym = std::get<symbol>(e.second);
                auto str  = corpus.print(sym)();
                if (sym.get_ordinal()) { str += std::to_string(*sym.get_ordinal()); }
                if (sym.get_description()) { str += *sym.get_description(); }
                actual.insert(str);
            }
            REQUIRE(expected == actual);
        }

        SECTION("Decorated qualified symbol list") {
            auto report = corpus.execute(R"(.qualified[symbol (7), list ("Test")];)", scope);

            REQUIRE(report.inserted().size() == 2);

            std::set<string_t> expected{".qualified[symbol]7.000000", ".qualified[list]Test"};
            std::set<string_t> actual;
            for (auto const &e : report.inserted()) {
                auto &sym = std::get<symbol>(e.second);
                auto str  = corpus.print(sym)();
                if (sym.get_ordinal()) { str += std::to_string(*sym.get_ordinal()); }
                if (sym.get_description()) { str += *sym.get_description(); }
                actual.insert(str);
            }
            REQUIRE(expected == actual);
        }
    }

    SECTION("Reference") {
        SECTION("Valid") {
            auto a1 = corpus.resolve<symbol>({"a", "b", "c", "d"}, scope, true).action();
            auto a2 = corpus.resolve<symbol>({"d", "e", "c", "d"}, scope, true).action();
            auto a3 = corpus.resolve<symbol>({"d", "e", "f", "g"}, scope, true).action();
            REQUIRE(a1 == action::INSERT);
            REQUIRE(a2 == action::INSERT);
            REQUIRE(a3 == action::INSERT);

            SECTION("Unqualified symbol") {
                auto [r, c, a] = corpus.ref("[g]", scope);

                REQUIRE(std::holds_alternative<symbol>(r));
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
                REQUIRE(corpus.print(r)() == ".d.e.f[g]");
            }

            SECTION("Partially qualified symbol") {
                auto [r, c, a] = corpus.ref("b.c[d]", scope);

                REQUIRE(std::holds_alternative<symbol>(r));
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
                REQUIRE(corpus.print(r)() == ".a.b.c[d]");
            }

            SECTION("Qualified symbol") {
                auto [r, c, a] = corpus.ref(".a.b.c[d]", scope);

                REQUIRE(std::holds_alternative<symbol>(r));
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
                REQUIRE(corpus.print(r)() == ".a.b.c[d]");
            }
        }

        SECTION("Ambiguous") {
            auto a1 = corpus.resolve<symbol>({"a", "b", "c", "d"}, scope, true).action();
            auto a2 = corpus.resolve<symbol>({"d", "e", "c", "d"}, scope, true).action();
            auto a3 = corpus.resolve<symbol>({"d", "e", "f", "d"}, scope, true).action();
            REQUIRE(a1 == action::INSERT);
            REQUIRE(a2 == action::INSERT);
            REQUIRE(a3 == action::INSERT);

            SECTION("Unqualified") {
                auto [r, c, a] = corpus.ref("[d]", scope);

                REQUIRE(std::holds_alternative<std::monostate>(r));
                REQUIRE(a == action::FAIL);
                REQUIRE(c.size() == 3);
                std::set<string_t> expected{".a.b.c[d]", ".d.e.c[d]", ".d.e.f[d]"};
                std::set<string_t> actual;
                for (auto const &e : c) { actual.insert(corpus.print(e)); }
                REQUIRE(expected == actual);
            }

            SECTION("Partially qualified") {
                auto [r, c, a] = corpus.ref("c[d]", scope);

                REQUIRE(std::holds_alternative<std::monostate>(r));
                REQUIRE(a == action::FAIL);
                REQUIRE(c.size() == 2);
                std::set<string_t> expected{".a.b.c[d]", ".d.e.c[d]"};
                std::set<string_t> actual;
                for (auto const &e : c) { actual.insert(corpus.print(e)); }
                REQUIRE(expected == actual);
            }

            SECTION("Unqualified with scope") {
                auto p = corpus.resolve<path>({"d"}, scope);
                REQUIRE(p.has_result());

                auto [r, c, a] = corpus.ref("[d]", p.entry());

                REQUIRE(std::holds_alternative<std::monostate>(r));
                REQUIRE(a == action::FAIL);
                REQUIRE(c.size() == 3);
                std::set<string_t> expected{".a.b.c[d]", ".d.e.c[d]", ".d.e.f[d]"};
                std::set<string_t> actual;
                for (auto const &e : c) { actual.insert(corpus.print(e)); }
                REQUIRE(expected == actual);
            }
        }

        SECTION("Invalid") {
            auto [r, c, a] = corpus.ref("[d]", scope);

            REQUIRE(std::holds_alternative<std::monostate>(r));
            REQUIRE(c.empty());
            REQUIRE(a == action::FAIL);
        }
    }

    corpus.wipe();
}
