import enum

from sqlalchemy import Column, Integer, String, Boolean, Enum, ForeignKey
from sqlalchemy.orm import relationship

from plang.db.base import Base


class CausalSymbolClass(Base):
    __tablename__ = 'plot_causal_symbol_class'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    symbol_class_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)

    causal = relationship('Causal')
    symbol_class = relationship('SymbolClass', foreign_keys=[symbol_class_id])


class CausalPointClass(Base):
    class Direction(enum.Enum):
        REQ = '<-'
        IPL = '->'

    __tablename__ = 'plot_causal_point_class'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    point_class_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    direction = Column(Enum(Direction), nullable=True)
    overlap = Column(Boolean, nullable=False)

    causal = relationship('Causal')
    point_class = relationship('PointClass')


class Causal(Base):
    class Operations(enum.Enum):
        ALL = '*'
        ONE = '+'
        UNQ = '~'

    __tablename__ = 'plot_causal'

    op = Column(Enum(Operations), nullable=True)

    symbol_classes = relationship('CausalSymbolClass', back_populates='causal')
    point_classes = relationship('CausalPointClass', back_populates='causal')
    elements_obj = relationship('CausalElementObj', back_populates='causal')
    elements_lit = relationship('CausalElementLit', back_populates='causal')
    elements_sym = relationship('CausalElementPnt', back_populates='causal')
    elements_pnt = relationship('CausalElementSym', back_populates='causal')


class CausalElementObj(Base):
    __tablename__ = 'plot_causal_element_obj'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    slot = Column(Integer, nullable=False)
    point_class_id = Column(Integer, ForeignKey('plot_point_class.id'), nullable=False)
    object_class_id = Column(Integer, ForeignKey('plot_object_class.id'), nullable=True)

    causal = relationship('Causal')


class CausalElementLit(Base):
    class Types(enum.Enum):
        LIKE = 'l'
        GLOB = 'g'
        REGEXP = 'r'
        MATCH = 'm'

    __tablename__ = 'plot_causal_element_lit'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    slot = Column(Integer, nullable=False)
    type = Column(Enum(Types), nullable=True)
    literal = Column(String, nullable=False)

    causal = relationship('Causal')


class CausalElementSym(Base):
    __tablename__ = 'plot_causal_element_sym'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    slot = Column(Integer, nullable=False)
    symbol_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)

    causal = relationship('Causal')
    symbol = relationship('Symbol')


class CausalElementPnt(Base):
    __tablename__ = 'plot_causal_element_pnt'

    causal_id = Column(Integer, ForeignKey('plot_causal.id'), nullable=False)
    slot = Column(Integer, nullable=False)
    point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)

    causal = relationship('Causal')
    point = relationship('Point')


class Causality(Base):
    __tablename__ = 'plot_causal_ity'

    cause_point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    effect_point_id = Column(Integer, ForeignKey('plot_point.id'), nullable=False)
    overlap = Column(Boolean, nullable=False)

    cause = relationship('Point', foreign_keys=[cause_point_id])
    effect = relationship('Point', foreign_keys=[effect_point_id])
