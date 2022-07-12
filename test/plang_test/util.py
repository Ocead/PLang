from typing import Optional

from sqlalchemy.orm import Session

from plang.cli.cli import CLI


__session: Optional[Session] = None


def getSession() -> Session:
    global __session
    if __session is None:
        __session = CLI.session(':memory:')
    return __session
