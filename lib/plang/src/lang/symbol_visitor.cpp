//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/forms.hpp"
#include "lang/visitor.hpp"

using namespace plang::plot;
using namespace plang::lang::generated;

namespace plang::lang {

    class symbol_visitor : public virtual visitor {
    public:
        std::any visitSymbolUnqualifiedClass(PlangParser::SymbolUnqualifiedClassContext *ctx) override {
            symbol_class_form form;

            if (ctx->pathUnqualifiedPath()) {
                auto path_form = std::any_cast<struct path_form>(visitPathUnqualifiedPath(ctx->pathUnqualifiedPath()));
                form           = symbol_class_form{path_form, {}, false};
            } else {
                form = symbol_class_form{{}, {}, false};
            }

            if (ctx->symbolClassList()) {
                auto list = std::any_cast<any_vector>(visitSymbolClassList(ctx->symbolClassList()));

                for (auto const &e : list) { form.list.push_back(e); }
            }

            return form;
        }

        std::any visitSymbolQualifiedClass(PlangParser::SymbolQualifiedClassContext *ctx) override {
            auto path_form = std::any_cast<struct path_form>(visitPathQualifiedPath(ctx->pathQualifiedPath()));
            auto form      = symbol_class_form{path_form, {}, false};

            if (ctx->symbolClassList()) {
                auto list = std::any_cast<any_vector>(visitSymbolClassList(ctx->symbolClassList()));

                for (auto const &e : list) { form.list.push_back(e); }
            }

            return form;
        }

        std::any visitSymbolClass(PlangParser::SymbolClassContext *ctx) override {
            auto form = std::any_cast<symbol_class_form>(visitSymbolSimpleClass(ctx->symbolSimpleClass()));
            if (ctx->OP_RECUR()) { form.recursive = true; }
            return form;
        }

        std::any visitSymbolClassDecl(PlangParser::SymbolClassDeclContext *ctx) override {
            auto form = std::any_cast<symbol_class_form>(visitSymbolSimpleClass(ctx->symbolSimpleClass()));
            if (ctx->decoration()) {
                form.path.decoration = std::any_cast<decoration_form>(visitDecoration(ctx->decoration()));
            }

            any_vector classes;

            auto path = form_to_path(*corpus, *scope, form.path);
            if (!path.has_result()) { return path; }
            auto result = corpus->resolve({}, detail::corpus::tag<symbol::clazz>(), path.entry(), true);
            classes.push_back(result);

            for (auto const &e : form.list) {
                *form.path.nodes.rbegin() = e;
                auto p                    = form_to_path(*corpus, *scope, form.path);
                if (!p.has_result()) { return p; }
                auto r = corpus->resolve({}, detail::corpus::tag<symbol::clazz>(), p.entry(), true);
                classes.push_back(r);
            }

            if (ctx->hintSymbolClassList()) {
                auto hint_forms = std::any_cast<any_vector>(visitHintSymbolClassList(ctx->hintSymbolClassList()));
                std::vector<symbol::clazz::hint> hints;
                any_vector hint_classes;

                for (auto const &e : hint_forms) {
                    auto hint_form = std::any_cast<symbol_class_form>(e);
                    auto p         = form_to_path(*corpus, *scope, hint_form.path);
                    if (!p.has_result()) { return p; }
                    auto result2 = corpus->resolve({},
                                                   detail::corpus::tag<symbol::clazz>(),
                                                   std::get<class path>(p.entry()),
                                                   implicit);

                    hint_classes.push_back(result2);

                    if (result2.action() == action::FAIL) {
                        //TODO: Throw runtime error
                    } else {
                        symbol::clazz::hint h{std::get<symbol::clazz>(result2.entry())};
                        h.set_recursive(hint_form.recursive);

                        hints.push_back(h);

                        for (auto const &e2 : hint_form.list) {
                            *hint_form.path.nodes.rbegin() = e2;
                            auto p2                        = form_to_path(*corpus, *scope, hint_form.path);
                            if (!p2.has_result()) { return p2; }
                            auto r2 = corpus->resolve({}, detail::corpus::tag<symbol::clazz>(), p2.entry(), implicit);
                            symbol::clazz::hint h2{std::get<symbol::clazz>(r2.entry())};
                            h2.set_recursive(hint_form.recursive);

                            hints.push_back(h2);
                            hint_classes.push_back(r2);
                        }
                    }
                }

                for (auto &c : classes) {
                    resolve_entry_result &r2 = *std::any_cast<resolve_entry_result>(&c);
                    symbol::clazz &clazz     = std::get<symbol::clazz>(r2.entry());
                    clazz.get_hints()        = hints;

                    auto a = corpus->update(clazz, true);
                    if (r2.action() != action::INSERT && r2.action() != action::FAIL) { r2 = {clazz, {}, a}; }
                }

                classes.reserve(classes.size() + hint_classes.size());
                classes.insert(classes.end(),
                               std::make_move_iterator(hint_classes.begin()),
                               std::make_move_iterator(hint_classes.end()));
            }

            return classes;
        }

