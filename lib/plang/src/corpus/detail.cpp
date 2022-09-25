//
// Created by Johannes on 16.08.2022.
//

#include <regex>
#include <unordered_map>
#include <plang/corpus/detail.hpp>
#include "sqlite.hpp"
#include "version.hpp"

EXTERN(ddl_pragma_sql);
EXTERN(ddl_base_sql);
EXTERN(ddl_plot_symbol_sql);
EXTERN(ddl_plot_point_sql);
EXTERN(ddl_plot_object_sql);
EXTERN(ddl_plot_causal_sql);

extern "C" {
extern int sqlite3UndoInit(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);
}

using namespace plang;
using namespace plang::op;

format::style const &format::style::for_op(lang::op op) {
    static style core{.text = format::style::color::BLUE, .font = format::style::font::BOLD};
    static style red{.text = format::style::color::RED, .font = format::style::font::BOLD};
    static style sym{.text = format::style::color::MAGENTA, .font = format::style::font::BOLD};
    static style sym_par{.text = format::style::color::MAGENTA};
    static style pnt{.text = format::style::color::CYAN, .font = format::style::font::BOLD};
    static style pnt_par{.text = format::style::color::CYAN};
    static style cau{.text = format::style::color::YELLOW, .font = format::style::font::BOLD};
    static style str{.text = format::style::color::GREEN, .font = format::style::font::BOLD};
    static style com{.text = format::style::color::LIGHT_GRAY, .font = format::style::font::ITALIC};
    static style com_par{.text = format::style::color::LIGHT_GRAY};
    static style def{};


    switch (op) {
        case lang::op::PATH:
        case lang::op::RECUR:
        case lang::op::ID:
        case lang::op::REQ_PRV:
        case lang::op::REQ_NOW:
        case lang::op::IPL_DEF:
        case lang::op::IPL_NOW:
        case lang::op::ESCAPE:
        case lang::op::CONTEXT:
            return core;

        case lang::op::SINGLE:
        case lang::op::NEGATE:
            return red;

        case lang::op::SYM:
            return sym;

        case lang::op::SYM_L:
        case lang::op::SYM_R:
            return sym_par;

        case lang::op::OBJ:
        case lang::op::OBJ_NAME:
        case lang::op::PNT:
            return pnt;

        case lang::op::PNT_L:
        case lang::op::PNT_R:
            return pnt_par;

        case lang::op::CAU_L:
        case lang::op::CAU_R:
        case lang::op::INDIR:
        case lang::op::REQ_ALL:
        case lang::op::REQ_ONE:
        case lang::op::REQ_UNQ:
            return cau;

        case lang::op::STR_DELIM_SINGLE:
        case lang::op::STR_DELIM_DOUBLE:
        case lang::op::STR_L:
        case lang::op::STR_G:
        case lang::op::STR_R:
        case lang::op::STR_M:
            return str;

        case lang::op::COMMENT:
        case lang::op::COMMENT_L:
        case lang::op::COMMENT_R:
            return com;

        case lang::op::HINT_L:
        case lang::op::HINT_R:
            return com_par;

        case lang::op::LIST:
        case lang::op::DECL:
        case lang::op::ASSIGN:
            return def;
    }

    return def;
}

string_t const &format::get_open() const {
    static string_t closes[4]{"", "", "<span style=\"", "<span style=\""};

    return closes[static_cast<decltype(value)>(get_output()) & OUTPUT_MASK];
}

