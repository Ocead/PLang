import random
from typing import Any, Union, Optional
from unittest import TestCase

from sqlalchemy.orm import Session

from plang.cli.handler import PlangHandler
from plang.cli.scope import PlangScope, PathScope
from plang.db.models import Path

from plang_test.util import getSession


class TestPathBase(TestCase):
    session: Session

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        TestPathBase.session = getSession()

    @classmethod
    def tearDownClass(cls) -> None:
        super().tearDownClass()
        TestPathBase.session.close()

    def execute(self, line: str) -> Any:
        result = self.handler.execute(self.session, self.scope, line + ';')
        self.session.commit()

        return result

    def isNotInDatabase(self, line: str):
        pass

    def isPersisted(self, obj: Any):
        self.assertIsNotNone(obj.id)

    def isDecorated(self, path: Path, ordinal: Union[int, float, None], description: Optional[str]):
        self.assertEqual(path.ordinal, ordinal)
        self.assertEqual(path.description, description)

    def isStringEquals(self, path: Path, line: str):
        self.assertEqual(str(path), line)

    def setUp(self) -> None:
        self.handler = PlangHandler()
        self.session = TestPathBase.session
        self.session.rollback()
        path = self.handler.ref(self.session, PlangScope(), '.;')
        if isinstance(path, Path):
            self.scope = PathScope('test', path)
        else:
            raise TypeError()


class TestPath(TestPathBase):
    def testQualifiedRootPath(self):
        line = '.'
        result: Path = self.execute(line)

        self.isPersisted(result)
        self.isStringEquals(result, line)

    def testUnqualifiedPath(self):
        line = 'unqualified'

        result: Path = self.execute(line)

        self.isPersisted(result)
        self.isStringEquals(result, '.' + line)

    def testUnqualifiedChildPath(self):
        line = 'unqualified.path'

        result: Path = self.execute(line)

        self.isPersisted(result)
        self.isStringEquals(result, '.' + line)

    def testQualifiedPath(self):
        line = '.qualified'

        result: Path = self.execute(line)

        self.isPersisted(result)
        self.isStringEquals(result, line)

    def testQualifiedChildPath(self):
        line = '.qualified.path'

        result: Path = self.execute(line)

        self.isPersisted(result)
        self.isStringEquals(result, line)


class TestPathDecorated(TestPathBase):
    def testDecoratedUnqualifiedPath(self):
        line = 'unqualified'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified path'

        result: Path = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line)

    def testDecoratedUnqualifiedChildPath(self):
        line = 'unqualified.path'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified child path'

        result: Path = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line)

    def testDecoratedQualifiedPath(self):
        line = '.qualified'
        ordinal = random.randint(-50, 50)
        description = 'Qualified path'

        result: Path = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line)

    def testDecoratedQualifiedChildPath(self):
        line = '.qualified.path'
        ordinal = random.randint(-50, 50)
        description = 'Qualified child path'

        result: Path = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line)
