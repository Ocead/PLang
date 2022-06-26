import sys
from argparse import *

from plang.cli.cli import CLI
from plang.cli.scope import PlangScope
from plang.error import PlangException


def parseArgs(args: list[str]) -> Namespace:
    """
    Parses the command line arguments

    :param args: List of command line arguments without the path of the executable
    :return: a namespace of all parsed arguments
    """
    parser = ArgumentParser()
    parser.add_argument('-f', '--file',
                        help='open the specified file',
                        required=False)
    parser.add_argument('-r', '--raw',
                        help='print python errors to console',
                        action='store_true',
                        required=False)
    parser.add_argument('-s', '--silent',
                        help='keep console output to a minimum',
                        action='store_true',
                        required=False)
    parser.add_argument('-v', '--version',
                        help='print the software version',
                        action='store_true',
                        required=False)

    namespace = parser.parse_args(args)
    return namespace


def main(args: list[str]) -> int:
    """
    Entry point for the PLang command line interface

    :return: Process exit code
    """
    ns = parseArgs(args)

    if ns.file is not None:
        session = CLI.session(ns.file)
    else:
        session = None

    cli = CLI(session, PlangScope())

    while True:
        try:
            result = cli.input()
            if isinstance(result, int):
                return result
        except PlangException as e:
            print(e, file=sys.stderr)