string_t format::get_text_color(const plang::format::style &style) const {
    static std::unordered_map<format::style::color, string_t> ansi{
            {format::style::color::BLACK,          "\033[38;5;0m" },
            {format::style::color::RED,            "\033[38;5;1m" },
            {format::style::color::GREEN,          "\033[38;5;2m" },
            {format::style::color::YELLOW,         "\033[38;5;3m" },
            {format::style::color::BLUE,           "\033[38;5;4m" },
            {format::style::color::MAGENTA,        "\033[38;5;5m" },
            {format::style::color::CYAN,           "\033[38;5;6m" },
            {format::style::color::LIGHT_GRAY,     "\033[38;5;7m" },
            {format::style::color::GRAY,           "\033[38;5;8m" },
            {format::style::color::BRIGHT_RED,     "\033[38;5;9m" },
            {format::style::color::BRIGHT_GREEN,   "\033[38;5;10m"},
            {format::style::color::BRIGHT_YELLOW,  "\033[38;5;11m"},
            {format::style::color::BRIGHT_BLUE,    "\033[38;5;12m"},
            {format::style::color::BRIGHT_MAGENTA, "\033[38;5;13m"},
            {format::style::color::BRIGHT_CYAN,    "\033[38;5;14m"},
            {format::style::color::WHITE,          "\033[38;5;15m"},
            {format::style::color::DEFAULT,        "\033[39m"     }
    };

    static std::unordered_map<format::style::color, string_t> html{
            {format::style::color::BLACK,          "color: Black;"        },
            {format::style::color::RED,            "color: Crimson;"      },
            {format::style::color::GREEN,          "color: Green;"        },
            {format::style::color::YELLOW,         "color: Gold;"         },
            {format::style::color::BLUE,           "color: MediumBlue;"   },
            {format::style::color::MAGENTA,        "color: DarkMagenta;"  },
            {format::style::color::CYAN,           "color: DarkTurquoise;"},
            {format::style::color::LIGHT_GRAY,     "color: LightGray;"    },
            {format::style::color::GRAY,           "color: Gray;"         },
            {format::style::color::BRIGHT_RED,     "color: Red;"          },
            {format::style::color::BRIGHT_GREEN,   "color: LimeGreen;"    },
            {format::style::color::BRIGHT_YELLOW,  "color: Yellow;"       },
            {format::style::color::BRIGHT_BLUE,    "color: Blue;"         },
            {format::style::color::BRIGHT_MAGENTA, "color: Magenta;"      },
            {format::style::color::BRIGHT_CYAN,    "color: Cyan;"         },
            {format::style::color::WHITE,          "color: White;"        },
            {format::style::color::DEFAULT,        ""                     }
    };

    switch (style.text.index()) {
        case 0:
            switch (get_output()) {
                case output::ANSI:
                    return ansi.at(std::get<format::style::color>(style.text));
                case output::READLINE:
                    return '\001' + ansi.at(std::get<format::style::color>(style.text)) + '\002';
                case output::HTML:
                    return html.at(std::get<format::style::color>(style.text));
                default:
                    break;
            }
        case 1:
            switch (get_output()) {
                case output::ANSI:
                    return "\033[38;5;" + std::to_string(std::get<std::uint8_t>(style.text)) + "m";
                case output::READLINE:
                    return "\001\033[38;5;" + std::to_string(std::get<std::uint8_t>(style.text)) + "m\002";
                case output::HTML:
                    break;
                default:
                    break;
            }
        case 2:
            switch (get_output()) {
                case output::ANSI: {
                    string_t const &c = std::get<char[22]>(style.text);
                    return "\033[38;2;" + c.substr(1, 2) + ';' + c.substr(3, 2) + ';' + c.substr(5, 2) + 'm';
                }
                case output::READLINE: {
                    string_t const &c = std::get<char[22]>(style.text);
                    return "\001\033[38;2;" + c.substr(1, 2) + ';' + c.substr(3, 2) + ';' + c.substr(5, 2) + "m\002";
                }
                case output::HTML:
                    return std::get<char[22]>(style.text);
                default:
                    break;
            }
    }

    return string_t{};
}

