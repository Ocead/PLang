from sqlalchemy import Column, Integer, Boolean, ForeignKey, UniqueConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base


class PointClass(Base):
    __tablename__ = 'plot_point_class'
    __table_args__ = (
        UniqueConstraint('id', 'path_id'),
    )

    path_id = Column(Integer, ForeignKey('path.id'), nullable=False)
    singleton = Column(Boolean, default=False, nullable=False)

    path = relationship('Path')
    hints = relationship('PointClassHint', back_populates='clazz')
    instances = relationship('Point', back_populates='clazz')


class PointClassHint(Base):
    __tablename__ = 'plot_point_class_hint'
    __table_args__ = (
        UniqueConstraint('class_id', 'symbol_id'),
    )

    class_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    symbol_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('PointClass', back_populates='hints')


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
