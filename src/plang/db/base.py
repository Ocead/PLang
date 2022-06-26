from abc import ABC
from typing import Optional

from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import declarative_base, declared_attr


class FormBase(ABC):
    pass


class Sourced:
    @declared_attr
    def id(cls):
        return Column(Integer, primary_key=True, index=True, unique=True)

    @declared_attr
    def source(cls):
        return Column(Integer, ForeignKey('source.id'), nullable=True)


class Decoratable:
    class Form(FormBase):
        def __init__(self, ordinal: Optional[float] = None, description: Optional[str] = None):
            self.ordinal = float(str(ordinal)) if ordinal is not None else None
            self.description = str(description)[1:-1] if description is not None else None

    ordinal = Column(Integer, nullable=True)
    description = Column(String, nullable=True)

    def __str__(self) -> str:
        list = []
        if self.ordinal is not None:
            list += [str(self.ordinal).rstrip("0").rstrip(".")]
        if self.description is not None:
            list += [self.description]
        if len(list) > 0:
            return f'({", ".join(list)})'
        else:
            return ''


Base = declarative_base(cls=Sourced)


def form(obj: Base):
    return obj.__form__()
