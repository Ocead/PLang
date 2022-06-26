from typing import List, Optional

from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy import UniqueConstraint, CheckConstraint
from sqlalchemy.orm import relationship, declared_attr

from plang.db.base import Base, Decoratable


class Config(Base):
    __tablename__ = 'config'

    option = Column(String, nullable=False, unique=True)
    value = Column(String, nullable=False)


class Source(Decoratable, Base):
    __tablename__ = 'source'

    id = Column(Integer, primary_key=True, nullable=False)
    name = Column(String, nullable=False)
    author = Column(String, nullable=False)
    origin = Column(String, nullable=False)


class Path(Decoratable, Base):
    class Form:
        def __init__(self, qualified: bool, nodes: List[str], decoration: Optional[Decoratable.Form] = None):
            self.qualified = qualified
            self.nodes = [str(x) for x in nodes]
            self.decoration = decoration

    __tablename__ = 'path'
    __table_args__ = (
        UniqueConstraint('name', 'parent_id'),
    )

    name = Column(String,
                  CheckConstraint("name regexp '^[^.?!()\\[\\]{\\}]*$'",
                                  name='path_name_check'),
                  nullable=False)
    parent_id = Column(Integer,
                       ForeignKey('path.id'),
                       nullable=False)

    @declared_attr
    def parent(cls):
        return relationship('Path', remote_side=f'{format(cls.__name__)}.id')
    children = relationship('Path', back_populates='parent')

    symbol_class = relationship('SymbolClass')
    point_class = relationship('PointClass')

    def get_ordinal(self, max: int) -> int:
        return self.ordinal or max

    def get_all_children(self, min: int, max: int, ordinal: str = '') -> dict['Path', str]:
        i = 0

        children = {c: ordinal +
                       ((chr(min + self.get_ordinal(max))) if self.id != c.id else '') +
                       (chr(min + c.ordinal) if c.ordinal is not None else chr(max + (i := i + 1)))
                    for c in self.children}

        for c, o in children.items():
            if c.id != self.id:
                children = {**children, **c.get_all_children(min=min, max=max, ordinal=o)}

        return children

    def get_all_paths(self) -> List[str]:
        path = self.name
        paths = [path]
        node = self

        while node.id != node.parent_id:
            node = node.parent
            if node is None:
                break
            path = f"{node.name}.{path}"
            paths += [path]

        return paths

    def __form__(self) -> Form:
        node = self
        nodes = []
        while node.id != node.parent_id:
            nodes = [node.name] + nodes
            node = node.parent

        return Path.Form(True, nodes)

    def __str__(self) -> str:
        path = self.name
        node = self

        while node != node.parent:
            node = node.parent
            if node is None:
                break
            path = node.name + '.' + path

        if len(path) != 0:
            str = path
        else:
            str =  '.'

        return f'{str} {Decoratable.__str__(self)}'

    def __repr__(self) -> str:
        return f'{type(self).__name__}({str(self)})'
