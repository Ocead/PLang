//
// Created by Johannes on 19.08.2022.
//

#include <set>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <plang/corpus.hpp>
#include "helper.hpp"

using namespace plang;
using namespace plang::op;

TEST_CASE("1.1.1. Path API", "[api]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<path>(false).size();
    REQUIRE(count == 1);

    auto root = corpus.fetch<path>(0).value();

    SECTION("Resolve") {

        SECTION("Root-scoped") {
            auto &ctx = root;

            SECTION("Empty node list") {
                auto [r, c, a] = corpus.resolve<path>({}, ctx, true);
                auto &n        = r.value();

                REQUIRE(a == action::NONE);
                REQUIRE(n.is_persisted());
                REQUIRE(n.is_root());
                REQUIRE(corpus.print(n)() == ".");
            }

            SECTION("Unambiguous persisted node list") {
                auto [m, co, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [r, rc, a]  = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto &o          = m.value();
                auto &n          = r.value();

                REQUIRE(a == action::NONE);
                REQUIRE(n.is_persisted());
                REQUIRE(n.get_name() == "def");
                REQUIRE(n.get_id() == o.get_id());
                REQUIRE(!n.is_root());
                REQUIRE(corpus.print(n)() == ".abc.def");
            }

            SECTION("Unambiguous unpersisted node list") {
                auto [r, c, a] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto &n        = r.value();

                REQUIRE(a == action::INSERT);
                REQUIRE(n.is_persisted());
                REQUIRE(n.get_name() == "def");
                REQUIRE(!n.is_root());
                REQUIRE(corpus.print(n)() == ".abc.def");
            }

            SECTION("Ambiguous persisted node list") {
                auto [l1, c1, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [l2, c2, a2] = corpus.resolve<path>({"ghi", "def"}, ctx, true);
                auto [r, c, ac]   = corpus.resolve<path>({"def"}, ctx, true);

                REQUIRE(ac == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> a{".abc.def", ".ghi.def"};
                std::set<string_t> b{corpus.print(c[0]), corpus.print(c[1])};
                REQUIRE(a == b);
            }

            SECTION("Ambiguous unpersisted node list") {
                auto [l1, c1, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [l2, c2, a2] = corpus.resolve<path>({"ghi", "def"}, ctx, true);
                auto [r, c, ac]   = corpus.resolve<path>({"def"}, ctx, true);

                REQUIRE(ac == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> a{".abc.def", ".ghi.def"};
                std::set<string_t> b{corpus.print(c[0]), corpus.print(c[1])};
                REQUIRE(a == b);
            }
        }

        SECTION("Non-root-scoped") {
            path ctx = corpus.resolve<path>({"mno"}, root, true).entry();

            SECTION("Empty node list") {
                auto [r, c, a] = corpus.resolve<path>({}, ctx, true);
                auto &n        = r.value();

                REQUIRE(a == action::NONE);
                REQUIRE(n.is_persisted());
                REQUIRE(!n.is_root());
                REQUIRE(corpus.print(n)() == ".mno");
            }

            SECTION("Unambiguous persisted node list") {
                auto [m, co, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [r, rc, a]  = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto &o          = m.value();
                auto &n          = r.value();

                REQUIRE(a == action::NONE);
                REQUIRE(n.is_persisted());
                REQUIRE(n.get_name() == "def");
                REQUIRE(n.get_id() == o.get_id());
                REQUIRE(!n.is_root());
                REQUIRE(corpus.print(n)() == ".mno.abc.def");
            }

            SECTION("Unambiguous unpersisted node list") {
                auto [r, c, a] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto &n        = r.value();

                REQUIRE(a == action::INSERT);
                REQUIRE(n.is_persisted());
                REQUIRE(n.get_name() == "def");
                REQUIRE(!n.is_root());
                REQUIRE(corpus.print(n)() == ".mno.abc.def");
            }

            SECTION("Ambiguous persisted node list") {
                auto [l1, c1, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [l2, c2, a2] = corpus.resolve<path>({"ghi", "def"}, ctx, true);
                auto [r, c, ac]   = corpus.resolve<path>({"def"}, ctx, true);

                REQUIRE(ac == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> a{".mno.abc.def", ".mno.ghi.def"};
                std::set<string_t> b{corpus.print(c[0]), corpus.print(c[1])};
                REQUIRE(a == b);
            }

            SECTION("Ambiguous unpersisted node list") {
                auto [l1, c1, a1] = corpus.resolve<path>({"abc", "def"}, ctx, true);
                auto [l2, c2, a2] = corpus.resolve<path>({"ghi", "def"}, ctx, true);
                auto [r, c, ac]   = corpus.resolve<path>({"def"}, ctx, true);

                REQUIRE(ac == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
                std::set<string_t> a{".mno.abc.def", ".mno.ghi.def"};
                std::set<string_t> b{corpus.print(c[0]), corpus.print(c[1])};
                REQUIRE(a == b);
            }
        }
    }

    SECTION("Fetch") {
        SECTION("Undecorated") {
            path undecorated{"undecorated", root};
            undecorated.set_ordinal();
            undecorated.set_description();
            corpus.insert(undecorated);

            REQUIRE(undecorated.is_persisted());

            SECTION("Persisted undecorated path without texts") {
                auto r = corpus.fetch<path>(undecorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_parent_id() == undecorated.get_parent_id());
                REQUIRE(!r->get_ordinal().has_value());
                REQUIRE(!r->get_description().has_value());
            }

            SECTION("Persisted undecorated path with texts") {
                auto r = corpus.fetch<path>(undecorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_parent_id() == undecorated.get_parent_id());
                REQUIRE(!r->get_ordinal().has_value());
                REQUIRE(!r->get_description().has_value());
            }
        }

        SECTION("Decorated") {
            path decorated{"decorated", root};
            decorated.set_ordinal(5);
            decorated.set_description("abcdef");
            corpus.insert(decorated);

            REQUIRE(decorated.is_persisted());

            SECTION("Persisted decorated path without texts") {
                auto r = corpus.fetch<path>(decorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_parent_id() == decorated.get_parent_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal().value());
                REQUIRE(!r->get_description().has_value());
            }

            SECTION("Persisted decorated path with texts") {
                auto r = corpus.fetch<path>(decorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_parent_id() == decorated.get_parent_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal().value());
                REQUIRE(r->get_description() == decorated.get_description().value());
            }
        }

        SECTION("Unpersisted path") {
            auto r = corpus.fetch<path>(root.get_id() + 1);

            REQUIRE(!r.has_value());
        }
    }

    SECTION("Insert") {
        SECTION("Non-replacing") {
            SECTION("Undecorated unique path") {
                path r1{"abc", root};

                REQUIRE_NOTHROW(corpus.insert(r1, false));

                auto r = corpus.fetch<path>(r1.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "abc");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(!r.get_ordinal().has_value());
                REQUIRE(!r.get_description().has_value());
            }

            SECTION("Decorated unique path") {
                path r1{"abc", root};
                r1.set_ordinal(5);
                r1.set_description("Test");

                REQUIRE_NOTHROW(corpus.insert(r1, false));

                auto r = corpus.fetch<path>(r1.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "abc");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == 5);
                REQUIRE(r.get_description() == "Test");
            }

            SECTION("Duplicate path") {
                auto [r1, c, a] = corpus.resolve<path>({"abc"}, root, true);
                path r{"abc", root};

                REQUIRE_NOTHROW(corpus.insert(r, false));

                REQUIRE(!r.is_persisted());
            }

            SECTION("Unpersisted parent") {
                path p{"abc", root};
                path r{"def", p};

                REQUIRE_NOTHROW(corpus.insert(r, false));

                REQUIRE(!r.is_persisted());
            }

            SECTION("Nameless path") {
                path r{"", root};

                REQUIRE_NOTHROW(corpus.insert(r, false));

                REQUIRE(!r.is_persisted());
            }
        }

        SECTION("Replacing") {
            SECTION("Undecorated unique path") {
                path r1{"abc", root};

                REQUIRE_NOTHROW(corpus.insert(r1, true));

                auto r = corpus.fetch<path>(r1.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "abc");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(!r.get_ordinal().has_value());
                REQUIRE(!r.get_description().has_value());
            }

            SECTION("Decorated unique path") {
                path r1{"abc", root};
                r1.set_ordinal(5);
                r1.set_description("Test");

                REQUIRE_NOTHROW(corpus.insert(r1, true));

                auto r = corpus.fetch<path>(r1.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "abc");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == 5);
                REQUIRE(r.get_description() == "Test");
            }

            SECTION("Duplicate path") {
                auto [r2, c, a] = corpus.resolve<path>({"abc"}, root, true);
                path r1{"abc", root};
                r1.set_ordinal(5);
                r1.set_description("Test");

                REQUIRE_NOTHROW(corpus.insert(r1, true));

                REQUIRE(r1.is_persisted());
                auto r = corpus.fetch<path>(r1.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "abc");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == 5);
                REQUIRE(r.get_description() == "Test");
            }

            SECTION("Unpersisted parent") {
                path p{"abc", root};
                path r{"def", p};

                REQUIRE_NOTHROW(corpus.insert(r, true));

                REQUIRE(!r.is_persisted());
            }

            SECTION("Nameless path") {
                path r{"", root};

                REQUIRE_NOTHROW(corpus.insert(r, true));

                REQUIRE(!r.is_persisted());
            }
        }
    }

    SECTION("Update") {
        path decorated{"decorated", root};
        decorated.set_ordinal(5);
        decorated.set_description("Test");
        path undecorated{"undecorated", root};
        corpus.insert(decorated);
        corpus.insert(undecorated);

        SECTION("Persisted path without texts") {
            SECTION("Add decoration") {
                undecorated.set_ordinal(5);
                undecorated.set_description("Test");

                REQUIRE_NOTHROW(corpus.update(undecorated, false));

                auto r = corpus.fetch<path>(undecorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "undecorated");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == 5);
                REQUIRE(!r.get_description().has_value());
            }

            SECTION("Remove decoration") {
                decorated.set_ordinal();
                decorated.set_description();

                REQUIRE_NOTHROW(corpus.update(decorated, false));

                auto r = corpus.fetch<path>(decorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "decorated");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(!r.get_ordinal().has_value());
                REQUIRE(r.get_description() == "Test");
            }
        }

        SECTION("Persisted path with texts") {
            SECTION("Add decoration") {
                undecorated.set_ordinal(5);
                undecorated.set_description("Test");

                REQUIRE_NOTHROW(corpus.update(undecorated, true));

                auto r = corpus.fetch<path>(undecorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "undecorated");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == 5);
                REQUIRE(r.get_description() == "Test");
            }

            SECTION("Remove decoration") {
                decorated.set_ordinal();
                decorated.set_description();

                REQUIRE_NOTHROW(corpus.update(decorated, true));

                auto r = corpus.fetch<path>(decorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "decorated");
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(!r.get_ordinal().has_value());
                REQUIRE(!r.get_description().has_value());
            }
        }

        SECTION("Rename") {
            SECTION("Unique name") {
                undecorated.set_name("renamed");

                REQUIRE_NOTHROW(corpus.update(undecorated));

                auto r = corpus.fetch<path>(undecorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == "renamed");
                REQUIRE(r.get_parent_id() == undecorated.get_parent_id());
                REQUIRE(r.get_ordinal() == undecorated.get_ordinal());
                REQUIRE(r.get_description() == undecorated.get_description());
            }

            SECTION("Duplicate name") {
                undecorated.set_name("decorated");

                REQUIRE_THROWS(corpus.update(undecorated));
            }
        }

        SECTION("Move") {
            SECTION("Unique path") {
                decorated.set_parent(undecorated);

                REQUIRE_NOTHROW(corpus.update(decorated));

                auto r = corpus.fetch<path>(decorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == decorated.get_name());
                REQUIRE(r.get_parent_id() == undecorated.get_id());
                REQUIRE(r.get_ordinal() == decorated.get_ordinal());
                REQUIRE(r.get_description() == decorated.get_description());
            }

            SECTION("Duplicate path") {
                corpus.resolve<path>({"", "undecorated", "decorated"}, root, true);
                decorated.set_parent(undecorated);

                REQUIRE_THROWS(corpus.update(decorated));

                auto r = corpus.fetch<path>(decorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == decorated.get_name());
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == decorated.get_ordinal());
                REQUIRE(r.get_description() == decorated.get_description());
            }

            SECTION("Cyclic path") {
                decorated.set_parent(decorated);

                REQUIRE_THROWS(corpus.update(decorated));

                auto r = corpus.fetch<path>(decorated.get_id(), true).value();
                REQUIRE(r.is_persisted());
                REQUIRE(r.get_name() == decorated.get_name());
                REQUIRE(r.get_parent_id() == root.get_id());
                REQUIRE(r.get_ordinal() == decorated.get_ordinal());
                REQUIRE(r.get_description() == decorated.get_description());
            }
        }

        SECTION("Unpersisted path") {
            path r{"unpersisted", root};

            REQUIRE_THROWS(corpus.update(r));
        }
    }

    SECTION("Print") {
        //TODO: Implement mechanism for shorter unique paths first
        //TODO: Add tests for applied formatting

        SECTION("Qualified paths") {
            SECTION("Root path") {
                NOT_IMPLEMENTED();
            }

            SECTION("Single-node path") {
                NOT_IMPLEMENTED();
            }

            SECTION("Multi-node path") {
                NOT_IMPLEMENTED();
            }
        }

        SECTION("Unique paths") {
            SECTION("Single-node path") {
                NOT_IMPLEMENTED();
            }

            SECTION("Multi-node path") {
                NOT_IMPLEMENTED();
            }
        }
    }

    SECTION("Remove") {
        SECTION("Non-cascading") {
            SECTION("Remove persisted path without children") {
                auto r  = corpus.resolve<path>({"abc"}, root, true).entry();
                auto id = r.get_id();

                auto [repr, action] = corpus.remove(r, false);

                REQUIRE(repr == ".abc");
                REQUIRE(action == action::REMOVE);
                REQUIRE(!corpus.fetch<path>(id).has_value());
            }

            SECTION("Remove persisted path with children") {
                auto r1 = corpus.resolve<path>({"abc", "def"}, root, true).entry();
                auto id = r1.get_parent_id();
                auto r  = corpus.fetch<path>(id).value();

                auto [repr, action] = corpus.remove(r, false);

                REQUIRE(repr == ".abc");
                REQUIRE(action == action::FAIL);
                REQUIRE(corpus.fetch<path>(id).has_value());
            }

            SECTION("Remove root path without children") {
                REQUIRE_THROWS(corpus.remove(root, false));
            }

            SECTION("Remove root path with children") {
                corpus.resolve<path>({"abc"}, root, true);
                REQUIRE_THROWS(corpus.remove(root, false));
            }
        }

        SECTION("Cascading") {
            SECTION("Remove persisted path without children") {
                auto r  = corpus.resolve<path>({"abc"}, root, true).entry();
                auto id = r.get_id();

                auto [repr, action] = corpus.remove(r, true);

                REQUIRE(repr == ".abc");
                REQUIRE(action == action::REMOVE);
                REQUIRE(!corpus.fetch<path>(id).has_value());
            }

            SECTION("Remove persisted path with children") {
                auto r1 = corpus.resolve<path>({"abc", "def"}, root, true).entry();
                auto id = r1.get_parent_id();
                auto r  = corpus.fetch<path>(id).value();

                auto [repr, action] = corpus.remove(r, true);

                REQUIRE(repr == ".abc");
                REQUIRE(action == action::REMOVE);
                REQUIRE(!corpus.fetch<path>(id).has_value());
            }

            SECTION("Remove root path without children") {
                REQUIRE_THROWS(corpus.remove(root, true));
            }

            SECTION("Remove root path with children") {
                corpus.resolve<path>({"abc"}, root, true);
                REQUIRE_THROWS(corpus.remove(root, true));
            }
        }
    }

    //corpus.wipe();
}

TEST_CASE("1.1.2. Path Parser", "[parser]") {
    auto &corpus = make_corpus();

    auto count = corpus.fetch_all<path>(false).size();
    REQUIRE(count == 1);

    auto root = corpus.fetch<path>(0).value();

    SECTION("Declarations") {
        SECTION("Valid") {
            SECTION("Qualified root path") {
                auto report = corpus.decl(R"(.;)", root);

                REQUIRE(report.mentioned().size() == 1);
                REQUIRE(report.inserted().size() == 0);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Unqualified single-node path") {
                auto report = corpus.decl(R"(unqualified;)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Unqualified multi-node path") {
                auto report = corpus.decl(R"(unqualified.path;)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Qualified single-node path") {
                auto report = corpus.decl(R"(.qualified;)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Qualified multi-node path") {
                auto report = corpus.decl(R"(.qualified.path;)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Decorated qualified root path") {
                auto report = corpus.decl(R"(. (0, "Test");)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 0);
                REQUIRE(report.updated().size() == 1);
                //TODO: Implement
            }

            SECTION("Decorated unqualified single-node path") {
                auto report = corpus.decl(R"(unqualified (1, "Test");)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Decorated unqualified multi-node path") {
                auto report = corpus.decl(R"(unqualified.path (-1, "Test");)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Decorated qualified single-node path") {
                auto report = corpus.decl(R"(.qualified (.1, "Test");)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }

            SECTION("Decorated qualified multi-node path") {
                auto report = corpus.decl(R"(.qualified.path (-1., "Test");)", root);

                REQUIRE(report.mentioned().size() == 0);
                REQUIRE(report.inserted().size() == 1);
                REQUIRE(report.updated().size() == 0);
                //TODO: Implement
            }
        }

        SECTION("Ambiguous") {
            auto [n1, c1, a1] = corpus.resolve<path>({"a", "unqualified"}, root, true);
            auto [n2, c2, a2] = corpus.resolve<path>({"b", "unqualified"}, root, true);

            REQUIRE(n1.has_value());
            REQUIRE(n1->is_persisted());
            REQUIRE(n2.has_value());
            REQUIRE(n2->is_persisted());

            SECTION("Unqualified single-node path") {
                auto report = corpus.decl(R"(unqualified;)", root);

                REQUIRE(report.failed().size() == 1);

                auto &vec = report.failed().begin()->second;
                std::set<string_t> set;
                for (auto const &e : vec) { set.insert(corpus.print(e)); }
                REQUIRE(set == std::set<string_t>{".a.unqualified", ".b.unqualified"});
            }

            SECTION("Unqualified multi-node path") {
                auto report = corpus.decl(R"(unqualified.path;)", root);

                REQUIRE(report.failed().size() == 1);

                auto &vec = report.failed().begin()->second;
                std::set<string_t> set;
                for (auto const &e : vec) { set.insert(corpus.print(e)); }
                REQUIRE(set == std::set<string_t>{".a.unqualified", ".b.unqualified"});
            }

            SECTION("Decorated unqualified single-node path") {
                auto report = corpus.decl(R"(unqualified (1, "Test");)", root);

                REQUIRE(report.failed().size() == 1);

                auto &vec = report.failed().begin()->second;
                std::set<string_t> set;
                for (auto const &e : vec) { set.insert(corpus.print(e)); }
                REQUIRE(set == std::set<string_t>{".a.unqualified", ".b.unqualified"});
            }

            SECTION("Decorated unqualified multi-node path") {
                auto report = corpus.decl(R"(unqualified.path (-1, "Test");)", root);

                REQUIRE(report.failed().size() == 1);

                auto &vec = report.failed().begin()->second;
                std::set<string_t> set;
                for (auto const &e : vec) { set.insert(corpus.print(e)); }
                REQUIRE(set == std::set<string_t>{".a.unqualified", ".b.unqualified"});
            }
        }
    }

    SECTION("References") {
        SECTION("Valid") {
            SECTION("Qualified root path") {
                auto result = corpus.ref(R"(.;)", root);

                REQUIRE(corpus.print(result.entry())() == ".");
            }

            SECTION("Unqualified single-node path") {
                corpus.resolve<path>({"unqualified"}, root, true);
                auto result = corpus.ref(R"(unqualified;)", root);

                REQUIRE(corpus.print(result.entry())() == ".unqualified");
            }

            SECTION("Unqualified multi-node path") {
                corpus.resolve<path>({"unqualified", "path"}, root, true);
                auto result = corpus.ref(R"(unqualified.path;)", root);

                REQUIRE(corpus.print(result.entry())() == ".unqualified.path");
            }

            SECTION("Qualified single-node path") {
                corpus.resolve<path>({"qualified"}, root, true);
                auto result = corpus.ref(R"(.qualified;)", root);

                REQUIRE(corpus.print(result.entry())() == ".qualified");
            }

            SECTION("Qualified multi-node path") {
                corpus.resolve<path>({"qualified", "path"}, root, true);
                auto result = corpus.ref(R"(.qualified.path;)", root);

                REQUIRE(corpus.print(result.entry())() == ".qualified.path");
            }
        }

        SECTION("Ambiguous") {
            auto [n1, c1, a1] = corpus.resolve<path>({"a", "unqualified"}, root, true);
            auto [n2, c2, a2] = corpus.resolve<path>({"b", "unqualified"}, root, true);

            SECTION("Unqualified single-node path") {
                auto result = corpus.ref(R"(unqualified;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().size() == 2);
            }

            SECTION("Unqualified multi-node path") {
                auto result = corpus.ref(R"(unqualified.path;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().size() == 2);
            }
        }

        SECTION("Invalid") {
            SECTION("Unqualified single-node path") {
                auto result = corpus.ref(R"(unqualified;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().empty());
            }

            SECTION("Unqualified multi-node path") {
                auto result = corpus.ref(R"(unqualified.path;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().empty());
            }

            SECTION("Qualified single-node path") {
                auto result = corpus.ref(R"(.qualified;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().empty());
            }

            SECTION("Qualified multi-node path") {
                auto result = corpus.ref(R"(.qualified.path;)", root);

                REQUIRE(!result.has_result());
                REQUIRE(result.candidates().empty());
            }
        }
    }

    //corpus.wipe();
}
