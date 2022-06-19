from argparse import *
import sys

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from plang import Base
from plang.cli.cli import CLI
from plang.cli.handler import CommandHandler, PlangHandler
from plang.cli.scope import PlangScope


def parseArgs(args: list[str]) -> Namespace:
    parser = ArgumentParser()
    parser.add_argument('-f', '--file', help='File to open', required=False)
    parser.add_argument('-v', '--version', help='Print the software version', required=False)

    namespace = parser.parse_args()
    return namespace


def main() -> int:
    ns = parseArgs(sys.argv)

    engine = create_engine(f'sqlite:///{ns.file}')
    Base.metadata.create_all(engine)

    Session = sessionmaker()
    Session.configure(bind=engine)
    session = Session()
    session.commit()

    cli = CLI(session, PlangScope())
    cli.addHandler(CommandHandler())
    cli.addHandler(PlangHandler())

    while True:
        ret = cli.input()
        if ret != 0:
            return ret
