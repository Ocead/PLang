from enum import Enum
from typing import List, Optional

from sqlalchemy.orm import Session

from plang import Decoratable
from plang.cli.scope import Scope
from plang.db import Sourced, Path
from plang.lang.generated import PlangVisitor as BasePlangVisitor, PlangParser
from plang.lang.logic.manager import Manager


class MarkupMode(Enum):
    STRICT = 0
    IMPLICIT = 1


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
        return super().visitDeclSVO(ctx)

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
        return super().visitHintSymbolClass(ctx)

    def visitHintPointClass(self, ctx: PlangParser.HintPointClassContext):
        return super().visitHintPointClass(ctx)

    def visitHint(self, ctx: PlangParser.HintContext):
        return super().visitHint(ctx)

    def visitHintList(self, ctx: PlangParser.HintListContext):
        return super().visitHintList(ctx)

    def visitHintSymbolClassList(self, ctx: PlangParser.HintSymbolClassListContext):
        return super().visitHintSymbolClassList(ctx)

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
        try:
            result = super().visitPathDecl(ctx)
        except Exception as e:
            return None
        pathForm = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        decorationForm = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if pathForm is not None and decorationForm is not None:
            pathForm.decoration = decorationForm
        path = Manager(self.session, self.scope).selectOrInsertPath(pathForm)
        return path

    def visitPathRef(self, ctx: PlangParser.PathRefContext):
        result = super().visitPathRef(ctx)[0]
        return result

    def visitSymbolUnqualifiedClass(self, ctx: PlangParser.SymbolUnqualifiedClassContext):
        return super().visitSymbolUnqualifiedClass(ctx)

    def visitSymbolQualifiedClass(self, ctx: PlangParser.SymbolQualifiedClassContext):
        return super().visitSymbolQualifiedClass(ctx)

    def visitSymbolClass(self, ctx: PlangParser.SymbolClassContext):
        return super().visitSymbolClass(ctx)

    def visitSymbolOrderedClass(self, ctx: PlangParser.SymbolOrderedClassContext):
        return super().visitSymbolOrderedClass(ctx)

    def visitSymbolRecursiveClass(self, ctx: PlangParser.SymbolRecursiveClassContext):
        return super().visitSymbolRecursiveClass(ctx)

    def visitSymbolClassListElement(self, ctx: PlangParser.SymbolClassListElementContext):
        return super().visitSymbolClassListElement(ctx)

    def visitSymbolClassListElements(self, ctx: PlangParser.SymbolClassListElementsContext):
        return super().visitSymbolClassListElements(ctx)

    def visitSymbolUnqualifiedClassList(self, ctx: PlangParser.SymbolUnqualifiedClassListContext):
        return super().visitSymbolUnqualifiedClassList(ctx)

    def visitSymbolQualifiedClassList(self, ctx: PlangParser.SymbolQualifiedClassListContext):
        return super().visitSymbolQualifiedClassList(ctx)

    def visitSymbolClassList(self, ctx: PlangParser.SymbolClassListContext):
        return super().visitSymbolClassList(ctx)

    def visitSymbolClassDecl(self, ctx: PlangParser.SymbolClassDeclContext):
        return super().visitSymbolClassDecl(ctx)

    def visitSymbolClassRef(self, ctx: PlangParser.SymbolClassRefContext):
        return super().visitSymbolClassRef(ctx)

    def visitSymbolName(self, ctx: PlangParser.SymbolNameContext):
        return ctx.IDENTIFIER()

    def visitSymbolOrderedName(self, ctx: PlangParser.SymbolOrderedNameContext):
        return super().visitSymbolOrderedName(ctx)

    def visitSymbol(self, ctx: PlangParser.SymbolContext):
        return super().visitSymbol(ctx)

    def visitSymbolList(self, ctx: PlangParser.SymbolListContext):
        return super().visitSymbolList(ctx)

    def visitSymbolDef(self, ctx: PlangParser.SymbolDefContext):
        return super().visitSymbolDef(ctx)

    def visitSymbolListDef(self, ctx: PlangParser.SymbolListDefContext):
        return super().visitSymbolListDef(ctx)

    def visitSymbolDecl(self, ctx: PlangParser.SymbolDeclContext):
        return super().visitSymbolDecl(ctx)

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
