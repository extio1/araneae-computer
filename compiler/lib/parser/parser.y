%{
#include "araneae/ast/Ast.h"
#include <stdio.h>

// extern to suppress warnings
extern int yylex();
extern void yyerror(char *);
%}

%union {
  const char* strval;
  ASTNode *node;
  enum ASTBinOp binOp;
  enum ASTUnOp unOp;
}

// Symbols
%token SYM_LPAREN_CIRCLE SYM_RPAREN_CIRCLE 
%token SYM_LPAREN_SQUARE SYM_RPAREN_SQUARE
%token SYM_COLON
%token SYM_COMMA
%token SYM_DOT
%token SYM_SEMICOLON
%right SYM_ASSIGN
%left SYM_OR
%left SYM_AND
%left SYM_EQ SYM_NEQ
%left SYM_GT SYM_GTE SYM_LT SYM_LTE
%left SYM_SHIFT_L SYM_SHIFT_R
%left SYM_PLUS SYM_MINUS
%left SYM_MUL SYM_DIV SYM_PERCENT
%left SYM_AMPERSAND SYM_VSPLASH
%right SYM_INC SYM_DEC
%right SYM_NOT SYM_TILDA
%right UNARY_OP
// Literals
%token <strval> LIT_CHAR
%token <strval> LIT_STR
%token <strval> LIT_BOOL
%token <strval> LIT_HEX
%token <strval> LIT_BITS
%token <strval> LIT_DEC
// Type names
%token <strval> TYPE_BOOL
%token <strval> TYPE_BYTE
%token <strval> TYPE_INT
%token <strval> TYPE_UINT
%token <strval> TYPE_LONG
%token <strval> TYPE_ULONG
%token <strval> TYPE_CHAR
%token <strval> TYPE_STRING
// Keywords
%token KW_IF
%token KW_THEN
%token KW_ELSE
%token KW_BEGIN
%token KW_END
%token KW_WHILE
%token KW_DO
%token KW_REPEAT
%token KW_UNTIL
%token KW_BREAK
%token KW_OF
%token KW_IN
%token KW_FROM
%token KW_ARRAY
%token KW_METHOD
%token KW_CLASS
%token KW_VAR
%token KW_RET
%token KW_PUBLIC
%token KW_PRIVATE
// Value place
%token <strval> IDENTIFIER

%nterm <node> program
%nterm <node> programItem
%nterm <node> programItemFuncDef

%nterm <node> programItemClassDef
%nterm <node> programItemClassDefMemberList
%nterm <node> programItemClassDefMember

%nterm <node> importSpec

%nterm <node> typeRef
%nterm <node> typeRefBultin
%nterm <node> typeRefArray

%nterm <node> funcBody
%nterm <node> funcBodyProlog
%nterm <node> funcBodyPrologList
%nterm <node> funcSignature
%nterm <node> funcArgumentList
%nterm <node> funcArgumentListItem
%nterm <node> funcBodyPrologListItem

%nterm <node> statement
%nterm <node> statementIf
%nterm <node> statementBlock
%nterm <node> statementWhile
%nterm <node> statementDo
%nterm <node> statementBreak
%nterm <node> statementReturn
%nterm <node> statementExpr
%nterm <node> statementList

%nterm <node> expression
%nterm <node> assignmentExpr
%nterm <node> logicalOrExpr
%nterm <node> logicalAndExpr
%nterm <node> equalityExpr
%nterm <node> relationalExpr
%nterm <node> shiftExpr
%nterm <node> additiveExpr
%nterm <node> multiplicativeExpr
%nterm <node> unaryExpr
%nterm <node> primaryExpr
%nterm <node> expressionIndexer
%nterm <node> expressionCall 
%nterm <node> expressionValuePlace
%nterm <node> expressionLiteral
%nterm <node> integerLiteral
%nterm <node> otherLiteral
%nterm <node> expressionList
%nterm <node> expressionCreateArray

%nterm <node> callee

%nterm <node> dimensionList
%nterm <node> identifierList
%nterm <node> sizesList

%nterm <unOp> unOp

%%
program: 
                        { $$ = createAstProgramEmpty(); }
  | program programItem { $$ = addChild($1, $2); };

