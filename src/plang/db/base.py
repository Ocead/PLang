from abc import ABC
from typing import Optional

from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import declarative_base, declared_attr

from plang.db.visual import OP


class FormBase(ABC):
    pass


class Sourced:
    @declared_attr
    def id(cls):
        return Column(Integer, primary_key=True, index=True, unique=True)

    @declared_attr
    def source(cls):
        return Column(Integer, ForeignKey('source.id'), nullable=True)

    def str(self, rich):
        return self.__str__(rich)

    def __str__(self, rich: bool = False):
        return super().__str__()

    def __repr__(self) -> str:
        return f'{type(self).__name__}({str(self)})'


class Decoratable:
    class Form(FormBase):
        def __init__(self, ordinal: Optional[float] = None, description: Optional[str] = None):
            self.ordinal = float(str(ordinal)) if ordinal is not None else None
            self.description = str(description)[1:-1] if description is not None else None

    ordinal = Column(Integer, nullable=True)
    description = Column(String, nullable=True)

    def __str__(self, rich: bool = True) -> str:
        list = []
        if self.ordinal is not None:
            num = str(self.ordinal).rstrip("0").rstrip(".")
            num_str = ('[bright_blue]' if rich else '') \
                  + num if len(num) > 0 else '0' \
                  + ('[/bright_blue]' if rich else '')
            list += [num_str]
        if self.description is not None:
            des = ('[bold][green]"' if rich else '"') + self.description + ('"[/bold][/green]' if rich else '"')
            list += [des]
        if len(list) > 0:
            return f'{OP.hint_l(rich)}{", ".join(list)}{OP.hint_r(rich)}'
        else:
            return ''


Base = declarative_base(cls=Sourced)


def form(obj: Base):
    return obj.__form__()
