# ![](docs/ocead.svg) <u>PLang</u> - Interactive markup language for narrative works

A formal language to plan and analyse prose, screenplays, games, etc.

---

This project aims to help:

* maintaining logical consistence and keeping track of consequences
* organising story threads
* tracking introduction and usage of plot points
* cataloguing the content and monitoring its visibility

in narrative works, regardless of form or topic.

## Getting started

### Requirements

* **Python [3.9](https://www.python.org/downloads/release/python-390/)** or higher
* SQLAlchemy ([SQLAlchemy~=1.4.37](https://pypi.org/project/SQLAlchemy/))
* ANTLR 4 runtime for Python 3 ([antlr4-python3-runtime](https://pypi.org/project/antlr4-python3-runtime/))

### Usage

You can invoke the command line tool via

```shell
python3 -m plang
```

## Design

### Syntax

The markup language is agnostic of natural languages. It does not contain keywords, only symbolic operators.
While the here included standard corpus is in English, it may be translated into a different language.

The statement syntax is derived from natural languages and follows a subject-verb-object order.
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
.character[andy] .character.behaviour.action.eat ?what:.world.culture.artifact.food[salad];
```
The namespaces before the subject, verb and object sort them into the hierarchy of the _corpus_.
These additional namespaces mark the _symbols_ and _points_ as being instances of a _class_:<br/>

* The subject "Andy" is a symbol of the class `.character`
* The verb "eat" makes the statement a point of the class `.character.behaviour.action`
* The object "salad" is a symbol of the class `.world.culture.artifact.food`

These classes, beside many more, are defined in the _corpus_.
The _corpus_ refers to the entirety of information that a work holds.
This repository includes such a template corpus to start from.

Explicit reasons and consequences may also be added inline:
```
.character[andy] .character.behaviour.action.eat ?what: .world.culture.artifact.food[salad]
  -< {.character[andy] .character.sensation.internal.hunger}
  -> {.character[andy] ~.character.sensation.internal.hunger}
;
```

meaning

```
Andy ate salad,
because he was hungry.
(And now he's sated.)
```

Because specifying these reasons and implications like so would be tedious,
the template _corpus_ already includes obvious ones, so in this case, both causals would be implied.

### Structure

This language differentiates between four contexts, each for a specific purpose.

* **Plot**: Defines the content of the work and its logical coherence
* **Story**: Orders the content chronologically
* **Outline**: Maps the story threads onto a presentative outline
* **Text**: Maps the outline onto the actual work and contextualizes stylistic devices

### Files

The interpreter mainly works with two types of files:

* **PLang Source File**<br/>
  MIME type: `application/x-plang`<br/>
  File extension: `.plang`<br/>
  Derived from: `plain/text`<br/>
* **PLang Database File**<br/>
  MIME Type: `application/x-plangdb`<br/>
  File extension: `.plangdb`<br/>
  Derived from: `application/vnd.sqlite3`<br/>

Executables may also write:

* **Log file**:<br/>
  MIME type: `application/x-log`<br/>
  File extension: `.log`<br/>
  Derived from: `plain/text`<br/>

Referencing content in external file will be supported for the following types:

* **PDF document**<br/>
  MIME type: `application/pdf`<br/>
  File extension: `.pdf`<br/>
  Referencing: document sections, pages
* **Webpage**<br/>
  MIME type: `text/html`<br/>
  File extension: `.htm`,`.html`<br/>
  Referencing: document sections
* **ePUB archive**<br/>
  MIME type: `application/epub+zip`<br/>
  File extension: `.epub`<br/>
  Referencing: document sections
* **LaTeX source**<br/>
  MIME type: `application/x-latex`<br/>
  File extension: `.tex`<br/>
  Referencing: document sections, lines, columns
* **Markdown source**<br/>
  MIME type: `text/markdown`<br/>
  File extension: `.md`<br/>
  Referencing: document sections, lines, columns
* **Plain text file**<br/>
  MIME type: `text/plain`<br/>
  File extension: `.txt`<br/>
  Referencing to: lines, columns

## Roadmap

`G`: Grammar, `D`: Data model, `I`: Interpreter, `S`: Serialization, `A`: Analysis

* [ ] Path `GD---`
* [ ] Plot `GD---`
    * [ ] Symbol classes `GD---`
    * [ ] Symbols `GD---`
    * [ ] Point classes `GD---`
    * [ ] Points `GD---`
    * [ ] Object classes `GD---`
    * [ ] Objects `GD---`
    * [ ] Requirements `GD---`
    * [ ] Implications `GD---`
    * [ ] Causality `GD---`
* [ ] Story `-----`
  * [ ] Element `-----`
  * [ ] Thread `-----`
  * [ ] Context `-----`
* [ ] Outline `-----`
  * [ ] Fragment `-----`
* [ ] Text `-----`
  * [ ] Location `-----`
  * [ ] Style `-----`

## License

The source code contained in this repository is published
under the [GNU Affero General Public License version 3](https://www.gnu.org/licenses/agpl-3.0.html), <br/>
unless specified otherwise in either the file or the containing directory.

See the [LICENSE](LICENSE) file for the full text.
