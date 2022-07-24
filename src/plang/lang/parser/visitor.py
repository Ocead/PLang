from copy import deepcopy
from enum import Enum
from typing import List, Optional

from sqlalchemy.orm import Session

from plang.db.base import Decoratable
from plang.db.plot.object.models import ObjectClass, ObjectClassHintLit
from plang.db.plot.point.models import PointClass
from plang.db.plot.symbol.models import SymbolClass, Symbol
from plang.cli.scope import Scope
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

    def aggregateResult(self, aggregate, next_result):
        if aggregate is None:
            result_list = []
        elif not isinstance(aggregate, List):
            result_list = [aggregate]
        else:
            result_list = aggregate

        if next_result is None:
            last = []
        elif not isinstance(next_result, List):
            last = [next_result]
        else:
            last = next_result

        result = result_list + last
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
        result = ctx.STRING()
        return ObjectClassHintLit.Form(ObjectClassHintLit.Type.COMMENT, str(result)[1:-1])

    def visitHintLikeLiteral(self, ctx: PlangParser.HintLikeLiteralContext):
        result = super().visitHintLikeLiteral(ctx)
        hint = result[0]
        hint.type = ObjectClassHintLit.Type.LIKE
        return hint

    def visitHintGlobLiteral(self, ctx: PlangParser.HintGlobLiteralContext):
        result = super().visitHintGlobLiteral(ctx)
        hint = result[0]
        hint.type = ObjectClassHintLit.Type.GLOB
        return hint

    def visitHintRegexpLiteral(self, ctx: PlangParser.HintRegexpLiteralContext):
        result = super().visitHintRegexpLiteral(ctx)
        hint = result[0]
        hint.type = ObjectClassHintLit.Type.REGEX
        return hint

    def visitHintMatchLiteral(self, ctx: PlangParser.HintMatchLiteralContext):
        result = super().visitHintMatchLiteral(ctx)
        hint = result[0]
        hint.type = ObjectClassHintLit.Type.MATCH
        return hint

    def visitHintLiteral(self, ctx: PlangParser.HintLiteralContext):
        return super().visitHintLiteral(ctx)

    def visitHintSymbolClass(self, ctx: PlangParser.HintSymbolClassContext):
        result = super().visitHintSymbolClass(ctx)
        return result[0]

    def visitHintPointClass(self, ctx: PlangParser.HintPointClassContext):
        result = super().visitHintPointClass(ctx)
        return result[0]

    def visitHint(self, ctx: PlangParser.HintContext):
        return super().visitHint(ctx)

    def visitHintList(self, ctx: PlangParser.HintListContext):
        return super().visitHintList(ctx)

    def visitHintSymbolClassList(self, ctx: PlangParser.HintSymbolClassListContext):
        result = super().visitHintSymbolClassList(ctx)
        flat = []
        for r in result:
            flat.append(r)
            if r.elements is not None:
                for e in r.elements:
                    form = deepcopy(r)
                    form.path.nodes[-1] = e
                    flat.append(form)
        return HintList(flat)

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
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        decoration_form = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if path_form is not None and decoration_form is not None:
            path_form.decoration = decoration_form
        path = Manager(self.session, self.scope).selectOrInsertPath(path_form)
        return path

    def visitPathRef(self, ctx: PlangParser.PathRefContext):
        result = super().visitPathRef(ctx)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        if path_form is None:
            return None
        path = Manager(self.session, self.scope).selectPath(path_form)
        return path

    def visitSymbolUnqualifiedClass(self, ctx: PlangParser.SymbolUnqualifiedClassContext):
        result = super().visitSymbolUnqualifiedClass(ctx)
        if result is None or len(result) == 0:
            result = Path.Form(False, [])
        return result

    def visitSymbolQualifiedClass(self, ctx: PlangParser.SymbolQualifiedClassContext):
        return super().visitSymbolQualifiedClass(ctx)

    def visitSymbolSimpleClass(self, ctx: PlangParser.SymbolSimpleClassContext):
        result = super().visitSymbolSimpleClass(ctx)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        if path_form is None:
            return None
        symbol_class_form = SymbolClass.Form(path=path_form)
        return symbol_class_form

    def visitSymbolRecursiveClass(self, ctx: PlangParser.SymbolRecursiveClassContext):
        result = super().visitSymbolRecursiveClass(ctx)
        symbol_class_form = next(filter(lambda x: isinstance(x, SymbolClass.Form), result), None)
        symbol_class_form.recursive = True

        return symbol_class_form

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
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        symbol_class_form = SymbolClass.Form(path=path_form)
        elements_list = ElementsList([str(x) for x in filter(lambda x: not isinstance(x, Path.Form), result)])
        if elements_list is not None:
            symbol_class_form.elements = elements_list
        return symbol_class_form

    def visitSymbolClass(self, ctx: PlangParser.SymbolClassContext):
        return super().visitSymbolClass(ctx)

    def visitSymbolClassDecl(self, ctx: PlangParser.SymbolClassDeclContext):
        result = super().visitSymbolClassDecl(ctx)
        symbol_class_form = next(filter(lambda x: isinstance(x, SymbolClass.Form), result), None)
        decoration_form = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if symbol_class_form is not None and decoration_form is not None:
            symbol_class_form.path.decoration = decoration_form
        hint_list = next(filter(lambda x: isinstance(x, HintList), result), None)
        if hint_list is not None:
            symbol_class_form.hints = hint_list
        symbol_class = Manager(self.session, self.scope).selectOrInsertSymbolClass(symbol_class_form)
        listed_symbol_classes = []
        for e in symbol_class_form.elements or []:
            symbol_class_form.path.nodes[-1] = str(e)
            listed_symbol_classes.append(Manager(self.session, self.scope).selectOrInsertSymbolClass(symbol_class_form))

        return [symbol_class] + listed_symbol_classes

    def visitSymbolClassRef(self, ctx: PlangParser.SymbolClassRefContext):
        result = super().visitSymbolClassRef(ctx)
        symbol_class_form = next(filter(lambda x: isinstance(x, SymbolClass.Form), result), None)
        symbol_class = Manager(self.session, self.scope).selectSymbolClass(symbol_class_form)
        return symbol_class

    def visitSymbolName(self, ctx: PlangParser.SymbolNameContext):
        return ctx.IDENTIFIER()

    def visitSymbolDecoratedName(self, ctx: PlangParser.SymbolDecoratedNameContext):
        result = super().visitSymbolDecoratedName(ctx)
        symbol_name = next(filter(lambda x: not isinstance(x, Decoratable.Form), result), None)
        decoration_form = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        symbol_form = Symbol.Form(name=str(symbol_name), decoration=decoration_form)
        return symbol_form

    def visitSymbol(self, ctx: PlangParser.SymbolContext):
        result = super().visitSymbol(ctx)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        symbol_name = next(filter(lambda x: not isinstance(x, Path.Form), result), None)

        symbol_class_form = SymbolClass.Form(path_form or Path.Form(False, []))
        symbol_form = Symbol.Form(symbol_name, symbol_class_form)
        return symbol_form

    def visitSymbolCompound(self, ctx: PlangParser.SymbolCompoundContext):
        return super().visitSymbolCompound(ctx)

    def visitSymbolList(self, ctx: PlangParser.SymbolListContext):
        return super().visitSymbolList(ctx)

    def visitSymbolDef(self, ctx: PlangParser.SymbolDefContext):
        return super().visitSymbolDef(ctx)

    def visitSymbolListDef(self, ctx: PlangParser.SymbolListDefContext):
        result = super().visitSymbolListDef(ctx)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        symbol_forms = list(filter(lambda x: isinstance(x, Symbol.Form), result))
        decoration_form = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)

        if path_form is None:
            path_form = Path.Form(False, [])

        if decoration_form is not None:
            path_form.decoration = decoration_form

        for f in symbol_forms:
            f.clazz = SymbolClass.Form(path_form)
        return symbol_forms

    def visitSymbolDecl(self, ctx: PlangParser.SymbolDeclContext):
        result = super().visitSymbolDecl(ctx)
        symbol_forms = list(filter(lambda x: isinstance(x, Symbol.Form), result))
        symbols = []
        manager = Manager(self.session, self.scope)
        for f in symbol_forms:
            symbol = manager.selectOrInsertSymbol(f, True)
            symbols.append(symbol)
        return symbols

    def visitSymbolRef(self, ctx: PlangParser.SymbolRefContext):
        result = super().visitSymbolRef(ctx)
        symbol_form = next(filter(lambda x: isinstance(x, Symbol.Form), result))
        symbol = Manager(self.session, self.scope).selectSymbol(symbol_form)
        return symbol

    def visitObjectClassName(self, ctx: PlangParser.ObjectClassNameContext):
        return ctx.IDENTIFIER()

    def visitObjectClassInlineRef(self, ctx: PlangParser.ObjectClassInlineRefContext):
        return super().visitObjectClassInlineRef(ctx)

    def visitObjectUnqualifiedClass(self, ctx: PlangParser.ObjectUnqualifiedClassContext):
        return super().visitObjectUnqualifiedClass(ctx)

    def visitObjectQualifiedClass(self, ctx: PlangParser.ObjectQualifiedClassContext):
        return super().visitObjectQualifiedClass(ctx)

    def visitObjectDefaultClass(self, ctx: PlangParser.ObjectDefaultClassContext):
        result = super().visitObjectDefaultClass(ctx)
        object_class_form = ObjectClass.Form(result[0], Path.Form(False, list()), False, True)
        return object_class_form

    def visitObjectInlineClass(self, ctx: PlangParser.ObjectInlineClassContext):
        result = super().visitObjectInlineClass(ctx)
        object_class_form = ObjectClass.Form(result[0], Path.Form(False, list()), False, False)
        return object_class_form

    def visitObjectClass(self, ctx: PlangParser.ObjectClassContext):
        result = super().visitObjectClass(ctx)
        object_class_form = ObjectClass.Form(result[0], Path.Form(False, list()), False, False)
        return object_class_form

    def visitObjectClassDef(self, ctx: PlangParser.ObjectClassDefContext):
        result = super().visitObjectClassDef(ctx)
        return HintList(result)

    def visitObjectSingleton(self, ctx: PlangParser.ObjectSingletonContext):
        result = super().visitObjectSingleton(ctx)
        return True

    def visitObjectClassDecl(self, ctx: PlangParser.ObjectClassDeclContext):
        return super().visitObjectClassDecl(ctx)

    def visitObjectDefaultClassDecl(self, ctx: PlangParser.ObjectDefaultClassDeclContext):
        result = super().visitObjectDefaultClassDecl(ctx)
        singleton = next(filter(lambda x: isinstance(x, bool), result), None)
        object_class_form = next(filter(lambda x: isinstance(x, ObjectClass.Form), result), None)
        hint_list = next(filter(lambda x: isinstance(x, HintList), result), None)

        if singleton is not None:
            object_class_form.singleton = singleton
        if hint_list is not None:
            object_class_form.hint_list = hint_list
        return object_class_form

    def visitObjectInlineClassDecl(self, ctx: PlangParser.ObjectInlineClassDeclContext):
        result = super().visitObjectInlineClassDecl(ctx)
        singleton = next(filter(lambda x: isinstance(x, bool), result), None)
        object_class_form = next(filter(lambda x: isinstance(x, ObjectClass.Form), result), None)
        hint_list = next(filter(lambda x: isinstance(x, HintList), result), None)

        if singleton is not None:
            object_class_form.singleton = singleton
        if hint_list is not None:
            object_class_form.hint_list = hint_list
        return object_class_form

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
        result = super().visitPointClassName(ctx)
        return result

    def visitPointSingleton(self, ctx: PlangParser.PointSingletonContext):
        result = super().visitPointSingleton(ctx)
        return True

    def visitPointClass(self, ctx: PlangParser.PointClassContext):
        result = super().visitPointClass(ctx)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        recursive = ctx.OP_RECUR() is not None

        point_class_form = PointClass.Form(path_form, False, list(), list(), recursive)
        return point_class_form

    def visitPointClassSVODecl(self, ctx: PlangParser.PointClassSVODeclContext):
        result = super().visitPointClassSVODecl(ctx)
        hint_list = next(filter(lambda x: isinstance(x, HintList), result), None)
        singleton = next(filter(lambda x: isinstance(x, bool), result), None)
        path_form = next(filter(lambda x: isinstance(x, Path.Form), result), None)
        decoration_form = next(filter(lambda x: isinstance(x, Decoratable.Form), result), None)
        if decoration_form is not None:
            path_form.decoration = decoration_form
        object_class_forms = list(filter(lambda x: isinstance(x, ObjectClass.Form), result))
        # TODO: causal

        point_class_form = PointClass.Form(path_form, singleton or False, object_class_forms, hint_list or list())
        point_class = Manager(self.session, self.scope).insertPointClass(point_class_form)

        return point_class

    def visitPointClassRef(self, ctx: PlangParser.PointClassRefContext):
        result = super().visitPointClassRef(ctx)
        point_class_form = next(filter(lambda x: isinstance(x, PointClass.Form), result), None)
        point_class = Manager(self.session, self.scope).selectPointClass(point_class_form)
        return point_class

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

    def visitContextDecl(self, ctx: PlangParser.ContextDeclContext):
        return super().visitContextDecl(ctx)
