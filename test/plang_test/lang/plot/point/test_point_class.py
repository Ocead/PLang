import random
from typing import Any, Union, Optional, List
from unittest import TestCase

from sqlalchemy.orm import Session

from plang.cli.handler import PlangHandler
from plang.cli.scope import PlangScope, PathScope
from plang.db.models import Path
from plang.db.plot.object.models import ObjectClass
from plang.db.plot.point.models import PointClass

from plang_test.util import getSession


class TestPointClassBase(TestCase):
    session: Session

    @classmethod
    def setUpClass(cls) -> None:
        super().setUpClass()
        TestPointClassBase.session = getSession()

    @classmethod
    def tearDownClass(cls) -> None:
        super().tearDownClass()
        TestPointClassBase.session.close()

    def execute(self, line: str) -> Any:
        result = self.handler.execute(self.session, self.scope, line + ';')
        self.session.commit()

        return result

    def isNotInDatabase(self, line: str):
        pass

    def isPersisted(self, obj: Any):
        self.assertIsNotNone(obj.id)

    def isDecorated(self, obj, ordinal: Union[int, float, None], description: Optional[str]):
        self.assertEqual(obj.ordinal, ordinal)
        self.assertEqual(obj.description, description)

    def isStringEquals(self, point_class: PointClass, line: str):
        self.assertEqual(str(point_class), line)

    def isNotHinted(self, object_class: ObjectClass):
        self.assertEqual(len(object_class.hints_lit), 0)
        self.assertEqual(len(object_class.hints_sym), 0)
        self.assertEqual(len(object_class.hints_pnt), 0)

    def setUp(self) -> None:
        self.handler = PlangHandler()
        self.session = TestPointClassBase.session
        self.session.rollback()
        path = self.handler.ref(self.session, PlangScope(), '.;')
        if isinstance(path, Path):
            self.scope = PathScope('test', path)
        else:
            raise TypeError()


class TestPointClass(TestPointClassBase):
    def testDefaultPointClass(self):
        line = '.default.point.class default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testPointClass(self):
        line = '.point.class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDualObjectPointClass(self):
        line = '.dual.point.class default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonDefaultPointClass(self):
        line = '! .singleton.default.point.class default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonPointClass(self):
        line = '! .singleton.point.class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonDualObjectPointClass(self):
        line = '! .singleton.dual.point.class default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDefaultSingletonObjectPointClass(self):
        line = '.default.point.class !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonObjectPointClass(self):
        line = '.point.class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDualSingletonObjectPointClass(self):
        line = '.dual.point.class !singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonDefaultSingletonObjectPointClass(self):
        line = '! .singleton.default.point.class !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonSingletonObjectPointClass(self):
        line = '! .singleton.point.class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testSingletonDualSingletonObjectPointClass(self):
        line = '! .dual.singleton.point.class !singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)


class TestHintedPointClass(TestPointClassBase):
    def testHintedDefaultPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) .hinted.default.point.class default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) .hinted.point.class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedDualObjectPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) .hinted.dual.point.class default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonDefaultPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) ! .hinted.singleton.default.point.class default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) ! .hinted.singleton.point.class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonDualObjectPointClass(self):
        hint = '.symbol.class[]'
        line = f'({hint}) ! .hinted.singleton.dual.point.class default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedDefaultSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) .hinted.default.point.class !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) .hinted.point.class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedDualSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) .hinted.dual.point.class !singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonDefaultSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) ! .hinted.singleton.default.point.class !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) ! .hinted.singleton.point.class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testHintedSingletonDualSingletonObjectPointClass(self):
        hint = '.recursive.symbol.class[]...'
        line = f'({hint}) ! .hinted.dual.singleton.point.class !singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, None, None)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.hints), 1)
        self.assertEqual(str(result.hints[0]), hint)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)


class TestDecoratedPointClass(TestPointClassBase):
    def testDecoratedDefaultPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.default.point.class ({ordinal}, "{description}") default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.point.class ({ordinal}, "{description}") ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedDualObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.dual.point.class ({ordinal}, "{description}") default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonDefaultPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .singleton.default.point.class ({ordinal}, "{description}") default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .singleton.point.class ({ordinal}, "{description}") ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonDualObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .singleton.dual.point.class ({ordinal}, "{description}") default_object_class ?object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedDefaultSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.default.point.class ({ordinal}, "{description}") !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.point.class ({ordinal}, "{description}") !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedDualSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'.dual.point.class ({ordinal}, "{description}") ' +\
               f'!singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertFalse(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonDefaultSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .singleton.default.point.class ({ordinal}, "{description}") !singleton_default_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertTrue(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .singleton.point.class ({ordinal}, "{description}") !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 1)
        self.assertFalse(result.objects[0].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)

    def testDecoratedSingletonDualSingletonObjectPointClass(self):
        ordinal = random.randint(-50, 50)
        description = 'Root symbol class'
        line = f'! .dual.singleton.point.class ({ordinal}, "{description}") ' +\
               f'!singleton_default_object_class !?singleton_object_class'

        result: PointClass = self.execute(line)

        self.isPersisted(result)
        self.isDecorated(result.path, ordinal, description)
        self.assertTrue(result.singleton)
        self.assertEqual(len(result.objects), 2)
        self.assertNotEqual(result.objects[0].default, result.objects[1].default)
        for o in result.objects:
            self.isDecorated(o, None, None)
            self.isNotHinted(o)


# class TestObjectDecoratedPointClass(TestPointClassBase):
#     pass
