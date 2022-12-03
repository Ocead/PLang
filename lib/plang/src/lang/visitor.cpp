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

void visitor::sort_result(string_t &&repr, plang::resolve_entry_result &ref) {
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
