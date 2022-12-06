//
// Created by Johannes on 27.08.2022.
//

/** \page cli The Command Line

- \subpage cli_man
- \subpage cli_commands
- \subpage cli_options
*/

/** \page cli_man Manual page
# PLANG(1)

## Name

plang - PLang command line program

## Synopsis

<div class="doxygen-awesome-fragment-wrapper">
<div class="fragment">
plang [options] <u>file</u>
</div>
</div>

## Description

## Options

 <dl class="params">
 <dt></dt>
 <dd>
 <table class="params">
 <tr>
 <td class="paramname">-h</td>
 <td class="paramname">----help</td>
 <td>Show this help text and exit.</td>
 </tr>
 <tr>
 <td class="paramname">-s</td>
 <td class="paramname">----subject</td>
 <td>Set the subject position in the word order. Must be used with <code>-v</code> and <code>-o</code>.</td>
 </tr>
 <tr>
 <td class="paramname">-v</td>
 <td class="paramname">----verb</td>
 <td>Set the verb position in the word order. Must be used with <code>-s</code> and <code>-o</code>.</td>
 </tr>
 <tr>
 <td class="paramname">-o</td>
 <td class="paramname">----object</td>
 <td>Set the object position in the word order. Must be used with <code>-s</code> and <code>-v</code>.</td>
 </tr>
 <tr>
 <td class="paramname">-i</td>
 <td class="paramname">----no-interactive</td>
 <td>Removes interactive elements from stdout, so it may be used by other programs.</td>
 </tr>
 <tr>
 <td class="paramname"></td>
 <td class="paramname">----word-order=&lt;svo&gt;</td>
 <td>Set the word order. Any combination of <code>s</code>, <code>v</code> and <code>v</code>.</td>
 </tr>
 </table>
 </dd>
 </dl>

## Files
- [libsqlite3.so]
(https://www.sqlite.org/download.html#:~:text=Precompiled%20Binaries%20for%20Linux)
<small>(linux)</small> /
  [sqlite3.dll]
(https://www.sqlite.org/download.html#:~:text=Precompiled%20Binaries%20for%20Windows)
<small>(win)</small> /
  [libsqlite3.dylib]
(https://www.sqlite.org/download.html#:~:text=Precompiled%20Binaries%20for%20Mac%20OS%20X%20(x86))
<small>(mac)</small>
*/

/** \page cli_commands Command Reference
[TOC]

# General

## Print help


```
:h
```

Prints a list of availabe commands with a brief description.

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>

## Get/Set options

```
:p <option> <value>
```

Gets or sets an option of the command line.

\param option Option to set
\param value <small>(<i>opt</i>)</small> New value of the option

See the \ref cli_options page for a list of available options.

## Print current information

```
:i <ref>
```

Prints information about an entry in the corpus.

\param ref <small>(<i>opt</i>)</small> Entry to print. If left blank, the current scope is printed out.

## Quit command line

```
:q
```

Quits the application.

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>

# Database files

## Creating a database file

```
   :n
```

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>

## Opening a database file

```
:o <file>
```

\param file

## Save changes to a database file

```
:s <file>
```

\param file <small>(<i>opt</i>)</small>

## Closing a database file

```
:w
```

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>

# Markup files

## Run a markup file

```
:r <file>
```

\param file

## Export corpus to a markup file

```
:e <file>
```

\param file

# Traversal, querying

## Change scope

```
@<path>
```

\param path

## List entries

```
:l <ref> ...
```

\param ref <small>(<i>var</i>)</small>

## Search entries

```
:f <name> ...
```

# Manipulation

\param name <small>(<i>var</i>)</small>

## Delete entries

```
:x <ref> ...
```

\param ref <small>(<i>var</i>)</small>

## Copy entries

```
:c <from> <to>
```

\param from
\param to


## Move entries

```
:v <from> <to>
```

\param from
\param to

## Undo

```
   :z
```

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>

## Redo

```
   :y
```

<dl class="params">
<dt>Parameters</dt>
<dd>(<i>none</i>)</dd>
</dl>
*/

