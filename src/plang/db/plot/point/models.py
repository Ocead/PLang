from typing import List

from sqlalchemy import Column, Integer, Boolean, ForeignKey, UniqueConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base, FormBase
from plang.db.models import Path
from plang.db.plot.object.models import ObjectClass
from plang.db.visual import OP


class PointClass(Base):
    class Form(FormBase):
        def __init__(self, path_form: Path.Form,
                     singleton: bool = False,
                     object_class_forms: List[ObjectClass.Form] = (),
                     hint_list: List = (),
                     recursive: bool = False):
            self.path_form = path_form
            self.singleton = singleton
            self.object_class_forms = object_class_forms
            self.hint_list = hint_list
            self.recursive = recursive

    __tablename__ = 'plot_point_class'
    __table_args__ = (
        UniqueConstraint('id', 'path_id'),
    )

    path_id = Column(Integer, ForeignKey('path.id'), nullable=False)
    singleton = Column(Boolean, default=False, nullable=False)

    path = relationship('Path', back_populates='point_class')
    hints = relationship('PointClassHint', cascade='all, delete', back_populates='clazz')
    objects = relationship('ObjectClass', cascade='all, delete', back_populates='point_class')
    instances = relationship('Point', back_populates='clazz')

    def str(self, rich: bool = False) -> str:
        return self.__str__(rich)

    def __str__(self, rich: bool = False) -> str:
        return f'{self.path.str(rich)}?:'


class PointClassHint(Base):
    __tablename__ = 'plot_point_class_hint'
    __table_args__ = (
        UniqueConstraint('class_id', 'hint_id'),
    )

    class_id = Column(Integer, ForeignKey('plot_point_class.id', ondelete='CASCADE'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('PointClass', back_populates='hints')
    hint = relationship('SymbolClass')

    def __str__(self, rich: bool = False) -> str:
        return f'{self.hint.__str__(rich)}{OP.recur(rich) if self.recursive else ""}'


class Point(Base):
    __tablename__ = 'plot_point'

    class_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    truth = Column(Boolean, default=True, nullable=True)

    clazz = relationship('PointClass', back_populates='instances')
    subject_symbol = relationship('PointSubjectSym', back_populates='point')
    subject_symbol_class = relationship('PointSubjectCls', back_populates='point')

    def __str__(self):
        pass # TODO: Implement


class PointSubjectSym(Base):
    __tablename__ = 'plot_point_subject_sym'

    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    symbol_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)

    point = relationship('Point', back_populates='subject_symbol')
    symbol = relationship('Symbol')


class PointSubjectCls(Base):
    __tablename__ = 'plot_point_subject_cls'

    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    symbol_class_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)

    point = relationship('Point', back_populates='subject_symbol_class')
    symbol_class = relationship('SymbolClass')

# TODO: incomplete point (maybe causal) as filter for subject
