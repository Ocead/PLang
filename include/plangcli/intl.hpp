//
// Created by Johannes on 14.09.2022.
//

#ifndef PLANGCLI_INTL_HPP
#define PLANGCLI_INTL_HPP

#include <unordered_map>
#include <plang/base.hpp>

namespace plang {

    enum class texts {
        LANG_ERROR_MALFORMED_EXPRESSION,
        LANG_ERROR_AMBIGUOUS_REFERENCE,
        LANG_ERROR_REDEFINITION,
        LANG_ERROR_HINT_MATCH,
        LANG_ERROR_DUPLICATE_PATH,
        LANG_ERROR_DUPLICATE_SYMBOL,
        LANG_ERROR_DUPLICATE_OBJECT_CLASS,

        CLI_YES,
        CLI_NO,

        CLI_OPEN_FILENAME,
        CLI_SAVE_FILENAME,
        CLI_CONFIRM_SAVE,
        CLI_CONFIRM_UNSAVED_QUIT,
        CLI_CONFIRM_OVERRIDE_SAVE,

        CLI_UNDID,
        CLI_REDID,

        CLI_REPORT_MENTIONED,
        CLI_REPORT_INSERTED,
        CLI_REPORT_UPDATED,
        CLI_REPORT_REMOVED,
        CLI_REPORT_FAILED,

        CLI_ERROR_UNKNOWN_COMMAND,
        CLI_ERROR_MALFORMED_COMMAND,
        CLI_ERROR_UNBALANCED_QUOTES,
        CLI_ERROR_NO_CORPUS,
        CLI_ERROR_READ_FAILURE,
        CLI_ERROR_WRITE_FAILURE,
        CLI_ERROR_MALFORMED_PATH
    };

    using dict_t = std::unordered_map<texts, string_t>;

    dict_t const &en();

}// namespace plang

#endif//PLANGCLI_INTL_HPP
