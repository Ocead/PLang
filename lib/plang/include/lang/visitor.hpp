//
// Created by Johannes on 22.08.2022.
//

#ifndef LIBPLANG_VISITOR_HPP
#define LIBPLANG_VISITOR_HPP

#include <memory>
#include <plang/corpus.hpp>
#include "lang/GENERATED/PlangBaseVisitor.h"
#include "lang/GENERATED/PlangParser.h"

namespace plang::lang {

    using any_vector_t = std::vector<std::any>;

    class any_vector : public any_vector_t {
    public:
        using any_vector_t::any_vector_t;
    };

    class visitor : public virtual generated::PlangBaseVisitor {
    private:
        void sort_result(string_t &&repr, resolve_entry_result &ref);

    protected:
        corpus *corpus;
        std::optional<path> scope;
        corpus::report _report;

        visitor();

        visitor(class corpus &corpus, path const &scope);

    public:
        std::any visitDeclSVO(generated::PlangParser::DeclSVOContext *ctx) override;

        std::any visitRef(generated::PlangParser::RefContext *ctx) override;

        corpus::report const &get_report() const;

        path const & get_scope() const;
    };

    std::unique_ptr<visitor> make_visitor(corpus &corpus, plang::path const &scope);

    std::unique_ptr<visitor> make_visitor(corpus const &corpus, plang::path const &scope);

}// namespace plang::lang

#endif//LIBPLANG_VISITOR_HPP
