import random
from typing import Any, Union, Optional, List
from unittest import TestCase

from sqlalchemy.orm import Session

from plang.cli.handler import PlangHandler
from plang.cli.scope import PlangScope, PathScope
from plang.db.models import Path
from plang.db.plot.symbol.models import SymbolClass

from plang_test.util import getSession


class TestSymbolClassBase(TestCase):
    session: Session

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        TestSymbolClassBase.session = getSession()

    @classmethod
    def tearDownClass(cls) -> None:
        super().tearDownClass()
        TestSymbolClassBase.session.close()

    def execute(self, line: str) -> Any:
        result = self.handler.execute(self.session, self.scope, line + ';')
        self.session.commit()

        return result

    def isNotInDatabase(self, line: str):
        pass

    def isPersisted(self, obj: Any):
        self.assertIsNotNone(obj.id)

    def isDecorated(self, symbol_class: SymbolClass, ordinal: Union[int, float, None], description: Optional[str]):
        self.assertEqual(symbol_class.path.ordinal, ordinal)
        self.assertEqual(symbol_class.path.description, description)

    def isHintingToItself(self, symbol_class: SymbolClass):
        self.assertEqual(len(symbol_class.hints), 1)
        self.assertEqual(symbol_class.hints[0].hint, symbol_class)

    def isHintingTo(self, symbol_class: SymbolClass, *hints: SymbolClass):
        self.assertEqual(len(symbol_class.hints), len(hints))
        self.assertEqual({x.hint for x in symbol_class.hints}, set(hints))

    def isStringEquals(self, symbol_class: SymbolClass, line: str):
        self.assertEqual(str(symbol_class), line)

    def setUp(self) -> None:
        self.handler = PlangHandler()
        self.session = TestSymbolClass.session
        self.session.rollback()
        path = self.handler.ref(self.session, PlangScope(), '.;')
        if isinstance(path, Path):
            self.scope = PathScope('test', path)
        else:
            raise TypeError()


class TestSymbolClass(TestSymbolClassBase):
    def testLocalSymbolClass(self):
        line = '[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testRootSymbolClass(self):
        line = '.[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testUnqualifiedSymbolClass(self):
        line = 'unqualified_symbol_class[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testUnqualifiedChildSymbolClass(self):
        line = 'unqualified_symbol.class[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, '.' + line)

    def testQualifiedSymbolClass(self):
        line = '.qualified_symbol_class[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testQualifiedChildSymbolClass(self):
        line = '.qualified_symbol.class[]'

        result: SymbolClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isStringEquals(result, line)

    def testUnqualifiedSymbolClassList(self):
        line = 'unqualified_symbol_class,list[]'

        result: List[SymbolClass] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.unqualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testUnqualifiedChildSymbolClassList(self):
        line = 'unqualified.symbol.class,list[]'

        result: List[SymbolClass] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.unqualified.symbol.class[]', '.unqualified.symbol.list[]'}, {str(x) for x in result})

    def testQualifiedSymbolClassList(self):
        line = '.qualified_symbol_class,list[]'

        result: List[SymbolClass] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.qualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testQualifiedChildSymbolClassList(self):
        line = '.qualified.symbol.class,list[]'

        result: List[SymbolClass] = self.execute(line)

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
        self.assertEqual({'.qualified.symbol.class[]', '.qualified.symbol.list[]'}, {str(x) for x in result})


