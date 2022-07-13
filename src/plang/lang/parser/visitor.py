from enum import Enum
from typing import List, Optional

from sqlalchemy.orm import Session

from plang.db.base import Decoratable
from plang.db.plot.symbol.models import SymbolClass, Symbol
from plang.cli.scope import Scope, PathScope
from plang.db.base import Sourced
from plang.db.models import Path
from plang.lang.generated.PlangVisitor import PlangVisitor as BasePlangVisitor
from plang.lang.generated.PlangParser import PlangParser
from plang.lang.logic.manager import Manager


class MarkupMode(Enum):
    STRICT = 0
    IMPLICIT = 1


class ElementsList(set):
    pass


class HintList(set):
    pass


class PlangVisitor(BasePlangVisitor):
    def __init__(self, session: Session, scope: Scope, mode: MarkupMode):
        self.session = session
        self.scope = scope
        self.mode = mode

        self.implied: List[Sourced] = []

    def aggregateResult(self, aggregate, nextResult):
        if aggregate is None:
            list = []
        elif not isinstance(aggregate, List):
            list = [aggregate]
        else:
            list = aggregate

        if nextResult is None:
            last = []
        elif not isinstance(nextResult, List):
            last = [nextResult]
        else:
            last = nextResult

        result = list + last
        return result

    def visitStart(self, ctx: PlangParser.StartContext):
        return super().visitStart(ctx)

    def visitStartSVO(self, ctx: PlangParser.StartSVOContext):
        return super().visitStartSVO(ctx)

    def visitExprSVO(self, ctx: PlangParser.ExprSVOContext):
        return super().visitExprSVO(ctx)

    def visitDeclCore(self, ctx: PlangParser.DeclCoreContext):
        return super().visitDeclCore(ctx)

    def visitDeclSVOCore(self, ctx: PlangParser.DeclSVOCoreContext):
        return super().visitDeclSVOCore(ctx)

    def visitDeclSVO(self, ctx: PlangParser.DeclSVOContext):
        result = super().visitDeclSVO(ctx)
        return result

    def visitRef(self, ctx: PlangParser.RefContext):
        return super().visitRef(ctx)

    def visitHintCommentLiteral(self, ctx: PlangParser.HintCommentLiteralContext):
        return super().visitHintCommentLiteral(ctx)

    def visitHintLikeLiteral(self, ctx: PlangParser.HintLikeLiteralContext):
        return super().visitHintLikeLiteral(ctx)

    def visitHintGlobLiteral(self, ctx: PlangParser.HintGlobLiteralContext):
        return super().visitHintGlobLiteral(ctx)

    def visitHintRegexpLiteral(self, ctx: PlangParser.HintRegexpLiteralContext):
        return super().visitHintRegexpLiteral(ctx)

    def visitHintMatchLiteral(self, ctx: PlangParser.HintMatchLiteralContext):
        return super().visitHintMatchLiteral(ctx)

    def visitHintLiteral(self, ctx: PlangParser.HintLiteralContext):
        return super().visitHintLiteral(ctx)

    def visitHintSymbolClass(self, ctx: PlangParser.HintSymbolClassContext):
        result = super().visitHintSymbolClass(ctx)
        return result

    def visitHintPointClass(self, ctx: PlangParser.HintPointClassContext):
        return super().visitHintPointClass(ctx)

    def visitHint(self, ctx: PlangParser.HintContext):
        return super().visitHint(ctx)

    def visitHintList(self, ctx: PlangParser.HintListContext):
        return super().visitHintList(ctx)

    def visitHintSymbolClassList(self, ctx: PlangParser.HintSymbolClassListContext):
        result = super().visitHintSymbolClassList(ctx)
        return HintList(result)

    def visitHintDecl(self, ctx: PlangParser.HintDeclContext):
        return super().visitHintDecl(ctx)

    def visitDecoration(self, ctx: PlangParser.DecorationContext) -> Decoratable.Form:
        return Decoratable.Form(ctx.INTEGER() or ctx.DECIMAL(), ctx.STRING())

    def visitPathUnqualifiedNode(self, ctx: PlangParser.PathUnqualifiedNodeContext):
        return ctx.IDENTIFIER()

    def visitPathQualifiedNode(self, ctx: PlangParser.PathQualifiedNodeContext):
        return [ctx.OP_PATH()] + super().visitPathQualifiedNode(ctx)

    def visitPathFollowingNode(self, ctx: PlangParser.PathFollowingNodeContext):
        return super().visitPathFollowingNode(ctx)

    def visitPathUnqualifiedPath(self, ctx: PlangParser.PathUnqualifiedPathContext):
        return Path.Form(False, super().visitPathUnqualifiedPath(ctx))

    def visitPathQualifiedPath(self, ctx: PlangParser.PathQualifiedPathContext):
        return Path.Form(True, super().visitPathQualifiedPath(ctx))

    def visitPath(self, ctx: PlangParser.PathContext):
        result = super().visitPath(ctx)
        return result

    def visitPathDecl(self, ctx: PlangParser.PathDeclContext) -> Optional[Path]:
        result = super().visitPathDecl(ctx)
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        decorationForm = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if pathForm is not None and decorationForm is not None:
            pathForm.decoration = decorationForm
        path = Manager(self.session, self.scope).selectOrInsertPath(pathForm)
        return path

    def visitPathRef(self, ctx: PlangParser.PathRefContext):
        result = super().visitPathRef(ctx)
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        if pathForm is None:
            return None
        path = Manager(self.session, self.scope).selectPath(pathForm)
        return path

    def visitSymbolUnqualifiedClass(self, ctx: PlangParser.SymbolUnqualifiedClassContext):
        result = super().visitSymbolUnqualifiedClass(ctx)
        if result is None or len(result) == 0:
            result = Path.Form(False, [])
        return result

    def visitSymbolQualifiedClass(self, ctx: PlangParser.SymbolQualifiedClassContext):
        return super().visitSymbolQualifiedClass(ctx)

    def visitSymbolClass(self, ctx: PlangParser.SymbolClassContext):
        result = super().visitSymbolClass(ctx)
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        if pathForm is None:
            return None
        symbolClassForm = SymbolClass.Form(path=pathForm)
        return symbolClassForm

    def visitSymbolRecursiveClass(self, ctx: PlangParser.SymbolRecursiveClassContext):
        result = super().visitSymbolRecursiveClass(ctx)
        symbolClassForm = next(filter(lambda x: isinstance(x, SymbolClass.Form), result), None)
        symbolClassForm.recursive = True

        return symbolClassForm

    def visitSymbolClassListElement(self, ctx: PlangParser.SymbolClassListElementContext):
        result = super().visitSymbolClassListElement(ctx)
        return result

    def visitSymbolClassListElements(self, ctx: PlangParser.SymbolClassListElementsContext):
        result = super().visitSymbolClassListElements(ctx)
        return result

    def visitSymbolUnqualifiedClassList(self, ctx: PlangParser.SymbolUnqualifiedClassListContext):
        result = super().visitSymbolUnqualifiedClassList(ctx)
        return result

    def visitSymbolQualifiedClassList(self, ctx: PlangParser.SymbolQualifiedClassListContext):
        result = super().visitSymbolQualifiedClassList(ctx)
        return result

    def visitSymbolClassList(self, ctx: PlangParser.SymbolClassListContext):
        result = super().visitSymbolClassList(ctx)
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        symbolClassForm = SymbolClass.Form(path=pathForm)
        elementsList = ElementsList([str(x) for x in filter(lambda x: not isinstance(x, Path.Form), result)])
        if elementsList is not None:
            symbolClassForm.elements = elementsList
        return symbolClassForm

    def visitSymbolClassDecl(self, ctx: PlangParser.SymbolClassDeclContext):
        result = super().visitSymbolClassDecl(ctx)
        symbolClassForm = next(filter(lambda x: isinstance(x, SymbolClass.Form), result), None)
        decorationForm = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if symbolClassForm is not None and decorationForm is not None:
            symbolClassForm.path.decoration = decorationForm
        hintList = next(filter(lambda x: isinstance(x, HintList), result), None)
        if hintList is not None:
            symbolClassForm.hints = hintList
        symbolClass = Manager(self.session, self.scope).selectOrInsertSymbolClass(symbolClassForm)
        listedSymbolClasses = []
        for e in symbolClassForm.elements or []:
            symbolClassForm.path.nodes[-1] = str(e)
            listedSymbolClasses.append(Manager(self.session, self.scope).selectOrInsertSymbolClass(symbolClassForm))

        return [symbolClass] + listedSymbolClasses

    def visitSymbolClassRef(self, ctx: PlangParser.SymbolClassRefContext):
        result = super().visitSymbolClassRef(ctx)
        return result

    def visitSymbolName(self, ctx: PlangParser.SymbolNameContext):
        return ctx.IDENTIFIER()

    def visitSymbolOrderedName(self, ctx: PlangParser.SymbolOrderedNameContext):
        result = super().visitSymbolOrderedName(ctx)
        symbolName = next(filter(lambda x: not isinstance(x, Decoratable.Form), result), None)
        decorationForm = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        symbolForm = Symbol.Form(name=str(symbolName), decoration=decorationForm)
        return symbolForm

    def visitSymbol(self, ctx: PlangParser.SymbolContext):
        return super().visitSymbol(ctx)

    def visitSymbolList(self, ctx: PlangParser.SymbolListContext):
        return super().visitSymbolList(ctx)

    def visitSymbolDef(self, ctx: PlangParser.SymbolDefContext):
        return super().visitSymbolDef(ctx)

    def visitSymbolListDef(self, ctx: PlangParser.SymbolListDefContext):
        result = super().visitSymbolListDef(ctx)
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        symbolForms = list(filter(lambda x: isinstance(x, Symbol.Form), result))
        decorationForm = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)

        if pathForm is None:
            pathForm = Path.Form(False, [])

        if decorationForm is not None:
            pathForm.decoration = decorationForm

        for f in symbolForms:
            f.clazz = SymbolClass.Form(pathForm)
        return symbolForms

    def visitSymbolDecl(self, ctx: PlangParser.SymbolDeclContext):
        result = super().visitSymbolDecl(ctx)
        symbolForms = list(filter(lambda x: isinstance(x, Symbol.Form), result))
        symbols = []
        manager = Manager(self.session, self.scope)
        for f in symbolForms:
            symbol = manager.selectOrInsertSymbol(f, True)
            symbols.append(symbol)
        return symbols

    def visitSymbolRef(self, ctx: PlangParser.SymbolRefContext):
        return super().visitSymbolRef(ctx)

    def visitSymbolCompoundRef(self, ctx: PlangParser.SymbolCompoundRefContext):
        return super().visitSymbolCompoundRef(ctx)

    def visitObjectClassName(self, ctx: PlangParser.ObjectClassNameContext):
        return ctx.IDENTIFIER()

    def visitObjectClassInlineRef(self, ctx: PlangParser.ObjectClassInlineRefContext):
        return super().visitObjectClassInlineRef(ctx)

    def visitObjectUnqualifiedClass(self, ctx: PlangParser.ObjectUnqualifiedClassContext):
        return super().visitObjectUnqualifiedClass(ctx)

    def visitObjectQualifiedClass(self, ctx: PlangParser.ObjectQualifiedClassContext):
        return super().visitObjectQualifiedClass(ctx)

    def visitObjectDefaultClass(self, ctx: PlangParser.ObjectDefaultClassContext):
        return super().visitObjectDefaultClass(ctx)

    def visitObjectClass(self, ctx: PlangParser.ObjectClassContext):
        return super().visitObjectClass(ctx)

    def visitObjectClassDef(self, ctx: PlangParser.ObjectClassDefContext):
        return super().visitObjectClassDef(ctx)

    def visitObjectClassDecl(self, ctx: PlangParser.ObjectClassDeclContext):
        return super().visitObjectClassDecl(ctx)

    def visitObjectDefaultClassDecl(self, ctx: PlangParser.ObjectDefaultClassDeclContext):
        return super().visitObjectDefaultClassDecl(ctx)

    def visitObjectInlineClassDecl(self, ctx: PlangParser.ObjectInlineClassDeclContext):
        return super().visitObjectInlineClassDecl(ctx)

    def visitObjectClassRef(self, ctx: PlangParser.ObjectClassRefContext):
        return super().visitObjectClassRef(ctx)

    def visitObjectSVOElement(self, ctx: PlangParser.ObjectSVOElementContext):
        return super().visitObjectSVOElement(ctx)

    def visitObjectList(self, ctx: PlangParser.ObjectListContext):
        return super().visitObjectList(ctx)

    def visitObjectDef(self, ctx: PlangParser.ObjectDefContext):
        return super().visitObjectDef(ctx)

    def visitObjectSVOCausalElement(self, ctx: PlangParser.ObjectSVOCausalElementContext):
        return super().visitObjectSVOCausalElement(ctx)

    def visitObjectCausalList(self, ctx: PlangParser.ObjectCausalListContext):
        return super().visitObjectCausalList(ctx)

    def visitObjectCausalDef(self, ctx: PlangParser.ObjectCausalDefContext):
        return super().visitObjectCausalDef(ctx)

    def visitObjectDefaultDecl(self, ctx: PlangParser.ObjectDefaultDeclContext):
        return super().visitObjectDefaultDecl(ctx)

    def visitObjectInlineDecl(self, ctx: PlangParser.ObjectInlineDeclContext):
        return super().visitObjectInlineDecl(ctx)

    def visitObjectCausalDefaultDecl(self, ctx: PlangParser.ObjectCausalDefaultDeclContext):
        return super().visitObjectCausalDefaultDecl(ctx)

    def visitObjectCausalInlineDecl(self, ctx: PlangParser.ObjectCausalInlineDeclContext):
        return super().visitObjectCausalInlineDecl(ctx)

    def visitPointClassName(self, ctx: PlangParser.PointClassNameContext):
        return super().visitPointClassName(ctx)

    def visitPointRecursiveClassName(self, ctx: PlangParser.PointRecursiveClassNameContext):
        return super().visitPointRecursiveClassName(ctx)

    def visitPointClassSVODecl(self, ctx: PlangParser.PointClassSVODeclContext):
        return super().visitPointClassSVODecl(ctx)

    def visitPointClassRef(self, ctx: PlangParser.PointClassRefContext):
        return super().visitPointClassRef(ctx)

    def visitPointClassSymbolRef(self, ctx: PlangParser.PointClassSymbolRefContext):
        return super().visitPointClassSymbolRef(ctx)

    def visitPointCoreSVODef(self, ctx: PlangParser.PointCoreSVODefContext):
        return super().visitPointCoreSVODef(ctx)

    def visitPointRequirementSVODef(self, ctx: PlangParser.PointRequirementSVODefContext):
        return super().visitPointRequirementSVODef(ctx)

    def visitPointImplicationSVODef(self, ctx: PlangParser.PointImplicationSVODefContext):
        return super().visitPointImplicationSVODef(ctx)

    def visitPointSVODecl(self, ctx: PlangParser.PointSVODeclContext):
        return super().visitPointSVODecl(ctx)

    def visitPointSVORef(self, ctx: PlangParser.PointSVORefContext):
        return super().visitPointSVORef(ctx)

    def visitCausalName(self, ctx: PlangParser.CausalNameContext):
        return ctx.IDENTIFIER()

    def visitCausalSymbolRef(self, ctx: PlangParser.CausalSymbolRefContext):
        return super().visitCausalSymbolRef(ctx)

    def visitCausalDef(self, ctx: PlangParser.CausalDefContext):
        return super().visitCausalDef(ctx)

    def visitCausalIndirectDef(self, ctx: PlangParser.CausalIndirectDefContext):
        return super().visitCausalIndirectDef(ctx)

    def visitCausalConcreteDef(self, ctx: PlangParser.CausalConcreteDefContext):
        return super().visitCausalConcreteDef(ctx)

    def visitCausalRequirementDef(self, ctx: PlangParser.CausalRequirementDefContext):
        return super().visitCausalRequirementDef(ctx)

    def visitCausalImplicationDef(self, ctx: PlangParser.CausalImplicationDefContext):
        return super().visitCausalImplicationDef(ctx)

    def visitContext(self, ctx: PlangParser.ContextContext):
        result = super().visitContext(ctx)
        path = next(filter(lambda x: isinstance(x, Path), result), None)
        if path is None:
            return None
        scope = self.scope.getScope(path)
        return scope
