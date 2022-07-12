from abc import ABC, abstractmethod
from typing import Any, List

from antlr4 import BailErrorStrategy, Parser, RecognitionException, InputStream, CommonTokenStream, ParserRuleContext
from antlr4.error.ErrorListener import ErrorListener
from sqlalchemy.orm import Session

from plang.cli.scope import Scope
from plang.lang.generated.PlangLexer import PlangLexer
from plang.lang.generated.PlangListener import PlangListener
from plang.lang.generated.PlangParser import PlangParser
from plang.lang.parser.visitor import PlangVisitor, MarkupMode


class Handler(ABC):
    @abstractmethod
    def complete(self, scope: Scope, line: str) -> List:
        pass

    @abstractmethod
    def execute(self, session: Session, scope: Scope, line: str) -> Any:
        pass

    @abstractmethod
    def decl(self, session: Session, scope: Scope, line: str) -> Any:
        pass

    @abstractmethod
    def ref(self, session: Session, scope: Scope, line: str) -> Any:
        pass

    @abstractmethod
    def pattern(self) -> str:
        pass


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

    def __prepare(self, line: str) -> PlangParser:
        input_stream = InputStream(data=line)
        lexer = PlangLexer(input_stream)
        stream = CommonTokenStream(lexer)
        parser = PlangParser(stream)
        parser._errHandler = MyErrorStrategy()
        parser.addErrorListener(TestErrorListener())
        parser.addParseListener(PlangListener())

        return parser

    def __visit(self, session: Session, scope: Scope, tree: ParserRuleContext) -> Any:
        try:
            visitor = PlangVisitor(session, scope, MarkupMode.IMPLICIT)
            result = visitor.visit(tree)
            if len(result) == 1:
                return result[0]
            else:
                return result
        except:
            return None


    def complete(self, scope: Scope, line: str) -> List:
        pass

    def execute(self, session: Session, scope: Scope, line: str) -> object:
        parser = self.__prepare(line)

        tree = parser.start()
        return self.__visit(session, scope, tree)

    def decl(self, session: Session, scope: Scope, line: str) -> object:
        parser = self.__prepare(line)

        tree = parser.declSVO()
        return self.__visit(session, scope, tree)

    def ref(self, session: Session, scope: Scope, line: str) -> object:
        parser = self.__prepare(line)

        tree = parser.ref()
        return self.__visit(session, scope, tree)

    def pattern(self) -> str:
        return r'^(?![\s\t]*:[\w]+).+'