programItem: 
    programItemFuncDef { $$ = $1; }
  | programItemClassDef { $$ = $1; }
  | error { $$ = createAstError("error while programItem"); };
programItemFuncDef: 
    KW_METHOD funcSignature funcBody      { $$ = createAstFuncDef($2, $3); }
  | KW_METHOD funcSignature SYM_SEMICOLON { $$ = createAstFuncDecl($2); }
  | KW_METHOD funcSignature importSpec { $$ = createAstFuncImported($2, $3); }
  | error { $$ = createAstError("error while programItemFuncDef"); };

funcBody:
    funcBodyProlog statementBlock { $$ = createAstFuncBody($1, $2); };
funcBodyProlog:
    { $$ = createAstFuncPrologEmpty(); }
  | KW_VAR funcBodyPrologList SYM_SEMICOLON { $$ = createAstFuncProlog($2); };
funcBodyPrologList: // "a, b, c; d, c, e"
    funcBodyPrologListItem                                  { $$ = createAstPrologItemList($1); }
  | funcBodyPrologList SYM_SEMICOLON funcBodyPrologListItem { $$ = addChild($1, $3); };    
funcBodyPrologListItem: // "a, b, c" OR "a, b, c : int"
    identifierList                    { $$ = $1; }
  | identifierList SYM_COLON typeRef  { $$ = createAstTypeVarDecl($1, $3); };
funcSignature:
    IDENTIFIER SYM_LPAREN_CIRCLE funcArgumentList SYM_RPAREN_CIRCLE                   { $$ = createAstMethodSignature($1, $3); }
  | IDENTIFIER SYM_LPAREN_CIRCLE funcArgumentList SYM_RPAREN_CIRCLE SYM_COLON typeRef { $$ = createAstFuncSignature($1, $3, $6); };
funcArgumentList:
                                                    { $$ = createAstFuncArgumentsEmpty(); }
  | funcArgumentListItem                            { $$ = createAstFuncArguments($1); }
  | funcArgumentList SYM_COMMA funcArgumentListItem { $$ = addChild($1, $3); };
funcArgumentListItem:
    expressionValuePlace SYM_COLON typeRef { $$ = createAstTypeVarDecl($1, $3); }
  | typeRef                      { $$ = $1; };

programItemClassDef:
    KW_CLASS IDENTIFIER funcBodyProlog KW_BEGIN programItemClassDefMemberList KW_END SYM_SEMICOLON { $$ = createAstClassDef($2, NULL, $3, $5); }
  | KW_CLASS IDENTIFIER SYM_COLON IDENTIFIER funcBodyProlog KW_BEGIN programItemClassDefMemberList KW_END SYM_SEMICOLON { $$ = createAstClassDef($2, $4, $5, $7); };
programItemClassDefMemberList:
    { $$ = createAstClassMemberListEmpty(); }
  | programItemClassDefMember { $$ = createAstClassMemberList($1); }
  | programItemClassDefMemberList programItemClassDefMember { $$ = addChild($1, $2); };
programItemClassDefMember:
    programItemFuncDef { $$ = createAstClassMemberDef(ASTMemberVisibilityPrivate, $1); }
  | KW_PUBLIC programItemFuncDef { $$ = createAstClassMemberDef(ASTMemberVisibilityPublic, $2); }
  | KW_PRIVATE programItemFuncDef { $$ = createAstClassMemberDef(ASTMemberVisibilityPrivate, $2); };

importSpec:
    KW_FROM IDENTIFIER SYM_SEMICOLON { $$ = createAstImportSpec(NULL, $2); }
  | KW_FROM IDENTIFIER KW_IN IDENTIFIER SYM_SEMICOLON { $$ = createAstImportSpec($2, $4); };

statement:
    statementIf     { $$ = $1; }
  | statementBlock  { $$ = $1; }
  | statementWhile  { $$ = $1; }
  | statementDo     { $$ = $1; }
  | statementBreak  { $$ = $1; }
  | statementReturn { $$ = $1; }
  | statementExpr   { $$ = $1; };
statementIf:
    KW_IF statementExpr KW_THEN statementBlock                   { $$ = createAstStatementIf($2, $4); }
  | KW_IF statementExpr KW_THEN statementBlock KW_ELSE statementBlock { $$ = createAstStatementIfElse($2, $4, $6); };
