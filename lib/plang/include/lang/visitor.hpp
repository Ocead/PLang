//
// Created by Johannes on 22.08.2022.
//

#ifndef LIBPLANG_VISITOR_HPP
#define LIBPLANG_VISITOR_HPP

#include <memory>
#include <plang/corpus.hpp>
#include "lang/forms.hpp"
#include "lang/GENERATED/PlangParser.h"

using namespace plang::plot;

namespace plang::lang {

    inline std::vector<string_t> form_to_vector(path_form const &form) {
        std::vector<string_t> nodes;
        nodes.reserve(form.nodes.size() + (form.qualified ? 1 : 0));
        if (form.qualified) { nodes.push_back(""); }
        std::transform(form.nodes.begin(), form.nodes.end(), std::back_inserter(nodes), [](auto &e) {
            return std::any_cast<antlr4::tree::TerminalNode *>(e)->toString();
        });

        return nodes;
    }

    inline resolve_entry_result form_to_path(corpus &corpus, path const &scope, path_form const &form) {
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

    inline any_vector update_object_class(corpus &corpus,
                                          path const &scope,
                                          plot::object::clazz &o_class,
                                          object_class_form &oc_form,
                                          bool_t implicit) {
        any_vector classes;
        o_class.set_default(oc_form._default);
        o_class.set_singleton(oc_form.singleton);
        auto &hints = o_class.get_hints();
        if (oc_form.hinted) { hints.clear(); }

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

                sstream_t ss;
                auto str = (l_form.modifier != '\0' ? l_form.string->toString().substr(1) : l_form.string->toString());
                ss << str;
                string_t literal;
                ss >> std::quoted(literal, str[0], '\\');

                object::clazz::hint::lit hint{literal, map.at(l_form.modifier)};
                hints.emplace_back(std::move(hint));
            } else if (type == typeid(symbol_class_form)) {
                auto sc_form = std::any_cast<symbol_class_form>(h);
                auto hinted  = corpus.resolve<symbol::clazz>(form_to_vector(sc_form.path), scope, implicit);
                classes.emplace_back(resolve_entry_result(hinted));
                if (!hinted.has_result()) {
                    // TODO: Throw exception
                }

                object::clazz::hint::sym hint{hinted.entry(), sc_form.recursive};
                hints.emplace_back(std::move(hint));
            } else if (type == typeid(point_class_form)) {
                auto pc_form = std::any_cast<point_class_form>(h);
                auto hinted  = corpus.resolve<point::clazz>(form_to_vector(pc_form.path), scope, implicit);
                classes.emplace_back(resolve_entry_result(hinted));
                if (!hinted.has_result()) {
                    // TODO: Throw exception
                }

                object::clazz::hint::pnt hint{hinted.entry(), pc_form.recursive};
                hints.emplace_back(std::move(hint));
            }
        }

        return classes;
    }

    class visitor : public virtual generated::PlangBaseVisitor {
    public:
        struct options {
            bool_t strict = false;  ///<`true`, if not matching should count as an error
            bool_t implicit = true; ///<`true`, if declarations may implicitly declare other entries
            bool_t assume = true;   ///<`true`, if point references may omit irrelevant objects
            bool_t exact = false;   ///<`true`, if object lists in point references must be exact
        };

    private:
        void sort_result(string_t &&repr, resolve_entry_result &ref);

    protected:
        corpus *corpus;
        std::optional<path> scope;
        corpus::report _report;

        options _options;

        visitor();

        visitor(class corpus &corpus, path const &scope, visitor::options options);

    public:
        std::any visitDeclSVO(generated::PlangParser::DeclSVOContext *ctx) override;

        std::any visitRef(generated::PlangParser::RefContext *ctx) override;

        corpus::report const &get_report() const;

        path const &get_scope() const;
    };

    std::unique_ptr<visitor>
    make_visitor(corpus &corpus, plang::path const &scope, visitor::options options = {});

    std::unique_ptr<visitor>
    make_visitor(corpus const &corpus, plang::path const &scope, visitor::options options = {});

}// namespace plang::lang

#endif//LIBPLANG_VISITOR_HPP
