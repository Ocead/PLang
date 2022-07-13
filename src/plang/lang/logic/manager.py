from typing import List, Optional, Dict

from sqlalchemy import inspect
from sqlalchemy.orm import Session

from plang.cli.scope import Scope
from plang.db.base import Base, FormBase
from plang.db.models import Path
from plang.db.plot.symbol.models import SymbolClass, SymbolClassHint, Symbol


class Report:
    def __init__(self, added: List[Base], modified: List[Base], deleted: List[Base], failed: List[FormBase]):
        self.added = added or []
        self.modified = modified or []
        self.deleted = deleted or []
        self.failed = failed or []

    def __len__(self):
        return len(self.added) + len(self.modified) + len(self.deleted) + len(self.failed)

    def __str__(self):
        sa = 'Added entries:\n\t' + '\n\t'.join([str(a) for a in self.added]) if len(self.added) > 0 else ''
        sm = 'Modified entries:\n\t' + '\n\t'.join([str(a) for a in self.modified]) if len(self.modified) > 0 else ''
        sd = 'Deleted entries:\n\t' + '\n\t'.join([str(a) for a in self.deleted]) if len(self.deleted) > 0 else ''
        sf = 'Failed entries:\n\t' + '\n\t'.join([str(a) for a in self.failed]) if len(self.failed) > 0 else ''
        return '\n'.join(s for s in [sa, sm, sd, sf] if len(s) > 0)


class Manager:
    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope

    def __refresh(self, instance):
        if inspect(instance).persistent:
            self.session.refresh(instance)

    def report(self):
        return Report(self.session.new, self.session.dirty, self.session.deleted, [])

    def selectOrInsertPath(self, form: Path.Form, insert: bool = True) -> Optional[Path]:
        node: Path
        if form.qualified:
            root: List[Path] = self.session.query(Path).where(Path.id == Path.parent_id).all()
            if len(root) != 1:
                return None
            node: Path = root[0]
        else:
            node = self.scope.getPath()

        leaf = len(form.nodes) - 1
        for i in range(0, len(form.nodes)):
            nodes = [n for n in node.children if n.name == form.nodes[i]]
            if len(nodes) != 1 and insert:
                new_path = Path()
                new_path.name = form.nodes[i]
                new_path.parent = node
                self.session.add(new_path)
                self.__refresh(node)  # Flush new child to parent
                node = new_path
            elif len(nodes) == 1:
                node = nodes[0]
            else:
                return None

        if insert and form.decoration is not None:
            node.ordinal = form.decoration.ordinal
            node.description = form.decoration.description
        return node

    def selectPath(self, form: Path.Form) -> Optional[Path]:
        if form.qualified:
            return self.selectOrInsertPath(form, False)
        else:
            scope_node = self.scope.getPath()
            if len(form.nodes) == 0:
                return scope_node
            candidates: Dict[Path, Path] = {x: x for x in
                                            self.session.query(Path).where(Path.name == form.nodes[-1]).all()}

            match_candidates = candidates

            for n in form.nodes[::-1][1:]:
                match_candidates = {x: y.parent for (x, y) in match_candidates.items() if y.parent.name == n}

            if len(match_candidates) == 1:
                return next(iter(match_candidates.keys()))

            self_candidates = {x: y for (x, y) in match_candidates.items() if x == self.scope.getPath()}

            if len(self_candidates) == 1:
                return next(iter(self_candidates.keys()))

            child_candidates = {x: y for (x, y) in match_candidates.items() if x.is_child_of(scope_node)}

            if len(child_candidates) == 1:
                return next(iter(child_candidates.keys()))

            parent_candidates = {x: y for (x, y) in match_candidates.items() if x.is_parent_of(scope_node)}

            if len(parent_candidates) == 1:
                return next(iter(parent_candidates.keys()))

            return None

    def insertPath(self, form: Path.Form) -> Optional[Path]:
        result = self.selectOrInsertPath(form)
        return result if result.id is not None else None

    def removePath(self, path: Path, recursive: bool = False):
        self.session.delete(path)

    def selectOrInsertSymbolClass(self, form: SymbolClass.Form, insert: bool = True) -> Optional[SymbolClass]:
        path = self.selectPath(form.path) or self.selectOrInsertPath(form.path, insert)
        if path.symbol_class is None:
            new_symbol_class = SymbolClass()
            new_symbol_class.path = path
            self.session.add(new_symbol_class)
            symbol_class = new_symbol_class
            self.__refresh(path)
        else:
            symbol_class = path.symbol_class

        if form.hints is not None:
            for h in symbol_class.hints:
                self.session.delete(h)

            for h in form.hints:
                hint = SymbolClassHint()
                hint.hint = self.selectOrInsertSymbolClass(h, insert)
                hint.recursive = h.recursive
                hint.clazz = symbol_class
                self.session.add(hint)

        # self.__refresh(path)

        if form.path.decoration is not None:
            path.ordinal = form.path.decoration.ordinal
            path.description = form.path.decoration.description

        return symbol_class

    def selectSymbolClass(self, form: SymbolClass.Form) -> Optional[SymbolClass]:
        path = self.selectPath(form.path)
        if path is None:
            return None
        else:
            return path.symbol_class

    def insertSymbolClass(self, form: SymbolClass.Form) -> Optional[SymbolClass]:
        path = self.selectPath(form.path)
        if path is None:
            return None
        elif path.symbol_class is None:
            new_symbol_class = SymbolClass()
            new_symbol_class.path = path
            self.session.add(new_symbol_class)
            return new_symbol_class
        else:
            return None

    def removeSymbolClass(self, symbol_class: SymbolClass):
        self.session.delete(symbol_class)

    def selectOrInsertSymbol(self, form: Symbol.Form, insert: bool = True) -> Optional[Symbol]:
        symbolClass = self.selectOrInsertSymbolClass(form.clazz, insert)
        if symbolClass is None:
            return None
        else:
            symbol = next(filter(lambda x: x.name == form.name, symbolClass.instances), None)
            if symbol is None:
                symbol = Symbol()
                symbol.name = form.name
                symbol.clazz = symbolClass
                self.session.add(symbol)

            if form.decoration is not None:
                symbol.ordinal = form.decoration.ordinal
                symbol.description = form.decoration.description

            return symbol

    def selectSymbol(self, form: Symbol.Form) -> Optional[Symbol]:
        pass

    def insertSymbol(self, form: Symbol.Form) -> Optional[Symbol]:
        pass

    def removeSymbol(self, symbol: Symbol):
        pass
