from typing import List, Optional, Dict, Union

from sqlalchemy import inspect
from sqlalchemy.orm import Session

from plang.db.plot.object.models import ObjectClass, ObjectClassHintLit, ObjectClassHintSym, ObjectClassHintPnt
from plang.db.plot.point.models import PointClass, PointClassHint
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

    def str(self, rich: bool):
        return self.__str__(rich)

    def __str__(self, rich: bool = False) -> str:
        sa = 'Added entries:\n\t' + '\n\t'.join([a.str(rich) for a in self.added]) if len(self.added) > 0 else ''
        sm = 'Modified entries:\n\t' + '\n\t'.join([a.str(rich) for a in self.modified]) if len(
            self.modified) > 0 else ''
        # sd = 'Deleted entries:\n\t' + '\n\t'.join([a.str(rich) for a in self.deleted]) if len(self.deleted) > 0 else ''
        sd = []
        sf = 'Failed entries:\n\t' + '\n\t'.join([str(a) for a in self.failed]) if len(self.failed) > 0 else ''
        return '\n'.join(s for s in [sa, sm, sd, sf] if len(s) > 0)


class Manager:
    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope

    def __refresh(self, instance):
        if inspect(instance).persistent:
            self.session.refresh(instance)

    def report(self) -> Report:
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
        if recursive or len(path.children) == 0:
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
        symbol_class = self.selectOrInsertSymbolClass(form.clazz, insert)
        if symbol_class is None:
            return None
        else:
            symbol = next(filter(lambda x: x.name == form.name, symbol_class.instances), None)
            if symbol is None:
                symbol = Symbol()
                symbol.name = form.name
                symbol.clazz = symbol_class
                self.session.add(symbol)

            if form.decoration is not None:
                symbol.ordinal = form.decoration.ordinal
                symbol.description = form.decoration.description

            return symbol

    def selectSymbol(self, form: Symbol.Form) -> Union[None, Symbol, List[Symbol]]:
        candidates: Dict[Symbol, Path] = {x: x.clazz.path for x in
                                          self.session.query(Symbol).where(Symbol.name == form.name).all()}

        match_candidates = candidates

        for n in form.clazz.path.nodes[::-1]:
            match_candidates = {x: y.parent for (x, y) in match_candidates.items() if y.name == n}

        if len(match_candidates) == 1:
            return next(iter(match_candidates.keys()))
        else:
            return list(iter(match_candidates.keys()))

    def insertSymbol(self, form: Symbol.Form) -> Optional[Symbol]:
        pass

    def removeSymbol(self, symbol: Symbol):
        self.session.delete(symbol)

    def selectObjectClass(self, form: ObjectClass.Form) -> Optional[ObjectClass]:
        pass

    def insertObjectClass(self, form: ObjectClass.Form) -> Optional[ObjectClass]:
        path = self.selectPath(form.path_form) or self.selectOrInsertPath(form.path_form, True)
        point_class = path.point_class
        if point_class is None:
            return None
        object_class = next(filter(lambda x: x.name == form.name, point_class.objects), None)
        if object_class is None:
            object_class = ObjectClass()
            object_class.name = form.name
            object_class.point_class = point_class
            self.session.add(object_class)
            self.__refresh(point_class)
        object_class.default = form.default
        object_class.singleton = form.singleton
        for h in object_class.hints_lit:
            self.session.delete(h)
        for h in object_class.hints_sym:
            self.session.delete(h)
        for h in object_class.hints_pnt:
            self.session.delete(h)
        for h in form.hint_list:
            pass
            if isinstance(h, ObjectClassHintLit.Form):
                lit_hint = ObjectClassHintLit()
                lit_hint.type = h.type
                lit_hint.hint = str(h.hint)
                lit_hint.clazz = object_class
                self.session.add(lit_hint)
            elif isinstance(h, SymbolClass.Form):
                sym_hint = ObjectClassHintSym()
                sym_hint.hint = self.selectOrInsertSymbolClass(h)
                sym_hint.recursive = h.recursive
                sym_hint.clazz = object_class
                self.session.add(sym_hint)
            elif isinstance(h, PointClass.Form):
                pnt_hint = ObjectClassHintPnt()
                pnt_hint.hint = self.selectOrInsertPointClass(h)
                pnt_hint.recursive = h.recursive
                pnt_hint.clazz = object_class
                self.session.add(pnt_hint)
            self.__refresh(object_class)

        return object_class

    def removeObjectClass(self, object_class: ObjectClass):
        self.session.delete(object_class)

    def selectOrInsertPointClass(self, form: PointClass.Form, insert: bool = True) -> PointClass:
        return self.selectPointClass(form) or self.insertPointClass(form)

    def selectPointClass(self, form: PointClass.Form) -> Optional[PointClass]:
        path = self.selectPath(form.path_form)
        if path is None:
            return None
        return path.point_class

    def insertPointClass(self, form: PointClass.Form) -> Optional[PointClass]:
        path = self.selectPath(form.path_form) or self.selectOrInsertPath(form.path_form, True)
        if form.path_form.decoration is not None:
            path.ordinal = form.path_form.decoration.ordinal
            path.description = form.path_form.decoration.description
        if path.point_class is None:
            point_class = PointClass()
            point_class.path = path
            self.session.add(point_class)
            self.__refresh(path)
        else:
            point_class = path.point_class  # TODO: rest

        point_class.singleton = form.singleton
        if len(point_class.hints) > 0:
            for h in point_class.hints:
                self.session.delete(h)
            self.__refresh(point_class)
        for h in form.hint_list:
            point_class_hint = PointClassHint()
            point_class_hint.hint = self.selectOrInsertSymbolClass(h)
            point_class_hint.clazz = point_class
            point_class_hint.recursive = h.recursive
            self.session.add(point_class_hint)

        if len(point_class.objects) > 0:
            for o in point_class.objects:
                self.session.delete(o)
            self.__refresh(point_class)
        if len(form.object_class_forms) > 0:
            point_class_scope = self.scope.getScope(path)
            object_class_manager = Manager(self.session, point_class_scope)
            for o in form.object_class_forms:
                object_class_manager.insertObjectClass(o)

        return point_class

    def removePointClass(self, point_class: PointClass):
        self.session.delete(point_class)
