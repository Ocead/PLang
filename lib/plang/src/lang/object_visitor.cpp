//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/forms.hpp"
#include "lang/visitor.hpp"

using namespace plang::plot;
using namespace plang::lang::generated;

namespace plang::lang {

    class object_visitor : public virtual visitor {
        std::any visitObjectClassName(PlangParser::ObjectClassNameContext *ctx) override {
            object_class_form form;
            form.name  = ctx->IDENTIFIER();
            form.local = false;
            return form;
        }

        std::any visitObjectLocalClassName(PlangParser::ObjectLocalClassNameContext *ctx) override {
            object_class_form form;
            form.name  = ctx->STRING();
            form.local = true;
            return form;
        }

        std::any visitObjectClassInlineRef(PlangParser::ObjectClassInlineRefContext *ctx) override {
            if (ctx->objectClassName()) {
                return visitObjectClassName(ctx->objectClassName());
            } else {
                return visitObjectLocalClassName(ctx->objectLocalClassName());
            }
        }

        std::any visitObjectUnqualifiedClass(PlangParser::ObjectUnqualifiedClassContext *ctx) override {
            auto form = std::any_cast<object_class_form>(visitObjectClassInlineRef(ctx->objectClassInlineRef()));
            if (ctx->pathUnqualifiedPath()) {
                form.path = std::any_cast<path_form>(visitPathUnqualifiedPath(ctx->pathUnqualifiedPath()));
            }

            return form;
        }

        std::any visitObjectQualifiedClass(PlangParser::ObjectQualifiedClassContext *ctx) override {
            auto form = std::any_cast<object_class_form>(visitObjectClassInlineRef(ctx->objectClassInlineRef()));
            form.path = std::any_cast<path_form>(visitPathQualifiedPath(ctx->pathQualifiedPath()));

            return form;
        }

        std::any visitObjectDefaultClass(PlangParser::ObjectDefaultClassContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(visitObjectClassName(ctx->objectClassName()));
            form._default          = true;

            return form;
        }

        std::any visitObjectInlineClass(PlangParser::ObjectInlineClassContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(
                    visitObjectClassInlineRef(ctx->objectClassInlineRef()));
            form._default = false;

            return form;
        }

        std::any visitObjectClass(PlangParser::ObjectClassContext *ctx) override {
            if (ctx->objectUnqualifiedClass()) {
                return visitObjectUnqualifiedClass(ctx->objectUnqualifiedClass());
            } else {
                return visitObjectQualifiedClass(ctx->objectQualifiedClass());
            }
        }

        std::any visitObjectClassDef(PlangParser::ObjectClassDefContext *ctx) override {
            if (ctx->hintList()) {
                return visitHintList(ctx->hintList());
            } else {
                return any_vector();
            }
        }

        std::any visitObjectClassDecl(PlangParser::ObjectClassDeclContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(visitObjectClass(ctx->objectClass()));

            form.singleton = ctx->objectSingleton();
            form.hinted    = ctx->objectClassDef();
            if (ctx->objectClassDef()) {
                form.hints = std::any_cast<any_vector>(visitObjectClassDef(ctx->objectClassDef()));
            }

            auto path = form_to_vector(form.path);
            path.emplace_back(form.name->toString());

            auto [r, c, a] = corpus->resolve(path, detail::corpus::tag<plot::object::clazz>(), *scope, true);
            if (!std::holds_alternative<object::clazz>(r)) {
                //TODO: throw exception
            }
            auto &clazz = std::get<plot::object::clazz>(r);
            clazz.set_singleton(form.singleton);

            auto classes = update_object_class(*corpus, *scope, clazz, form, implicit);

            auto a2 = corpus->update(clazz, ctx->objectClassDef());

            classes.emplace(classes.begin(),
                            resolve_entry_result{r,
                                                 c,
                                                 (a == action::NONE && (a2 == action::INSERT || a2 == action::UPDATE)
                                                          ? action::UPDATE
                                                          : a)});

            return classes;
        }

        std::any visitObjectDefaultClassDecl(PlangParser::ObjectDefaultClassDeclContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(
                    visitObjectDefaultClass(ctx->objectDefaultClass()));

            form.singleton = ctx->objectSingleton();
            form.hinted    = ctx->objectClassDef();
            if (ctx->objectClassDef()) {
                form.hints = std::any_cast<any_vector>(visitObjectClassDef(ctx->objectClassDef()));
            }

            return form;
        }

        std::any visitObjectInlineClassDecl(PlangParser::ObjectInlineClassDeclContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(visitObjectInlineClass(ctx->objectInlineClass()));

            form.singleton = ctx->objectSingleton();
            form.hinted    = ctx->objectClassDef();
            if (ctx->objectClassDef()) {
                form.hints = std::any_cast<any_vector>(visitObjectClassDef(ctx->objectClassDef()));
            }

            return form;
        }

        std::any visitObjectClassRef(PlangParser::ObjectClassRefContext *ctx) override {
            auto form   = std::any_cast<object_class_form>(visitObjectClass(ctx->objectClass()));
            auto vector = form_to_vector(form.path);
            vector.push_back(form.name->toString());

            auto clazz = corpus->resolve(vector, detail::corpus::tag<object::clazz>(), *scope, false);
            return clazz;
        }
    };
}// namespace plang::lang
