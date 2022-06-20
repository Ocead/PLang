import re
import sys
from typing import List, Set, Union

from sqlalchemy.orm import Session

from plang.cli.handler import Handler
from plang.cli.scope import Scope

try:
    import readline
except ImportError:
    from pyreadline3 import Readline

    readline = Readline()


class CLI:
    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope
        self.handlers: Set[Handler] = set()
        readline.parse_and_bind('tab: complete')
        readline.set_completer(self.__complete)
        if readline in sys.modules:
            readline.set_auto_history(True)

        self.incomplete: Union[str, None] = None
        self.completions: List[str] = []

    def __complete(self, text, state) -> Union[str, None]:
        # cursor = self.__readLine.mode.l_buffer.point  # TODO: Implement
        if state == 0:
            self.incomplete = text
            completion = []
            for h in self.handlers:
                if re.compile(h.pattern(), re.UNICODE | re.MULTILINE).match(text) is not None:
                    completion += h.complete(self.scope, text)
            self.completions = [str(x) for x in completion]

        if len(self.completions) > state:
            return self.completions[state] or None
        else:
            return None

    def addHandler(self, handler: Handler):
        self.handlers.add(handler)

    def input(self) -> int:
        line = input(f'{self.scope}> ')
        for h in self.handlers:
            if re.compile(h.pattern(), re.UNICODE | re.MULTILINE).match(line) is not None:
                result = h.execute(self.scope, line)
            if result is Scope:
                self.scope = result

        return 0