/**
\page cli_options Command Line Options

[TOC]

## Formatting options

### output

Sets the assumed frontend connected to stdout

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname"><u>ansi</u></td>
<td>Includes formatting through ANSI escape sequences</td>
</tr>
<tr>
<td class="paramname">readline</td>
<td>Includes formatting through ANSI escape sequences with additional markers for Readline</td>
</tr>
<tr>
<td class="paramname">html</td>
<td>Includes formatting through HTML tags and CSS stylesheets</td>
</tr>
<tr>
<td class="paramname">doxygen</td>
<td>Includes formatting through HTML tags and CSS stylesheets provided by Doxygen</td>
</tr>
</table>
</dd>
</dl>

### enrich

Sets formatting to apply

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname">plain</td>
<td>Plain text without formatting</td>
</tr>
<tr>
<td class="paramname">color</td>
<td>Colored text</td>
</tr>
<tr>
<td class="paramname">font</td>
<td>Text with font modifications (bold, italic, etc.)</td>
</tr>
<tr>
<td class="paramname"><u>rich</u></td>
<td>Text with color and font modifications</td>
</tr>
</table>
</dd>
</dl>

### detail

Sets how PLang entries should be displayed in output

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname">id_ref</td>
<td>As reference through IDs</td>
</tr>
<tr>
<td class="paramname">explicit_ref</td>
<td>As reference without IDs</td>
</tr>
<tr>
<td class="paramname"><u>definition</u></td>
<td>Equivalent PLang markup</td>
</tr>
<tr>
<td class="paramname">full</td>
<td>Equivalent PLang markup and related information</td>
</tr>
</table>
</dd>
</dl>

### qualification

Sets how paths are output

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname">unique</td>
<td>Shortened paths where possible</td>
</tr>
<tr>
<td class="paramname"><u>full</u></td>
<td>Always fully-qualified paths</td>
</tr>
</table>
</dd>
</dl>

### indent

Sets how PLang markup is indented

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname">compact</td>
<td>Most compact notation</td>
</tr>
<tr>
<td class="paramname">one_line</td>
<td>Spaced one-line notation</td>
</tr>
<tr>
<td class="paramname"><u>left</u></td>
<td>Multiline notation, left aligned</td>
</tr>
<tr>
<td class="paramname">center</td>
<td>Multiline notation, centered</td>
</tr>
</table>
</dd>
</dl>

## Parser options

### implicit

Whether entries may be declared implicitly

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname"><u>true</u></td>
<td></td>
</tr>
<tr>
<td class="paramname">false</td>
<td></td>
</tr>
</table>
</dd>
</dl>

### strict

Whether hints in class declarations are binding

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname"><u>true</u></td>
<td></td>
</tr>
<tr>
<td class="paramname">false</td>
<td></td>
</tr>
</table>
</dd>
</dl>

### topology

Sets the order for plot points.

<dl class="params">
<dt>Allowed values</dt>
<dd>
<table class="params">
<tr>
<td class="paramname"><u>svo</u></td>
<td>Subject &ndash; Verb &ndash; Object<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: Green;">symbol.class[symbol]</span></u> <u><span style="color: Crimson;">point.class</span></u> <u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u>;
</div></div></div>
</td>
</tr>
<tr>
<td class="paramname">sov</td>
<td>Subject &ndash; Object &ndash; Verb<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: Green;">symbol.class[symbol]</span></u> <u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u> <u><span style="color: Crimson;">point.class</span></u>;
</div></div></div></td>
</tr>
<tr>
<td class="paramname">vso</td>
<td>Verb &ndash; Subject &ndash; Object<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: Crimson;">point.class</span></u> <u><span style="color: Green;">symbol.class[symbol]</span></u> <u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u>;
</div></div></div></td>
</tr>
<tr>
<td class="paramname">vos</td>
<td>Verb &ndash; Object &ndash; Subject<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: Crimson;">point.class</span></u> <u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u> <u><span style="color: Green;">symbol.class[symbol]</span></u>;
</div></div></div></td>
</tr>
<tr>
<td class="paramname">ovs</td>
<td>Object &ndash; Verb &ndash; Subject<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u> <u><span style="color: Crimson;">point.class</span></u> <u><span style="color: Green;">symbol.class[symbol]</span></u>;
</div></div></div></td>
</tr>
<tr>
<td class="paramname">osv</td>
<td>Object &ndash; Subject &ndash; Verb<br/>
<b>Example:</b>
<div class="doxygen-awesome-fragment-wrapper"><div class="fragment"><div class="line">
<u><span style="color: MediumBlue;">: default_object</span> <span style="color: DarkMagenta;">?object_class: object</span></u> <u><span style="color: Green;">symbol.class[symbol]</span></u> <u><span style="color: Crimson;">point.class</span></u>;
</div></div></div></td>
</tr>
</table>
</dd>
</dl>
*/

