import enum

from sqlalchemy import Column, Integer, String, Boolean, Enum, ForeignKey, UniqueConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base, Decoratable


class ObjectClass(Decoratable, Base):
    __tablename__ = 'plot_object_class'
    __table_args__ = (
        UniqueConstraint('point_class_id', 'name'),
    )

    point_class_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    name = Column(String(), nullable=False)
    default = Column(Boolean, default=False, nullable=False)
    singleton = Column(Boolean, default=False, nullable=False)

    point_class = relationship('PointClass')
    hints_lit = relationship('ObjectClassHintLit', back_populates='clazz')
    hints_sym = relationship('ObjectClassHintSym', back_populates='clazz')
    hints_pnt = relationship('ObjectClassHintPnt', back_populates='clazz')
    instances_lit = relationship('ObjectLit', back_populates='clazz')
    instances_sym = relationship('ObjectSym', back_populates='clazz')
    instances_pnt = relationship('ObjectPnt', back_populates='clazz')


class ObjectClassHint(Base):
    __abstract__ = True
    pass


class ObjectClassHintLit(ObjectClassHint):
    class Types(enum.Enum):
        LIKE = 'l'
        GLOB = 'g'
        REGEXP = 'r'
        MATCH = 'm'

    __tablename__ = 'plot_object_class_hint_lit'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    hint = Column(String, nullable=False)
    type = Column(Enum(Types), nullable=True)

    clazz = relationship('ObjectClass', back_populates='hints_lit')


class ObjectClassHintSym(ObjectClassHint):
    __tablename__ = 'plot_object_class_hint_sym'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('ObjectClass', back_populates='hints_sym')
    hint = relationship('SymbolClass')


class ObjectClassHintPnt(ObjectClassHint):
    __tablename__ = 'plot_object_class_hint_pnt'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('ObjectClass', back_populates='hints_pnt')
    hint = relationship('PointClass')



class ObjectLit(Base):
    __tablename__ = 'plot_object_lit'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object = Column(String, nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_lit')
    point_class = relationship('Point')

class ObjectSym(Base):
    __tablename__ = 'plot_object_sym'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_sym')
    point_class = relationship('Point')
    object_symbol = relationship('Symbol')


class ObjectPnt(Base):
    __tablename__ = 'plot_object_pnt'

    class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    object_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)

    clazz = relationship('ObjectClass', back_populates='instances_pnt')
    point = relationship('Point', foreign_keys=[point_id])
    object = relationship('Point', foreign_keys=[object_id])
