from sqlalchemy import Column, Integer, String, ForeignKey, UniqueConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base


class SymbolClass(Base):
    __tablename__ = 'plot_symbol_class'
    __table_args__ = (
        UniqueConstraint('id', 'path_id'),
    )

    id = Column(Integer, primary_key=True)
    path_id = Column(Integer, ForeignKey('path.id'), nullable=False)

    path = relationship('Path')
    instances = relationship('Symbol', back_populates='clazz')


class Symbol(Base):
    __tablename__ = 'plot_symbol'
    __table_args__ = (
        UniqueConstraint('class_id', 'name'),
    )

    id = Column(Integer, primary_key=True)
    name = Column(String, nullable=False)
    class_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    ordinal = Column(Integer, nullable=True)

    clazz = relationship('SymbolClass', back_populates='instances')

    def __str__(self):
        return f'{str(self.clazz.path)}[{self.name}]'


class SymbolCompound(Base):
    __tablename__ = 'plot_symbol_compound'
    __table_args__ = (
        UniqueConstraint('symbol_id', 'distance'),
    )

    id = Column(Integer, primary_key=True)
    symbol_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)
    compound_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)
    distance = Column(Integer, nullable=False)

    symbol = relationship('Symbol', foreign_keys='SymbolCompound.symbol_id')
    compound = relationship('Symbol', foreign_keys='SymbolCompound.compound_id')