statementBlock:
    KW_BEGIN statementList KW_END SYM_SEMICOLON { $$ = createAstStatementBlock($2); };
statementWhile:
    KW_WHILE statementExpr KW_DO statementBlock { $$ = createAstStatementWhile($2, $4); };
statementDo:
    KW_REPEAT statementBlock KW_WHILE statementExpr { $$ = createAstStatementDoWhile($2, $4); }
  | KW_REPEAT statementBlock KW_UNTIL statementExpr { $$ = createAstStatementDoUntil($2, $4); };
statementBreak:
    KW_BREAK SYM_SEMICOLON { $$ = createAstStatementBreak(); };
statementReturn:
    KW_RET SYM_SEMICOLON { $$ = createAstStatementRetVoid(); }
  | KW_RET statementExpr { $$ = createAstStatementRet($2); };
statementExpr:
    expression SYM_SEMICOLON { $$ = createAstStatementExpr($1); };
statementList:
    statement                { $$ = createAstStatementList($1); }
  | statementList statement  { $$ = addChild($1, $2); };

expression:
    assignmentExpr { $$ = $1; };
assignmentExpr:
    logicalOrExpr { $$ = $1; }
  | expressionValuePlace SYM_ASSIGN assignmentExpr { $$ = createAstExpressionAssign($1, $3); }
  | expressionIndexer SYM_ASSIGN assignmentExpr { $$ = createAstExpressionAssign($1, $3); };
logicalOrExpr:
    logicalAndExpr { $$ = $1; }
  | logicalOrExpr SYM_OR logicalAndExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicOr, $3); };
logicalAndExpr:
    equalityExpr { $$ = $1; }
  | logicalAndExpr SYM_AND equalityExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicAnd, $3); };
equalityExpr:
    relationalExpr { $$ = $1; }
  | equalityExpr SYM_EQ relationalExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicEq, $3); }
  | equalityExpr SYM_NEQ relationalExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicNeq, $3); };
relationalExpr:
    shiftExpr { $$ = $1; }
  | relationalExpr SYM_LT shiftExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicLt, $3); }
  | relationalExpr SYM_GT shiftExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicGt, $3); }
  | relationalExpr SYM_LTE shiftExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicLte, $3); }
  | relationalExpr SYM_GTE shiftExpr { $$ = createAstExpressionBinary($1, ASTBinOpLogicGte, $3); };
shiftExpr:
    additiveExpr { $$ = $1; }
  | shiftExpr SYM_SHIFT_L additiveExpr { $$ = createAstExpressionBinary($1, ASTBinOpBinShl, $3); }
  | shiftExpr SYM_SHIFT_R additiveExpr { $$ = createAstExpressionBinary($1, ASTBinOpBinShr, $3); };
additiveExpr:
    multiplicativeExpr { $$ = $1; }
  | additiveExpr SYM_PLUS multiplicativeExpr { $$ = createAstExpressionBinary($1, ASTBinOpArithSum, $3); }
  | additiveExpr SYM_MINUS multiplicativeExpr { $$ = createAstExpressionBinary($1, ASTBinOpArithSub, $3); };
multiplicativeExpr:
    unaryExpr { $$ = $1; }
  | multiplicativeExpr SYM_MUL unaryExpr { $$ = createAstExpressionBinary($1, ASTBinOpArithMul, $3); }
  | multiplicativeExpr SYM_DIV unaryExpr { $$ = createAstExpressionBinary($1, ASTBinOpArithDiv, $3); }
  | multiplicativeExpr SYM_PERCENT unaryExpr { $$ = createAstExpressionBinary($1, ASTBinOpArithPercent, $3); };
unaryExpr:
    primaryExpr { $$ = $1; }
  | unOp unaryExpr %prec UNARY_OP { $$ = createAstExpressionUnary($1, $2); };
primaryExpr:
    expressionCall { $$ = $1; }
  | expressionIndexer { $$ = $1; }
  | expressionLiteral { $$ = $1; }
  | expressionValuePlace { $$ = $1; }
  | expressionCreateArray { $$ = $1; }
  | SYM_LPAREN_CIRCLE expression SYM_RPAREN_CIRCLE { $$ = $2; };
