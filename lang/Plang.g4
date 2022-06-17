grammar Plang;

// Lexer

OP_PATH : '.';
OP_RECUR : '...';
OP_LIST : ',';
OP_OBJ : ':';
OP_DECL : ';';
OP_OBJ_NAME : '?';
OP_SINGLE : '!';
OP_NEGATE : '~';
fragment OP_STR_DELIM : '"';
fragment OP_STR_L : 'l';
fragment OP_STR_G : 'g';
fragment OP_STR_R : 'r';
fragment OP_STR_M : 'm';
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
fragment OP_CONTEXT : '@';

fragment XID_START : [\p{XID_START}];
fragment XID_CONTINUE : [\p{XID_CONTINUE}];
IDENTIFIER : (XID_START | '_') XID_CONTINUE*;

LINE_COMMENT : (OP_COMMENT (~[/!] | OP_COMMENT)~[\n]* | OP_COMMENT) -> skip;

fragment ESC : '\\"' | '\\\\';
fragment STRING_LITERAL : (ESC|.)*?;

WS : [ \n\t\r\p{White_Space}]+;

// Types

INTEGER : [0-9]+;
DECIMAL : ('+' | '-')? (INTEGER? '.' INTEGER | INTEGER '.' INTEGER? | INTEGER);

STRING
    : OP_STR_DELIM STRING_LITERAL OP_STR_DELIM;
LSTRING
    : OP_STR_L STRING;
GSTRING
    : OP_STR_G STRING;
RSTRING
    : OP_STR_R STRING;
MSTRING
    : OP_STR_M STRING;

// Parser

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
    | symbolCompoundRef
    | objectClassRef
    | pointClassRef);

// Hint

hintCommentLiteral
    : STRING;
hintLikeLiteral
    : LSTRING;
hintGlobLiteral
    : GSTRING;
hintRegexpLiteral
    : RSTRING;
hintMatchLiteral
    : MSTRING;

hintLiteral
    : hintCommentLiteral
    | hintLikeLiteral
    | hintGlobLiteral
    | hintRegexpLiteral
    | hintMatchLiteral;
hintSymbolClass : symbolClassRef;
hintPointClass : pointClassRef;

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
    : (OP_HINT_L WS*
      ((INTEGER | DECIMAL) WS*
      | STRING WS*
      | (INTEGER | DECIMAL) WS* OP_LIST WS* STRING) WS*
      OP_HINT_R);

// Path

pathUnqualifiedNode
    : IDENTIFIER;
pathQualifiedNode
    : OP_PATH (IDENTIFIER)*;
pathFollowingNode
    : OP_PATH IDENTIFIER;
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
symbolClass
    : symbolUnqualifiedClass
    | symbolQualifiedClass;
symbolOrderedClass
    : symbolClass
      (WS* decoration)?;
symbolRecursiveClass
    : symbolClass OP_RECUR;
symbolClassListElement
    : pathUnqualifiedPath;
symbolClassListElements
    : WS* OP_LIST WS* symbolClassListElement;
symbolUnqualifiedClassList
    : pathUnqualifiedPath symbolClassListElements+ OP_SYM;
symbolQualifiedClassList
    : pathQualifiedPath symbolClassListElements+ OP_SYM;
symbolClassList
    : symbolUnqualifiedClassList
    | symbolQualifiedClassList;

symbolClassDecl
    : (hintSymbolClassList WS*)?
      (symbolOrderedClass
    | symbolClassList);
symbolClassRef
    : symbolClass
    | symbolClassList
    | symbolRecursiveClass;

// Symbol

symbolName
    : IDENTIFIER;
symbolOrderedName
    : symbolName
      (WS* decoration)?;
symbol
    : pathRef? OP_SYM_L symbolName OP_SYM_R;
symbolList
    : pathRef? OP_SYM_L symbolName WS* (OP_LIST WS* symbolName WS*)+ OP_SYM_R;
symbolDef
    : pathRef? OP_SYM_L symbolOrderedName OP_SYM_R;
symbolListDef
    : pathRef? OP_SYM_L WS*
      symbolOrderedName WS*
      (OP_LIST WS* symbolOrderedName WS*)*
      OP_SYM_R;

symbolDecl
    : (hintSymbolClassList WS*)?
      symbolListDef
      (WS* decoration)?;
symbolRef
    : symbol;
symbolCompoundRef
    : symbolRef (WS* symbolRef)* (WS* symbolClassRef)?;
// TODO: Symbol alias ?

// Point object class

objectClassName
    : IDENTIFIER;
objectClassInlineRef
    : OP_OBJ_NAME objectClassName;
objectUnqualifiedClass
    : pathUnqualifiedPath? objectClassInlineRef;
objectQualifiedClass
    : pathQualifiedPath objectClassInlineRef;
objectDefaultClass
    : objectClassName;
objectClass
    : objectUnqualifiedClass
    | objectQualifiedClass;
objectClassDef
    : OP_OBJ (WS* hintList)?;

objectClassDecl
    : objectClass objectClassDef;
objectDefaultClassDecl
    : (OP_SINGLE WS*)? objectDefaultClass WS* objectClassDef?;
objectInlineClassDecl
    : (OP_SINGLE WS*)? objectClassInlineRef WS* objectClassDef?;
objectClassRef
    : objectClass;

// Point object

objectSVOElement
    : STRING
    | symbolCompoundRef
    | pointSVORef;
objectList
    : objectSVOElement
      (WS* OP_LIST WS* objectSVOElement)*;
objectDef
    : OP_OBJ (WS* objectList)?;
objectSVOCausalElement
    : STRING
    | symbolCompoundRef
    | pointSVORef
    | symbolClassRef
    | objectClassRef;
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
    : pathRef;
pointRecursiveClassName
    : pathRef OP_RECUR;

pointClassSVODecl
    : (hintSymbolClassList WS*)?
      (OP_SINGLE WS*)?
      pointClassName WS+
      (decoration WS*)?
      (objectDefaultClassDecl | objectInlineClassDecl) WS*
      (objectInlineClassDecl WS*)*
      ((causalRequirementDef | causalImplicationDef) WS*)*;
pointClassRef
    : pointClassName OP_OBJ_NAME OP_OBJ;
pointClassSymbolRef
    : pointClassRef OP_SYM;

// Point

// TODO: Allow symbol classes subject or object as subject
pointCoreSVODef
    : (symbolCompoundRef | symbolClassRef) WS*
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
    : symbolCompoundRef
    | symbolClassRef
    | objectClassRef;

causalDef
    : causalSymbolRef WS*
      (OP_NEGATE WS*)?
      (pointClassName | pointRecursiveClassName) WS*
      (objectCausalDefaultDecl WS*)?
      (objectCausalInlineDecl WS*)*;
causalIndirectDef
    : (causalDef OP_INDIR WS*)?
      causalSymbolRef WS*
      (OP_NEGATE WS*)?
      (pointClassName | pointRecursiveClassName) WS*
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