string_t format::get_background_color(const plang::format::style &style) const {
    static std::unordered_map<format::style::color, string_t> ansi{
            {format::style::color::BLACK,          "\033[48;5;0m" },
            {format::style::color::RED,            "\033[48;5;1m" },
            {format::style::color::GREEN,          "\033[48;5;2m" },
            {format::style::color::YELLOW,         "\033[48;5;3m" },
            {format::style::color::BLUE,           "\033[48;5;4m" },
            {format::style::color::MAGENTA,        "\033[48;5;5m" },
            {format::style::color::CYAN,           "\033[48;5;6m" },
            {format::style::color::LIGHT_GRAY,     "\033[48;5;7m" },
            {format::style::color::GRAY,           "\033[48;5;8m" },
            {format::style::color::BRIGHT_RED,     "\033[48;5;9m" },
            {format::style::color::BRIGHT_GREEN,   "\033[48;5;10m"},
            {format::style::color::BRIGHT_YELLOW,  "\033[48;5;11m"},
            {format::style::color::BRIGHT_BLUE,    "\033[48;5;12m"},
            {format::style::color::BRIGHT_MAGENTA, "\033[48;5;13m"},
            {format::style::color::BRIGHT_CYAN,    "\033[48;5;14m"},
            {format::style::color::WHITE,          "\033[48;5;15m"},
            {format::style::color::DEFAULT,        "\033[49m"     }
    };

    static std::unordered_map<format::style::color, string_t> html{
            {format::style::color::BLACK,          "background-color: Black;"        },
            {format::style::color::RED,            "background-color: Crimson;"      },
            {format::style::color::GREEN,          "background-color: Green;"        },
            {format::style::color::YELLOW,         "background-color: Gold;"         },
            {format::style::color::BLUE,           "background-color: MediumBlue;"   },
            {format::style::color::MAGENTA,        "background-color: DarkMagenta;"  },
            {format::style::color::CYAN,           "background-color: DarkTurquoise;"},
            {format::style::color::LIGHT_GRAY,     "background-color: LightGray;"    },
            {format::style::color::GRAY,           "background-color: Gray;"         },
            {format::style::color::BRIGHT_RED,     "background-color: Red;"          },
            {format::style::color::BRIGHT_GREEN,   "background-color: LimeGreen;"    },
            {format::style::color::BRIGHT_YELLOW,  "background-color: Yellow;"       },
            {format::style::color::BRIGHT_BLUE,    "background-color: Blue;"         },
            {format::style::color::BRIGHT_MAGENTA, "background-color: Magenta;"      },
            {format::style::color::BRIGHT_CYAN,    "background-color: Cyan;"         },
            {format::style::color::WHITE,          "background-color: White;"        },
            {format::style::color::DEFAULT,        ""                                }
    };

    switch (style.text.index()) {
        case 0:
            switch (get_output()) {
                case output::ANSI:
                    return ansi.at(std::get<format::style::color>(style.background));
                case output::READLINE:
                    return '\001' + ansi.at(std::get<format::style::color>(style.background)) + '\002';
                case output::HTML:
                    return html.at(std::get<format::style::color>(style.background));
                default:
                    break;
            }
        case 1:
            switch (get_output()) {
                case output::ANSI:
                    return std::to_string(std::get<std::uint8_t>(style.background));
                case output::READLINE:
                    return '\001' + std::to_string(std::get<std::uint8_t>(style.background)) + '\002';
                case output::HTML:
                    break;
                default:
                    break;
            }
        case 2:
            switch (get_output()) {
                case output::ANSI: {
                    string_t const &c = std::get<char[22]>(style.background);
                    return c.substr(1, 2) + ';' + c.substr(3, 2) + ';' + c.substr(5, 2) + 'm';
                }
                case output::READLINE: {
                    string_t const &c = std::get<char[22]>(style.background);
                    return '\001' + c.substr(1, 2) + ';' + c.substr(3, 2) + ';' + c.substr(5, 2) + "m\002";
                }
                case output::HTML:
                    return std::get<char[22]>(style.background);
                default:
                    break;
            }
    }

    return string_t{};
}