expressionIndexer:
    expressionValuePlace SYM_LPAREN_SQUARE expressionList SYM_RPAREN_SQUARE { $$ = createAstExpressionIndexer($1, $3); };
expressionCall: 
    callee SYM_LPAREN_CIRCLE expressionList SYM_RPAREN_CIRCLE { $$ = createAstExpressionCall($1, $3); };
expressionCreateArray:
    KW_ARRAY dimensionList { $$ = createAstCreateArray($2); };  
expressionValuePlace:
    IDENTIFIER { $$ = createAstExpressionValuePlace(PlaceTypeFree, NULL, $1); }
  | IDENTIFIER SYM_DOT IDENTIFIER { $$ = createAstExpressionValuePlace(PlaceTypeClass, $1, $3); };
expressionLiteral:
    integerLiteral { $$ = $1; }
  | otherLiteral { $$ = $1; };
callee:
    IDENTIFIER { $$ = createAstCallee(CalleeTypeFree, $1, NULL); }
  | IDENTIFIER SYM_DOT IDENTIFIER { $$ = createAstCallee(CalleeTypeClass, $3, $1); }
  
integerLiteral:
    LIT_DEC   { $$ = createAstExpressionLiteralDec($1); }
  | LIT_HEX   { $$ = createAstExpressionLiteralHex($1); }
  | LIT_BITS  { $$ = createAstExpressionLiteralBits($1); };
otherLiteral:
    LIT_STR   { $$ = createAstExpressionLiteralStr($1); } 
  | LIT_CHAR  { $$ = createAstExpressionLiteralChar($1); } 
  | LIT_BOOL  { $$ = createAstExpressionLiteralBool($1); };
expressionList:
                                        { $$ = createAstExpressionListEmpty(); }
  | expression                          { $$ = createAstExpressionList($1); }
  | expressionList SYM_COMMA expression { $$ = addChild($1, $3); };

unOp:
    SYM_INC { $$ = ASTUnOpArithInc; } | SYM_DEC { $$ = ASTUnOpArithDec; } | SYM_TILDA { $$ = ASTUnOpBinNot; } | SYM_MINUS { $$ = ASTUnOpArithMinus; } | SYM_NOT { $$  = ASTUnOpLogicNot; };

typeRef:
    typeRefBultin { $$ = $1; }
  | IDENTIFIER    { $$ = createAstTypeRefUser($1); }
  | typeRefArray  { $$ = $1; };
typeRefBultin:
    TYPE_BOOL   { $$ = createAstTypeRefBuiltinBool(); }
  | TYPE_BYTE   { $$ = createAstTypeRefBuiltinByte(); } 
  | TYPE_INT    { $$ = createAstTypeRefBuiltinInt(); } 
  | TYPE_UINT   { $$ = createAstTypeRefBuiltinUint(); } 
  | TYPE_LONG   { $$ = createAstTypeRefBuiltinLong(); } 
  | TYPE_ULONG  { $$ = createAstTypeRefBuiltinUlong(); } 
  | TYPE_CHAR   { $$ = createAstTypeRefBuiltinChar(); } 
  | TYPE_STRING { $$ = createAstTypeRefBuiltinString(); }
  | error { $$ = createAstError("error while typeRefBultin"); };
typeRefArray:
    KW_ARRAY dimensionList KW_OF typeRef { $$ = createAstTypeRefArray($2, $4); };
dimensionList:
    SYM_LPAREN_SQUARE sizesList SYM_RPAREN_SQUARE { $$ = $2; };
sizesList:
                                            { $$ = createAstSizeListEmpty(); }
  | integerLiteral                          { $$ = createAstSizeList($1); }
  | sizesList SYM_COMMA integerLiteral      { $$ = addChild($1, $3); };

identifierList:
                                                  { $$ = createAstIdentifierListEmpty(); }
  | expressionValuePlace                          { $$ = createAstIdentifierList($1); }
  | identifierList SYM_COMMA expressionValuePlace { $$ = addChild($1, $3); }; 
%%

ASTNode* ProgramRoot = NULL;
