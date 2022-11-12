//
// Created by Johannes on 11.09.2022.
//

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <readline/history.h>
#include <readline/readline.h>
#include <plang/error.hpp>
#include <plangcli/cli.hpp>
#include <plangcli/error.hpp>

using namespace plang;
using namespace plang::op;

//From https://gist.github.com/Lee-R/3839813
// FNV-1a 32bit hashing algorithm.
constexpr std::uint32_t fnv1a_32(char const *s, std::size_t count) {
    return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

constexpr std::uint32_t operator"" _hash(char const *s, std::size_t count) {
    return fnv1a_32(s, count);
}

template<template<typename...> typename M, typename K, typename V>
auto invert(M<K, V> const &map) -> M<V, K> {
    M<V, K> result;
    std::transform(map.begin(), map.end(), std::inserter(result, result.end()), [](auto const &e) {
        return std::make_pair(e.second, e.first);
    });
    return result;
}

std::map<string_t, format::output> cli::output_option_map{
        {"ansi",     format::output::ANSI    },
        {"readline", format::output::READLINE},
        {"html",     format::output::HTML    },
        {"doxygen",  format::output::DOXYGEN }
};

std::map<string_t, format::enrich> cli::enrich_option_map{
        {"plain", format::enrich::PLAIN},
        {"color", format::enrich::COLOR},
        {"font",  format::enrich::FONT },
        {"rich",  format::enrich::RICH }
};

std::map<string_t, format::detail> cli::detail_option_map{
        {"id_ref",       format::detail::ID_REF      },
        {"explicit_ref", format::detail::EXPLICIT_REF},
        {"definition",   format::detail::DEFINITION  },
        {"full",         format::detail::FULL        }
};

std::map<string_t, format::qualification> cli::qualification_option_map{
        {"unique", format::qualification::UNIQUE},
        {"full",   format::qualification::FULL  }
};

std::map<string_t, format::indent> cli::indent_option_map{
        {"compact",  format::indent::COMPACT },
        {"one_line", format::indent::ONE_LINE},
        {"left",     format::indent::LEFT    },
        {"center",   format::indent::CENTER  }
};

std::map<string_t, bool_t> cli::bool_option_map{
        {"1",     true },
        {"on",    true },
        {"yes",   true },
        {"true",  true },
        {"false", false},
        {"off",   false},
        {"no",    false},
        {"0",     false}
};

std::map<string_t, enum cli::prompt> cli::prompt_option_map {
    {"none", cli::prompt::NONE},
    {"unicode", cli::prompt::UNICODE},
    {"powerline", cli::prompt::POWERLINE}
};

cli *cli::current_instance = nullptr;

std::vector<string_t> cli::autocomplete_candidates{};

std::tuple<std::vector<string_t>, std::size_t, std::size_t> cli::string_to_args(string_view_t const &str, int_t cur) {
    //From https://www.unix.com/programming/126160-building-argc-argv-style-structure-string-char.html
    bool in_token;
    bool in_container;
    bool escaped;
    char container_start;
    char c;
    int len;
    int i;
    bool space;
    int index    = -1;
    int position = -1;

    std::vector<string_t> args;
    sstream_t ss;

    container_start = 0;
    in_token        = false;
    in_container    = false;
    escaped         = false;

    len = str.size();

    {}//argv_init();
    for (i = 0; i < len; i++) {
        c = str[i];

        if (i == cur) {
            index = args.size();
            ss.seekp(0, std::ios::end);
            position = ss.tellg() / sizeof(string_t::value_type);
        }

        switch (c) {
            /* handle whitespace */
            case ' ':
            case '\t':
            case '\n':
                if (!in_token) continue;

                if (in_container) {
                    ss << c;
                    space = false;
                    continue;
                }

                if (escaped) {
                    escaped = false;
                    ss << c;
                    space = false;
                    continue;
                }

                if (ss.str().empty()) { space = true; }

                /* if reached here, we're at end of token */
                in_token = false;
                {
                    auto next_arg = ss.str();
                    if (!next_arg.empty()) { args.emplace_back(ss.str()); }
                    ss.str(string_t());
                }//argv_token_finish();
                break;

            /* handle quotes */
            case '\'':
            case '\"':

                if (escaped) {
                    ss << c;
                    space   = false;
                    escaped = false;
                    continue;
                }

                if (!in_token) {
                    in_token        = true;
                    in_container    = true;
                    container_start = c;
                    continue;
                }

                if (in_container) {
                    if (c == container_start) {
                        in_container = false;
                        in_token     = false;
                        {
                            args.emplace_back(ss.str());
                            ss.str(string_t());
                        }//argv_token_finish();
                        continue;
                    } else {
                        ss << c;
                        space = false;
                        continue;
                    }
                }

                /* XXX in this case, we:
             *    1. have a quote
             *    2. are in a token
             *    3. and not in a container
             * e.g.
             *    hell"o
             *
             * what's done here appears shell-dependent,
             * but overall, it's an error.... i *think*
             */
                printf("Parse Error! Bad quotes\n");
                break;

            case '\\':

                if (in_container && str[i + 1] != container_start) {
                    ss << c;
                    space = false;
                    continue;
                }

                if (escaped) {
                    ss << c;
                    space = false;
                    continue;
                }

                escaped = true;
                break;

            default:
                if (!in_token) { in_token = true; }

                ss << c;
                space = false;
        }
    }

    if (in_container) printf("Parse Error! Still in container\n");

    if (escaped) printf("Parse Error! Unused escape (\\)\n");

    {
        auto next_arg = ss.str();
        if (!next_arg.empty()) { args.emplace_back(ss.str()); }
        ss.str(string_t());
    }//argv_token_finish();

    if (index != -1 && position != -1) {
        return {args, index, position};
    } else {
        return {args, args.size(), 0};
    }
}

char **cli::completion_function(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    string_view_t view{rl_line_buffer};

    //language=regexp
    static std::regex regex{R"(^[\s\t]*:\w*)"};
    if (auto it = std::regex_iterator(view.begin(), view.end(), regex); it != decltype(it)()) {
        auto command = it->str().substr(1);
        if (command.size() == view.size() - 1) {
            if (!command.empty()) {
                std::vector<string_t> candidates;
                for (auto const &[k, v] : current_instance->commands) {
                    if (k.rfind(command, 0) == 0) { candidates.push_back(k); }
                }
                cli::autocomplete_candidates = candidates;
            } else {
                std::vector<string_t> candidates;
                candidates.reserve(current_instance->commands.size());
                std::transform(current_instance->commands.begin(),
                               current_instance->commands.end(),
                               std::back_inserter(candidates),
                               [](auto const &p) -> string_t { return ":" + std::get<0>(p); });
                cli::autocomplete_candidates = candidates;
            }
        } else {
            auto com = current_instance->completers.find(command);
            if (com != current_instance->completers.end()) {
                auto spos                    = view.find(' ') + 1;
                cli::autocomplete_candidates = com->second(current_instance->corpus ? &*(current_instance->corpus)
                                                                                    : nullptr,
                                                           spos != 0 ? view.substr(spos) : string_view_t(),
                                                           start - spos,
                                                           end - spos);
            } else {
                cli::autocomplete_candidates.clear();
            }
        }
    } else {
        cli::autocomplete_candidates.clear();
    }
    return rl_completion_matches(text, cli::candidate_generator);
}

char *cli::candidate_generator(const char *text, int state) {
    static size_t match_index = 0;

    if (state == 0) {
        match_index = 0;

        std::string textstr(text);
    }

    if (match_index >= autocomplete_candidates.size()) {
        return nullptr;
    } else {
        return strdup(autocomplete_candidates[match_index++].c_str());
    }
}

std::vector<string_t> cli::complete_filename(string_view_t const &arg) {
    std::filesystem::path path{arg};
    std::vector<string_t> candidates;
    auto filename     = path.filename();
    auto filename_str = filename.string();
    std::filesystem::path parent;
    if (arg.empty()) {
        parent = std::filesystem::current_path();
    } else {
        parent = (std::filesystem::is_directory(path) ? path : std::filesystem::absolute(path).parent_path());
    }

    for (auto const &e : std::filesystem::directory_iterator(parent)) {
        if (e.is_regular_file()) {
            auto relative = std::filesystem::relative(e.path(), parent);
            auto str      = relative.string();
            if (str.substr(0, std::min(str.size(), filename_str.size())) == filename_str) {
                candidates.emplace_back(relative.string());
            }
        }
    }

    return candidates;
}

void cli::check_corpus() const {
    if (!corpus) { throw exception::no_corpus_error(); }
}

void cli::set_format() {
    corpus->set_format(options.format);
}

class format cli::get_prompt_format() const {
    auto prompt_format = options.format;
    prompt_format.set_output(format::output::READLINE);
    return prompt_format;
}

void cli::set_scope(std::optional<path> _scope) {
    scope      = std::move(_scope);
    scope_path = corpus->print(scope.value(),
                               plang::format(format::output::ANSI,
                                             format::enrich::PLAIN,
                                             format::detail::EXPLICIT_REF,
                                             format::qualification::FULL,
                                             format::indent::ONE_LINE));
}

bool_t cli::prompt_yes_no(string_t const &base_prompt) const {
    char_t y     = std::tolower(dict.get().at(texts::CLI_YES)[0]);
    char_t n     = std::toupper(dict.get().at(texts::CLI_NO)[0]);
    auto prompt  = base_prompt + " [" + y + '/' + n + "]: ";
    char *buffer = readline(prompt.c_str());
    string_t result(buffer);
    free(buffer);

    if (result.empty()) {
        return false;
    } else if (result.size() == 1) {
        return std::tolower(result[0]) == y || std::tolower(result[0]) == 'y';
    } else {
        string_t yes = dict.get().at(texts::CLI_YES);
        std::transform(yes.begin(), yes.end(), yes.begin(), [](char_t c) { return std::tolower(c); });
        std::transform(result.begin(), result.end(), result.begin(), [](char_t c) { return std::tolower(c); });

        return result.substr(0, yes.size()) == yes;
    }
}

int_t cli::_help(class corpus *, string_view_t const &) {
    for (auto const &e : help) { std::cout << e << '\n'; }
    return 0;
}

int_t cli::_option(class corpus *, std::vector<string_t> const &args) {
    switch (args.size()) {
        case 2: {
            switch (fnv1a_32(args[0].c_str(), args[0].size())) {
                case "output"_hash: {
                    options.format.set_output(output_option_map[args[1]]);
                    break;
                }
                case "enrich"_hash: {
                    options.format.set_enrich(enrich_option_map[args[1]]);
                    break;
                }
                case "detail"_hash: {
                    options.format.set_detail(detail_option_map[args[1]]);
                    break;
                }
                case "qualification"_hash: {
                    options.format.set_qualification(qualification_option_map[args[1]]);
                    break;
                }
                case "indent"_hash: {
                    options.format.set_indent(indent_option_map[args[1]]);
                    break;
                }
                case "prompt"_hash: {
                    options.prompt = prompt_option_map[args[1]];
                }
                case "implicit"_hash: {
                    options.implicit = bool_option_map[args[1]];
                }
                case "strict"_hash: {
                    options.strict = bool_option_map[args[1]];
                }
                default:
                    break;
            }
            if (corpus.has_value()) { corpus->set_format(options.format); }
        }
        case 1: {
            switch (fnv1a_32(args[0].c_str(), args[0].size())) {
                case "output"_hash: {
                    auto map = invert(output_option_map);
                    std::cout << "output=" << map[options.format.get_output()] << '\n';
                    break;
                }
                case "enrich"_hash: {
                    auto map = invert(enrich_option_map);
                    std::cout << "enrich=" << map[options.format.get_enrich()] << '\n';
                    break;
                }
                case "detail"_hash: {
                    auto map = invert(detail_option_map);
                    std::cout << "detail=" << map[options.format.get_detail()] << '\n';
                    break;
                }
                case "qualification"_hash: {
                    auto map = invert(qualification_option_map);
                    std::cout << "qualification=" << map[options.format.get_qualification()] << '\n';
                    break;
                }
                case "indent"_hash: {
                    auto map = invert(indent_option_map);
                    std::cout << "indent=" << map[options.format.get_indent()] << '\n';
                    break;
                }
                case "prompt"_hash: {
                    auto map = invert(prompt_option_map);
                    std::cout << "prompt=" << map[options.prompt] << '\n';
                    break;
                }
                case "implicit"_hash: {
                    std::cout << "implicit=" << (options.implicit ? "true" : "false") << '\n';
                    break;
                }
                case "strict"_hash: {
                    std::cout << "strict=" << (options.strict ? "true" : "false") << '\n';
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default: {
            std::cout << "output=" << invert(output_option_map)[options.format.get_output()] << '\n';
            std::cout << "enrich=" << invert(enrich_option_map)[options.format.get_enrich()] << '\n';
            std::cout << "detail=" << invert(detail_option_map)[options.format.get_detail()] << '\n';
            std::cout << "qualification=" << invert(qualification_option_map)[options.format.get_qualification()]
                      << '\n';
            std::cout << "indent=" << invert(indent_option_map)[options.format.get_indent()] << '\n';
            std::cout << "prompt=" << invert(prompt_option_map)[options.prompt] << '\n';
            std::cout << "implicit=" << (options.implicit ? "true" : "false") << '\n';
            std::cout << "strict=" << (options.strict ? "true" : "false") << '\n';
            break;
        }
    }

    return 0;
}

std::vector<string_t> cli::_option_complete(class corpus *, const string_view_t &str, uint_t start, uint_t end) {
    auto [args, i, pos] = string_to_args(str, start);

    std::vector<string_t> values;
    std::vector<string_t> candidates;

    if (i == 0) {
        values = {"output", "enrich", "detail", "qualification", "indent", "implicit", "strict"};
    } else if (i == 1) {
        switch (fnv1a_32(args[0].c_str(), args[0].size())) {
            case "output"_hash: {
                for (auto const &p : output_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "enrich"_hash: {
                for (auto const &p : enrich_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "detail"_hash: {
                for (auto const &p : detail_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "qualification"_hash: {
                for (auto const &p : qualification_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "indent"_hash: {
                for (auto const &p : indent_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "prompt"_hash: {
                for (auto const &p : prompt_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "implicit"_hash: {
                for (auto const &p : bool_option_map) { values.emplace_back(p.first); };
                break;
            }
            case "strict"_hash: {
                for (auto const &p : bool_option_map) { values.emplace_back(p.first); };
                break;
            }
            default:
                break;
        }
    }

    if (i < args.size()) {
        for (auto const &o : values) {
            if (o.rfind(args[i], 0) == 0) { candidates.push_back(o); }
        }
        return candidates;
    } else {
        return values;
    }
}

int_t cli::_info(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_quit(class corpus *_, string_view_t const &args) {
    int_t r = _close(_, args);
    if (r != 0) { return r; }
    stop = true;

    return 0;
}

int_t cli::_new(class corpus *, string_view_t const &args) {
    if (unsaved) {}
    if (!corpus.has_value()) {
        if (args.empty()) {
            corpus.emplace();
            set_scope(corpus->fetch<path>(corpus->get_root_path_id()));
            corpus->set_format(options.format);

            return 0;
        } else {
            std::filesystem::path path(args);

            if (path.has_filename()) {
                corpus.emplace(path);
                set_scope(corpus->fetch<class path>(corpus->get_root_path_id()));
                corpus->set_format(options.format);

                return 0;
            } else {
                return 1;
            }
        }
    }

    return 1;
}

int_t cli::_open(class corpus *_, std::vector<string_t> const &args) {
    if (args.size() != 1) { return 3; }
    if (_close(_, {}) == 0) {
        if (args.empty()) {
            corpus.emplace();
            set_scope(corpus->fetch<path>(corpus->get_root_path_id()));
            corpus->set_format(options.format);

            return 0;
        } else {
            std::filesystem::path path(args[0]);

            if (path.has_filename()) {
                corpus.emplace(path);
                set_scope(corpus->fetch<class path>(corpus->get_root_path_id()));
                corpus->set_format(options.format);
                file = path;

                return 0;
            } else {
                return 2;
            }
        }
    }

    return 1;//Could not close current db
}

std::vector<string_t> cli::_open_complete(class corpus *, const string_view_t &arg, uint_t start, uint_t end) {
    return complete_filename(arg.substr(start, end));
}

int_t cli::_save(class corpus *, string_view_t const &args) {
    if (unsaved) {
        if (!file.has_value()) {
            string_t filename;
            if (args.empty()) {
                char_t *buffer = readline((dict.get().at(texts::CLI_SAVE_FILENAME) + ": ").c_str());
                if (buffer) {
                    filename = string_t(buffer);
                    free(buffer);
                } else {
                    return 4;//Reading filename was canceled
                }
            } else {
                filename = args;
            }
            if (!filename.empty()) {
                std::filesystem::path p{filename};
                if (std::filesystem::is_directory(p)) {
                    // TODO: print path is directory
                    return 3;//Path is directory
                }
                if (std::filesystem::exists(p)) {
                    bool_t override = prompt_yes_no(dict.get().at(texts::CLI_CONFIRM_OVERRIDE_SAVE));
                    if (!override) {
                        return 2;//Don't override file
                    }
                }

                plang::corpus db{p};
                corpus->backup(db);
                corpus = db;
                scope  = corpus->fetch<class path>(corpus->get_root_path_id());
                corpus->set_format(options.format);
                file = p;

                unsaved = false;
            } else {
                return 1;//No filename
            }
        }
    }

    if (file.has_value()) {
        unsaved = !corpus->flush();
        return unsaved ? 1 : 0;
    }

    return 1;
}

std::vector<string_t> cli::_save_complete(class corpus *, const string_view_t &arg, uint_t start, uint_t end) {
    return complete_filename(arg.substr(start, end));
}

int_t cli::_close(class corpus *, string_view_t const &) {
    if (unsaved) {
        if (!prompt_yes_no(dict.get().at(texts::CLI_CONFIRM_UNSAVED_QUIT))) { return 1; }
    }

    corpus.reset();
    scope.reset();
    file.reset();

    return 0;
}

int_t cli::_run(class corpus *, string_view_t const &args) {
    using namespace std::chrono;

    std::filesystem::path path(args);

    if (path.has_filename()) {
        std::ifstream ifs{path};

        auto report = corpus->execute(ifs, *scope);

        if (report.diff_size() > 0) { unsaved = true; }

        std::cout << "Executed file \"" << path.string() << "\"\n"
                  << "Lexing time    : "
                  << duration_cast<std::chrono::milliseconds>(report.post_lex() - report.start()).count() << "ms\n"
                  << "Parsing time   : "
                  << duration_cast<std::chrono::milliseconds>(report.post_parse() - report.post_lex()).count() << "ms\n"
                  << "Visiting time  : "
                  << duration_cast<std::chrono::milliseconds>(report.post_visit() - report.post_parse()).count()
                  << "ms\n"
                  << "Persisting time: "
                  << duration_cast<std::chrono::milliseconds>(report.post_commit() - report.post_visit()).count()
                  << "ms\n";

        return 0;
    } else {
        return 2;
    }
}

int_t cli::_export(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_inspect(class corpus *, string_view_t const &args) {
    if (corpus.has_value()) {
        auto result = corpus->ref(string_t(args), *scope);

        if (result.has_result()) {
            std::cout << corpus->print(result.entry()) << '\n';

            return 0;
        } else {
            if (!result.candidates().empty()) {
                std::cout << "Candidates:\n";
                for (auto const &c : result.candidates()) { std::cout << corpus->print(c) << '\n'; }
            } else {
                std::cout << "No match found.\n";
            }

            return 1;
        }
    }

    return 1;
}

int_t cli::_scope(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_list(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_find(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_remove(class corpus *, string_view_t const &args) {
    if (corpus.has_value()) {
        auto result = corpus->ref(string_t(args), *scope);
        if (result.has_result()) {
            auto [repr, action] = corpus->remove(result.entry());
            if (action == action::REMOVE) {
                unsaved = true;
                return 0;
            } else {
                return 3;
            }
        } else {
            return 2;
        }
    } else {
        return 1;
    }
}

int_t cli::_copy(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_move(class corpus *, string_view_t const &) {
    return 0;
}

int_t cli::_undo(class corpus *, string_view_t const &) {
    if (corpus.has_value()) {
        auto [s, desc] = corpus->undo();
        if (s >= 0) {
            std::cout << dict.get().at(texts::CLI_UNDID) << std::quoted(desc) << '\n';
            return 0;
        } else {
            return 2;
        }
    }

    return 1;
}

int_t cli::_redo(class corpus *, string_view_t const &) {
    if (corpus.has_value()) {
        auto [s, desc] = corpus->redo();
        if (s >= 0) {
            std::cout << dict.get().at(texts::CLI_REDID) << std::quoted(desc) << '\n';
            return 0;
        } else {
            return 2;
        }
    }

    return 1;
}

int_t cli::handle_command(string_view_t const &) {
    return 0;
}

int_t cli::handle_plang(string_view_t const &view) {
    corpus::report report;

    try {
        report = corpus->execute(string_t(view), *scope);
    } catch (plang::exception::lang_error const &e) {
        std::cerr << e.what() << std::endl;
    } catch (std::runtime_error const &e) { std::cerr << e.what() << std::endl; } catch (std::logic_error const &e) {
        std::cerr << e.what() << std::endl;
    }

    if (report.diff_size() > 0) { unsaved = true; }

    if (!report.mentioned().empty()) {
        std::cout << dict.get().at(texts::CLI_REPORT_MENTIONED) << ":\n";

        for (auto &[id, e] : report.mentioned()) {
            std::cout << "\t" << corpus->print(e, corpus->get_inner_format()) << "\n";
        }
    }
    if (!report.inserted().empty()) {
        std::cout << dict.get().at(texts::CLI_REPORT_INSERTED) << ":\n";

        for (auto &[id, e] : report.inserted()) {
            std::cout << "\t" << corpus->print(e, corpus->get_inner_format()) << "\n";
        }
    }
    if (!report.updated().empty()) {
        std::cout << dict.get().at(texts::CLI_REPORT_UPDATED) << ":\n";

        for (auto &[id, e] : report.updated()) {
            std::cout << "\t" << corpus->print(e, corpus->get_inner_format()) << "\n";
        }
    }
    if (!report.removed().empty()) {
        std::cout << dict.get().at(texts::CLI_REPORT_REMOVED) << ":\n";

        for (auto &[id, e] : report.removed()) { std::cout << "\t" << e << "\n"; }
    }
    if (!report.failed().empty()) {
        std::cout << dict.get().at(texts::CLI_REPORT_FAILED) << ":\n";

        for (auto &[str, e] : report.failed()) { std::cout << "\t" << std::quoted(str) << "\n"; }
    }

    if (report.scope().has_value()) {
        scope      = report.scope();
        scope_path = corpus->print(scope.value(),
                                   plang::format(format::output::ANSI,
                                                 format::enrich::PLAIN,
                                                 format::detail::EXPLICIT_REF,
                                                 format::qualification::FULL,
                                                 format::indent::ONE_LINE));
    }

    return 0;
}

int_t cli::handle_input(string_view_t const &view) {
    //language=regexp
    static std::regex regex{R"(^[\s\t]*:\w*)"};
    if (auto it = std::regex_iterator(view.begin(), view.end(), regex); it != decltype(it)()) {
        auto command = it->str().substr(1);

        auto com = commands.find(command);
        if (com != commands.end()) {
            auto spos = view.find(' ') + 1;
            try {
                return com->second(corpus ? &*corpus : nullptr, spos != 0 ? view.substr(spos) : string_view_t());
            } catch (std::exception const &e) { std::cerr << e.what() << std::endl; }
        } else {
            //TODO: throw unknown_command_error
        }

    } else if (corpus.has_value()) {
        handle_plang(view);
    }

    return 1;
}

cli::cli()
    : options{
              .format = {format::output::ANSI,
                         format::enrich::RICH,
                         format::detail::FULL,
                         format::qualification::FULL,
                         format::indent::LEFT},
              .prompt = prompt::POWERLINE
},
      corpus(), scope(), commands(), help(), dict(en()), unsaved(false), stop() {

    rl_initialize();
    rl_variable_bind("horizontal-scroll-mode", "off");

    old_instance                     = current_instance;
    current_instance                 = this;
    old_rl_acf                       = rl_attempted_completion_function;
    old_w_br_chars                   = rl_basic_word_break_characters;
    rl_basic_word_break_characters   = " \t\n'\"\\@$=.,;!|&([{{<>}])";
    rl_attempted_completion_function = &cli::completion_function;

    register_command(
            "h",
            [this](plang::corpus *c, string_view_t const &a) { return _help(c, a); },
            "Prints this list");
    register_command(
            "p",
            [this](plang::corpus *c, std::vector<string_t> const &a) { return _option(c, a); },
            "Get/Set options",
            [this](auto... args) { return _option_complete(args...); });
    register_command(
            "i",
            [this](plang::corpus *c, string_view_t const &a) { return _info(c, a); },
            "Print current information");
    register_command(
            "q",
            [this](plang::corpus *c, string_view_t const &a) { return _quit(c, a); },
            "Quit command line");
    register_command(
            "n",
            [this](plang::corpus *c, string_view_t const &a) { return _new(c, a); },
            "Create a new corpus");
    register_command(
            "o",
            [this](plang::corpus *c, std::vector<string_t> const &a) { return _open(c, a); },
            "Open a corpus",
            [this](auto... args) { return _open_complete(args...); });
    register_command(
            "s",
            [this](plang::corpus *c, string_view_t const &a) { return _save(c, a); },
            "Save the current corpus",
            [this](auto... args) { return _save_complete(args...); });
    register_command(
            "w",
            [this](plang::corpus *c, string_view_t const &a) { return _close(c, a); },
            "Close the current corpus");
    register_command(
            "r",
            [this](plang::corpus *c, string_view_t const &a) { return _run(c, a); },
            "Run a PLang file");
    register_command(
            "e",
            [this](plang::corpus *c, string_view_t const &a) { return _export(c, a); },
            "Export the current corpus");
    register_command(
            "",
            [this](plang::corpus *c, string_view_t const &a) { return _inspect(c, a); },
            "Prints an entry");
    register_command(
            "l",
            [this](plang::corpus *c, string_view_t const &a) { return _list(c, a); },
            "List contained entries");
    register_command(
            "f",
            [this](plang::corpus *c, string_view_t const &a) { return _find(c, a); },
            "Search entries");
    register_command(
            "x",
            [this](plang::corpus *c, string_view_t const &a) { return _remove(c, a); },
            "Remove an entry");
    register_command(
            "c",
            [this](plang::corpus *c, string_view_t const &a) { return _copy(c, a); },
            "Copy an entry");
    register_command(
            "v",
            [this](plang::corpus *c, string_view_t const &a) { return _move(c, a); },
            "Move an entry");
    register_command(
            "z",
            [this](plang::corpus *c, string_view_t const &a) { return _undo(c, a); },
            "Undo the last step");
    register_command(
            "y",
            [this](plang::corpus *c, string_view_t const &a) { return _redo(c, a); },
            "Redo the last undone step");
}

bool_t cli::register_command(string_t command,
                             std::function<int_t(plang::corpus *, string_view_t const &)> func,
                             string_t description,
                             cmd_complete_func_t completor) {
    auto [it, ok] = commands.emplace(std::move(command), std::move(func));
    if (ok) {
        if (completor) { completers.emplace(it->first, std::move(completor)); }
        help.emplace_back(":" + it->first + "\t" + description);
    }
    return ok;
}

bool_t cli::register_command(string_t command,
                             std::function<int_t(plang::corpus *, std::vector<string_t> const &)> func,
                             string_t description,
                             cmd_complete_func_t completor) {
    auto argv_func = [func](plang::corpus *c, string_view_t const &args) {
        auto [argv, argn, argp] = cli::string_to_args(args);
        return func(c, argv);
    };
    return register_command(command, argv_func, description, completor);
}

bool_t cli::unregister_command(plang::column_types::string_t command) {
    auto i  = commands.erase(command) > 0;
    auto it = std::remove_if(help.begin(), help.end(), [&command](auto const &e) {
        return e.substr(0, e.find(':')) == command;
    });
    completers.erase(command);
    help.erase(it, help.end());

    return i > 0;
}

int_t cli::prompt() {
    static const string_t name{"PLang"};

    string_t prompt;

    if (options.prompt == prompt::POWERLINE) {
        format::style file_style{.text       = format::style::color::WHITE,
                                 .background = format::style::color::MAGENTA,
                                 .font       = format::style::font::BOLD};
        format::style file_arrow_style{.text = format::style::color::MAGENTA};

        auto prompt_format = get_prompt_format();

        prompt = prompt_format(" " + (file.has_value() ? file->filename().string() : name) + " ", file_style);

        if (scope.has_value()) {
            file_arrow_style.background = format::style::color::BLUE;
            prompt += prompt_format("\uE0B0 ", file_arrow_style);

            auto prompt_scope_path = scope_path;

            if (prompt_scope_path.size() > 1) {
                string_t::size_type n = 0;
                prompt_scope_path     = prompt_scope_path.substr(1);
                while ((n = prompt_scope_path.find('.', n)) != string_t::npos) {
                    char delim[] = " \001\033[38;5;6m\002\uE0B1\001\033[38;5;15m\002 ";
                    prompt_scope_path.replace(n, 1, delim);
                    n += sizeof(delim);
                }
            }

            format::style path_style{.text       = format::style::color::WHITE,
                                     .background = format::style::color::BLUE,
                                     .font       = format::style::font::BOLD};
            format::style path_delim_style{.text       = format::style::color::CYAN,
                                           .background = format::style::color::BLUE};
            format::style path_arrow_style{.text = format::style::color::BLUE};

            prompt += prompt_format(prompt_scope_path + ' ', path_style);
            prompt += prompt_format("\uE0B0 ", path_arrow_style);
        } else {
            prompt += prompt_format("\uE0B0 ", file_arrow_style);
        }
    } else if (options.prompt == prompt::UNICODE) {
        format::style file_style{.text = format::style::color::MAGENTA, .font = format::style::font::BOLD};
        auto prompt_format = get_prompt_format();

        prompt = prompt_format((file.has_value() ? file->filename().string() : name), file_style);

        if (scope.has_value()) {
            format::style path_style{.text = format::style::color::BRIGHT_BLUE, .font = format::style::font::BOLD};
            prompt += ' ' + options.format(scope_path, path_style);
        }

        prompt += "> ";
    }

    if (options.prompt != prompt::NONE) {
        char *buffer;
        buffer = readline(prompt.c_str());
        if (buffer) {
            string_view_t view{buffer};
            add_history(buffer);
            int result = handle_input(view);
            free(buffer);
            return result;
        }
    } else {
        string_t buffer;
        std::getline(std::cin, buffer, '\n');

        if (!buffer.empty()) {
            string_view_t view{buffer};
            add_history(buffer.c_str());
            return handle_input(view);
        }
    }

    return -1;
}

int_t cli::prompt_loop() {
    stop = false;
    int_t result;

    while (!stop) { result = prompt(); }

    return result;
}

cli::~cli() {
    rl_attempted_completion_function = old_rl_acf;
    current_instance                 = old_instance;
    rl_basic_word_break_characters   = old_w_br_chars;
}
