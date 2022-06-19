from abc import ABC, abstractmethod
from typing import List

from plang.db import Path


class Scope(ABC):
    @abstractmethod
    def lookup(self, Noneidentifier: str) -> List:
        pass

    @abstractmethod
    def lookupRecursive(self, identifier: str) -> List:
        pass

    @abstractmethod
    def up(self) -> "Scope":
        pass


class PlangScope(Scope):
    def __init__(self) -> None:
        super().__init__()

    def lookup(self, identifier: str) -> List:
        return []

    def lookupRecursive(self, identifier: str) -> List:
        return []

    def up(self) -> "Scope":
        return self

    def __str__(self):
        return "PLang"


class PathScope(Scope):
    def __init__(self, path: Path) -> None:
        super().__init__()
        self.path = path

    def lookup(self, identifier: str) -> List[Path]:
        pass

    def lookupRecursive(self, identifier: str) -> List[Path]:
        pass

    def up(self) -> "PathScope":
        return PathScope(self.path.parent)