class TestSymbolClassHinted(TestSymbolClassBase):
    def testHintedLocalSymbolClass(self):
        line = '[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedRootSymbolClass(self):
        line = '.[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedUnqualifiedSymbolClass(self):
        line = 'unqualified_symbol_class[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedUnqualifiedChildSymbolClass(self):
        line = 'unqualified_symbol.class[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedQualifiedSymbolClass(self):
        line = '.qualified_symbol_class[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedQualifiedChildSymbolClass(self):
        line = '.qualified_symbol.class[]'

        result: SymbolClass = self.execute(f'({line}) {line}')

        self.isPersisted(result)
        self.isDecorated(result, None, None)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedUnqualifiedSymbolClassList(self):
        line = 'unqualified_symbol_class,list[]'

        result: List[SymbolClass] = self.execute(f'({line}) {line}')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
            self.isHintingTo(r, *result)
        self.assertEqual({'.unqualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testHintedUnqualifiedChildSymbolClassList(self):
        line = 'unqualified.symbol.class,list[]'

        result: List[SymbolClass] = self.execute(f'({line}) {line}')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
            self.isHintingTo(r, *result)
        self.assertEqual({'.unqualified.symbol.class[]', '.unqualified.symbol.list[]'}, {str(x) for x in result})

    def testHintedQualifiedSymbolClassList(self):
        line = '.qualified_symbol_class,list[]'

        result: List[SymbolClass] = self.execute(f'({line}) {line}')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
            self.isHintingTo(r, *result)
        self.assertEqual({'.qualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testHintedQualifiedChildSymbolClassList(self):
        line = '.qualified.symbol.class,list[]'

        result: List[SymbolClass] = self.execute(f'({line}) {line}')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, None, None)
            self.isHintingTo(r, *result)
        self.assertEqual({'.qualified.symbol.class[]', '.qualified.symbol.list[]'}, {str(x) for x in result})


class TestSymbolClassDecorated(TestSymbolClassBase):
    def testDecoratedLocalSymbolClass(self):
        line = '[]'
        ordinal = random.randint(-50, 50)
        description = 'Local Symbol class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line)

    def testDecoratedRootSymbolClass(self):
        line = '.[]'
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line)

    def testDecoratedUnqualifiedSymbolClass(self):
        line = 'unqualified_symbol_class[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line)

    def testDecoratedUnqualifiedChildSymbolClass(self):
        line = 'unqualified_symbol.class[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol child class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, '.' + line)

    def testDecoratedQualifiedSymbolClass(self):
        line = '.qualified_symbol_class[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line)

    def testDecoratedQualifiedChildSymbolClass(self):
        line = '.qualified_symbol.class[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol child class'

        result: SymbolClass = self.execute(line + f'({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isStringEquals(result, line)

    def testDecoratedUnqualifiedSymbolClassList(self):
        line = 'unqualified_symbol_class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol class list'

        result: List[SymbolClass] = self.execute(line + f'({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
        self.assertEqual({'.unqualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testDecoratedUnqualifiedChildSymbolClassList(self):
        line = 'unqualified.symbol.class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol child class list'

        result: List[SymbolClass] = self.execute(line + f'({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
        self.assertEqual({'.unqualified.symbol.class[]', '.unqualified.symbol.list[]'}, {str(x) for x in result})

    def testDecoratedQualifiedSymbolClassList(self):
        line = '.qualified_symbol_class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol class list'

        result: List[SymbolClass] = self.execute(line + f'({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
        self.assertEqual({'.qualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testDecoratedQualifiedChildSymbolClassList(self):
        line = '.qualified.symbol.class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol child class list'

        result: List[SymbolClass] = self.execute(line + f'({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
        self.assertEqual({'.qualified.symbol.class[]', '.qualified.symbol.list[]'}, {str(x) for x in result})


class TestSymbolClassHintedDecorated(TestSymbolClassBase):
    def testHintedDecoratedLocalSymbolClass(self):
        line = '[]'
        ordinal = random.randint(-50, 50)
        description = 'Local Symbol class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedDecoratedRootSymbolClass(self):
        line = '.[]'
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedDecoratedUnqualifiedSymbolClass(self):
        line = 'unqualified_symbol_class[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedDecoratedUnqualifiedChildSymbolClass(self):
        line = 'unqualified_symbol.class[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol child class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, '.' + line)

    def testHintedDecoratedQualifiedSymbolClass(self):
        line = '.qualified_symbol_class[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedDecoratedQualifiedChildSymbolClass(self):
        line = '.qualified_symbol.class[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol child class'

        result: SymbolClass = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.isPersisted(result)
        self.isDecorated(result, ordinal, description)
        self.isHintingToItself(result)
        self.isStringEquals(result, line)

    def testHintedDecoratedUnqualifiedSymbolClassList(self):
        line = 'unqualified_symbol_class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol class list'

        result: List[SymbolClass] = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
            self.isHintingTo(r, *result)
        self.assertEqual({'.unqualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testHintedDecoratedUnqualifiedChildSymbolClassList(self):
        line = 'unqualified.symbol.class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Unqualified symbol child class list'

        result: List[SymbolClass] = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
            self.isHintingTo(r, *result)
        self.assertEqual({'.unqualified.symbol.class[]', '.unqualified.symbol.list[]'}, {str(x) for x in result})

    def testHintedDecoratedQualifiedSymbolClassList(self):
        line = '.qualified_symbol_class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol class list'

        result: List[SymbolClass] = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
            self.isHintingTo(r, *result)
        self.assertEqual({'.qualified_symbol_class[]', '.list[]'}, {str(x) for x in result})

    def testHintedDecoratedQualifiedChildSymbolClassList(self):
        line = '.qualified.symbol.class,list[]'
        ordinal = random.randint(-50, 50)
        description = 'Qualified symbol child class list'

        result: List[SymbolClass] = self.execute(f'({line}) {line} ({ordinal}, "{description}")')

        self.assertEqual(len(result), 2)
        for r in result:
            self.isPersisted(r)
            self.isDecorated(r, ordinal, description)
            self.isHintingTo(r, *result)
        self.assertEqual({'.qualified.symbol.class[]', '.qualified.symbol.list[]'}, {str(x) for x in result})

