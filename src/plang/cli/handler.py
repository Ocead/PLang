import shlex
from abc import ABC, abstractmethod
from typing import List

from antlr4.error.ErrorListener import ErrorListener

from plang.cli.scope import Scope
from plang.lang.generated import *


class Handler(ABC):
    @abstractmethod
    def complete(self, scope: Scope, line: str) -> List:
        pass

    @abstractmethod
    def execute(self, scope: Scope, line: str) -> object:
        pass

    @abstractmethod
    def pattern(self) -> str:
        pass


class CommandHandler(Handler):
    def __init__(self) -> None:
        super().__init__()

    def complete(self, scope: Scope, line: str) -> List:
        pass

    def execute(self, scope: Scope, line: str) -> object:
        args = shlex.split(line)

        return None

    def pattern(self) -> str:
        return r'^[\s\t]*:[\w]+'


class TestErrorListener(ErrorListener):
    def __init__(self):
        super()

    def syntaxError(self, recognizer, offendingSymbol, line, column, msg, e):
        super().syntaxError(recognizer, offendingSymbol, line, column, msg, e)

    def reportAmbiguity(self, recognizer, dfa, startIndex, stopIndex, exact, ambigAlts, configs):
        super().reportAmbiguity(recognizer, dfa, startIndex, stopIndex, exact, ambigAlts, configs)

    def reportAttemptingFullContext(self, recognizer, dfa, startIndex, stopIndex, conflictingAlts, configs):
        super().reportAttemptingFullContext(recognizer, dfa, startIndex, stopIndex, conflictingAlts, configs)

    def reportContextSensitivity(self, recognizer, dfa, startIndex, stopIndex, prediction, configs):
        super().reportContextSensitivity(recognizer, dfa, startIndex, stopIndex, prediction, configs)


class MyErrorStrategy(BailErrorStrategy):
    def recover(self, recognizer: Parser, e: RecognitionException):
        recognizer._errHandler.reportError(recognizer, e)
        super().recover(recognizer, e)


class PlangHandler(Handler):
    def __init__(self) -> None:
        super().__init__()

    def complete(self, scope: Scope, line: str) -> List:
        pass

    def execute(self, scope: Scope, line: str) -> object:
        input_stream = InputStream(data=line)
        lexer = PlangLexer(input_stream)
        stream = CommonTokenStream(lexer)
        parser = PlangParser(stream)
        parser._errHandler = MyErrorStrategy()
        parser.addErrorListener(TestErrorListener())
        parser.addParseListener(PlangListener())

        try:
            tree = parser.start()
            visitor = PlangVisitor()
            result = visitor.visit(tree)
            return result
        finally:
            return None

    def pattern(self) -> str:
        return r'^(?![\s\t]*:[\w]+).+'