#ifndef PLANGCLI_CLI_HPP
#define PLANGCLI_CLI_HPP

#include <map>
#include <optional>
#include <utility>
#include <unordered_map>
#include <plang/corpus.hpp>
#include <plangcli/intl.hpp>

namespace plang {

    /// \brief Core class for the interactive PLang command line
    class cli {

        /// The type of prompt being used by the command line
        enum class prompt {
            NONE,    ///< \brief No prompt
            UNICODE, ///< \brief Standard unicode
            POWERLINE///< \brief Unicode + powerline codepoints
        };

        /// \brief Contains options to control the command line and the parser
        struct {
            format format;  ///< \brief Set output format
            prompt prompt;  ///< \brief Assumed codepage of the output
            bool_t implicit;///< \brief <code>implicit</code> option for the parser
            bool_t strict;  ///< \brief <code>strict</code> option for the parser
            bool_t assume;  ///< \brief<code>assume</code> option for the parser
            bool_t exact;   ///< \brief<code>exact</code> option for the parser
        } options;

        /// \brief Callback type expecting a string
        using raw_cmd_func_t = std::function<int_t(plang::corpus *, string_view_t const &)>;

        /// \brief Callback type expecting an argument vector
        using argv_cmd_func_t = std::function<int_t(plang::corpus *, std::vector<string_t> const &)>;

        using cmd_complete_result_t = std::variant<char *(*) (const char *, int), std::vector<string_t>>;

        /// \brief Callback type for autocompletion
        using cmd_complete_func_t =
                std::function<cmd_complete_result_t(plang::corpus *, string_view_t const &, uint_t start, uint_t end)>;

    private:
        static std::map<string_t, format::output> output_option_map;
        static std::map<string_t, format::enrich> enrich_option_map;
        static std::map<string_t, format::detail> detail_option_map;
        static std::map<string_t, format::qualification> qualification_option_map;
        static std::map<string_t, format::indent> indent_option_map;
        static std::map<string_t, bool_t> bool_option_map;
        static std::map<string_t, prompt> prompt_option_map;
        static cli *current_instance;
        static cmd_complete_result_t autocomplete_candidates;

        std::optional<std::filesystem::path> file;///< \brief Path to current file
        std::optional<corpus> corpus;             ///< \brief Corpus for the command line
        std::optional<path> scope;                ///< \brief Path scope for the command line
        string_t scope_path;                      ///< \brief Representation of the current scope
        std::unordered_map<string_t, std::pair<raw_cmd_func_t, bool_t>> commands;///< \brief Registered commands
        std::unordered_map<string_t, cmd_complete_func_t> completers;            ///< \brief Registered completers
        std::vector<string_t> help;
        std::reference_wrapper<const dict_t> dict;
        bool_t unsaved;///< \brief Set to `true`, after each execution of PLang code that changed the corpus
        bool_t stop;   ///< \brief Stop token for \ref prompt_loop

        char **(*old_rl_acf)(const char *, int, int);///< \brief Old Readline autocomplete function
        cli *old_instance;                           ///< \brief Old instance managing Readline
        const char *old_w_br_chars;                  ///< \brief Old word breaking characters

