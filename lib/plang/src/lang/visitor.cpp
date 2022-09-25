//
// Created by Johannes on 22.08.2022.
//

#include "lang/visitor.hpp"
#include <algorithm>
#include "lang/forms.hpp"

using namespace plang::lang;
using namespace plang::lang::generated;

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
        case action::DELETE:
            break;
        case action::FAIL:
            _report.fail(std::move(repr), std::move(ref.candidates()));
    }
}

visitor::visitor()
    : corpus(nullptr),
      scope(std::nullopt),
      _report() {}

visitor::visitor(class corpus &corpus, const plang::root::path &scope)
    : corpus(&corpus),
      scope(scope),
      _report() {}

std::any visitor::visitDeclSVO(generated::PlangParser::DeclSVOContext *ctx) {
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
            auto path = std::any_cast<plang::resolve_entry_result>(result);
            if (path.has_result()) {
                scope           = std::get<class path>(path.entry());
                _report.scope() = scope;
            }

            return {};
        }
    };

    class hint_visitor : public virtual visitor {
        std::any visitHintCommentLiteral(PlangParser::HintCommentLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), {}};
        }

        std::any visitHintLikeLiteral(PlangParser::HintLikeLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), ctx->OP_STR_L()};
        }

        std::any visitHintGlobLiteral(PlangParser::HintGlobLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), ctx->OP_STR_G()};
        }

        std::any visitHintRegexpLiteral(PlangParser::HintRegexpLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), ctx->OP_STR_R()};
        }

        std::any visitHintMatchLiteral(PlangParser::HintMatchLiteralContext *ctx) override {
            return literal_form{ctx->STRING(), ctx->OP_STR_M()};
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
            std::vector<string_t> nodes;
            nodes.reserve(form.nodes.size() + (form.qualified ? 1 : 0));
            if (form.qualified) { nodes.push_back(""); }
            std::transform(form.nodes.begin(), form.nodes.end(), std::back_inserter(nodes), [](auto &e) {
                return std::any_cast<antlr4::tree::TerminalNode *>(e)->toString();
            });

            auto result = corpus->resolve(nodes, detail::corpus::tag<path>(), scope.value(), true);

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
            auto form = std::any_cast<path_form>(visitPathUnqualifiedPath(ctx->pathUnqualifiedPath()));
            return symbol_class_form{form, {}, false};
        }

        std::any visitSymbolQualifiedClass(PlangParser::SymbolQualifiedClassContext *ctx) override {
            auto form = std::any_cast<path_form>(visitPathQualifiedPath(ctx->pathQualifiedPath()));
            return symbol_class_form{form, {}, false};
        }

        std::any visitSymbolRecursiveClass(PlangParser::SymbolRecursiveClassContext *ctx) override {
            auto form      = std::any_cast<symbol_class_form>(visitSymbolSimpleClass(ctx->symbolSimpleClass()));
            form.recursive = true;
            return form;
        }
    };

    class visitor_impl : public virtual visitor, public path_visitor, public context_visitor, public hint_visitor {

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
        visitor_impl(class corpus &corpus, plang::path const &scope)
            : visitor(corpus, scope) {}
    };

}// namespace plang::lang

std::unique_ptr<visitor> plang::lang::make_visitor(corpus &corpus, plang::path const &scope) {
    return std::make_unique<visitor_impl>(std::ref(corpus), std::cref(scope));
}

std::unique_ptr<visitor> plang::lang::make_visitor(corpus const &corpus, plang::path const &scope) {
    return std::make_unique<visitor_impl>(std::ref(const_cast<class corpus &>(corpus)), std::cref(scope));
}
