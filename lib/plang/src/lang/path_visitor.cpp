//
// Created by Johannes on 03.12.2022.
//

#include <any>
#include "lang/forms.hpp"
#include "lang/visitor.hpp"

using namespace plang::lang::generated;

namespace plang::lang {

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
}// namespace plang::lang