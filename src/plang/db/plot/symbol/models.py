from typing import List, Optional, Set

from sqlalchemy import Column, Integer, String, ForeignKey, UniqueConstraint, Boolean
from sqlalchemy.orm import relationship

from plang.db.models import Path
from plang.db.base import Base, Decoratable


class SymbolClass(Base):
    class Form:
        def __init__(self,
                     path: Path.Form, decoration: Optional[Decoratable.Form] = None,
                     elements: List[str] = None,
                     hints: Set["Form"] = None,
                     recursive: bool = False):
            self.path = path
            self.elements = elements
            self.decoration = decoration
            self.hints = hints
            self.recursive = recursive

    __tablename__ = 'plot_symbol_class'
    __table_args__ = (
        UniqueConstraint('id', 'path_id'),
    )

    path_id = Column(Integer, ForeignKey('path.id'), nullable=False)

    path = relationship('Path', back_populates='symbol_class')
    instances = relationship('Symbol', back_populates='clazz')
    hints = relationship('SymbolClassHint', back_populates='clazz', foreign_keys='SymbolClassHint.class_id')

    def __str__(self) -> str:
        return f'{self.path}[]'


class SymbolClassHint(Base):
    __tablename__ = 'plot_symbol_class_hint'

    class_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    hint_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)
    recursive = Column(Boolean, default=False, nullable=False)

    clazz = relationship('SymbolClass', back_populates='hints', foreign_keys=[class_id])
    hint = relationship('SymbolClass', foreign_keys=[hint_id])

    def __str__(self):
        return f'{str(self.hint)}{"..." if self.recursive else ""}'


class Symbol(Decoratable, Base):
    class Form:
        def __init__(self, clazz: SymbolClass.Form, name: str):
            self.clazz = clazz
            self.name = name

    __tablename__ = 'plot_symbol'
    __table_args__ = (
        UniqueConstraint('class_id', 'name'),
    )

    name = Column(String, nullable=False)
    class_id = Column(Integer, ForeignKey('plot_symbol_class.id'), nullable=False)

    clazz = relationship('SymbolClass', back_populates='instances')
    compounds = relationship('SymbolCompound', foreign_keys='SymbolCompound.symbol_id')

    def __str__(self):
        return f'{str(self.clazz.path)}[{self.name}]'


class SymbolCompound(Base):
    class Form:
        def __init__(self, symbols: List[Symbol.Form]):
            self.symbols = symbols

    __tablename__ = 'plot_symbol_compound'
    __table_args__ = (
        UniqueConstraint('symbol_id', 'distance'),
    )

    symbol_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)
    compound_id = Column(Integer, ForeignKey('plot_symbol.id'), nullable=False)
    distance = Column(Integer, nullable=False)

    symbol = relationship('Symbol', foreign_keys=[symbol_id], back_populates='compounds')
    compound = relationship('Symbol', foreign_keys=[compound_id])
