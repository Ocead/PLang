//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/visitor.hpp"

#include "context_visitor.cpp"
#include "hint_visitor.cpp"
#include "object_visitor.cpp"
#include "path_visitor.cpp"
#include "point_visitor.cpp"
#include "symbol_visitor.cpp"

using namespace plang::plot;
using namespace plang::lang::generated;

namespace plang::lang {

    class visitor_impl : public virtual visitor,
                         public path_visitor,
                         public symbol_visitor,
                         public object_visitor,
                         public point_visitor,
                         public context_visitor,
                         public hint_visitor {

    protected:
        std::any aggregateResult(std::any aggregate, std::any next_result) override {
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
        visitor_impl(class corpus &corpus, plang::path const &scope, visitor::options options)
            : visitor(corpus, scope, options) {}
    };

    std::unique_ptr<visitor> make_visitor(corpus &corpus, plang::path const &scope, visitor::options options) {
        return std::make_unique<visitor_impl>(std::ref(corpus), std::cref(scope), options);
    }

    std::unique_ptr<visitor>
    make_visitor(corpus const &corpus, plang::path const &scope, visitor::options options) {
        return std::make_unique<visitor_impl>(std::ref(const_cast<class corpus &>(corpus)),
                                              std::cref(scope),
                                              options);
    }

}// namespace plang::lang
