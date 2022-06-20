import logging
from typing import Dict, List, Set, Union

from antlr4 import Parser, Token, ParserRuleContext
from antlr4.IntervalSet import IntervalSet
from antlr4.atn import ATN
from antlr4.atn.ATNState import ATNState


class CandidatesCollection:
    def __init__(self):
        self.tokens: Dict[int, List[int]] = {}
        self.rules: Dict[int, List[int]] = {}
        self.rulePositions: Dict[int, List[int]] = {}


class FollowSetWithPath:
    def __init__(self):
        self.intervals = IntervalSet()
        self.path: List[int] = []
        self.following: List[int] = []


class FollowSetsHolder:
    def __init__(self):
        self.sets: "List[FollowSetWithPath]" = []
        self.combined = IntervalSet()


class PipelineEntry:
    def __init__(self, state: ATNState, tokenIndex: int):
        self.state = state
        self.tokenIndex = tokenIndex


class CodeCompletionCore:
    logger = logging.getLogger(__name__)

    class CandidatesCollection:
        def __init__(self):
            self.tokens: Dict[int, List[int]] = {}
            self.rules: Dict[int, List[int]] = {}
            self.rulePositions: Dict[int, List[int]] = {}

        def __str__(self):
            return f"CandidatesCollection{{tokens={self.tokens}, rules={self.rules}, ruleStrings={self.rulePositions}}}"

    def __init__(self, parser: Parser, preferred_rules: Union[Set[int], None], ignored_tokens: Union[Set[int], None]):
        self.__showResult = True
        self.__showDebugOutput = True
        self.__debugOutputWithTransistions = True
        self.__showRuleState = True

        self.__ignoredTokens: Set[int] = ignored_tokens or set()
        self.__preferredRules: Set[int] = preferred_rules or set()

        self.__parser: Parser = parser
        self.__atn: ATN = parser.getATNWithBypassAlts()
        self.__vocabulary = parser.getVocabulary()
        self.__ruleNames: List[str] = []
        self.__tokens: List[Token] = []

        self.__tokenStartIndex = 0
        self.__statesProcessed = 0

        self.__shortcutMap: Dict[int, Dict[int, Set[int]]] = {}
        self.__candidates = CandidatesCollection()

    __followSetsByATN: "Dict[str, Dict[int, FollowSetsHolder]]" = {}

    def collectCandidates(self, caretTokenIndex: int, context: ParserRuleContext) -> CandidatesCollection:
        self.__shortcutMap = {}
        self.__candidates.rules = {}
        self.__candidates.tokens = {}
        self.__statesProcessed = 0

        self.__tokenStartIndex = context.start.tokenIndex if context is not None else 0
        tokenStream = self.__parser.getInputStream()

        currentIndex = tokenStream.index
        tokenStream.seek(self.__tokenStartIndex)
        self.__tokens = []
        offset = 1

        while True:
            token = tokenStream.lt(offset)
            offset += 1
            self.__tokens.append(token)

            if token.tokenIndex >= caretTokenIndex or token.type == Token.EOF:
                break

        tokenStream.seek(currentIndex)

        callStack: List[int] = []
        startRule = context.getRuleIndex() if context is not None else 0
        self.processRule(self.__atn.ruleToStartState[startRule], 0, callStack, '\n')

    def getCandidates(self, caretTokenIndex: int = 0, context: ParserRuleContext = None) -> CandidatesCollection:
        return self.collectCandidates(caretTokenIndex, context)
