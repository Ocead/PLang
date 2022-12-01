//
// Created by Johannes on 15.11.2022.
//

#include <set>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <plang/corpus.hpp>
#include "helper.hpp"

using namespace plang;
using namespace plang::plot;
using namespace plang::op;

TEST_CASE("2.3.1 Object Class API", "[api]") {
    auto &corpus = make_corpus();
    auto scope   = corpus.fetch<path>(0).value();

    auto direct   = corpus.resolve<point::clazz>({}, scope, true).entry();
    auto indirect = corpus.resolve<point::clazz>({"point", "class"}, scope, true).entry();

    SECTION("Resolve") {

        SECTION("via path") {

            SECTION("Direct existent object class") {
                corpus.resolve<object::clazz>({"object_class"}, indirect, true);

                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, scope, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "object_class");
                REQUIRE(r->get_point_class_id() == indirect.get_id());
            }

            SECTION("Indirect existent object class") {
                auto [_r, _c, _a] = corpus.resolve<object::clazz>({"object_class"}, indirect, true);

                auto [r, c, a] = corpus.resolve<object::clazz>({"point", "class", "object_class"}, scope, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "object_class");
                REQUIRE(r->get_point_class_id() == indirect.get_id());
            }

            SECTION("Direct inexistent object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Indirect inexistent object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"point", "clazz", "object_class"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Direct ambiguous object class") {
                auto o1 = corpus.resolve<object::clazz>({"object_class"}, direct, true);
                auto o2 = corpus.resolve<object::clazz>({"object_class"}, indirect, true);

                REQUIRE(o1.has_result());
                REQUIRE(o2.has_result());

                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Indirect ambiguous object class") {
                auto p  = corpus.resolve<point::clazz>({"schmoint", "class"}, scope, true);
                auto o1 = corpus.resolve<object::clazz>({"object_class"}, indirect, true);
                auto o2 = corpus.resolve<object::clazz>({"object_class"}, p.entry(), true);

                REQUIRE(o1.has_result());
                REQUIRE(o2.has_result());

                auto [r, c, a] = corpus.resolve<object::clazz>({"class", "object_class"}, scope, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
                REQUIRE(c.size() == 2);
            }

            SECTION("Direct inserting object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, scope, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Indirect inserting object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"point", "class", "object_class"}, scope, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }
        }

        SECTION("via point class") {
            SECTION("Existent object class") {
                auto [r1, c1, a1] = corpus.resolve<object::clazz>({"object_class"}, indirect, true);

                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, indirect, false);

                REQUIRE(a == action::NONE);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "object_class");
                REQUIRE(r->get_point_class_id() == indirect.get_id());
            }

            SECTION("Inexistent object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, indirect, false);

                REQUIRE(a == action::FAIL);
                REQUIRE(!r.has_value());
            }

            SECTION("Inserting object class") {
                auto [r, c, a] = corpus.resolve<object::clazz>({"object_class"}, indirect, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(r.has_value());
                REQUIRE(r->get_name() == "object_class");
                REQUIRE(r->get_point_class_id() == indirect.get_id());
            }
        }
    }

    SECTION("Fetch") {

        SECTION("Undecorated and unhinted") {
            auto undecorated = corpus.resolve<object::clazz>({"object_class"}, indirect, true).entry();
            REQUIRE(undecorated.is_persisted());

            SECTION("Undecorated and unhinted without dynamic") {
                auto r = corpus.fetch<object::clazz>(undecorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_point_class_id() == undecorated.get_point_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_ordinal() == undecorated.get_ordinal());
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_description() == undecorated.get_description());
                REQUIRE(r->get_hints().empty());
                REQUIRE(r->get_source_id() == undecorated.get_source_id());
            }

            SECTION("Undecorated and unhinted with dynamic") {
                auto r = corpus.fetch<object::clazz>(undecorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == undecorated.get_id());
                REQUIRE(r->get_name() == undecorated.get_name());
                REQUIRE(r->get_point_class_id() == undecorated.get_point_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().empty());
                REQUIRE(r->get_source_id() == undecorated.get_source_id());
            }
        }

        SECTION("Decorated") {
            auto decorated = corpus.resolve<object::clazz>({"object_class"}, indirect, true).entry();
            REQUIRE(decorated.is_persisted());
            decorated.set_ordinal(12.);
            decorated.set_description("Gnampf");
            corpus.update(decorated, true);

            SECTION("Decorated without dynamic") {
                auto r = corpus.fetch<object::clazz>(decorated.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_point_class_id() == decorated.get_point_class_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal());
                REQUIRE(r->get_ordinal() == 12.);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().empty());
                REQUIRE(r->get_source_id() == decorated.get_source_id());
            }

            SECTION("Decorated with dynamic") {
                auto r = corpus.fetch<object::clazz>(decorated.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == decorated.get_id());
                REQUIRE(r->get_name() == decorated.get_name());
                REQUIRE(r->get_point_class_id() == decorated.get_point_class_id());
                REQUIRE(r->get_ordinal() == decorated.get_ordinal());
                REQUIRE(r->get_ordinal() == 12.);
                REQUIRE(r->get_description() == decorated.get_description());
                REQUIRE(r->get_description() == "Gnampf");
                REQUIRE(r->get_hints().empty());
                REQUIRE(r->get_source_id() == decorated.get_source_id());
            }
        }

        SECTION("Hinted") {
            auto s_class  = corpus.resolve<symbol::clazz>({}, scope, true).entry();
            auto rs_class = corpus.resolve<symbol::clazz>({"recursive"}, scope, true).entry();
            auto p_class  = corpus.resolve<point::clazz>({}, scope, true).entry();
            auto rp_class = corpus.resolve<point::clazz>({"recursive"}, scope, true).entry();

            auto hinted = corpus.resolve<object::clazz>({"object_class"}, indirect, true).entry();
            REQUIRE(hinted.is_persisted());
            hinted.get_hints().emplace_back("Comment");
            hinted.get_hints().emplace_back("Like", object::clazz::hint::lit::type::LIKE);
            hinted.get_hints().emplace_back("Glob", object::clazz::hint::lit::type::GLOB);
            hinted.get_hints().emplace_back("Regex", object::clazz::hint::lit::type::REGEX);
            hinted.get_hints().emplace_back("Match", object::clazz::hint::lit::type::MATCH);
            hinted.get_hints().emplace_back(s_class, false);
            hinted.get_hints().emplace_back(rs_class, true);
            hinted.get_hints().emplace_back(p_class, false);
            hinted.get_hints().emplace_back(rp_class, true);
            corpus.update(hinted, true);

            SECTION("Hinted without dynamic") {
                auto r = corpus.fetch<object::clazz>(hinted.get_id(), false);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == hinted.get_id());
                REQUIRE(r->get_name() == hinted.get_name());
                REQUIRE(r->get_point_class_id() == hinted.get_point_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_ordinal() == hinted.get_ordinal());
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_description() == hinted.get_description());
                REQUIRE(r->get_hints().empty());
                REQUIRE(r->get_source_id() == hinted.get_source_id());
            }

            SECTION("Hinted with dynamic") {
                auto r = corpus.fetch<object::clazz>(hinted.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_id() == hinted.get_id());
                REQUIRE(r->get_name() == hinted.get_name());
                REQUIRE(r->get_point_class_id() == hinted.get_point_class_id());
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_ordinal() == hinted.get_ordinal());
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_description() == hinted.get_description());
                REQUIRE(r->get_hints().size() == hinted.get_hints().size());
                REQUIRE(r->get_hints().size() == 9);
                REQUIRE(r->get_source_id() == hinted.get_source_id());
            }
        }

        SECTION("Unpersisted") {
            auto r = corpus.fetch<object::clazz>(255, true);

            REQUIRE(!r.has_value());
        }
    }

    SECTION("Insert") {

        SECTION("Non-replacing") {

            SECTION("Single default object class") {
                object::clazz clazz{direct, "_", true};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == true);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Singleton object class") {
                object::clazz clazz{direct, "_", false, true};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == true);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Undecorated and unhinted unique object class") {
                object::clazz clazz{direct, "_"};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Decorated unique object class") {
                object::clazz clazz{direct, "_"};
                clazz.set_ordinal(7);
                clazz.set_description("Gnampf");

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_hints().size() == 0);
                REQUIRE(r->get_ordinal() == 7);
                REQUIRE(r->get_description() == "Gnampf");
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Hinted unique object class") {
                auto s_class  = corpus.resolve<symbol::clazz>({}, scope, true).entry();
                auto rs_class = corpus.resolve<symbol::clazz>({"recursive"}, scope, true).entry();
                auto p_class  = corpus.resolve<point::clazz>({}, scope, true).entry();
                auto rp_class = corpus.resolve<point::clazz>({"recursive"}, scope, true).entry();

                object::clazz clazz{direct, "_"};
                clazz.get_hints().emplace_back("Comment");
                clazz.get_hints().emplace_back("Like", object::clazz::hint::lit::type::LIKE);
                clazz.get_hints().emplace_back("Glob", object::clazz::hint::lit::type::GLOB);
                clazz.get_hints().emplace_back("Regex", object::clazz::hint::lit::type::REGEX);
                clazz.get_hints().emplace_back("Match", object::clazz::hint::lit::type::MATCH);
                clazz.get_hints().emplace_back(s_class, false);
                clazz.get_hints().emplace_back(rs_class, true);
                clazz.get_hints().emplace_back(p_class, false);
                clazz.get_hints().emplace_back(rp_class, true);

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 9);
            }

            SECTION("Second default object class") {
                object::clazz _{direct, "_", true};
                corpus.insert(_);

                object::clazz clazz{direct, "_2", true};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Duplicate object class") {
                object::clazz _{direct, "object_class", false};
                corpus.insert(_);

                object::clazz clazz{direct, "object_class", false};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Unpersisted point class") {
                point::clazz p_class{scope};

                object::clazz clazz{p_class, "_"};

                auto a = corpus.insert(clazz);

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Nameless object class") {
                object::clazz clazz{direct, ""};

                auto a = corpus.insert(clazz);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }
        }

        SECTION("Replacing") {

            SECTION("Single default object class") {
                object::clazz clazz{direct, "_", true};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == true);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Singleton object class") {
                object::clazz clazz{direct, "_", false, true};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == true);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Undecorated and unhinted unique object class") {
                object::clazz clazz{direct, "_"};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id());

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Decorated unique object class") {
                object::clazz clazz{direct, "_"};
                clazz.set_ordinal(7);
                clazz.set_description("Gnampf");

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_hints().size() == 0);
                REQUIRE(r->get_ordinal() == 7);
                REQUIRE(r->get_description() == "Gnampf");
                REQUIRE(r->get_hints().size() == 0);
            }

            SECTION("Hinted unique object class") {
                auto s_class  = corpus.resolve<symbol::clazz>({}, scope, true).entry();
                auto rs_class = corpus.resolve<symbol::clazz>({"recursive"}, scope, true).entry();
                auto p_class  = corpus.resolve<point::clazz>({}, scope, true).entry();
                auto rp_class = corpus.resolve<point::clazz>({"recursive"}, scope, true).entry();

                object::clazz clazz{direct, "_"};
                clazz.get_hints().emplace_back("Comment");
                clazz.get_hints().emplace_back("Like", object::clazz::hint::lit::type::LIKE);
                clazz.get_hints().emplace_back("Glob", object::clazz::hint::lit::type::GLOB);
                clazz.get_hints().emplace_back("Regex", object::clazz::hint::lit::type::REGEX);
                clazz.get_hints().emplace_back("Match", object::clazz::hint::lit::type::MATCH);
                clazz.get_hints().emplace_back(s_class, false);
                clazz.get_hints().emplace_back(rs_class, true);
                clazz.get_hints().emplace_back(p_class, false);
                clazz.get_hints().emplace_back(rp_class, true);

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::INSERT);
                REQUIRE(clazz.is_persisted());

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(r.has_value());
                REQUIRE(r->get_default() == false);
                REQUIRE(r->get_singleton() == false);
                REQUIRE(r->get_ordinal() == std::nullopt);
                REQUIRE(r->get_description() == std::nullopt);
                REQUIRE(r->get_hints().size() == 9);
            }

            SECTION("Second default object class") {
                object::clazz _{direct, "_", true};
                corpus.insert(_);

                object::clazz clazz{direct, "_2", true};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Duplicate object class") {
                object::clazz _{direct, "object_class", false};
                corpus.insert(_);

                object::clazz clazz{direct, "object_class", false};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Unpersisted point class") {
                point::clazz p_class{scope};

                object::clazz clazz{p_class, "_"};

                auto a = corpus.insert(clazz, true);

                auto r = corpus.fetch<object::clazz>(clazz.get_id(), true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }

            SECTION("Nameless object class") {
                object::clazz clazz{direct, ""};

                auto a = corpus.insert(clazz, true);

                REQUIRE(a == action::FAIL);
                REQUIRE(!clazz.is_persisted());
            }
        }
    }

    SECTION("Update") {
        auto [_r, _c, _a] = corpus.resolve<object::clazz>({"d"}, indirect, true);
        REQUIRE(_r.has_value());
        auto &expected = _r.value();

        SECTION("Persisted object class without dynamic") {

            SECTION("Add decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == 7.);
                REQUIRE(actual.get_description() == std::nullopt);
                REQUIRE(actual.get_default() == false);
                REQUIRE(actual.get_singleton() == false);
                REQUIRE(actual.get_hints().empty());
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
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_ordinal() == std::nullopt);
                REQUIRE(actual.get_description() == std::nullopt);
                REQUIRE(actual.get_default() == false);
                REQUIRE(actual.get_singleton() == false);
                REQUIRE(actual.get_hints().empty());
            }
        }

        SECTION("Persisted object class with dynamic") {

            SECTION("Add decoration") {
                expected.set_ordinal(7.);
                expected.set_description("Test");
                REQUIRE(corpus.update(expected, true) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();

                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_ordinal() == expected.get_ordinal());
                REQUIRE(actual.get_ordinal() == 7.);
                REQUIRE(actual.get_description() == expected.get_description());
                REQUIRE(actual.get_description() == "Test");
                REQUIRE(actual.get_default() == false);
                REQUIRE(actual.get_singleton() == false);
                REQUIRE(actual.get_hints().empty());
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
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_ordinal() == std::nullopt);
                REQUIRE(actual.get_description() == std::nullopt);
                REQUIRE(actual.get_default() == false);
                REQUIRE(actual.get_singleton() == false);
                REQUIRE(actual.get_hints().empty());
            }

            SECTION("Add hints") {

                SECTION("Add comment") {
                    expected.get_hints().emplace_back("Test");
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                    REQUIRE(std::holds_alternative<object::clazz::hint::lit>(actual.get_hints()[0]));

                    auto &hint = std::get<object::clazz::hint::lit>(actual.get_hints()[0]);
                    REQUIRE(hint.get_hint() == "Test");
                    REQUIRE(hint.get_type() == object::clazz::hint::lit::type::COMMENT);
                }

                SECTION("Add like filter") {
                    expected.get_hints().emplace_back("%Test%", object::clazz::hint::lit::type::LIKE);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                    REQUIRE(std::holds_alternative<object::clazz::hint::lit>(actual.get_hints()[0]));

                    auto &hint = std::get<object::clazz::hint::lit>(actual.get_hints()[0]);
                    REQUIRE(hint.get_hint() == "%Test%");
                    REQUIRE(hint.get_type() == object::clazz::hint::lit::type::LIKE);
                }

                SECTION("Add glob filter") {
                    expected.get_hints().emplace_back("Test*", object::clazz::hint::lit::type::GLOB);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                    REQUIRE(std::holds_alternative<object::clazz::hint::lit>(actual.get_hints()[0]));

                    auto &hint = std::get<object::clazz::hint::lit>(actual.get_hints()[0]);
                    REQUIRE(hint.get_hint() == "Test*");
                    REQUIRE(hint.get_type() == object::clazz::hint::lit::type::GLOB);
                }

                SECTION("Add regex filter") {
                    expected.get_hints().emplace_back("^Test$", object::clazz::hint::lit::type::REGEX);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                    REQUIRE(std::holds_alternative<object::clazz::hint::lit>(actual.get_hints()[0]));

                    auto &hint = std::get<object::clazz::hint::lit>(actual.get_hints()[0]);
                    REQUIRE(hint.get_hint() == "^Test$");
                    REQUIRE(hint.get_type() == object::clazz::hint::lit::type::REGEX);
                }

                SECTION("Add match filter") {
                    expected.get_hints().emplace_back("Test OR test", object::clazz::hint::lit::type::MATCH);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                    REQUIRE(std::holds_alternative<object::clazz::hint::lit>(actual.get_hints()[0]));

                    auto &hint = std::get<object::clazz::hint::lit>(actual.get_hints()[0]);
                    REQUIRE(hint.get_hint() == "Test OR test");
                    REQUIRE(hint.get_type() == object::clazz::hint::lit::type::MATCH);
                }

                SECTION("Add symbol class hint") {
                    auto s_clazz = corpus.resolve<symbol::clazz>({"symbol", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(s_clazz);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                }

                SECTION("Add recursive symbol class hint") {
                    auto s_clazz = corpus.resolve<symbol::clazz>({"recursive", "symbol", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(s_clazz, true);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                }

                SECTION("Add point class hint") {
                    auto p_clazz = corpus.resolve<point::clazz>({"point", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(p_clazz);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                }

                SECTION("Add recursive point class hint") {
                    auto p_clazz = corpus.resolve<point::clazz>({"recursive", "symbol", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(p_clazz, true);
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().size() == expected.get_hints().size());
                }
            }

            SECTION("Remove hints") {

                SECTION("Remove comment") {
                    expected.get_hints().emplace_back("Test");
                    corpus.update(expected, true);
                    expected.get_hints().clear();
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().empty());
                }

                SECTION("Remove symbol class hint") {
                    auto p_clazz = corpus.resolve<symbol::clazz>({"symbol", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(p_clazz);
                    corpus.update(expected, true);
                    expected.get_hints().clear();
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().empty());
                }

                SECTION("Remove point class hint") {
                    auto p_clazz = corpus.resolve<point::clazz>({"point", "class"}, scope, true).entry();
                    expected.get_hints().emplace_back(p_clazz);
                    corpus.update(expected, true);
                    expected.get_hints().clear();
                    corpus.update(expected, true);

                    auto actual = corpus.fetch(expected.get_id(), true).value();

                    REQUIRE(actual.get_hints().empty());
                }
            }
        }

        SECTION("Set as default") {

            SECTION("Without collision") {
                expected.set_default(true);
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();
                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_default() == expected.get_default());
                REQUIRE(actual.get_default() == true);
            }

            SECTION("With collision") {
                auto default_class = corpus.resolve<object::clazz>({"default"}, indirect, true).entry();
                default_class.set_default(true);
                corpus.update(default_class);

                expected.set_default(true);
                REQUIRE(corpus.update(expected, false) == action::FAIL);
            }
        }

        SECTION("Unset as default") {
            expected.set_default(false);
            REQUIRE(corpus.update(expected, false) == action::UPDATE);

            expected.set_default(false);
            REQUIRE(corpus.update(expected, false) == action::UPDATE);

            auto actual = corpus.fetch(expected.get_id(), true).value();
            REQUIRE(actual.get_id() == expected.get_id());
            REQUIRE(actual.get_name() == expected.get_name());
            REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
            REQUIRE(actual.get_default() == expected.get_default());
            REQUIRE(actual.get_default() == false);
        }

        SECTION("Set as singleton") {
            expected.set_singleton(true);
            REQUIRE(corpus.update(expected, false) == action::UPDATE);

            auto actual = corpus.fetch(expected.get_id(), true).value();
            REQUIRE(actual.get_id() == expected.get_id());
            REQUIRE(actual.get_name() == expected.get_name());
            REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
            REQUIRE(actual.get_singleton() == expected.get_singleton());
            REQUIRE(actual.get_singleton() == true);
        }

        SECTION("Unset as singleton") {
            expected.set_singleton(true);
            REQUIRE(corpus.update(expected, false) == action::UPDATE);

            expected.set_singleton(false);
            REQUIRE(corpus.update(expected, false) == action::UPDATE);

            auto actual = corpus.fetch(expected.get_id(), true).value();
            REQUIRE(actual.get_id() == expected.get_id());
            REQUIRE(actual.get_name() == expected.get_name());
            REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
            REQUIRE(actual.get_singleton() == expected.get_singleton());
            REQUIRE(actual.get_singleton() == false);
        }

        SECTION("Rename") {

            SECTION("Without collision") {
                expected.set_name("renamed");
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();
                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_name() == "renamed");
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
            }

            SECTION("With collision") {
                auto colliding = corpus.resolve<object::clazz>({"colliding"}, indirect, true).entry();

                expected.set_name("colliding");
                REQUIRE(corpus.update(expected, false) == action::FAIL);

                auto actual = corpus.resolve<object::clazz>({"colliding"}, indirect, true).entry();
                REQUIRE(actual.get_id() == colliding.get_id());
                REQUIRE(actual.get_name() == colliding.get_name());
                REQUIRE(actual.get_name() == "colliding");
                REQUIRE(actual.get_point_class_id() == colliding.get_point_class_id());
            }
        }

        SECTION("Move") {

            SECTION("Without collision") {
                expected.set_point_class(direct);
                REQUIRE(corpus.update(expected, false) == action::UPDATE);

                auto actual = corpus.fetch(expected.get_id(), true).value();
                REQUIRE(actual.get_id() == expected.get_id());
                REQUIRE(actual.get_name() == expected.get_name());
                REQUIRE(actual.get_point_class_id() == expected.get_point_class_id());
                REQUIRE(actual.get_point_class_id() == direct.get_id());
            }

            SECTION("With collision") {
                auto colliding = corpus.resolve<object::clazz>({expected.get_name()}, direct, true);
                REQUIRE(colliding.has_result());

                expected.set_point_class(direct);
                REQUIRE(corpus.update(expected, false) == action::FAIL);
            }
        }
    }

    SECTION("Print") {
        NOT_IMPLEMENTED();
    }

    SECTION("Remove") {
        auto clazz = corpus.resolve<object::clazz>({"object_class"}, indirect, true).entry();
        REQUIRE(clazz.is_persisted());

        SECTION("Non-cascading") {
            auto id = clazz.get_id();

            SECTION("Remove persisted object class without objects") {
                auto [repr, a] = corpus.remove(clazz, false);
                REQUIRE(repr == ".point.class?object_class");
                REQUIRE(a == action::REMOVE);

                REQUIRE(!corpus.fetch(id).has_value());
            }

            SECTION("Remove persisted object class with objects") {
                NOT_IMPLEMENTED();
            }
        }

        SECTION("Cascading") {
            auto id = clazz.get_id();

            SECTION("Remove persisted object class without objects") {
                auto [repr, a] = corpus.remove(clazz, true);
                REQUIRE(repr == ".point.class?object_class");
                REQUIRE(a == action::REMOVE);

                REQUIRE(!corpus.fetch(id).has_value());
            }

            SECTION("Remove persisted object class with objects") {
                NOT_IMPLEMENTED();
            }
        }
    }
}

TEST_CASE("2.3.2 Object Class Parser", "[parser]") {
    auto &corpus = make_corpus();
    auto scope   = corpus.fetch<path>(0).value();

    SECTION("Declarations") {

        SECTION("Within point classes") {
            SECTION("Default object class") {
                auto report = corpus.execute(R"(.point.class _;)", scope);

                auto &clazz = std::get<object::clazz>(
                        std::find_if(report.inserted().begin(), report.inserted().end(), [](auto const &e) {
                            return std::holds_alternative<object::clazz>(std::get<entry>(e));
                        })->second);
                REQUIRE(report.inserted().size() == 2);
                REQUIRE(clazz.get_hints().empty());
            }

            SECTION("Singleton object class") {
                auto report = corpus.execute(R"(.point.class ! ?_;)", scope);

                auto &clazz = std::get<object::clazz>(
                        std::find_if(report.inserted().begin(), report.inserted().end(), [](auto const &e) {
                            return std::holds_alternative<object::clazz>(std::get<entry>(e));
                        })->second);
                REQUIRE(report.inserted().size() == 2);
                REQUIRE(clazz.get_hints().empty());
            }

            SECTION("Unhinted object class") {
                auto report = corpus.execute(R"(.point.class ?_;)", scope);

                auto &clazz = std::get<object::clazz>(
                        std::find_if(report.inserted().begin(), report.inserted().end(), [](auto const &e) {
                            return std::holds_alternative<object::clazz>(std::get<entry>(e));
                        })->second);
                REQUIRE(report.inserted().size() == 2);
                REQUIRE(clazz.get_hints().empty());
            }

            SECTION("Unhinted object class with persisted hints") {
                corpus.execute(R"(.point.class ?_ : ("Test");)", scope);

                auto report = corpus.execute(R"(.point.class ?_;)", scope);

                auto &clazz = std::get<object::clazz>(
                        std::find_if(report.updated().begin(), report.updated().end(), [](auto const &e) {
                            return std::holds_alternative<object::clazz>(std::get<entry>(e));
                        })->second);
                REQUIRE(report.inserted().size() == 0);
                REQUIRE(!clazz.get_hints().empty());
            }

            SECTION("Hinted object class with empty hint list") {
                auto report = corpus.execute(R"(.point.class ?_: ();)", scope);

                auto &clazz = std::get<object::clazz>(
                        std::find_if(report.inserted().begin(), report.inserted().end(), [](auto const &e) {
                            return std::holds_alternative<object::clazz>(std::get<entry>(e));
                        })->second);
                REQUIRE(report.inserted().size() == 2);
                REQUIRE(clazz.get_hints().empty());
            }

            SECTION("Hinted object class") {
                auto report = corpus.execute(R"(.point.class ?_: ("Comment",
                                                                 l"Like filter",
                                                                 g"Glob filter",
                                                                 r"Regex filter",
                                                                 m"Match filter",
                                                                 .symbol.class[],
                                                                 .recursive.symbol.class[]...,
                                                                 .point.class?:,
                                                                 .recursive.point.class?:...);)",
                                             scope);
                auto &clazz = std::find_if(report.inserted().begin(), report.inserted().end(), [](auto const &e) {
                                  return std::holds_alternative<object::clazz>(std::get<entry>(e));
                              })->second;
                REQUIRE(report.inserted().size() == 5);

                auto actual = print_to_set(corpus, report.inserted());

                REQUIRE(actual == std::set<string_t>({".point.class?:",
                                                      ".recursive.point.class?:",
                                                      ".point.class?_",
                                                      ".symbol.class[]",
                                                      ".recursive.symbol.class[]"}));

                auto decl_format = corpus.get_format();
                decl_format.set_detail(format::detail::DEFINITION);
                string_t decl_str = corpus.print(clazz, decl_format);
                REQUIRE(decl_str ==
                        R"(.point.class?_ : ("Comment", l"Like filter", g"Glob filter", r"Regex filter", )"
                        R"(m"Match filter", .symbol.class[], .recursive.symbol.class[]..., .point.class?:, )"
                        R"(.recursive.point.class?:...);)");
            }
        }

        SECTION("Separate") {
            auto p_class = corpus.resolve<point::clazz>({"point", "class"}, scope, true).entry();
            auto o_class = corpus.resolve<object::clazz>({"object_class"}, p_class, true).entry();

            SECTION("Unhinted object class") {
                auto report = corpus.execute(R"(.point.class?object_class;)", scope);

                REQUIRE(report.mentioned().size() == 1);
            }

            SECTION("Unhinted object class with persisted hints") {
                corpus.execute(R"(.point.class?object_class : (r"^Test$");)", scope);
                auto report = corpus.execute(R"(.point.class?object_class;)", scope);

                REQUIRE(report.mentioned().size() == 1);
                REQUIRE(std::get<object::clazz>(report.updated().begin()->second).get_hints().size() == 1);
            }

            SECTION("Hinted object class with empty hint list") {
                auto report = corpus.execute(R"(.point.class?object_class : ();)", scope);

                REQUIRE(report.updated().size() == 1);
                REQUIRE(std::get<object::clazz>(report.updated().begin()->second).get_hints().empty());
            }

            SECTION("Hinted object class") {
                auto report = corpus.execute(R"(.point.class?object_class : ("Test");)", scope);

                REQUIRE(report.updated().size() == 1);
                REQUIRE(std::get<object::clazz>(report.updated().begin()->second).get_hints().size() == 1);
            }
        }
    }

    SECTION("References") {
        auto pc1 = corpus.resolve<point::clazz>({"point", "class"}, scope, true).entry();
        auto pc2 = corpus.resolve<point::clazz>({"schmoint", "class"}, scope, true).entry();
        auto oc1 = corpus.resolve<object::clazz>({"object_class"}, pc1, true).entry();

        SECTION("Valid") {

            SECTION("Unqualified") {
                auto [r, c, a] = corpus.ref("?object_class", scope);

                REQUIRE(std::holds_alternative<object::clazz>(r));
                REQUIRE(std::get<object::clazz>(r).get_id() == oc1.get_id());
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
            }

            SECTION("Partially qualified") {
                auto [r, c, a] = corpus.ref("class?object_class", scope);

                REQUIRE(std::holds_alternative<object::clazz>(r));
                REQUIRE(std::get<object::clazz>(r).get_id() == oc1.get_id());
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
            }

            SECTION("Qualified") {
                auto [r, c, a] = corpus.ref(".point.class?object_class", scope);

                REQUIRE(std::holds_alternative<object::clazz>(r));
                REQUIRE(std::get<object::clazz>(r).get_id() == oc1.get_id());
                REQUIRE(c.empty());
                REQUIRE(a == action::NONE);
            }
        }

        SECTION("Ambiguous") {
            auto oc2 = corpus.resolve<object::clazz>({"object_class"}, pc2, true).entry();

            SECTION("Unqualified") {
                auto [r, c, a] = corpus.ref("?object_class", scope);

                REQUIRE(std::holds_alternative<std::monostate>(r));
                REQUIRE(c.size() == 2);
                REQUIRE(a == action::FAIL);
            }

            SECTION("Partially qualified") {
                auto [r, c, a] = corpus.ref("class?object_class", scope);

                REQUIRE(std::holds_alternative<std::monostate>(r));
                REQUIRE(c.size() == 2);
                REQUIRE(a == action::FAIL);
            }
        }

        SECTION("Invalid") {
            auto [r, c, a] = corpus.ref(".invalid.point.class?object_class", scope);

            REQUIRE(std::holds_alternative<std::monostate>(r));
            REQUIRE(c.size() == 1);
            REQUIRE(std::get<object::clazz>(c[0]).get_id() == oc1.get_id());
            REQUIRE(a == action::FAIL);
        }
    }
}
