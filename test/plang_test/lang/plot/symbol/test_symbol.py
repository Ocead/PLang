import random
from typing import Any, Union, Optional, List
from unittest import TestCase

from sqlalchemy.orm import Session

from plang.cli.handler import PlangHandler
from plang.cli.scope import PlangScope, PathScope
from plang.db.models import Path
from plang.db.plot.symbol.models import Symbol

from plang_test.util import getSession


class TestSymbolBase(TestCase):
    session: Session

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        TestSymbolBase.session = getSession()

    @classmethod
    def tearDownClass(cls) -> None:
        super().tearDownClass()
        TestSymbolBase.session.close()

    def execute(self, line: str) -> Any:
        result = self.handler.execute(self.session, self.scope, line + ';')
        self.session.commit()

        return result

    def isNotInDatabase(self, line: str):
        pass

    def isPersisted(self, obj: Any):
        self.assertIsNotNone(obj.id)

    def isDecorated(self, symbol: Symbol, ordinal: Union[int, float, None], description: Optional[str]):
        self.assertEqual(symbol.ordinal, ordinal)
        self.assertEqual(symbol.description, description)

    def isStringEquals(self, symbol: Symbol, line: str):
        self.assertEqual(str(symbol), line)

    def setUp(self) -> None:
        self.handler = PlangHandler()
        self.session = TestSymbolBase.session
        self.session.rollback()
        path = self.handler.ref(self.session, PlangScope(), '.;')
        if isinstance(path, Path):
            self.scope = PathScope('test', path)
        else:
            raise TypeError()


class TestSymbol(TestSymbolBase):
    def testLocalSymbol(self):
        line = '[local_symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testRootSymbol(self):
        line = '.[root_symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testUnqualifiedSymbol(self):
        line = 'unqualified[symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testUnqualifiedChildSymbol(self):
        line = 'unqualified.child[symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testQualifiedSymbol(self):
        line = '.qualified[symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testQualifiedChildSymbol(self):
        line = '.qualified.child[symbol]'

        result: Symbol = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testLocalSymbolList(self):
        line = '[local_symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.[local_symbol]', '.[list]'}, {str(x) for x in result})

    def testRootSymbolList(self):
        line = '.[root_symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.[root_symbol]', '.[list]'}, {str(x) for x in result})

    def testUnqualifiedSymbolList(self):
        line = 'unqualified[symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.unqualified[symbol]', '.unqualified[list]'}, {str(x) for x in result})

    def testUnqualifiedChildSymbolList(self):
        line = 'unqualified.child[symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.unqualified.child[symbol]', '.unqualified.child[list]'}, {str(x) for x in result})

    def testQualifiedSymbolList(self):
        line = '.qualified[symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.qualified[symbol]', '.qualified[list]'}, {str(x) for x in result})

    def testQualifiedChildSymbolList(self):
        line = '.qualified.child[symbol, list]'

        result: List[Symbol] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.qualified.child[symbol]', '.qualified.child[list]'}, {str(x) for x in result})


class TestSymbolDecorated(TestSymbolBase):
    def testDecoratedLocalSymbol(self):
        line = '[local_symbol{}]'
        ordinal = random.randint(-50, 50)
        description = 'Local symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line.format(''))

    def testDecoratedRootSymbol(self):
        line = '.[root_symbol{}]'

        ordinal = random.randint(-50, 50)
        description = 'Root symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line.format(''))

    def testDecoratedUnqualifiedSymbol(self):
        line = 'unqualified[symbol{}]'

        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line.format(''))

    def testDecoratedUnqualifiedChildSymbol(self):
        line = 'unqualified.child[symbol{}]'

        ordinal = random.randint(-50, 50)
        description = 'Unqualified child symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line.format(''))

    def testDecoratedQualifiedSymbol(self):
        line = '.qualified[symbol{}]'

        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line.format(''))

    def testDecoratedQualifiedChildSymbol(self):
        line = '.qualified.child[symbol{}]'

        ordinal = random.randint(-50, 50)
        description = 'Qualified child symbol'

        result: Symbol = self.execute(line.format(f'({ordinal}, "{description}")'))

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line.format(''))

    def testDecoratedLocalSymbolList(self):
        pass

    def testDecoratedRootSymbolList(self):
        pass

    def testDecoratedUnqualifiedSymbolList(self):
        pass

    def testDecoratedUnqualifiedChildSymbolList(self):
        pass

    def testDecoratedQualifiedSymbolList(self):
        pass

    def testDecoratedQualifiedChildSymbolList(self):
        pass