string_t format::get_font_begin(const plang::format::style &style) const {
    static std::unordered_map<format::style::font_t, string_t> ansi{
            {format::style::font::NORMAL,        "\033[0m"},
            {format::style::font::BOLD,          "\033[1m"},
            {format::style::font::ITALIC,        "\033[3m"},
            {format::style::font::UNDERLINE,     "\033[4m"},
            {format::style::font::CODE,          ""       },
            {format::style::font::STRIKETHROUGH, "\033[9m"},
            {format::style::font::INVERTED,      "\033[7m"}
    };

    static std::unordered_map<format::style::font_t, string_t> readline{
            {format::style::font::NORMAL,        "\001\033[0m\002"},
            {format::style::font::BOLD,          "\001\033[1m\002"},
            {format::style::font::ITALIC,        "\001\033[3m\002"},
            {format::style::font::UNDERLINE,     "\001\033[4m\002"},
            {format::style::font::CODE,          ""               },
            {format::style::font::STRIKETHROUGH, "\001\033[9m\002"},
            {format::style::font::INVERTED,      "\001\033[7m\002"}
    };

    static std::unordered_map<format::style::font_t, string_t> html{
            {format::style::font::NORMAL,        ""        },
            {format::style::font::BOLD,          "<b>"     },
            {format::style::font::ITALIC,        "<i>"     },
            {format::style::font::UNDERLINE,     "<u>"     },
            {format::style::font::CODE,          "<code>"  },
            {format::style::font::STRIKETHROUGH, "<strike>"},
            {format::style::font::INVERTED,      ""        }
    };

    static std::unordered_map<format::style::font_t, string_t> doxygen{
            {format::style::font::NORMAL,        ""        },
            {format::style::font::BOLD,          "<b>"     },
            {format::style::font::ITALIC,        "<i>"     },
            {format::style::font::UNDERLINE,     "<u>"     },
            {format::style::font::CODE,          "<code>"  },
            {format::style::font::STRIKETHROUGH, "<strike>"},
            {format::style::font::INVERTED,
             "<span style=\"background-color: var(--page-foreground-color); "
             "color: var(--page-background-color);\">"     }
    };

    auto &map = [this]() -> auto & {
        switch (get_output()) {
            case output::ANSI:
                return ansi;
            case output::READLINE:
                return readline;
            case output::HTML:
                return html;
            case output::DOXYGEN:
                return doxygen;
        }

        return ansi;
    }
    ();

    string_t buffer;

    if (style.font & style::font::NORMAL) { return buffer; }
    if (style.font & style::font::BOLD) { buffer += map[style::font::BOLD]; }
    if (style.font & style::font::ITALIC) { buffer += map[style::font::ITALIC]; }
    if (style.font & style::font::UNDERLINE) { buffer += map[style::font::UNDERLINE]; }
    if (style.font & style::font::CODE) { buffer += map[style::font::CODE]; }
    if (style.font & style::font::STRIKETHROUGH) { buffer += map[style::font::STRIKETHROUGH]; }
    if (style.font & style::font::INVERTED) { buffer += map[style::font::INVERTED]; }

    return buffer;
}

