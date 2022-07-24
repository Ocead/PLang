import os
import re
import sys
import shlex
from sqlite3 import IntegrityError
from typing import List, Optional, Iterable

from antlr4 import FileStream
from rich.console import Console
from rich.markup import escape
from sqlalchemy import create_engine
from sqlalchemy.orm import Session, sessionmaker

from plang.cli.handler import Handler, PlangHandler
from plang.cli.scope import Scope, PathScope
from plang.db.base import Base
from plang.db.models import Path
from plang.db.plot.symbol.models import *
from plang.db.plot.point.models import *
from plang.db.plot.object.models import *
from plang.db.plot.causal.models import *
from plang.error import CLIExceptions, LangExceptions
from plang.lang.logic.manager import Manager

try:
    import readline
except ImportError:
    from pyreadline3 import Readline

    readline = Readline()


class CLI:
    @classmethod
    def session(cls, path: str) -> Optional[Session]:
        """
        Creates a new database session

        :param path: Path to the database file
        :return: The new session, if it could be created
        """
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

    def printReport(self):
        report = Manager(self.session, self.scope).report()
        if len(report) > 0:
            self.console.print(report.str(True))

    def checkSession(self):
        if self.session is None or not self.session.is_active:
            raise CLIExceptions.NoSessionException()

    def __execute(self, data):
        self.checkSession()

        try:
            result = self.handler.execute(self.session, self.scope, data)
        except Exception as e:
            self.session.rollback()
            raise LangExceptions.MalformedExpressionException from e
        if isinstance(result, Scope):
            self.scope = result
        self.printReport()
        self.commit()

        return result

    def commit(self):
        try:
            self.session.commit()
        except IntegrityError as e:
            pass
        finally:
            self.session.rollback()

    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope
        self.handler: Handler = PlangHandler()
        self.console = Console()
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

    def __info(self, *args) -> None:
        print(f'Current database file: {self.session.bind.url[-2]}')

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
            self.scope = PathScope(args[0], self.handler.ref(self.session, self.scope, '.;'))
        else:
            raise NotImplementedError()  # TODO: implement error

    def __save(self, *args) -> None:
        self.checkSession()
        if len(args) != 0:
            raise NotImplementedError()  # TODO: implement error
        self.session.flush()

    def __close(self, *args) -> None:
        if len(args) != 0:
            raise NotImplementedError()  # TODO: implement error

        if len(self.session.dirty) != 0:
            self.commit()
        else:
            raise NotImplementedError()  # TODO: implement error

    def __run(self, *args) -> None:
        self.checkSession()
        for f in args:
            if not os.path.isfile(f):
                raise CLIExceptions.ReadFileException()
            fs = FileStream(f)
            self.__execute(fs)

    def __export(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __inspect(self, *args) -> None:
        if len(args) == 0:
            self.console.print(self.scope.getPath().str(True))
        else:
            entry = self.handler.ref(self.session, self.scope, args[0])

            if entry is None:
                raise LangExceptions.InvalidReferenceException()
            elif not isinstance(entry, Iterable):
                entry = [entry]

            if len(entry) == 0:
                raise LangExceptions.InvalidReferenceException()

            for e in entry:
                if isinstance(e, Base):
                    self.console.print(e.str(True))
                else:
                    self.console.print(escape(repr(entry)))

    def __scope(self, *args) -> None:
        if len(args) == 0:
            args = ['.']
        elif len(args) > 1:
            raise NotImplementedError()  # TODO: implement
        raise NotImplementedError()  # TODO: implement

    def __list(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __find(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __delete(self, *args) -> None:
        for a in args:
            result = self.handler.ref(self.session, self.scope, a)
            if result is not None:
                if not isinstance(result, Iterable):
                    result = [result]
                for r in result:
                    self.session.delete(r)
            else:
                pass  # TODO: implement error
        self.printReport()
        self.commit()

    def __copy(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    def __move(self, *args) -> None:
        raise NotImplementedError()  # TODO: implement

    __commands = {
        ':h': __help,
        ':help': __help,

        ':p': __pragma,
        ':option': __pragma,

        ':i': __info,
        ':info': __info,

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

        ':': __inspect,
        ':inspect': __inspect,

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
            line = self.console.input(f'{self.scope.str(True)}[magenta]>[/magenta] ')
            if CLI.isCommand(line):
                args = shlex.split(line)
                try:
                    command = CLI.__commands[args[0]]
                except KeyError as e:
                    raise CLIExceptions.UnknownCommandException() from e
                return command(self, *args[1:])
            else:
                result = self.__execute(line)
                if isinstance(result, Scope):
                    self.scope = result
        except KeyboardInterrupt as e:
            return 2
