//
// Created by Johannes on 14.09.2022.
//

#include <plangcli/intl.hpp>

using namespace plang;

dict_t const &plang::en() {
    static dict_t en{
            {texts::LANG_ERROR_MALFORMED_EXPRESSION,   "The expression %s is malformed"                        },
            {texts::LANG_ERROR_AMBIGUOUS_REFERENCE,    "The reference to %s is ambiguous"                      },
            {texts::LANG_ERROR_REDEFINITION,           "The entry %s was redefined within the same declaration"},
            {texts::LANG_ERROR_HINT_MATCH,             "The entry %s does not match any hint %s"               },
            {texts::LANG_ERROR_DUPLICATE_PATH,         "The path %s already exists"                            },
            {texts::LANG_ERROR_DUPLICATE_SYMBOL,       "The symbol %s is already defined"                      },
            {texts::LANG_ERROR_DUPLICATE_OBJECT_CLASS, "The object class is already defined"                   },

            {texts::CLI_YES,                           "Yes"                                                   },
            {texts::CLI_NO,                            "No"                                                    },

            {texts::CLI_OPEN_FILENAME,                 "File to open"                                          },
            {texts::CLI_SAVE_FILENAME,                 "Save to file"                                          },
            {texts::CLI_CONFIRM_SAVE,                  "Save changes?"                                         },
            {texts::CLI_CONFIRM_UNSAVED_QUIT,          "You have unsaved changes. Quit anyways?"               },
            {texts::CLI_CONFIRM_OVERRIDE_SAVE,         "The file %s already exists. Override?"                 },

            {texts::CLI_UNDID,                         "Undid "                                                },
            {texts::CLI_REDID,                         "Redid "                                                },

            {texts::CLI_REPORT_MENTIONED,              "Mentioned entries"                                     },
            {texts::CLI_REPORT_INSERTED,               "Inserted entries"                                      },
            {texts::CLI_REPORT_UPDATED,                "Updated entries"                                       },
            {texts::CLI_REPORT_REMOVED,                "Removed entries"                                       },
            {texts::CLI_REPORT_FAILED,                 "Failed entries"                                        },

            {texts::CLI_ERROR_UNKNOWN_COMMAND,         "Unknown command"                                       },
            {texts::CLI_ERROR_MALFORMED_COMMAND,       "Malformed command"                                     },
            {texts::CLI_ERROR_UNBALANCED_QUOTES,       "Unbalanced quotes"                                     },
            {texts::CLI_ERROR_NO_CORPUS,               "No corpus opened"                                      },
            {texts::CLI_ERROR_READ_FAILURE,            "Cannot read from %s"                                   },
            {texts::CLI_ERROR_WRITE_FAILURE,           "Cannot write to %s"                                    },
            {texts::CLI_ERROR_MALFORMED_PATH,          "Malformed path"                                        }
    };

    return en;
}