        std::any visitSymbolClassRef(PlangParser::SymbolClassRefContext *ctx) override {
            auto form = std::any_cast<symbol_class_form>(visitSymbolClass(ctx->symbolClass()));

            auto clazz =
                    corpus->resolve(form_to_vector(form.path), detail::corpus::tag<symbol::clazz>(), *scope, false);

            return clazz;
        }

        std::any visitSymbolDecoratedName(PlangParser::SymbolDecoratedNameContext *ctx) override {
            std::optional<decoration_form> decoration_form;
            if (ctx->decoration()) {
                decoration_form = std::any_cast<struct decoration_form>(visitDecoration(ctx->decoration()));
            }

            return single_symbol_form{.name = ctx->symbolName()->IDENTIFIER(), .decoration = decoration_form};
        }

        std::any visitSymbol(PlangParser::SymbolContext *ctx) override {
            symbol_form form;
            if (ctx->path()) { form.path = std::any_cast<path_form>(visitPath(ctx->path())); }
            form.list.push_back(single_symbol_form{.name = ctx->symbolName()->IDENTIFIER()});

            return form;
        }

        std::any visitSymbolListDef(PlangParser::SymbolListDefContext *ctx) override {
            symbol_form form;
            if (ctx->path()) { form.path = std::any_cast<path_form>(visitPath(ctx->path())); }

            std::size_t i                              = 0;
            PlangParser::SymbolDecoratedNameContext *c = nullptr;
            while ((c = ctx->symbolDecoratedName(i++))) {
                auto single_form = visitSymbolDecoratedName(c);
                form.list.emplace_back(std::move(single_form));
            };

            return form;
        }

        std::any visitSymbolDecl(PlangParser::SymbolDeclContext *ctx) override {
            auto symbol_form = std::any_cast<struct symbol_form>(visitSymbolListDef(ctx->symbolListDef()));

            any_vector symbols;
            auto p = form_to_path(*corpus, *scope, symbol_form.path);
            if (!p.has_result()) { return p; }
            auto c = corpus->resolve({}, detail::corpus::tag<symbol::clazz>(), p.entry(), true);

            for (auto const &s : symbol_form.list) {
                auto const &form = *std::any_cast<single_symbol_form>(&s);
                auto result = corpus->resolve({form.name->toString()}, detail::corpus::tag<symbol>(), c.entry(), true);
                if (result.action() != action::FAIL && form.decoration.has_value()) {
                    auto &sym = std::get<plot::symbol>(result.entry());
                    if (form.decoration->ordinal) {
                        sym.set_ordinal(std::stod(form.decoration->ordinal->toString()));
                    } else {
                        sym.set_ordinal();
                    }
                    if (form.decoration->description) {
                        string_t desc_str = form.decoration->description->toString();
                        char delim        = desc_str[0];
                        std::istringstream ss{desc_str, std::ios_base::in};
                        string_t desc;
                        ss >> std::quoted(desc, delim, '\\');
                        sym.set_description(desc);
                    } else {
                        sym.set_description();
                    }
                    auto r2 = corpus->update(sym, true);
                    symbols.emplace_back(std::move(r2));
                }
                symbols.emplace_back(std::move(result));
            }

            return symbols;
        }

        std::any visitSymbolRef(PlangParser::SymbolRefContext *ctx) override {
            if (ctx->symbol()) {
                auto form = std::any_cast<symbol_form>(visitSymbol(ctx->symbol()));
                std::vector<string_t> path;
                if (form.path.qualified) { path.push_back(""); }
                std::transform(form.path.nodes.begin(), form.path.nodes.end(), std::back_inserter(path), [](auto &e) {
                    return std::any_cast<antlr4::tree::TerminalNode *>(e)->toString();
                });
                path.push_back(std::any_cast<single_symbol_form>(&form.list[0])->name->toString());
                auto result = corpus->resolve(path, detail::corpus::tag<plot::symbol>(), *scope);
                return result;
            } else {
                //TODO: Implement compound symbol reference
                return resolve_entry_result();
            }
        }
    };
}// namespace plang::lang
