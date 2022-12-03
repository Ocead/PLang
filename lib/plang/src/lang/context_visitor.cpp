//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/visitor.hpp"

using namespace plang::lang::generated;

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
}// namespace plang::lang
