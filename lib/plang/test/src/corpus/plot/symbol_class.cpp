//
// Created by Johannes on 26.09.2022.
//

#include <set>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <plang/corpus.hpp>
#include "helper.hpp"

using namespace plang;
using namespace plang::plot;
using namespace plang::op;

TEST_CASE("2.1.1. Symbol Class API", "[api]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<symbol::clazz>(false).size();
    REQUIRE(count == 0);

    auto root = corpus.fetch<path>(0).value();
    auto p    = corpus.resolve<path>({"abc"}, root, true).entry();

    SECTION("Resolve") {

        SECTION("Direct existent symbol class") {
            corpus.resolve<symbol::clazz>({}, p, true);
            auto [r, c, a] = corpus.resolve<symbol::clazz>({}, p);

            REQUIRE(a == action::NONE);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect existent symbol class") {
            corpus.resolve<symbol::clazz>({}, p, true);
            auto [r, c, a] = corpus.resolve<symbol::clazz>({"abc"}, root);

            REQUIRE(a == action::NONE);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Direct inexistent symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({}, p);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
        }

        SECTION("Indirect inexistent symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({"abc"}, root);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
        }

        SECTION("Indirect inexistent path and symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({"def"}, root);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
            REQUIRE(!corpus.resolve<path>({"def"}, root).has_result());
        }

        SECTION("Direct inserting symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({}, p, true);

            REQUIRE(a == action::INSERT);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect inserting symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({"abc"}, root, true);

            REQUIRE(a == action::INSERT);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect inserting path and symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({"def"}, root, true);
            auto p2        = corpus.resolve<path>({"def"}, root);

            REQUIRE(a == action::INSERT);
            REQUIRE(p2.has_result());
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p2.entry().get_id());
        }
    }

    SECTION("Fetch") {

        SECTION("Existent unhinted symbol class") {
            auto [r, c, a] = corpus.resolve<symbol::clazz>({}, p, true);
            auto result    = corpus.fetch<symbol::clazz>(r->get_id());

            REQUIRE(result.has_value());
            REQUIRE(result->get_id() == r->get_id());
            REQUIRE(result->get_path_id() == r->get_path_id());
            REQUIRE(result->get_source_id() == r->get_source_id());
        }

        SECTION("Existent hinted symbol class") {
            auto hint_a    = corpus.resolve<symbol::clazz>({"hint", "a"}, root, true).entry();
            auto hint_b    = corpus.resolve<symbol::clazz>({"hint", "b"}, root, true).entry();
            auto [r, c, a] = corpus.resolve<symbol::clazz>({}, p, true);
            r->get_hints().emplace_back(*r, false);
            r->get_hints().emplace_back(hint_a, false);
            r->get_hints().emplace_back(hint_b, true);
            corpus.update(r.value(), true);
            auto result = corpus.fetch<symbol::clazz>(r->get_id(), true);

            REQUIRE(result.has_value());
            REQUIRE(result->get_id() == r->get_id());
            REQUIRE(result->get_path_id() == r->get_path_id());
            REQUIRE(result->get_source_id() == r->get_source_id());

            REQUIRE(result->get_hints().size() == 3);

            //TODO: Compare hints
        }

        SECTION("Inexistent symbol class") {
            auto result = corpus.fetch<symbol::clazz>(1);

            REQUIRE(!result.has_value());
        }
    }

    SECTION("Insert") {

        SECTION("Unhinted") {

            SECTION("Non-replacing") {

                SECTION("New unhinted symbol class") {
                    symbol::clazz clazz{p};

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                }

                SECTION("Overriding unhinted symbol class") {
                    corpus.resolve<symbol::clazz>({}, p, true);
                    symbol::clazz clazz{p};

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::FAIL);
                    REQUIRE(!clazz.is_persisted());
                }
            }

            SECTION("Replacing") {

                SECTION("New unhinted symbol class") {
                    symbol::clazz clazz{p};

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                }

                SECTION("Overriding unhinted symbol class") {
                    corpus.resolve<symbol::clazz>({}, p, true);
                    symbol::clazz clazz{p};

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                }
            }
        }

        SECTION("Hinted") {
            auto hint_a = corpus.resolve<symbol::clazz>({"hint", "a"}, root, true).entry();
            auto hint_b = corpus.resolve<symbol::clazz>({"hint", "b"}, root, true).entry();

            SECTION("Non-replacing") {
                SECTION("New hinted symbol class") {
                    symbol::clazz clazz{
                            p,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                    for (auto const &h : clazz.get_hints()) { REQUIRE(h.is_persisted()); }
                }

                SECTION("Overriding hinted symbol class") {
                    corpus.resolve<symbol::clazz>({}, p, true);
                    symbol::clazz clazz{
                            p,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::FAIL);
                    REQUIRE(!clazz.is_persisted());
                }
            }

            SECTION("Replacing") {

                SECTION("New hinted symbol class") {
                    symbol::clazz clazz{
                            p,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                    for (auto const &h : clazz.get_hints()) { REQUIRE(h.is_persisted()); }
                }

                SECTION("Overriding hinted symbol class") {
                    corpus.resolve<symbol::clazz>({}, p, true);
                    symbol::clazz clazz{
                            p,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                    for (auto const &h : clazz.get_hints()) { REQUIRE(h.is_persisted()); }
                }
            }
        }
    }

    SECTION("Update") {
        auto hint_a = corpus.resolve<symbol::clazz>({"hint", "a"}, root, true).entry();
        auto hint_b = corpus.resolve<symbol::clazz>({"hint", "b"}, root, true).entry();

        auto p2 = corpus.resolve<path>({"def"}, root, true).entry();

        SECTION("Existent unhinted symbol class") {
            auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();

            clazz.set_path(p2);

            auto action = corpus.update(clazz, false);

            auto clazz2 = corpus.resolve<symbol::clazz>({}, p2, false).entry();

            REQUIRE(action == action::UPDATE);
            REQUIRE(clazz.get_path_id() == p2.get_id());
            REQUIRE(clazz2.get_id() == clazz.get_id());
            REQUIRE(clazz2.get_path_id() == p2.get_id());
        }

        SECTION("Existent unhinted to hinted symbol class") {
            auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();
            clazz.get_hints().emplace_back(hint_a);

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<symbol::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz.get_hints()[0].is_persisted());
            REQUIRE(clazz.get_hints()[0].get_id() == clazz2.get_hints()[0].get_id());
            REQUIRE(clazz.get_hints()[0].get_hint_id() == clazz2.get_hints()[0].get_hint_id());
            REQUIRE(clazz.get_hints()[0].get_recursive() == clazz2.get_hints()[0].get_recursive());
        }

        SECTION("Existent hinted to unhinted symbol class") {
            symbol::clazz clazz{p, {hint_a}};
            corpus.insert(clazz);
            clazz.get_hints().clear();

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<symbol::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz2.get_hints().empty());
        }

        SECTION("Existent hinted to recursively hinted symbol class") {
            symbol::clazz clazz{p, {{hint_a, false}}};
            corpus.insert(clazz);
            clazz.get_hints()[0].set_recursive(true);

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<symbol::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz.get_hints()[0].is_persisted());
            REQUIRE(clazz.get_hints()[0].get_id() == clazz2.get_hints()[0].get_id());
            REQUIRE(clazz.get_hints()[0].get_hint_id() == clazz2.get_hints()[0].get_hint_id());
            REQUIRE(clazz.get_hints()[0].get_recursive() == clazz2.get_hints()[0].get_recursive());
        }

        SECTION("Inexistent symbol class") {
            symbol::clazz clazz{p};

            auto action = corpus.update(clazz);

            REQUIRE(action == action::FAIL);
        }
    }

    SECTION("Print") {
        NOT_IMPLEMENTED();
    }

    SECTION("Remove") {

        SECTION("Non-cascading") {

            SECTION("Remove unreferenced persisted symbol class") {
                auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();

                auto [repr, action] = corpus.remove(clazz, false);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc[]");
            }

            SECTION("Remove self-referenced persisted symbol class") {
                auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();
                clazz.get_hints().emplace_back(clazz);
                corpus.update(clazz, true);

                auto [repr, action] = corpus.remove(clazz, false);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc[]");
            }

            SECTION("Remove referenced persisted symbol class") {
                auto clazz     = corpus.resolve<symbol::clazz>({}, p, true).entry();
                auto ref_class = corpus.resolve<symbol::clazz>({"ref"}, p, true).entry();
                ref_class.get_hints().emplace_back(clazz);
                corpus.update(ref_class, true);

                auto [repr, action] = corpus.remove(clazz, false);

                REQUIRE(action == action::FAIL);
                REQUIRE(repr == ".abc[]");
            }
        }

        SECTION("Cascading") {

            SECTION("Remove unreferenced persisted symbol class") {
                auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();

                auto [repr, action] = corpus.remove(clazz, true);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc[]");
            }

            SECTION("Remove self-referenced persisted symbol class") {
                auto clazz = corpus.resolve<symbol::clazz>({}, p, true).entry();
                clazz.get_hints().emplace_back(clazz);
                corpus.update(clazz, true);

                auto [repr, action] = corpus.remove(clazz, true);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc[]");
            }

            SECTION("Remove referenced persisted symbol class") {
                auto clazz     = corpus.resolve<symbol::clazz>({}, p, true).entry();
                auto ref_class = corpus.resolve<symbol::clazz>({"ref"}, p, true).entry();
                ref_class.get_hints().emplace_back(clazz);
                corpus.update(ref_class, true);

                auto [repr, action] = corpus.remove(clazz, true);
                ref_class           = corpus.fetch<symbol::clazz>(ref_class.get_id(), true).value();

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc[]");
                REQUIRE(ref_class.get_hints().empty());
            }
        }
    }

    // corpus.wipe();
}

