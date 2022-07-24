import enum
from typing import Optional, List

from sqlalchemy import Column, Integer, String, Boolean, Enum, ForeignKey, UniqueConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base, Decoratable, FormBase
from plang.db.models import Path
from plang.db.visual import OP


class ObjectClass(Decoratable, Base):
    class Form(FormBase):
        def __init__(self,
                     name: str,
                     path_form: Optional[Path.Form] = Path.Form(False, list()),
                     singleton: bool = False,
                     default: bool = False,
                     hint_list: List = ()):
            self.name = str(name)
            self.path_form = path_form
            self.singleton = singleton
            self.default = default
            self.hint_list = hint_list

    __tablename__ = 'plot_object_class'
    __table_args__ = (
        UniqueConstraint('point_class_id', 'name'),
    )

    point_class_id = Column(Integer, ForeignKey('plot_point_class.id', ondelete='CASCADE'), nullable=False)
    name = Column(String(), nullable=False)
    default = Column(Boolean, default=False, nullable=False)
    singleton = Column(Boolean, default=False, nullable=False)

    point_class = relationship('PointClass')
    hints_lit = relationship('ObjectClassHintLit', cascade='all, delete', back_populates='clazz')
    hints_sym = relationship('ObjectClassHintSym', cascade='all, delete', back_populates='clazz')
    hints_pnt = relationship('ObjectClassHintPnt', cascade='all, delete', back_populates='clazz')
    instances_lit = relationship('ObjectLit', back_populates='clazz')
    instances_sym = relationship('ObjectSym', back_populates='clazz')
    instances_pnt = relationship('ObjectPnt', back_populates='clazz')

    def __str__(self, rich: bool = False):
        return f'{(OP.single(rich) + " ") if self.singleton else ""}'\
               f'{OP.obj_name(rich) if not self.default else ""}{self.name}'


class ObjectClassHint(Base):
    __abstract__ = True
    pass


class ObjectClassHintLit(ObjectClassHint):
    class Type(enum.Enum):
        COMMENT = 0
        LIKE = 1
        GLOB = 2
        REGEX = 3
        MATCH = 4

    class Form(FormBase):
        def __init__(self, type: "Types", hint: str):
            self.type = type
            self.hint = hint

    __tablename__ = 'plot_object_class_hint_lit'

    class_id = Column(Integer, ForeignKey('plot_object_class.id', ondelete='CASCADE'), nullable=False)
    hint = Column(String, nullable=False)
    type = Column(Enum(Type), nullable=False)

    clazz = relationship('ObjectClass', back_populates='hints_lit')

    def __str__(self, rich: bool = False):
        return f'{self.type.value or ""}{repr(self.hint)}'


class ObjectClassHintSym(ObjectClassHint):
    __tablename__ = 'plot_object_class_hint_sym'

    class_id = Column(Integer, ForeignKey('plot_object_class.id', ondelete='CASCADE'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('ObjectClass', back_populates='hints_sym')
    hint = relationship('SymbolClass')

    def __str__(self, rich: bool = False):
        return f'{self.hint.__str__(rich)}{OP.recur(rich) if self.recursive else ""}'


class ObjectClassHintPnt(ObjectClassHint):
    __tablename__ = 'plot_object_class_hint_pnt'

    class_id = Column(Integer, ForeignKey('plot_object_class.id', ondelete='CASCADE'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('ObjectClass', back_populates='hints_pnt')
    hint = relationship('PointClass')

    def __str__(self, rich: bool = False):
        return f'{self.hint.str(rich)}{OP.recur(rich) if self.recursive else ""}'


class ObjectLit(Base):
    __tablename__ = 'plot_object_lit'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object = Column(String, nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_lit')
    point_class = relationship('Point')

    def __str__(self, rich: bool = False):
        return f'{repr(object)}'


class ObjectSym(Base):
    __tablename__ = 'plot_object_sym'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_sym')
    point_class = relationship('Point')
    object = relationship('Symbol')

    def __str__(self, rich: bool = False):
        return f'"{self.object.str(rich)}"'


class ObjectPnt(Base):
    __tablename__ = 'plot_object_pnt'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_pnt')
    point = relationship('Point', foreign_keys=[point_id])
    object = relationship('Point', foreign_keys=[object_id])

    def __str__(self, rich: bool = False):
        return f'"{self.object.str(rich)}"'
