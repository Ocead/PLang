from abc import ABC, abstractmethod
from typing import List, Any, Union, Optional

from plang.db import Path


class Scope(ABC):
    @abstractmethod
    def lookup(self, identifier: str) -> List:
        """
        Looks up a path under the current scope path

        :param identifier: identifier to look for
        :return: list of candidates
        """
        pass

    @abstractmethod
    def lookupDirect(self, identifier: str) -> Any:
        """
        Looks up a path directly under the current scope path

        :param identifier: identifier to look for
        :return: list of candidates
        """
        pass

    @abstractmethod
    def lookupRecursive(self, identifier: str) -> List:
        """
        Looks up a path directly under the current scope path or in children of the current scope paths parents

        :param identifier: identifier to look for
        :return: list of candidates
        """
        pass

    @abstractmethod
    def getOrCreatePath(self, nodes: List[str]) -> Any:
        pass

    @abstractmethod
    def getPath(self) -> Any:
        pass

    @abstractmethod
    def up(self) -> "Scope":
        """
        Gives the scope of the current scope paths parent

        :return: the parent scope
        """
        pass


class PlangScope(Scope):
    def __init__(self) -> None:
        super().__init__()

    def lookup(self, identifier: str) -> List:
        return []

    def lookupDirect(self, identifier: str) -> Any:
        pass

    def lookupRecursive(self, identifier: str) -> List:
        return []

    def getOrCreatePath(self, nodes: List[str]) -> Any:
        pass

    def getPath(self) -> Any:
        pass

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

    def lookupDirect(self, identifier: str) -> Optional[Path]:
        return filter(lambda x: x.name == identifier, self.path.children) or None

    def lookupRecursive(self, identifier: str) -> List[Path]:
        pass

    def getOrCreatePath(self, nodes: List[str]) -> Any:
        scope = self
        for node in nodes:
            path = scope.lookupDirect(node)
            if path is None:
                new_path = Path()
                new_path.name = str(node)
                new_path.parent = scope.getPath()
                scope.getPath().children += [new_path]

    def getPath(self) -> Path:
        return self.path

    def up(self) -> "PathScope":
        return PathScope(self.path.parent)