TEST_CASE("2.1.2. Symbol Class Parser", "[parser]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<symbol::clazz>(false).size();
    REQUIRE(count == 0);

    auto root = corpus.fetch<path>(0).value();

    SECTION("Declarations") {
        SECTION("Valid") {
            SECTION("Local symbol class") {
                auto report = corpus.execute(R"([];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 1);
            }

            SECTION("Unqualified symbol class") {
                auto report = corpus.execute(R"(unqualified[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 1);
            }

            SECTION("Qualified symbol class") {
                auto report = corpus.execute(R"(.qualified[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 1);
            }

            SECTION("Decorated symbol class") {
                auto report = corpus.execute(R"(.decorated[] (6, 'Gnampf');)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 1);
            }

            SECTION("Empty hinting symbol class") {
                auto report = corpus.execute(R"(() unhinted[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 1);
            }

            SECTION("Simply hinting symbol class") {
                auto report = corpus.execute(R"((.hinted[]) hinting[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Recursively hinting symbol class") {
                auto report = corpus.execute(R"((.recursively.hinted[]...) hinting[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Mixed hinting symbol class") {
                auto report = corpus.execute(R"((.hinted[], .recursively.hinted[]...) hinting[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 3);
            }

            SECTION("Mixed list hinting symbol class") {
                auto report = corpus.execute(R"((.hinted,list1[], .recursively.hinted,list2[]...) hinting[];)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 5);
            }
        }

        SECTION("Ambiguous") {
            SECTION("Ambiguously hinted symbol class") {
                corpus.resolve<symbol::clazz>({"path1", "ambiguous"}, root, true);
                corpus.resolve<symbol::clazz>({"path2", "ambiguous"}, root, true);

                auto report = corpus.execute(R"((ambiguous[]) hinting[];)", root);

                REQUIRE(report.failed().size() == 1);
            }
        }
    }

    SECTION("References") {
        SECTION("Valid") {
            SECTION("Unqualified symbol class") {
                corpus.resolve<symbol::clazz>({"unqualified"}, root, true);
                auto ref = corpus.ref(R"(unqualified[])", root);

                REQUIRE(ref.has_result());
                REQUIRE(corpus.print(ref.entry())() == ".unqualified[]");
            }

            SECTION("Qualified symbol class") {
                corpus.resolve<symbol::clazz>({"qualified"}, root, true);
                auto ref = corpus.ref(R"(.qualified[])", root);

                REQUIRE(ref.has_result());
                REQUIRE(corpus.print(ref.entry())() == ".qualified[]");
            }
        }

        SECTION("Ambiguous") {
            SECTION("Ambiguous symbol class") {
                corpus.resolve<symbol::clazz>({"path1", "ambiguous"}, root, true);
                corpus.resolve<symbol::clazz>({"path2", "ambiguous"}, root, true);

                auto ref = corpus.ref(R"(ambiguous[])", root);

                REQUIRE(!ref.has_result());
            }
        }
    }

    // corpus.wipe();
}