string_t format::get_font_end(const plang::format::style &style) const {
    using namespace plang::op;

    static std::unordered_map<format::style::font_t, string_t> ansi{
            {format::style::font::NORMAL,        "\033[0m" },
            {format::style::font::BOLD,          "\033[22m"},
            {format::style::font::ITALIC,        "\033[23m"},
            {format::style::font::UNDERLINE,     "\033[24m"},
            {format::style::font::CODE,          ""        },
            {format::style::font::STRIKETHROUGH, "\033[29m"},
            {format::style::font::INVERTED,      "\033[27m"}
    };

    static std::unordered_map<format::style::font_t, string_t> readline{
            {format::style::font::NORMAL,        "\001\033[0m\002" },
            {format::style::font::BOLD,          "\001\033[22m\002"},
            {format::style::font::ITALIC,        "\001\033[23m\002"},
            {format::style::font::UNDERLINE,     "\001\033[24m\002"},
            {format::style::font::CODE,          ""                },
            {format::style::font::STRIKETHROUGH, "\001\033[29m\002"},
            {format::style::font::INVERTED,      "\001\033[27m\002"}
    };

    static std::unordered_map<format::style::font_t, string_t> html{
            {format::style::font::NORMAL,        ""         },
            {format::style::font::BOLD,          "</b>"     },
            {format::style::font::ITALIC,        "</i>"     },
            {format::style::font::UNDERLINE,     "</u>"     },
            {format::style::font::CODE,          "</code>"  },
            {format::style::font::STRIKETHROUGH, "</strike>"},
            {format::style::font::INVERTED,      "</span>"  }
    };

    static std::unordered_map<format::style::font_t, string_t> doxygen{
            {format::style::font::NORMAL,        ""         },
            {format::style::font::BOLD,          "</b>"     },
            {format::style::font::ITALIC,        "</i>"     },
            {format::style::font::UNDERLINE,     "</u>"     },
            {format::style::font::CODE,          "</code>"  },
            {format::style::font::STRIKETHROUGH, "</strike>"},
            {format::style::font::INVERTED,      "</span>"  }
    };

    auto &map = [this]() -> auto & {
        switch (get_output()) {
            case output::ANSI:
                return ansi;
            case output::READLINE:
                return readline;
            case output::HTML:
                return html;
            case output::DOXYGEN:
                return doxygen;
        }

        return ansi;
    }
    ();

    string_t buffer;

    if (style.font & style::font::BOLD) { buffer += map[style::font::BOLD]; }
    if (style.font & style::font::ITALIC) { buffer += map[style::font::ITALIC]; }
    if (style.font & style::font::UNDERLINE) { buffer += map[style::font::UNDERLINE]; }
    if (style.font & style::font::CODE) { buffer += map[style::font::CODE]; }
    if (style.font & style::font::STRIKETHROUGH) { buffer += map[style::font::STRIKETHROUGH]; }
    if (style.font & style::font::INVERTED) { buffer += map[style::font::INVERTED]; }

    return buffer;
}

string_t const &format::get_close() const {
    static string_t closes[4]{"\033[0m", "\001\033[0m\002", "</span>", "</span>"};

    return closes[static_cast<decltype(value)>(get_output()) & OUTPUT_MASK];
}

string_t format::operator()(string_t const &str, style const &style) const {
    switch (get_output()) {
        case output::ANSI: {
            return (*this & enrich::COLOR ? get_text_color(style) + get_background_color(style) : "") +
                   (*this & enrich::FONT ? get_font_begin(style) : "") + str +
                   (*this & enrich::RICH ? get_close() : "");
        }
        case output::READLINE: {
            return (*this & enrich::COLOR ? get_text_color(style) + get_background_color(style) : "") +
                   (*this & enrich::FONT ? get_font_begin(style) : "") + str +
                   (*this & enrich::RICH ? get_close() : "");
        }
        case output::HTML:
        case output::DOXYGEN: {
            std::string buffer;
            buffer.reserve(str.size());
            for (size_t pos = 0; pos != str.size(); ++pos) {
                switch (str[pos]) {
                    case '&':
                        buffer.append("&amp;");
                        break;
                    case '\"':
                        buffer.append("&quot;");
                        break;
                    case '\'':
                        buffer.append("&apos;");
                        break;
                    case '<':
                        buffer.append("&lt;");
                        break;
                    case '>':
                        buffer.append("&gt;");
                        break;
                    default:
                        buffer.append(&str[pos], 1);
                        break;
                }
            }

            auto css = get_text_color(style) + get_background_color(style);

            if (!css.empty()) {
                return get_open() + css + "\">" + get_font_begin(style) + buffer + get_font_end(style) + get_close();
            } else {
                return get_font_begin(style) + buffer + get_font_end(style);
            }
        }
    }

    return str;
}

