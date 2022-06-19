from re import sub

from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import declarative_base, declared_attr


class Sourced:
    @declared_attr
    def id(cls):
        return Column(Integer, primary_key=True, index=True, unique=True)

    @declared_attr
    def source(cls):
        return Column(Integer, ForeignKey('source.id'), nullable=True)


class Decoratable:
    ordinal = Column(Integer, nullable=True)
    description = Column(String, nullable=True)


Base = declarative_base(cls=Sourced)
