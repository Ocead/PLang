//
// Created by Johannes on 22.08.2022.
//

#include <antlr4-runtime.h>
#include <plang/corpus.hpp>
#include "lang/GENERATED/PlangLexer.h"
#include "lang/GENERATED/PlangParser.h"
#include "lang/GENERATED/PlangBaseVisitor.h"

int main(int argc, char *argv[]) {
    using namespace antlr4;
    using namespace plang::lang::generated;

    std::istringstream is{".gnampf;"};

    ANTLRInputStream input(is);
    PlangLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    PlangParser parser(&tokens);
    parser.setBuildParseTree(true);
    PlangParser::DeclSVOContext *tree = parser.declSVO();

    auto visitor = std::make_unique<PlangBaseVisitor>();
    visitor->visitDeclSVO(tree);
}
