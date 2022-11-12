//
// Created by Johannes on 22.08.2022.
//

#include "lang/visitor.hpp"
#include <algorithm>
#include <plang/error.hpp>
#include "lang/forms.hpp"

using namespace plang;
using namespace plang::plot;
using namespace plang::lang;
using namespace plang::lang::generated;

std::vector<string_t> form_to_vector(path_form const &form) {
    std::vector<string_t> nodes;
    nodes.reserve(form.nodes.size() + (form.qualified ? 1 : 0));
    if (form.qualified) { nodes.push_back(""); }
    std::transform(form.nodes.begin(), form.nodes.end(), std::back_inserter(nodes), [](auto &e) {
        return std::any_cast<antlr4::tree::TerminalNode *>(e)->toString();
    });

    return nodes;
}

resolve_entry_result form_to_path(corpus &corpus, path const &scope, path_form const &form) {
    auto nodes  = form_to_vector(form);
    auto result = corpus.resolve(nodes, detail::corpus::tag<path>(), scope, true);

    if (result.has_result() && form.decoration) {
        auto &path = std::get<class path>(result.entry());
        if (form.decoration->ordinal) {
            string_t ordinal_str   = form.decoration->ordinal->toString();
            plang::float_t ordinal = std::stod(ordinal_str);
            path.set_ordinal(ordinal);
        } else {
            path.set_ordinal();
        }

        if (form.decoration->description) {
            string_t desc_str = form.decoration->description->toString();
            char delim        = desc_str[0];
            std::istringstream ss{desc_str, std::ios_base::in};
            string_t desc;
            ss >> std::quoted(desc, delim, '\\');
            path.set_description(desc);
        } else {
            path.set_description();
        }

        auto action = corpus.update(path, true);
        return {result.entry(), {}, result.action() != action::NONE ? result.action() : action};
    }

    return result;
}

void visitor::sort_result(plang::column_types::string_t &&repr, plang::resolve_entry_result &ref) {
    switch (ref.action()) {
        case action::NONE:
            _report.mention(std::move(ref.entry()));
            break;
        case action::INSERT:
            _report.insert(std::move(ref.entry()));
            break;
        case action::UPDATE:
            _report.update(std::move(ref.entry()));
            break;
        case action::REMOVE:
            break;
        case action::FAIL:
            _report.fail(std::move(repr), std::move(ref.candidates()));
    }
}

visitor::visitor()
    : corpus(nullptr),
      scope(std::nullopt),
      _report() {}

visitor::visitor(class corpus &corpus, const plang::root::path &scope, bool_t strict, bool_t implicit)
    : corpus(&corpus),
      scope(scope),
      _report(),
      strict(strict),
      implicit(implicit) {}

std::any visitor::visitDeclSVO(generated::PlangParser::DeclSVOContext *ctx) {
    try {
        auto result = PlangBaseVisitor::visitDeclSVO(ctx);

        if (result.type() == typeid(resolve_entry_result)) {
            sort_result(ctx->getText(), *std::any_cast<resolve_entry_result>(&result));
        } else if (result.type() == typeid(any_vector)) {
            for (auto &r : *std::any_cast<any_vector>(&result)) {
                if (r.type() == typeid(resolve_entry_result)) {
                    sort_result(ctx->getText(), *std::any_cast<resolve_entry_result>(&r));
                }
            }
        }

        return result;
    } catch (std::exception const &e) {
        auto s = format::style{.text = format::style::color::BRIGHT_RED, .font = format::style::font::BOLD};
        std::cerr << corpus->get_format()("ERROR", s) << " in line " << ctx->getStart()->getLine() << ':'
                  << ctx->getStart()->getCharPositionInLine() << '\n';

        throw e;
    }
}

std::any visitor::visitRef(generated::PlangParser::RefContext *ctx) {
    auto result = PlangBaseVisitor::visitRef(ctx);
    if (result.type() == typeid(resolve_entry_result)) {
        return std::any_cast<resolve_entry_result>(result);
    } else if (result.type() == typeid(any_vector)) {
        auto vector = std::any_cast<any_vector>(result);
        if (vector.size() == 1) { return std::move(vector.at(0)); }
    }

    return resolve_entry_result();
}

plang::corpus::report const &visitor::get_report() const {
    return _report;
}

plang::path const &visitor::get_scope() const {
    return scope.value();
}

namespace plang::lang {

