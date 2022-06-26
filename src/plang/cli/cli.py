import re
import sys
import shlex
from typing import List, Optional

from sqlalchemy import create_engine
from sqlalchemy.orm import Session, sessionmaker

from plang.cli.handler import Handler, PlangHandler
from plang.cli.scope import Scope
from plang.db import Base, Path
from plang.error import NoSessionException
from plang.lang.parser import Manager

try:
    import readline
except ImportError:
    from pyreadline3 import Readline

    readline = Readline()


class CLI:
    @classmethod
    def session(cls, path: str) -> Optional[Session]:
        try:
            engine = create_engine(f'sqlite:///{path}')
            Base.metadata.create_all(engine)

            Session = sessionmaker()
            Session.configure(bind=engine)
            session = Session()

            if len(session.query(Path).all()) == 0:
                new_path = Path()
                new_path.id = 0
                new_path.name = ''
                new_path.parent_id = 0
                session.add(new_path)
                session.commit()

            result = session.query(Path).all()

            return session
        except Exception as e:
            return None

    @classmethod
    def isCommand(cls, line: str) -> bool:
        return re.compile(r'^[\s\t]*:[\w]*', re.UNICODE | re.MULTILINE).match(line) is not None

    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope
        self.handler: Handler = PlangHandler()
        readline.parse_and_bind('tab: complete')
        readline.set_completer(self.__complete)
        if readline in sys.modules:
            readline.set_auto_history(True)

        self.incomplete: Optional[str] = None
        self.completions: List[str] = []

    def __help(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __pragma(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __quit(self, *args) -> Optional[int]:
        if len(args) != 0:
            raise NotImplementedError()  # TODO: implement error

        if self.session is None or len(self.session.dirty) == 0:
            return 0
        else:
            raise NotImplementedError()  # TODO: implement error

    def __open(self, *args) -> None:
        if len(args) == 1:
            self.session = CLI.session(args[0])
        else:
            raise NotImplementedError()  # TODO: implement error

    def __save(self, *args) -> None:
        if len(args) != 0 or self.session is None:
            raise NotImplementedError()  # TODO: implement error
        self.session.flush()

    def __close(self, *args) -> None:
        if len(args) != 0:
            raise NotImplementedError()  # TODO: implement error

        if len(self.session.dirty) != 0:
            self.session.commit()
        else:
            raise NotImplementedError()  # TODO: implement error

    def __run(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __export(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __scope(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __list(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __find(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __delete(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __copy(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __move(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    __commands = {
        ':h': __help,
        ':help': __help,

        ':p': __pragma,
        ':option': __pragma,

        ':q': __quit,
        ':quit': __quit,
        ':exit': __quit,

        ':o': __open,
        ':open': __open,

        ':s': __save,
        ':save': __save,

        ':w': __close,
        ':close': __close,

        ':r': __run,
        ':run': __run,

        ':e': __export,
        ':export': __export,

        ':cd': __scope,

        ':l': __list,
        ':ls': __list,

        ':f': __find,
        ':find': __find,
        ':search': __find,

        ':x': __delete,
        ':rm': __delete,

        ':c': __copy,
        ':cp': __copy,

        ':v': __move,
        ':mv': __move,
    }

    def __complete(self, text, state) -> Optional[str]:
        cursor = readline.mode.l_buffer.point
        if state == 0:
            self.incomplete = text
            completion = []
            if CLI.isCommand(text):
                raise NotImplementedError()  # TODO: implement command completion
            else:
                completion += self.handler.complete(self.scope, text)
            self.completions = [str(x) for x in completion]

        if len(self.completions) > state:
            return self.completions[state] or None
        else:
            return None

    def input(self) -> Optional[int]:
        result = None
        try:
            line = input(f'{self.scope}> ')
            if CLI.isCommand(line):
                args = shlex.split(line)
                try:
                    command = CLI.__commands[args[0]]
                except KeyError:
                    print(f'ERROR: Unknown command "{args[0]}"!', file=sys.stderr)
                    return None
                return command(self, *args[1:])
            else:
                if self.session is not None:
                    result = self.handler.execute(self.session, self.scope, line)
                    report = Manager(self.session, self.scope).report()
                    if len(report) > 0:
                        print(report)
                    self.session.commit()
                else:
                    raise NoSessionException()
        except KeyboardInterrupt as e:
            return 2
