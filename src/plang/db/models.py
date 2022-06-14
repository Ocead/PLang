from typing import List

from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy import UniqueConstraint, CheckConstraint, PrimaryKeyConstraint
from sqlalchemy.orm import relationship

from plang.db.base import Base


class Config(Base):
    __tablename__ = 'config'
    __table_args__ = (
        UniqueConstraint('option', name=__tablename__ + '_option_uindex'),
    )

    option = Column(String, primary_key=True, nullable=False)
    value = Column(String, nullable=False)


class Source(Base):
    __tablename__ = 'source'

    id = Column(Integer, primary_key=True, nullable=False)
    name = Column(String, nullable=False)
    author = Column(String, nullable=False)
    origin = Column(String, nullable=False)

class Path(Base):
    __tablename__ = 'path'
    __table_args__ = (
        PrimaryKeyConstraint('id', name=__tablename__ + '_pk'),
        UniqueConstraint('name', 'path_id', name=__tablename__ + '_name_parent_id_uindex'),
        UniqueConstraint('id', name=__tablename__ + '_id_uindex'),
    )

    id = Column(Integer().with_variant(Integer, "sqlite"),
                primary_key=True,
                nullable=False)
    name = Column(String,
                  CheckConstraint("name regexp '^[^.?!()\\[\\]{\\}]*$'",
                                  name=__tablename__ + '_name_check'),
                  nullable=False)
    path_id = Column(Integer,
                     ForeignKey(__tablename__ + '.id'),
                     nullable=False)
    ordinal = Column(Integer,
                     nullable=True)
    description = Column(String,
                         nullable=True)

    parent = relationship('Path', remote_side=[id])
    children = relationship('Path', back_populates='parent')

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

    def __str__(self) -> str:
        path = self.name
        node = self

        while node.id != node.super:
            node = node.parent
            if node is None:
                break
            path = node.name + '.' + path

        if len(path) != 0:
            return path
        else:
            return '.'

    def __repr__(self) -> str:
        return f'{type(self).__name__}({str(self)})'
