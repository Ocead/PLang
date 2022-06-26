from typing import List, Optional

from sqlalchemy.orm import Session

from plang.db import Path, SymbolClass, Symbol, Base, FormBase
from plang.cli.scope import Scope


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


class Manager():
    def __init__(self, session: Session, scope: Scope):
        self.session = session
        self.scope = scope

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
            if len(nodes) != 1:
                new_path = Path()
                new_path.name = form.nodes[i]
                new_path.parent = node
                if i == leaf and form.decoration is not None:
                    new_path.ordinal = form.decoration.ordinal
                    new_path.description = form.decoration.description
                self.session.add(new_path)
                node = new_path
            elif insert:
                node = nodes[0]
            else:
                return None

        return node

    def selectPath(self, form: Path.Form) -> Optional[Path]:
        return self.selectOrInsertPath(form, False)

    def insertPath(self, form: Path.Form) -> Optional[Path]:
        result = self.selectOrInsertPath(form)
        return result if result.id is not None else None

    def removePath(self, path: Path):
        pass

    def insertSymbolClass(self, form: SymbolClass.Form) -> Optional[SymbolClass]:
        pass

    def removeSymbolClass(self, symbol_class: SymbolClass):
        pass

    def insertSymbol(self, form: Symbol.Form) -> Optional[Symbol]:
        pass

    def removeSymbol(self, symbol: Symbol):
        pass