    class context_visitor : public virtual visitor {
    public:
        std::any visitContext(PlangParser::ContextContext *ctx) override {
            auto result = visitPathRef(ctx->pathRef());
            return result;
        }
        std::any visitContextDecl(PlangParser::ContextDeclContext *ctx) override {
            auto result = visitContext(ctx->context());
            auto path   = std::any_cast<plang::resolve_entry_result>(result);
            if (path.has_result()) {
                scope           = std::get<class path>(path.entry());
                _report.scope() = scope;
            }

            return {};
        }
    };

    class hint_visitor : public virtual visitor {
        std::any visitHintCommentLiteral(PlangParser::HintCommentLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), '\0'};
        }

        std::any visitHintLikeLiteral(PlangParser::HintLikeLiteralContext *ctx) override {
            return literal_form{ctx->LSTRING(), 'l'};
        }

        std::any visitHintGlobLiteral(PlangParser::HintGlobLiteralContext *ctx) override {
            return literal_form{ctx->GSTRING(), 'g'};
        }

        std::any visitHintRegexpLiteral(PlangParser::HintRegexpLiteralContext *ctx) override {
            return literal_form{ctx->RSTRING(), 'r'};
        }

        std::any visitHintMatchLiteral(PlangParser::HintMatchLiteralContext *ctx) override {
            return literal_form{ctx->MSTRING(), 'm'};
        }

        std::any visitHintLiteral(PlangParser::HintLiteralContext *ctx) override {
            if (ctx->hintCommentLiteral()) {
                return visitHintCommentLiteral(ctx->hintCommentLiteral());
            } else if (ctx->hintLikeLiteral()) {
                return visitHintLikeLiteral(ctx->hintLikeLiteral());
            } else if (ctx->hintGlobLiteral()) {
                return visitHintGlobLiteral(ctx->hintGlobLiteral());
            } else if (ctx->hintRegexpLiteral()) {
                return visitHintRegexpLiteral(ctx->hintRegexpLiteral());
            } else {
                return visitHintMatchLiteral(ctx->hintMatchLiteral());
            }
        }

        std::any visitHintSymbolClass(PlangParser::HintSymbolClassContext *ctx) override {
            return visitSymbolClass(ctx->symbolClass());
        }

        std::any visitHintPointClass(PlangParser::HintPointClassContext *ctx) override {
            return visitPointClass(ctx->pointClass());
        }

        std::any visitHint(PlangParser::HintContext *ctx) override {
            if (ctx->hintLiteral()) {
                return visitHintLiteral(ctx->hintLiteral());
            } else if (ctx->hintSymbolClass()) {
                return visitHintSymbolClass(ctx->hintSymbolClass());
            } else {
                return visitHintPointClass(ctx->hintPointClass());
            }
        }

        std::any visitHintList(PlangParser::HintListContext *ctx) override {
            any_vector result;
            for (auto const &e : ctx->hint()) { result.emplace_back(visitHint(e)); }

            return result;
        }

    public:
        std::any visitDecoration(PlangParser::DecorationContext *ctx) override {
            auto result = visitor::visitDecoration(ctx);
            return decoration_form{.ordinal = ctx->INTEGER() ?: ctx->DECIMAL(), .description = ctx->STRING()};
        }
    };

    class path_visitor : public virtual visitor {
    public:
        std::any visitPathUnqualifiedNode(PlangParser::PathUnqualifiedNodeContext *ctx) override {
            return any_vector{ctx->IDENTIFIER()};
        }

        std::any visitPathQualifiedNode(PlangParser::PathQualifiedNodeContext *ctx) override {
            auto result = std::any_cast<any_vector>(visitor::visitPathQualifiedNode(ctx));
            result.insert(result.begin(), ctx->OP_PATH());
            return result;
        }

        std::any visitPathUnqualifiedPath(PlangParser::PathUnqualifiedPathContext *ctx) override {
            auto result = visitor::visitPathUnqualifiedPath(ctx);
            return path_form{.nodes = std::any_cast<any_vector>(result), .qualified = false};
        }

        std::any visitPathQualifiedPath(PlangParser::PathQualifiedPathContext *ctx) override {
            auto result = visitor::visitPathQualifiedPath(ctx);
            return path_form{.nodes = std::any_cast<any_vector>(result), .qualified = true};
        }

        std::any visitPath(PlangParser::PathContext *ctx) override {
            auto result = visitor::visitPath(ctx);
            return result;
        }

        std::any visitPathDecl(PlangParser::PathDeclContext *ctx) override {
            path_form form = std::any_cast<path_form>(visitPath(ctx->path()));
            if (ctx->decoration()) {
                form.decoration = std::any_cast<decoration_form>(visitDecoration(ctx->decoration()));
            }

            auto result = form_to_path(*corpus, *scope, form);

            if (result.has_result() && form.decoration) {
                auto &path = std::get<class path>(result.entry());
                if (form.decoration->ordinal) {
                    string_t ordinal_str = form.decoration->ordinal->toString();
                    float_t ordinal      = std::stod(ordinal_str);
                    path.set_ordinal(ordinal);
                } else {
                    path.set_ordinal();
                }

                if (form.decoration->description) {
                    string_t desc_str = form.decoration->description->toString();
                    char delim        = desc_str[0];
                    std::istringstream ss{desc_str, std::ios_base::in};
                    string_t desc;
                    ss >> std::quoted(desc, delim, '\\');
                    path.set_description(desc);
                } else {
                    path.set_description();
                }

                auto action = corpus->update(path, true);
                return any_vector{result, resolve_entry_result({path, {}, action})};
            }

            return result;
        }

        std::any visitPathRef(PlangParser::PathRefContext *ctx) override {
            path_form form = std::any_cast<path_form>(visitPath(ctx->path()));

            std::vector<string_t> nodes;
            nodes.reserve(form.nodes.size() + (form.qualified ? 1 : 0));
            if (form.qualified) { nodes.push_back(""); }
            std::transform(form.nodes.begin(), form.nodes.end(), std::back_inserter(nodes), [](auto &e) {
                return std::any_cast<antlr4::tree::TerminalNode *>(e)->toString();
            });

            auto result = corpus->resolve(nodes, detail::corpus::tag<path>(), scope.value(), false);

            return result;
        }
    };

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

            auto p = form_to_path(*corpus, *scope, form.path);
            if (p.has_result()) {
                auto result = corpus->resolve({},
                                              detail::corpus::tag<symbol::clazz>(),
                                              std::get<class path>(p.entry()),
                                              false);
                return result;
            } else {
                return p;
            }
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

        std::any visitObjectClassDef(PlangParser::ObjectClassDefContext *ctx) override {
            if (ctx->hintList()) {
                return visitHintList(ctx->hintList());
            } else {
                return any_vector();
            }
        }

        std::any visitObjectDefaultClassDecl(PlangParser::ObjectDefaultClassDeclContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(
                    visitObjectDefaultClass(ctx->objectDefaultClass()));

            form.singleton = ctx->objectSingleton();
            if (ctx->objectClassDef()) {
                form.hints = std::any_cast<any_vector>(visitObjectClassDef(ctx->objectClassDef()));
            }

            return form;
        }

        std::any visitObjectInlineClassDecl(PlangParser::ObjectInlineClassDeclContext *ctx) override {
            object_class_form form = std::any_cast<object_class_form>(visitObjectInlineClass(ctx->objectInlineClass()));

            form.singleton = ctx->objectSingleton();
            if (ctx->objectClassDef()) {
                form.hints = std::any_cast<any_vector>(visitObjectClassDef(ctx->objectClassDef()));
            }

            return form;
        }
    };

    class point_visitor : public virtual visitor {
        std::any visitPointClassName(PlangParser::PointClassNameContext *ctx) override {
            point_class_form form;
            form.path = std::any_cast<struct path_form>(visitPath(ctx->path()));

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

            auto path = form_to_path(*corpus, *scope, form.path);
            if (!path.has_result()) { return path; }
            auto result = corpus->resolve({}, detail::corpus::tag<point::clazz>(), path.entry(), true);
            auto &clazz = std::get<point::clazz>(result.entry());
            clazz.set_singleton(form.singleton);

            if (ctx->hintSymbolClassList()) {
                form.hints = std::any_cast<any_vector>(visitHintSymbolClassList(ctx->hintSymbolClassList()));
                std::vector<point::clazz::hint> hints;
                any_vector hint_classes;

                for (auto const &e : form.hints) {
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
                            hint_classes.push_back(r2);
                        }
                    }
                }

                clazz.get_hints() = hints;
            }
            corpus->update(clazz, true);

            auto object_classes = corpus->resolve<plot::object::clazz>({}, clazz).candidates();
            std::set<pkey<object::clazz>> reused;

            for (auto const &c : form.object_classes) {
                auto oc_form = std::any_cast<object_class_form>(c);
                auto it      = std::find_if(object_classes.begin(), object_classes.end(), [&oc_form](auto const &c) {
                    return c.get_name() == oc_form.name->toString();
                });

                if (it != object_classes.end()) { reused.insert(it->get_id()); }

                auto o_class = (it != object_classes.end()) ? *it
                                                            : plot::object::clazz(clazz, oc_form.name->toString());

                o_class.set_default(oc_form._default);
                o_class.set_singleton(oc_form.singleton);
                auto &hints = o_class.get_hints();
                hints.clear();

                for (auto const &h : oc_form.hints) {
                    auto const &type = h.type();

                    if (type == typeid(literal_form)) {
                        static const std::map<char_t, object::clazz::hint::lit::type> map{
                                {'\0', object::clazz::hint::lit::type::COMMENT},
                                {'l',  object::clazz::hint::lit::type::LIKE   },
                                {'g',  object::clazz::hint::lit::type::GLOB   },
                                {'r',  object::clazz::hint::lit::type::REGEX  },
                                {'m',  object::clazz::hint::lit::type::MATCH  }
                        };

                        auto l_form = std::any_cast<literal_form>(h);

                        object::clazz::hint::lit hint{l_form.string->toString(), map.at(l_form.modifier)};
                        hints.emplace_back(std::move(hint));
                    } else if (type == typeid(symbol_class_form)) {
                        auto sc_form = std::any_cast<symbol_class_form>(h);
                        auto hinted  = corpus->resolve<symbol::clazz>(form_to_vector(sc_form.path), *scope, implicit);
                        if (!hinted.has_result()) {
                            // TODO: Throw exception
                        }

                        object::clazz::hint::sym hint{hinted.entry(), sc_form.recursive};
                        hints.emplace_back(std::move(hint));
                    } else if (type == typeid(point_class_form)) {
                        auto pc_form = std::any_cast<point_class_form>(h);
                        auto hinted  = corpus->resolve<point::clazz>(form_to_vector(pc_form.path), *scope, implicit);
                        if (!hinted.has_result()) {
                            // TODO: Throw exception
                        }

                        object::clazz::hint::pnt hint{hinted.entry(), pc_form.recursive};
                        hints.emplace_back(std::move(hint));
                    }
                }

                if (it != object_classes.end()) {
                    auto a = corpus->update(o_class, true);
                } else {
                    auto a = corpus->insert(o_class);
                }
            }

            for (auto &c : object_classes) {
                if (reused.find(c.get_id()) == reused.end()) { corpus->remove(c); }
            }

            return result;
        }
    };

    class visitor_impl : public virtual visitor,
                         public path_visitor,
                         public symbol_visitor,
                         public object_visitor,
                         public point_visitor,
                         public context_visitor,
                         public hint_visitor {

    protected:
        std::any aggregateResult(std::any aggregate, std::any next_result) override {
            /*
            any_vector result_list;
            if (!aggregate.has_value()) {
                result_list = {};
            } else if (aggregate.type() != typeid(any_vector)) {
                result_list = {aggregate};
            } else {
                result_list = std::any_cast<any_vector>(std::move(aggregate));
            }

            any_vector last;
            if (!next_result.has_value()) {
                last = {};
            } else if (next_result.type() != typeid(any_vector)) {
                last = {aggregate};
            } else {
                last = std::any_cast<any_vector>(std::move(next_result));
            }

            result_list.insert(result_list.end(),
                               std::make_move_iterator(last.begin()),
                               std::make_move_iterator(last.end()));

            return result_list;
            */

            if (!aggregate.has_value()) {
                return next_result.has_value() ? next_result : any_vector();
            } else if (!next_result.has_value()) {
                return aggregate.has_value() ? aggregate : any_vector();
            }

            if (aggregate.type() != typeid(any_vector)) { aggregate = any_vector({aggregate}); }

            if (next_result.type() != typeid(any_vector)) { next_result = any_vector({next_result}); }

            auto &result = *std::any_cast<any_vector>(&aggregate);
            auto &join   = *std::any_cast<any_vector>(&next_result);

            result.reserve(result.size() + join.size());
            std::move(join.begin(), join.end(), std::back_inserter(result));

            return result;
        }

    public:
        visitor_impl(class corpus &corpus, plang::path const &scope, bool_t strict, bool_t implicit)
            : visitor(corpus, scope, strict, implicit) {}
    };

}// namespace plang::lang

std::unique_ptr<visitor>
plang::lang::make_visitor(corpus &corpus, plang::path const &scope, bool_t strict, bool_t implicit) {
    return std::make_unique<visitor_impl>(std::ref(corpus), std::cref(scope), strict, implicit);
}

std::unique_ptr<visitor>
plang::lang::make_visitor(corpus const &corpus, plang::path const &scope, bool_t strict, bool_t implicit) {
    return std::make_unique<visitor_impl>(std::ref(const_cast<class corpus &>(corpus)),
                                          std::cref(scope),
                                          strict,
                                          implicit);
}