        static std::tuple<std::vector<string_t>, std::size_t, std::size_t> string_to_args(string_view_t const &str,
                                                                                          int_t cur = 0);

        /// \brief Global callback function for Readline autocomplete
        /// \param text
        /// \param start
        /// \param end
        /// \return Array of candidates
        static char **completion_function(const char *text, int start, int end);

        /// \brief Global callback function for Readline autocompletion matches
        /// \param text
        /// \param state
        /// \return
        static char *candidate_generator(const char *text, int state);

        static cli::cmd_complete_result_t complete_filename(string_view_t const &arg);

        void check_corpus() const;

        void set_format();

        class format get_prompt_format() const;

        void set_scope(std::optional<path> scope);

        bool_t prompt_yes_no(string_t const &prompt) const;

        int_t _help(class corpus *, string_view_t const &);
        int_t _option(class corpus *, std::vector<string_t> const &);
        cmd_complete_result_t _option_complete(class corpus *, string_view_t const &, uint_t start, uint_t end);
        int_t _info(class corpus *, string_view_t const &);
        int_t _quit(class corpus *, string_view_t const &);
        int_t _new(class corpus *, string_view_t const &);
        int_t _open(class corpus *, std::vector<string_t> const &);
        cmd_complete_result_t _open_complete(class corpus *, string_view_t const &, uint_t start, uint_t end);
        int_t _save(class corpus *, string_view_t const &);
        cmd_complete_result_t _save_complete(class corpus *, string_view_t const &, uint_t start, uint_t end);
        int_t _close(class corpus *, string_view_t const &);
        int_t _run(class corpus *, std::vector<string_t> const &);
        int_t _export(class corpus *, string_view_t const &);
        int_t _inspect(class corpus *, string_view_t const &);
        int_t _scope(class corpus *, string_view_t const &);
        int_t _list(class corpus *, string_view_t const &);
        int_t _find(class corpus *, string_view_t const &);
        int_t _remove(class corpus *, string_view_t const &);
        int_t _copy(class corpus *, string_view_t const &);
        int_t _move(class corpus *, string_view_t const &);
        int_t _undo(class corpus *, string_view_t const &);
        int_t _redo(class corpus *, string_view_t const &);

        int_t handle_command(string_view_t const &view);

        int_t handle_plang(string_view_t const &view);

        int_t handle_input(string_view_t const &view);

    public:
        /// \brief Default constructor
        cli();

        /// \brief Registers a new command to the command line
        /// \param command Name of the command (0th argument)
        /// \param func Callback function implementing the command
        /// \param description Short description of the command displayed by <code>:h></code>
        /// \param completor Callback function for autocomplete
        /// \return `true`, if the command could be registered
        /// \details The callback function will receive a pointer to the opened corpus
        /// and a view of the entered command without the 0th argument
        bool_t register_command(string_t command,
                                raw_cmd_func_t func,
                                string_t description,
                                cmd_complete_func_t completor = nullptr);

        /// \brief Registers a new command to the command line
        /// \param command Name of the command (0th argument)
        /// \param func Callback function implementing the command
        /// \param description Short description of the command displayed by <code>:h></code>
        /// \param completor Callback function for autocomplete
        /// \return `true`, if the command could be registered
        /// \details The callback function will receive a pointer to the opened corpus
        /// and a argument vector as parsed by system shells
        bool_t register_command(string_t command,
                                argv_cmd_func_t func,
                                string_t description,
                                cmd_complete_func_t completor = nullptr);

        /// Unregisters a command
        /// \param command Name of the command to unregister
        /// \return `true`, if the command could be unregistered
        bool_t unregister_command(string_t command);

        /// \brief Prompts and executes a single command
        /// \return Return code of the executed command
        int_t prompt();

        /// \brief Prompts and executes commands in a loop
        /// \return Return code of the quitting / throwing command
        int_t prompt_loop();

        /// \brief Default destructor
        ~cli();
    };

}// namespace plang

#endif//PLANGCLI_CLI_HPP
