grammar Plang;

@lexer::postinclude{#include "parser_namespace.hpp"}
@parser::postinclude{#include "parser_namespace.hpp"}

OP_PATH : '.';
OP_RECUR : '...';
OP_LIST : ',';
OP_OBJ : ':';
OP_DECL : ';';
OP_OBJ_NAME : '?';
OP_SINGLE : '!';
OP_NEGATE : '~';
fragment OP_STR_DELIM_SINGLE : '\'';
fragment OP_STR_DELIM_DOUBLE : '"';
OP_STR_L : 'l';
OP_STR_G : 'g';
OP_STR_R : 'r';
OP_STR_M : 'm';
OP_HINT_L : '(';
OP_HINT_R : ')';
OP_SYM_L : '[';
OP_SYM_R : ']';
OP_SYM : '[]';
OP_PNT_L : '{';
OP_PNT_R : '}';
OP_PNT : '{}';
OP_ID : '*';
fragment OP_REQ_ALL : '*';
fragment OP_REQ_ONE : '+';
fragment OP_REQ_UNQ : '~';
fragment OP_CAU_L : '<';
OP_REQ_L : '<' (OP_REQ_ALL | OP_REQ_ONE | OP_REQ_UNQ)?;
OP_IPL_L : '<';
OP_CAU_R : '>';
OP_INDIR : '>>';
OP_REQ_PRV : '<-';
OP_REQ_NOW : '-<';
OP_IPL_DEF : '->';
OP_IPL_NOW : '>-';
OP_ASSIGN : '=';
fragment OP_COMMENT : '//';
fragment OP_COMMENT_L : '/*';
fragment OP_COMMENT_R : '*/';
OP_ESCAPE : '\\';
OP_CONTEXT : '@';

fragment XID_START : [\p{XID_START}];
fragment XID_CONTINUE : [\p{XID_CONTINUE}];
IDENTIFIER : (XID_START | '_') XID_CONTINUE*;

LINE_COMMENT : (OP_COMMENT (~[/!] | OP_COMMENT)~[\n]* | OP_COMMENT) -> skip;

fragment ESC_SINGLE : '\\\'' | '\\\\';
fragment STRING_LITERAL_SINGLE : (ESC_SINGLE | ~[\\'] | '\\'.)*?;
fragment ESC_DOUBLE : '\\"' | '\\\\';
fragment STRING_LITERAL_DOUBLE : (ESC_DOUBLE | ~[\\"] | '\\'.)*?;

WS : [ \n\t\r\p{White_Space}]+;

// Types

INTEGER : [0-9]+;
DECIMAL : ('+' | '-')? (INTEGER? '.' INTEGER | INTEGER '.' INTEGER? | INTEGER);

STRING
    : (OP_STR_DELIM_SINGLE STRING_LITERAL_SINGLE OP_STR_DELIM_SINGLE)
    | (OP_STR_DELIM_DOUBLE STRING_LITERAL_DOUBLE OP_STR_DELIM_DOUBLE);

// TODO: Implement other word orders
start
    : startSVO;

startSVO
    : exprSVO* WS* EOF;

exprSVO
    : WS* declSVO;

declCore
    : (pathDecl
    | symbolClassDecl
    | symbolDecl
    | hintDecl
    | objectClassDecl
    | contextDecl
    | STRING
    | INTEGER);

declSVOCore
    : (declCore
    | pointClassSVODecl
    | pointSVODecl);

declSVO
    : declSVOCore? WS* OP_DECL;

ref
    : (pathRef
    | symbolClassRef
    | symbolRef
    | objectClassRef
    | pointClassRef
    | pointSVORef);

// Hint

hintCommentLiteral
    : STRING;
hintLikeLiteral
    : OP_STR_L STRING;
hintGlobLiteral
    : OP_STR_G STRING;
hintRegexpLiteral
    : OP_STR_R STRING;
hintMatchLiteral
    : OP_STR_M STRING;

hintLiteral
    : hintCommentLiteral
    | hintLikeLiteral
    | hintGlobLiteral
    | hintRegexpLiteral
    | hintMatchLiteral;
hintSymbolClass : symbolClass;
hintPointClass : pointClass;

hint
    : hintLiteral
    | hintSymbolClass
    | hintPointClass;
hintList
    : (OP_HINT_L WS* OP_HINT_R)
    | (OP_HINT_L WS* hint (WS* OP_LIST WS* hint)* WS* OP_HINT_R);

hintSymbolClassList
    : (OP_HINT_L WS* OP_HINT_R)
    | (OP_HINT_L WS* hintSymbolClass (WS* OP_LIST WS* hintSymbolClass)* WS* OP_HINT_R);

hintDecl
    : hintList;

decoration
    : OP_HINT_L WS*
      ((INTEGER | DECIMAL) WS*
      | STRING WS*
      | (INTEGER | DECIMAL) WS* OP_LIST WS* STRING)? WS*
      OP_HINT_R;

// Path

pathUnqualifiedNode
    : IDENTIFIER;
pathQualifiedNode
    : OP_PATH pathUnqualifiedNode?;
pathFollowingNode
    : OP_PATH pathUnqualifiedNode;
pathUnqualifiedPath
    : pathUnqualifiedNode (pathFollowingNode)*;
pathQualifiedPath
    : OP_PATH
    | (pathFollowingNode)+;
path
    : pathQualifiedPath
    | pathUnqualifiedPath;

pathDecl
    : path (WS* decoration)?;
pathRef
    : path;

// Symbol class

symbolUnqualifiedClass
    : pathUnqualifiedPath? OP_SYM;
symbolQualifiedClass
    : pathQualifiedPath OP_SYM;
symbolSimpleClass
    : symbolUnqualifiedClass
    | symbolQualifiedClass;
symbolRecursiveClass
    : symbolSimpleClass OP_RECUR;
symbolClassListElement
    : pathUnqualifiedNode;
symbolClassListElements
    : WS* OP_LIST WS* symbolClassListElement;
symbolUnqualifiedClassList
    : pathUnqualifiedPath symbolClassListElements+ OP_SYM;
symbolQualifiedClassList
    : pathQualifiedPath symbolClassListElements+ OP_SYM;
symbolClassList
    : symbolUnqualifiedClassList
    | symbolQualifiedClassList;
symbolClass
    : symbolSimpleClass
    | symbolClassList
    | symbolRecursiveClass;

symbolClassDecl
    : (hintSymbolClassList WS*)?
      (symbolSimpleClass
    | symbolClassList)
      (WS* decoration)?;
symbolClassRef
    : symbolSimpleClass;

// Symbol

symbolName
    : IDENTIFIER;
symbolDecoratedName
    : symbolName
      (WS* decoration)?;
symbol
    : path? OP_SYM_L symbolName OP_SYM_R;
symbolCompound
    : symbol (WS* symbol)* (WS* symbolClass)?;
symbolList
    : path? OP_SYM_L symbolName WS* (OP_LIST WS* symbolName WS*)+ OP_SYM_R;
symbolDef
    : path? OP_SYM_L symbolDecoratedName OP_SYM_R;
symbolListDef
    : path? OP_SYM_L WS*
      symbolDecoratedName WS*
      (OP_LIST WS* symbolDecoratedName WS*)*
      OP_SYM_R;

symbolDecl
    : (hintSymbolClassList WS*)?
      symbolListDef
      (WS* decoration)?;
symbolRef
    : symbol
    | symbolCompound;
// TODO: Symbol alias ?

// Point object class

objectClassName
    : IDENTIFIER;
objectLocalClassName
    : STRING;
objectClassInlineRef
    : OP_OBJ_NAME
    (objectClassName | objectLocalClassName);
objectUnqualifiedClass
    : pathUnqualifiedPath? objectClassInlineRef;
objectQualifiedClass
    : pathQualifiedPath objectClassInlineRef;
objectDefaultClass
    : objectClassName;
objectInlineClass
    : objectClassInlineRef;
objectClass
    : objectUnqualifiedClass
    | objectQualifiedClass;
objectClassDef
    : OP_OBJ WS* hintList;
objectSingleton
    : OP_SINGLE WS*;

objectClassDecl
    : (objectSingleton)? objectClass (WS* objectClassDef)?;
objectDefaultClassDecl
    : (objectSingleton)? objectDefaultClass (WS* objectClassDef)?;
objectInlineClassDecl
    : (objectSingleton)? objectInlineClass (WS* objectClassDef)?;
objectClassRef
    : objectClass;

// Point object

objectSVOElement
    : STRING
    | symbolCompound
    | pointSVORef;
objectList
    : objectSVOElement
      (WS* OP_LIST WS* objectSVOElement)*;
objectDef
    : OP_OBJ (WS* objectList)?;
objectSVOCausalElement
    : STRING
    | symbolCompound
    | pointSVORef
    | symbolClass
    | objectClass;
objectCausalList
    : objectSVOCausalElement
      (WS* OP_LIST WS* objectSVOCausalElement)*;
objectCausalDef
    : OP_OBJ (WS* objectCausalList)?;
objectDefaultDecl
    : (objectDefaultClass WS*)? objectDef;
objectInlineDecl
    : objectClassInlineRef WS* objectDef;
objectCausalDefaultDecl
    : (objectDefaultClass WS*)? objectCausalDef;
objectCausalInlineDecl
    : objectClassInlineRef WS* objectCausalDef;

// Point class

pointClassName
    : path;
pointSingleton
    : OP_SINGLE WS*;
pointClass
    : pointClassName OP_OBJ_NAME OP_OBJ (OP_RECUR)?;

pointClassSVODecl
    : (hintSymbolClassList WS*)?
      (pointSingleton)?
      pointClassName WS+
      (decoration WS*)?
      (objectDefaultClassDecl | objectInlineClassDecl) WS*
      (objectInlineClassDecl WS*)*
      ((causalRequirementDef | causalImplicationDef) WS*)*;
pointClassRef
    : pointClass;
pointClassSymbolRef
    : pointClassRef OP_SYM;

// Point

// TODO: Allow symbol classes subject or object as subject
pointCoreSVODef
    : (symbolCompound | symbolClass) WS*
      (OP_NEGATE WS*)?
      pointClassName WS*
      (objectDefaultDecl WS*)?
      (objectInlineDecl WS*)*;
pointRequirementSVODef
    : (OP_REQ_PRV | OP_REQ_NOW) WS*
      pointSVORef WS*;
pointImplicationSVODef
    : (OP_IPL_NOW | OP_IPL_DEF) WS*
      pointSVORef WS*;

pointSVODecl
    : pointCoreSVODef
      (pointRequirementSVODef | pointImplicationSVODef)*;
pointSVORef
    : OP_PNT_L
      (pointCoreSVODef | OP_ID (INTEGER | IDENTIFIER))
      OP_PNT_R;

// Causal

causalName
    : IDENTIFIER;
causalSymbolRef
    : symbolCompound
    | symbolClass
    | objectClassRef;

causalDef
    : causalSymbolRef WS*
      (OP_NEGATE WS*)?
      (pointClass) WS*
      (objectCausalDefaultDecl WS*)?
      (objectCausalInlineDecl WS*)*;
causalIndirectDef
    : (causalDef OP_INDIR WS*)?
      causalSymbolRef WS*
      (OP_NEGATE WS*)?
      (pointClass) WS*
      (objectCausalDefaultDecl WS*)?
      (objectCausalInlineDecl WS*)*;
causalConcreteDef
    : causalSymbolRef WS*
      (OP_NEGATE WS*)?
      pointClassName WS*
      (objectCausalDefaultDecl WS*)?
      (objectCausalInlineDecl WS*)*;

// TODO: Indirect requirements
causalRequirementDef
    : (OP_REQ_NOW | OP_REQ_PRV) WS*
      OP_REQ_L WS*
      causalName WS*
      causalIndirectDef WS*
      OP_CAU_R;
causalImplicationDef
    : (OP_IPL_NOW | OP_IPL_DEF) WS*
      OP_IPL_L WS*
      causalName WS*
      causalConcreteDef WS*
      OP_CAU_R;

// Context
context
    : OP_CONTEXT WS*
    (pathRef);

contextDecl
    : context;
