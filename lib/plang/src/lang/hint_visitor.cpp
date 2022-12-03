//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/visitor.hpp"

using namespace plang::lang::generated;

namespace plang::lang {

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
}// namespace plang::lang
