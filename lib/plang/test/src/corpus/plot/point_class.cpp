//
// Created by Johannes on 14.11.2022.
//

#include <set>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <plang/corpus.hpp>
#include "helper.hpp"

using namespace plang;
using namespace plang::plot;
using namespace plang::op;

TEST_CASE("2.4.1 Point Class API", "[api]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<point::clazz>(false).size();
    REQUIRE(count == 0);

    auto root = corpus.fetch<path>(0).value();
    auto p    = corpus.resolve<path>({"abc"}, root, true).entry();

    SECTION("Resolve") {

        SECTION("Direct existent point class") {
            corpus.resolve<point::clazz>({}, p, true);
            auto [r, c, a] = corpus.resolve<point::clazz>({}, p);

            REQUIRE(a == action::NONE);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect existent point class") {
            corpus.resolve<point::clazz>({}, p, true);
            auto [r, c, a] = corpus.resolve<point::clazz>({"abc"}, root);

            REQUIRE(a == action::NONE);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Direct inexistent point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({}, p);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
        }

        SECTION("Indirect inexistent point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({"abc"}, root);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
        }

        SECTION("Indirect inexistent path and point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({"def"}, root);

            REQUIRE(a == action::FAIL);
            REQUIRE(!r.has_value());
            REQUIRE(!corpus.resolve<path>({"def"}, root).has_result());
        }

        SECTION("Direct inserting point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({}, p, true);

            REQUIRE(a == action::INSERT);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect inserting point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({"abc"}, root, true);

            REQUIRE(a == action::INSERT);
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p.get_id());
        }

        SECTION("Indirect inserting path and point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({"def"}, root, true);
            auto p2        = corpus.resolve<path>({"def"}, root);

            REQUIRE(a == action::INSERT);
            REQUIRE(p2.has_result());
            REQUIRE(r.has_value());
            REQUIRE(r->get_path_id() == p2.entry().get_id());
        }
    }

    SECTION("Fetch") {

        SECTION("Existent unhinted point class") {
            auto [r, c, a] = corpus.resolve<point::clazz>({}, p, true);
            auto result    = corpus.fetch<point::clazz>(r->get_id());

            REQUIRE(result.has_value());
            REQUIRE(result->get_id() == r->get_id());
            REQUIRE(result->get_path_id() == r->get_path_id());
            REQUIRE(result->get_source_id() == r->get_source_id());
        }

        SECTION("Existent hinted point class") {
            auto hint_a    = corpus.resolve<symbol::clazz>({"hint", "a"}, root, true).entry();
            auto hint_b    = corpus.resolve<symbol::clazz>({"hint", "b"}, root, true).entry();
            auto [r, c, a] = corpus.resolve<point::clazz>({}, p, true);
            r->get_hints().emplace_back(hint_a, false);
            r->get_hints().emplace_back(hint_b, true);
            corpus.update(r.value(), true);
            auto result = corpus.fetch<point::clazz>(r->get_id(), true);

            REQUIRE(result.has_value());
            REQUIRE(result->get_id() == r->get_id());
            REQUIRE(result->get_path_id() == r->get_path_id());
            REQUIRE(result->get_source_id() == r->get_source_id());

            REQUIRE(result->get_hints().size() == 2);

            //TODO: Compare hints
        }

        SECTION("Inexistent point class") {
            auto result = corpus.fetch<point::clazz>(1);

            REQUIRE(!result.has_value());
        }
    }

    SECTION("Insert") {

        SECTION("Unhinted") {

            SECTION("Non-replacing") {

                SECTION("New unhinted point class") {
                    point::clazz clazz{p};

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                }

                SECTION("Overriding unhinted point class") {
                    corpus.resolve<point::clazz>({}, p, true);
                    point::clazz clazz{p};

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::FAIL);
                    REQUIRE(!clazz.is_persisted());
                }
            }

            SECTION("Replacing") {

                SECTION("New unhinted point class") {
                    point::clazz clazz{p};

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                }

                SECTION("Overriding unhinted point class") {
                    corpus.resolve<point::clazz>({}, p, true);
                    point::clazz clazz{p};

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
                SECTION("New hinted point class") {
                    point::clazz clazz{
                            p,
                            false,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                    for (auto const &h : clazz.get_hints()) { REQUIRE(h.is_persisted()); }
                }

                SECTION("Overriding hinted point class") {
                    corpus.resolve<point::clazz>({}, p, true);
                    point::clazz clazz{
                            p,
                            false,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, false);

                    REQUIRE(action == action::FAIL);
                    REQUIRE(!clazz.is_persisted());
                }
            }

            SECTION("Replacing") {

                SECTION("New hinted point class") {
                    point::clazz clazz{
                            p,
                            false,
                            {hint_a, hint_b}
                    };

                    auto action = corpus.insert(clazz, true);

                    REQUIRE(action == action::INSERT);
                    REQUIRE(clazz.is_persisted());
                    REQUIRE(clazz.get_path_id() == p.get_id());
                    for (auto const &h : clazz.get_hints()) { REQUIRE(h.is_persisted()); }
                }

                SECTION("Overriding hinted point class") {
                    corpus.resolve<point::clazz>({}, p, true);
                    point::clazz clazz{
                            p,
                            false,
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

        SECTION("Existent unhinted point class") {
            auto clazz = corpus.resolve<point::clazz>({}, p, true).entry();

            clazz.set_path(p2);

            auto action = corpus.update(clazz, false);

            auto clazz2 = corpus.resolve<point::clazz>({}, p2, false).entry();

            REQUIRE(action == action::UPDATE);
            REQUIRE(clazz.get_path_id() == p2.get_id());
            REQUIRE(clazz2.get_id() == clazz.get_id());
            REQUIRE(clazz2.get_path_id() == p2.get_id());
        }

        SECTION("Existent unhinted to hinted point class") {
            auto clazz = corpus.resolve<point::clazz>({}, p, true).entry();
            clazz.get_hints().emplace_back(hint_a);

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<point::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz.get_hints()[0].is_persisted());
            REQUIRE(clazz.get_hints()[0].get_id() == clazz2.get_hints()[0].get_id());
            REQUIRE(clazz.get_hints()[0].get_hint_id() == clazz2.get_hints()[0].get_hint_id());
            REQUIRE(clazz.get_hints()[0].get_recursive() == clazz2.get_hints()[0].get_recursive());
        }

        SECTION("Existent hinted to unhinted point class") {
            point::clazz clazz{p, false, {hint_a}};
            corpus.insert(clazz);
            clazz.get_hints().clear();

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<point::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz2.get_hints().empty());
        }

        SECTION("Existent hinted to recursively hinted point class") {
            point::clazz clazz{p, false, {{hint_a, false}}};
            corpus.insert(clazz);
            clazz.get_hints()[0].set_recursive(true);

            corpus.update(clazz, true);
            auto clazz2 = corpus.fetch<point::clazz>(clazz.get_id(), true).value();

            REQUIRE(clazz.get_hints()[0].is_persisted());
            REQUIRE(clazz.get_hints()[0].get_id() == clazz2.get_hints()[0].get_id());
            REQUIRE(clazz.get_hints()[0].get_hint_id() == clazz2.get_hints()[0].get_hint_id());
            REQUIRE(clazz.get_hints()[0].get_recursive() == clazz2.get_hints()[0].get_recursive());
        }

        SECTION("Inexistent point class") {
            point::clazz clazz{p};

            auto action = corpus.update(clazz);

            REQUIRE(action == action::FAIL);
        }
    }

    SECTION("Print") {
        NOT_IMPLEMENTED();
    }

    SECTION("Remove") {

        SECTION("Non-cascading") {

            SECTION("Remove unreferenced persisted point class") {
                auto clazz = corpus.resolve<point::clazz>({}, p, true).entry();

                auto [repr, action] = corpus.remove(clazz, false);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc?:");
            }

            SECTION("Remove point class with object classes") {
                NOT_IMPLEMENTED();
            }

            SECTION("Remove point class hinted to in own object class") {
                NOT_IMPLEMENTED();
            }

            SECTION("Remove point class hinted to in other's object class") {
                NOT_IMPLEMENTED();
            }
        }

        SECTION("Cascading") {

            SECTION("Remove unreferenced persisted point class") {
                auto clazz = corpus.resolve<point::clazz>({}, p, true).entry();

                auto [repr, action] = corpus.remove(clazz, true);

                REQUIRE(action == action::REMOVE);
                REQUIRE(repr == ".abc?:");
            }

            SECTION("Remove point class with object classes") {
                NOT_IMPLEMENTED();
            }

            SECTION("Remove point class hinted to in own object class") {
                NOT_IMPLEMENTED();
            }

            SECTION("Remove point class hinted to in other's object class") {
                NOT_IMPLEMENTED();
            }
        }
    }

    // corpus.wipe();
}

TEST_CASE("2.4.2 Point Class Parser", "[parser]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<symbol::clazz>(false).size();
    REQUIRE(count == 0);

    auto root = corpus.fetch<path>(0).value();

    SECTION("Declarations") {

        SECTION("Valid") {

            SECTION("Unqualified point class") {
                auto report = corpus.execute(R"(unqualified _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Qualified point class") {
                auto report = corpus.execute(R"(.qualified _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Decorated point class") {
                auto report = corpus.execute(R"(.decorated (6, 'Gnampf') _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Empty hinting point class") {
                auto report = corpus.execute(R"(() unhinted _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 2);
            }

            SECTION("Simply hinting point class") {
                auto report = corpus.execute(R"((.hinted[]) hinting _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 3);
            }

            SECTION("Recursively hinting point class") {
                auto report = corpus.execute(R"((.recursively.hinted[]...) hinting _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 3);
            }

            SECTION("Mixed hinting point class") {
                auto report = corpus.execute(R"((.hinted[], .recursively.hinted[]...) hinting _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 4);
            }

            SECTION("Mixed list hinting point class") {
                auto report = corpus.execute(R"((.hinted,list1[], .recursively.hinted,list2[]...) hinting _;)", root);

                REQUIRE(report.mentioned().empty());
                REQUIRE(report.inserted().size() == 6);
            }
        }
    }

    SECTION("References") {

        SECTION("Valid") {
            SECTION("Unqualified point class") {
                corpus.resolve<point::clazz>({"unqualified"}, root, true);
                auto ref = corpus.ref(R"(unqualified?:)", root);

                REQUIRE(ref.has_result());
                REQUIRE(corpus.print(ref.entry())() == ".unqualified?:");
            }

            SECTION("Qualified point class") {
                corpus.resolve<point::clazz>({"qualified"}, root, true);
                auto ref = corpus.ref(R"(.qualified?:)", root);

                REQUIRE(ref.has_result());
                REQUIRE(corpus.print(ref.entry())() == ".qualified?:");
            }
        }

        SECTION("Ambiguous") {
            SECTION("Ambiguous symbol class") {
                auto r1 = corpus.resolve<point::clazz>({"path1", "ambiguous"}, root, true);
                auto r2 = corpus.resolve<point::clazz>({"path2", "ambiguous"}, root, true);

                auto ref = corpus.ref(R"(ambiguous?:)", root);

                REQUIRE(!ref.has_result());
            }
        }
    }

    // corpus.wipe();
}