void detail::stmt_finalizer::operator()(sqlite3_stmt *stmt) {
    sqlite3_finalize(stmt);
}

format detail::corpus::_make_inner_format(format format) {
    class format result = format;
    result.set_detail(format::detail::EXPLICIT_REF);
    result.set_breaks(format::breaks::COMPACT);
    return result;
}

detail::corpus::corpus()
    : source({}, PLANG_VERSION, {}),
      outer_format(format::output::ANSI,
                   format::enrich::PLAIN,
                   format::detail::DEFINITION,
                   format::qualification::FULL,
                   format::breaks::LEFT),
      inner_format(format::output::ANSI,
                   format::enrich::PLAIN,
                   format::detail::EXPLICIT_REF,
                   format::qualification::FULL,
                   format::breaks::ONE_LINE),
      db() {}

void detail::corpus::regexp(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
    std::regex re{reinterpret_cast<const char *>(sqlite3_value_text(argv[0]))};

    sqlite3_result_int(ctx, std::regex_match(reinterpret_cast<const char *>(sqlite3_value_text(argv[1])), re));
}

void detail::corpus::_open(char const *file) {
    sqlite3 *ptr{};
    sqlite3_auto_extension(reinterpret_cast<void (*)(void)>(sqlite3UndoInit));
    _check(sqlite3_open(file, &ptr));
    db.reset(ptr, sqlite3_close);
    sqlite3_create_function(db.get(), "regexp", 2, SQLITE_UTF8, nullptr, regexp, nullptr, nullptr);
}

int detail::corpus::_check(int code) const {
    switch (code) {
        case SQLITE_OK:
        case SQLITE_ROW:
        case SQLITE_DONE:
            return code;

        default:
            throw std::logic_error(sqlite3_errmsg(db.get()));
    }
}

void detail::corpus::_prepare(stmt &stmt, const string_t &query) const {
    sqlite3_stmt *ptr;
    sqlite3_prepare_v2(&*db, query.c_str(), -1, &ptr, nullptr);
    if (!ptr) {
        throw std::runtime_error(sqlite3_errmsg(&*db));
    } else {
        stmt.reset(ptr);
    }
}

void detail::corpus::_reset(stmt &stmt) const {
    sqlite3_reset(&*stmt);
}

int detail::corpus::_exec(string_t const &query, int (*fun)(int, char *[], char *[])) {
    char *errmsg;

    int ret = sqlite3_exec(
            &*db,
            query.c_str(),
            fun ? [](void *par, int argc, char **argv, char **argn) {
                decltype(fun) fun = reinterpret_cast<decltype(fun)>(par);
                return fun(argc, argv, argn);
            } : static_cast<int (*)(void *,int,char **,char **)>(nullptr),
            fun ? reinterpret_cast<void *>(fun) : nullptr,
            &errmsg);
    if (errmsg) {
        std::logic_error error{string_t(errmsg)};
        sqlite3_free(errmsg);
        throw error;
    } else {
        return ret;
    }
}

void detail::corpus::_set_pragmas() {
    _exec({reinterpret_cast<char *>(ddl_pragma_sql), ddl_pragma_sql_len});
}

void detail::corpus::_create_schema() {
    for (auto &[q, l] : {std::make_tuple(ddl_base_sql, ddl_base_sql_len),
                         std::make_tuple(ddl_plot_symbol_sql, ddl_plot_symbol_sql_len),
                         std::make_tuple(ddl_plot_point_sql, ddl_plot_point_sql_len),
                         std::make_tuple(ddl_plot_object_sql, ddl_plot_object_sql_len),
                         std::make_tuple(ddl_plot_causal_sql, ddl_plot_causal_sql_len)}) {
        _exec({reinterpret_cast<char const *>(q), l});
    }
}

