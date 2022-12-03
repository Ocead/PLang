//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/forms.hpp"
#include "lang/visitor.hpp"

using namespace plang::plot;
using namespace plang::lang::generated;

namespace plang::lang {

    class point_visitor : public virtual visitor {
        std::any visitPointClassName(PlangParser::PointClassNameContext *ctx) override {
            point_class_form form;
            form.path = std::any_cast<struct path_form>(visitPath(ctx->path()));

            return form;
        }

        std::any visitPointClass(PlangParser::PointClassContext *ctx) override {
            auto form      = std::any_cast<point_class_form>(visitPointClassName(ctx->pointClassName()));
            form.recursive = ctx->OP_RECUR();

            return form;
        }

        std::any visitPointClassSVODecl(PlangParser::PointClassSVODeclContext *ctx) override {
            auto form = std::any_cast<struct point_class_form>(visitPointClassName(ctx->pointClassName()));
            if (ctx->pointSingleton()) { form.singleton = true; }
            if (ctx->decoration()) {
                form.path.decoration = std::any_cast<decoration_form>(visitDecoration(ctx->decoration()));
            }
            if (ctx->objectDefaultClassDecl()) {
                form.object_classes.emplace_back(visitObjectDefaultClassDecl(ctx->objectDefaultClassDecl()));
            }
            for (auto const &e : ctx->objectInlineClassDecl()) {
                form.object_classes.emplace_back(visitObjectInlineClassDecl(e));
            }

            //TODO: handle requirements
            //TODO: handle implications

            any_vector classes;

            auto path = form_to_path(*corpus, *scope, form.path);
            if (!path.has_result()) { return path; }
            auto result = corpus->resolve({}, detail::corpus::tag<point::clazz>(), path.entry(), true);
            auto &clazz = std::get<point::clazz>(result.entry());
            clazz.set_singleton(form.singleton);

            if (ctx->hintSymbolClassList()) {
                form.hints = std::any_cast<any_vector>(visitHintSymbolClassList(ctx->hintSymbolClassList()));
                std::vector<point::clazz::hint> hints;

                for (auto const &e : form.hints) {
                    auto hint_form = std::any_cast<symbol_class_form>(e);
                    auto p         = form_to_path(*corpus, *scope, hint_form.path);
                    if (!p.has_result()) { return p; }
                    auto result2 = corpus->resolve({},
                                                   detail::corpus::tag<symbol::clazz>(),
                                                   std::get<class path>(p.entry()),
                                                   implicit);

                    classes.push_back(result2);

                    if (result2.action() == action::FAIL) {
                        //TODO: Throw runtime error
                    } else {
                        point::clazz::hint h{std::get<symbol::clazz>(result2.entry())};
                        h.set_recursive(hint_form.recursive);

                        hints.push_back(h);

                        for (auto const &e2 : hint_form.list) {
                            *hint_form.path.nodes.rbegin() = e2;
                            auto p2                        = form_to_path(*corpus, *scope, hint_form.path);
                            if (!p2.has_result()) { return p2; }
                            auto r2 = corpus->resolve({}, detail::corpus::tag<symbol::clazz>(), p2.entry(), implicit);
                            point::clazz::hint h2{std::get<symbol::clazz>(r2.entry())};
                            h2.set_recursive(hint_form.recursive);

                            hints.push_back(h2);
                            classes.push_back(r2);
                        }
                    }
                }

                clazz.get_hints() = hints;
            }
            corpus->update(clazz, true);

            auto object_classes = corpus->resolve<plot::object::clazz>({}, clazz, false, true).candidates();
            std::set<pkey<object::clazz>> reused;

            bool_t updated_object_classes = false;
            uint_t ordinal                = 0;
            for (auto const &c : form.object_classes) {
                auto oc_form = std::any_cast<object_class_form>(c);
                auto it      = std::find_if(object_classes.begin(), object_classes.end(), [&oc_form](auto const &c) {
                    return c.get_name() == oc_form.name->toString();
                });

                if (it != object_classes.end()) { reused.insert(it->get_id()); }

                auto o_class = (it != object_classes.end()) ? *it
                                                            : plot::object::clazz(clazz, oc_form.name->toString());
                o_class.set_ordinal(ordinal++);

                auto o_classes = update_object_class(*corpus, *scope, o_class, oc_form, implicit);

                action a;
                if (it != object_classes.end()) {
                    a = corpus->update(o_class, true);
                } else {
                    a = corpus->insert(o_class);
                }

                if (a != action::FAIL) {
                    if (a != action::NONE) { updated_object_classes = true; }
                    classes.emplace_back(resolve_entry_result{o_class, {}, a});
                    classes.insert(classes.end(),
                                   std::make_move_iterator(o_classes.begin()),
                                   std::make_move_iterator(o_classes.end()));
                };
            }

            for (auto &c : object_classes) {
                if (reused.find(c.get_id()) == reused.end()) {
                    //classes.emplace_back(resolve_entry_result{c, {}, action::REMOVE});
                    corpus->remove(c);
                }
            }

            classes.emplace(classes.begin(),
                            resolve_entry_result{result.entry(),
                                                 result.candidates(),
                                                 (result.action() == action::NONE && updated_object_classes)
                                                         ? action::UPDATE
                                                         : result.action()});
            return classes;
        }

        std::any visitPointClassRef(PlangParser::PointClassRefContext *ctx) override {
            auto form = std::any_cast<point_class_form>(visitPointClass(ctx->pointClass()));

            auto clazz = corpus->resolve(form_to_vector(form.path), detail::corpus::tag<point::clazz>(), *scope, false);
            return clazz;
        }
    };
}// namespace plang::lang
