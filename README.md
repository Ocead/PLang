# ![](docs/img/ocead.svg) <u>PLang</u> - Interactive markup language for narrative works

A formal language to plan and analyse prose, screenplays, games, etc.

<div style="text-align: center;">

![](https://img.shields.io/badge/C++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white")
![](https://img.shields.io/badge/build-passing-success?style=flat)
![](https://img.shields.io/badge/platform-linux_x64_|_win_x64-informational?style=flat)
![](https://img.shields.io/badge/license-AGPL_v3-informational?style=flat)
![](https://img.shields.io/badge/version-v0.0.1-important?style=flat)<br/>
![](https://img.shields.io/badge/tests-155_passed,_11_failed-critical?style=flat)
![](https://img.shields.io/badge/line_coverage-75%25-critical?style=flat)
![](https://img.shields.io/badge/branch_coverage-16%25-critical?style=flat)
</div>

---

This project aims to help:

- `🔗` maintaining logical consistence and keeping track of consequences
- `🧵` organising story threads
- `🔍` tracking introduction and usage of plot points
- `📖` cataloguing the content and monitoring its visibility

in narrative works, regardless of form or topic.

## Usage

### Requirements

- [SQLite3](https://www.sqlite.org/index.html) 3.38.0 or higher

## Building

### Requirements

- A C++17-compatible toolchain (currently tested on [GCC](https://gcc.gnu.org/) and [Clang](https://clang.llvm.org/))
- [xxd](https://linux.die.net/man/1/xxd)
- [CMake](https://cmake.org/) 3.13 or higher
- [SQLite3](https://www.sqlite.org/index.html) 3.38.0 or higher
- [antlr4](https://github.com/antlr/antlr4) (included as submodule)

#### Additionally required for building the command line:

- [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html)

#### Additionally required for testing:

- [Catch2](https://github.com/catchorg/Catch2) (included as submodule)

#### Additionally required for building the documentation:

- [Doxygen](https://doxygen.nl/) 1.9.1 or higher
- [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) (included as submodule)

### Targets

This repository provides the following build targets:

| Target            | Description                      |
|-------------------|----------------------------------|
| `plang::libplang` | Builds the core library          |
| `plang::plangcli` | Builds the terminal application  |
| `plang__doc`      | Builds the documentation         |

## Design

### Syntax

The markup language is agnostic of natural languages. It does not contain keywords, only symbolic operators.
While the herein included standard corpus is in English, it may be translated into a different language.

The statement syntax is derived from natural languages and follows a subject→verb→object order.
(Additional word orders will be added in later versions.)
Namespaces that catalogue the symbols in the corpus may be omitted as long as the statement remains unambiguous.

Take this natural sentence:
```
Andy eats salad.
```

Its shortest possible PLang equivalent would be:
```
[andy] eat: [salad];
```
Here,`[andy]` and `[salad]` are _symbols_. In PLang, a _symbol_ is anything you can make a statement about.
In the example, _symbols_ serve as the subject and the object of what PLang calls a _plot point_, that is:
any statement with at least a subject and a verb.

Its fully-qualified form looks like this:
```
.character[andy] .character.behaviour.action.eat
          ?what: .world.culture.artifact.food[salad];
```
The namespaces before the subject, verb and object sort them into the hierarchy of the _corpus_.
These additional namespaces mark the _symbols_ and _points_ as being instances of a _class_:<br/>

- The subject "Andy" is a symbol of the class `.character`
- The verb "eat" makes the statement a point of the class `.character.behaviour.action.eat`
- The object "salad" is a symbol of the class `.world.culture.artifact.food`

These classes, beside many more, are defined in the _corpus_.
The _corpus_ refers to the entirety of information that a work holds.
This repository includes such a [template corpus](corpus/std.en.plang) to start from.

Explicit reasons and consequences may also be added inline:
```
.character[andy] .character.behaviour.action.eat
          ?what: .world.culture.artifact.food[salad]
  -< {.character[andy] .world.biology.fauna.sensation.internal.hunger}
  -> {.character[andy] ~.world.biology.fauna.sensation.internal.hunger}
;
```

meaning

```
Andy ate salad,
because he was hungry.
(And now he's sated.)
```

Because specifying these reasons and implications like so would be tedious,
the template [_corpus_](corpus/std.en.plang) already includes some obvious ones, so in this case, both causals would be implied.

### Structure

This language differentiates between four contexts, each for a specific purpose.

- **Plot**: Defines the content of the work and its logical coherence
- **Story**: Orders the content chronologically
- **Outline**: Maps the story threads onto a presentative outline
- **Text**: Maps the outline onto the actual work and contextualizes stylistic devices

### Files

The interpreter mainly works with two types of files:

- **PLang Source File**<br/>
  MIME type: `application/x-plang`<br/>
  File extension: `.plang`<br/>
  Derived from: `plain/text`<br/>
- **PLang Database File**<br/>
  MIME Type: `application/x-plangdb`<br/>
  File extension: `.plangdb`<br/>
  Derived from: `application/vnd.sqlite3`<br/>

Executables may also write:

- **Log file**:<br/>
  MIME type: `application/x-log`<br/>
  File extension: `.log`<br/>
  Derived from: `plain/text`<br/>

Referencing content in external file will be supported for the following types:

- **PDF document**<br/>
  MIME type: `application/pdf`<br/>
  File extension: `.pdf`<br/>
  Referencing: document sections, pages
- **Webpage**<br/>
  MIME type: `text/html`<br/>
  File extension: `.htm`, `.html`<br/>
  Referencing: document sections
- **ePUB archive**<br/>
  MIME type: `application/epub+zip`<br/>
  File extension: `.epub`<br/>
  Referencing: document sections
- **LaTeX source**<br/>
  MIME type: `application/x-latex`<br/>
  File extension: `.tex`<br/>
  Referencing: document sections, lines, columns
- **Markdown source**<br/>
  MIME type: `text/markdown`<br/>
  File extension: `.md`<br/>
  Referencing: document sections, lines, columns
- **Plain text file**<br/>
  MIME type: `text/plain`<br/>
  File extension: `.txt`<br/>
  Referencing to: lines, columns

## License

The source code contained in this repository is published
under the [GNU Affero General Public License version 3](https://www.gnu.org/licenses/agpl-3.0.html), <br/>
unless specified otherwise in either the file or the containing directory.

See the [LICENSE](LICENSE) file for the full text.
