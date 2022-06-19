from typing import Union

import pkg_resources
from pkg_resources import DistributionNotFound, VersionConflict

dependencies = [
    'antlr4-python3-runtime>=4.10',
    'SQLAlchemy>=1.4.37',
    'rich>=12.4.4',
    'pyreadline3>=3.4.1'
]


def checkDependencies():
    try:
        pkg_resources.require(dependencies)
    except Union[DistributionNotFound, VersionConflict] as e:
        pass # TODO: print appropriate error