void detail::corpus::_make_undoable() {
    //Ignore possible syntax errors in the query. The undo symbols only exist at runtime.
    //language=sqlite
    static const string_t query{R"__SQL__(
WITH sub AS (SELECT name
             FROM sqlite_master
             WHERE type = 'table'
               and name regexp '(source|path|plot_.*|story_.*|outline_.*|text_.*)')
SELECT *
FROM sub
         LEFT JOIN undoable_table(sub.name, 2);
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    _reuse(stmt, query);
}

void detail::corpus::_create_source() {
    //language=sqlite
    static const string_t query{R"__SQL__(
INSERT INTO source (name, version, url, start)
VALUES (?1, ?2, ?3, ?4)
RETURNING id;
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    source.id = _reuse(stmt, query, [&] {
        if (auto const &name = source.get_url()) {
            sqlite3_bind_text(&*stmt, 1, name->c_str(), name->length(), nullptr);
        }
        if (auto const &version = source.get_url()) {
            sqlite3_bind_text(&*stmt, 2, version->c_str(), version->length(), nullptr);
        }
        if (auto const &url = source.get_url()) { sqlite3_bind_text(&*stmt, 3, url->c_str(), url->length(), nullptr); }
        sqlite3_bind_int64(&*stmt, 4, source.get_start().time_since_epoch().count());

        if (_check(sqlite3_step(&*stmt))) {
            return sqlite3_column_int(&*stmt, 0);
        } else {
            throw std::logic_error("Could not create source.");
        }
    });
}

pkey_t detail::corpus::_get_source_id() {
    if (!source.is_persisted()) { _create_source(); }

    return source.get_id();
}

void detail::corpus::_close() {
    if (source.is_persisted()) {
        static const string_t query{R"__SQL__(
UPDATE source
SET end = CURRENT_TIMESTAMP
WHERE id = ?1;
)__SQL__"};

        /*static thread_local*/ stmt stmt;

        source.id = _reuse(stmt, query, [&] {
            sqlite3_bind_int(&*stmt, 1, source.get_id());

            if (_check(sqlite3_step(&*stmt))) {
                return sqlite3_column_int(&*stmt, 0);
            } else {
                throw std::logic_error("Could not set end timestamp on source.");
            }
        });
    }
    db.reset();
}

std::tuple<uint_t, string_t> detail::corpus::undo() {
    //Ignore possible syntax errors in the query. The undo symbols only exist at runtime.
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT undo();
SELECT count(*) FROM _sqlite_undo
WHERE status='U';
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::tuple<uint_t, string_t> {
        _check(sqlite3_step(&*stmt));
        return {sqlite3_column_int(&*stmt, 0), ""};
    });
}

std::tuple<uint_t, string_t> detail::corpus::redo() {
    //Ignore possible syntax errors in the query. The undo symbols only exist at runtime.
    //language=sqlite
    static const string_t query{R"__SQL__(
SELECT redo();
SELECT count(*) FROM _sqlite_undo
WHERE status='R';
)__SQL__"};

    /*static thread_local*/ stmt stmt;

    return _reuse(stmt, query, [&]() -> std::tuple<uint_t, string_t> {
        _check(sqlite3_step(&*stmt));
        return {sqlite3_column_int(&*stmt, 0), ""};
    });
}

bool_t detail::corpus::flush() {
    return _check(sqlite3_db_cacheflush(&*db)) == 0;
}

const format &detail::corpus::get_format() const {
    return outer_format;
}

void detail::corpus::set_format(const plang::format &format) {
    outer_format = format;
    inner_format = _make_inner_format(format);
}

const format &detail::corpus::get_inner_format() const {
    return inner_format;
}

bool_t detail::corpus::backup(detail::corpus &target) {
    auto backup = sqlite3_backup_init(&*target.db, "main", &*db, "main");
    if (backup) {
        _check(sqlite3_backup_step(backup, -1));
        _check(sqlite3_backup_finish(backup));
    }

    return backup;
}

detail::corpus::~corpus() noexcept {
    _close();
}
