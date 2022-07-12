from sqlalchemy.orm import Session

from plang.cli.cli import CLI


def getSession() -> Session:
    return CLI.session(':memory:')
