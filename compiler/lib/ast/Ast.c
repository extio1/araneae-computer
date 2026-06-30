#include "araneae/ast/Ast.h"
#include "araneae/ast/Lexer.h"
#include "araneae/aux/Aux.h"
#include "araneae/error/ErrorCollector.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *addChild(ASTNode *To, ASTNode *NewChild) {
  // TODO: use custom allocator insdead of realloc
  ++To->ChildrenCount;
  To->Children = realloc(To->Children, To->ChildrenCount * sizeof(ASTNode));
  To->Children[To->ChildrenCount - 1] = NewChild;
  return To;
}

#define CREATE_AST_PROLOG(TypeName, ...)                                       \
  ASTNode *N = malloc(sizeof(ASTNode));                                        \
  if (!N)                                                                      \
    errorCollectorAddError(ErrorStageParser, getTokenStartLine(),              \
                           getTokenStartColumn(),                              \
                           "Error while allocating AST node");                 \
  N->Type = TypeName;                                                          \
  do {                                                                         \
    ASTNode *Temp[] = {__VA_ARGS__};                                           \
    N->ChildrenCount = sizeof(Temp) / sizeof(ASTNode *);                       \
    N->Children = malloc(N->ChildrenCount * sizeof(ASTNode *));                \
    if (N->ChildrenCount > 0 && !N->Children)                                  \
      errorCollectorAddError(ErrorStageParser, getTokenStartLine(),            \
                             getTokenStartColumn(),                            \
                             "Error while allocating child AST nodes");        \
    memcpy(N->Children, Temp, N->ChildrenCount * sizeof(ASTNode *));           \
  } while (0)
#define CREATE_AST_EPILOG() return N;

ASTNode *createAstProgramEmpty() {
  assert(ProgramRoot == NULL);
  CREATE_AST_PROLOG(ASTProgram);
  ProgramRoot = N;
  return N;
}

ASTNode *createAstFuncImported(ASTNode *FSignN, ASTNode *ImportSpecN) {
  CREATE_AST_PROLOG(ASTFuncImported, FSignN, ImportSpecN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncDecl(ASTNode *FSignN) {
  CREATE_AST_PROLOG(ASTFuncDecl, FSignN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstClassDef(const char *Name, const char *BaseName, ASTNode *VarDeclsN, ASTNode *MemberListN) {
  CREATE_AST_PROLOG(ASTClassDef, VarDeclsN, MemberListN);
  N->Data.ClassDef.ClassName = strdup(Name);
  N->Data.ClassDef.BaseClassName = BaseName ? strdup(BaseName) : NULL;
  CREATE_AST_EPILOG();
}

ASTNode *createAstMember() {
  CREATE_AST_PROLOG(ASTClassMemberList);
  CREATE_AST_EPILOG();
}
ASTNode *createAstClassMemberListEmpty() {
  CREATE_AST_PROLOG(ASTClassMemberList);
  CREATE_AST_EPILOG();
}
ASTNode *createAstClassMemberList(ASTNode *Node) {
  CREATE_AST_PROLOG(ASTClassMemberList, Node);
  CREATE_AST_EPILOG();
}
ASTNode *createAstClassMemberDef(enum ASTMemberVisibility MemberVisibility, ASTNode *FuncN) {
  CREATE_AST_PROLOG(ASTClassMember, FuncN);
  N->Data.MemberVisibility = MemberVisibility;
  CREATE_AST_EPILOG();
}

ASTNode *createAstImportSpec(const char* EntryName, const char *DllName) {
  CREATE_AST_PROLOG(ASTImportSpec);
  N->Data.ImportSpec.DllName = DllName ? strdup(DllName) : NULL;
  N->Data.ImportSpec.EntryName = EntryName ? strdup(EntryName) : NULL;
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncDef(ASTNode *FSignN, ASTNode *FBodyN) {
  CREATE_AST_PROLOG(ASTFuncDef, FSignN, FBodyN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstMethodSignature(const char *Name, ASTNode *FArgsN) {
  CREATE_AST_PROLOG(ASTMethSignature, FArgsN);
  N->Data.MethodSignature.MethodName = strdup(Name);
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncSignature(const char *Name, ASTNode *FArgsN,
                                ASTNode *RetTypeN) {
  CREATE_AST_PROLOG(ASTFuncSignature, FArgsN, RetTypeN);
  N->Data.FunctionSignature.FunctionName = strdup(Name);
  CREATE_AST_EPILOG();
}

ASTNode *createAstCallee(enum CalleeType Type, const char *MethodName, const char *ObjectName) {
  CREATE_AST_PROLOG(ASTCallee);
  N->Data.Callee.Type = Type;
  N->Data.Callee.MethodName = strdup(MethodName);
  if (Type == CalleeTypeClass)
    N->Data.Callee.ObjName = strdup(ObjectName);
  else 
    N->Data.Callee.ObjName = NULL;
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncBody(ASTNode *FPrologF, ASTNode *FBodyN) {
  CREATE_AST_PROLOG(ASTFuncBody, FPrologF, FBodyN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncPrologEmpty() {
  CREATE_AST_PROLOG(ASTFuncBodyProlog);
  CREATE_AST_EPILOG();
}
ASTNode *createAstFuncProlog(ASTNode *VarDeclsN) {
  CREATE_AST_PROLOG(ASTFuncBodyProlog, VarDeclsN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstPrologItemList(ASTNode *PrologItemN) {
  CREATE_AST_PROLOG(ASTFuncBodyPrologList, PrologItemN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstFuncArgumentsEmpty() {
  CREATE_AST_PROLOG(ASTFuncArguments);
  CREATE_AST_EPILOG();
}
ASTNode *createAstFuncArguments(ASTNode *Node) {
  CREATE_AST_PROLOG(ASTFuncArguments, Node);
  CREATE_AST_EPILOG();
}

ASTNode *createAstTypeRefBuiltinBool() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinBool);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinByte() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinByte);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinInt() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinInt);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinUint() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinUint);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinLong() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinLong);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinUlong() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinUlong);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinChar() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinChar);
  CREATE_AST_EPILOG();
}
ASTNode *createAstTypeRefBuiltinString() {
  CREATE_AST_PROLOG(ASTTypeRefBuiltinString);
  CREATE_AST_EPILOG();
}

ASTNode *createAstTypeRefUser(const char *Name) {
  CREATE_AST_PROLOG(ASTTypeRefUser);
  N->Data.UserType.TypeName = strdup(Name);
  CREATE_AST_EPILOG();
}

ASTNode *createAstTypeVarDecl(ASTNode *VarName, ASTNode *TypeNode) {
  CREATE_AST_PROLOG(ASTTypeVarDecl, VarName, TypeNode);
  CREATE_AST_EPILOG();
}

ASTNode *createAstError(const char *Msg) {
  CREATE_AST_PROLOG(ASTError);
  N->Data.Error.Message = strdup(Msg);
  CREATE_AST_EPILOG();
}

ASTNode *createAstSizeListEmpty() {
  CREATE_AST_PROLOG(ASTSizeList);
  CREATE_AST_EPILOG();
}
ASTNode *createAstSizeList(ASTNode *Node) {
  CREATE_AST_PROLOG(ASTSizeList, Node);
  CREATE_AST_EPILOG();
}
ASTNode *createAstIdentifierListEmpty() {
  CREATE_AST_PROLOG(ASTIdentifierList);
  CREATE_AST_EPILOG();
}
ASTNode *createAstIdentifierList(ASTNode *Node) {
  CREATE_AST_PROLOG(ASTIdentifierList, Node);
  CREATE_AST_EPILOG();
}

ASTNode *createAstTypeRefArray(ASTNode *DimsN, ASTNode *ElementTypeN) {
  CREATE_AST_PROLOG(ASTTypeRefArray, DimsN, ElementTypeN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstStatementIf(ASTNode *CondN, ASTNode *IfTrueN) {
  CREATE_AST_PROLOG(ASTStatementIf, CondN, IfTrueN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementIfElse(ASTNode *CondN, ASTNode *IfTrueN,
                                  ASTNode *IfFalseN) {
  CREATE_AST_PROLOG(ASTStatementIfElse, CondN, IfTrueN, IfFalseN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementBlock(ASTNode *StatementListN) {
  CREATE_AST_PROLOG(ASTStatementBlock, StatementListN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementWhile(ASTNode *CondN, ASTNode *BlockN) {
  CREATE_AST_PROLOG(ASTStatementWhile, CondN, BlockN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementDoWhile(ASTNode *CondN, ASTNode *BlockN) {
  CREATE_AST_PROLOG(ASTStatementDoWhile, CondN, BlockN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementDoUntil(ASTNode *CondN, ASTNode *BlockN) {
  CREATE_AST_PROLOG(ASTStatementDoUntil, CondN, BlockN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementBreak() {
  CREATE_AST_PROLOG(ASTStatementBreak);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementRetVoid() {
  CREATE_AST_PROLOG(ASTStatementReturnVoid);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementRet(ASTNode *RetVal) {
  CREATE_AST_PROLOG(ASTStatementReturn, RetVal);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementExpr(ASTNode *ExprN) {
  CREATE_AST_PROLOG(ASTStatementExpr, ExprN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstStatementList(ASTNode *StatementN) {
  CREATE_AST_PROLOG(ASTStatementList, StatementN);
  CREATE_AST_EPILOG();
}

ASTNode *createAstExpressionAssign(ASTNode *PlaceN, ASTNode *ValueN) {
  CREATE_AST_PROLOG(ASTExpressionAssign, PlaceN, ValueN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionBinary(ASTNode *LHSN, enum ASTBinOp Op,
                                   ASTNode *RHSN) {
  CREATE_AST_PROLOG(ASTExpressionBinary, LHSN, RHSN);
  N->Data.BinOp.OpType = Op;
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionUnary(enum ASTUnOp Op, ASTNode *OpndN) {
  CREATE_AST_PROLOG(ASTExpressionUnary, OpndN);
  N->Data.UnOp.OpType = Op;
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionCall(ASTNode *CalleN, ASTNode *ArgsN) {
  CREATE_AST_PROLOG(ASTExpressionCall, CalleN, ArgsN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionIndexer(ASTNode *IndexedValueN, ASTNode *IndexN) {
  CREATE_AST_PROLOG(ASTExpressionIndexer, IndexedValueN, IndexN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstCreateArray(ASTNode *DimsN) {
  CREATE_AST_PROLOG(ASTExpressionCreateArray, DimsN);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionValuePlace(enum PlaceType Type, const char* Object, const char *Place) {
  CREATE_AST_PROLOG(ASTExpressionValuePlace);
  N->Data.ValuePlace.Type = Type;
  N->Data.ValuePlace.Name = strdup(Place);
  N->Data.ValuePlace.ObjName = Type == PlaceTypeClass ? strdup(Object) : NULL;
  static uint64_t Id = 0;
  N->Data.ValuePlace.Id = Id++; 
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralStr(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralStr);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralChar(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralChar);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralHex(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralHex);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralBits(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralBits);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralDec(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralDec);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionLiteralBool(const char *Initializer) {
  CREATE_AST_PROLOG(ASTExpressionLiteralBool);
  N->Data.Literal.Rawstr = strdup(Initializer);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionListEmpty() {
  CREATE_AST_PROLOG(ASTExpressionList);
  CREATE_AST_EPILOG();
}
ASTNode *createAstExpressionList(ASTNode *ExprN) {
  CREATE_AST_PROLOG(ASTExpressionList, ExprN);
  CREATE_AST_EPILOG();
}

#undef CREATE_AST_PROLOG
#undef CREATE_AST_EPILOG

const char *astTypeToString(enum ASTType Type) {
  switch (Type) {
  case ASTProgram:
    return "ASTProgram";
  case ASTFuncDecl:
    return "ASTFuncDecl";
  case ASTFuncDef:
    return "ASTFuncDef";
  case ASTFuncSignature:
    return "ASTFuncSignature";
  case ASTMethSignature:
    return "ASTMethSignature";
  case ASTFuncArguments:
    return "ASTFuncArguments";
  case ASTFuncBody:
    return "ASTFuncBody";
  case ASTFuncBodyProlog:
    return "ASTFuncBodyProlog";
  case ASTFuncBodyPrologList:
    return "ASTFuncBodyPrologList";
  case ASTPrologItemList:
    return "ASTPrologItemList";
  case ASTTypeRefArray:
    return "ASTTypeRefArray";
  case ASTTypeRefUser:
    return "ASTTypeRefUser";
  case ASTTypeRefBuiltinBool:
    return "ASTTypeRefBuiltinBool";
  case ASTTypeRefBuiltinByte:
    return "ASTTypeRefBuiltinByte";
  case ASTTypeRefBuiltinInt:
    return "ASTTypeRefBuiltinInt";
  case ASTTypeRefBuiltinUint:
    return "ASTTypeRefBuiltinUint";
  case ASTTypeRefBuiltinLong:
    return "ASTTypeRefBuiltinLong";
  case ASTTypeRefBuiltinUlong:
    return "ASTTypeRefBuiltinUlong";
  case ASTTypeRefBuiltinChar:
    return "ASTTypeRefBuiltinChar";
  case ASTTypeRefBuiltinString:
    return "ASTTypeRefBuiltinString";
  case ASTTypeVarDecl:
    return "ASTTypeVarDecl";
  case ASTStatementIf:
    return "ASTStatementIf";
  case ASTStatementIfElse:
    return "ASTStatementIfElse";
  case ASTStatementBlock:
    return "ASTStatementBlock";
  case ASTStatementWhile:
    return "ASTStatementWhile";
  case ASTStatementDoWhile:
    return "ASTStatementDoWhile";
  case ASTStatementDoUntil:
    return "ASTStatementDoUntil";
  case ASTStatementBreak:
    return "ASTStatementBreak";
  case ASTStatementReturn:
    return "ASTStatementReturn";
  case ASTStatementReturnVoid:
    return "ASTStatementReturnVoid";
  case ASTStatementExpr:
    return "ASTStatementExpr";
  case ASTStatementList:
    return "ASTStatementList";
  case ASTExpressionAssign:
    return "ASTExpressionAssign";
  case ASTExpressionBinary:
    return "ASTExpressionBinary";
  case ASTExpressionUnary:
    return "ASTExpressionUnary";
  case ASTExpressionCall:
    return "ASTExpressionCall";
  case ASTExpressionIndexer:
    return "ASTExpressionIndexer";
  case ASTExpressionValuePlace:
    return "ASTExpressionValuePlace";
  case ASTExpressionLiteralStr:
    return "ASTExpressionLiteralStr";
  case ASTExpressionLiteralChar:
    return "ASTExpressionLiteralChar";
  case ASTExpressionLiteralHex:
    return "ASTExpressionLiteralHex";
  case ASTExpressionLiteralBits:
    return "ASTExpressionLiteralBits";
  case ASTExpressionLiteralDec:
    return "ASTExpressionLiteralDec";
  case ASTExpressionLiteralBool:
    return "ASTExpressionLiteralBool";
  case ASTExpressionList:
    return "ASTExpressionList";
  case ASTExpressionCreateArray:
    return "ASTExpressionCreateArray";
  case ASTSizeList:
    return "ASTSizeList";
  case ASTIdentifierList:
    return "ASTIdentifierList";
  case ASTImportSpec:
    return "ASTImportSpec";
  case ASTFuncImported:
    return "ASTFuncImported";
  case ASTClassDef:
    return "ASTClassDef";
  case ASTClassMember:
    return "ASTClassMember";
  case ASTClassMemberList:
    return "ASTClassMemberList";
  case ASTCallee:
    return "ASTCallee";
  case ASTError:
    return "ASTError";
  default:
    assert(0 && "Unknown ast node enum element");
  }
}

const char *astBinOpToString(enum ASTBinOp BinOp) {
  switch (BinOp) {
  case ASTBinOpArithPercent:
    return "ASTBinOpArithPercent";
  case ASTBinOpArithSum:
    return "ASTBinOpArithSum";
  case ASTBinOpArithSub:
    return "ASTBinOpArithSub";
  case ASTBinOpArithDiv:
    return "ASTBinOpArithDiv";
  case ASTBinOpArithMul:
    return "ASTBinOpArithMul";
  case ASTBinOpLogicEq:
    return "ASTBinOpLogicEq";
  case ASTBinOpLogicNeq:
    return "ASTBinOpLogicNeq";
  case ASTBinOpLogicGt:
    return "ASTBinOpLogicGt";
  case ASTBinOpLogicGte:
    return "ASTBinOpLogicGte";
  case ASTBinOpLogicLt:
    return "ASTBinOpLogicLt";
  case ASTBinOpLogicLte:
    return "ASTBinOpLogicLte";
  case ASTBinOpLogicOr:
    return "ASTBinOpLogicOr";
  case ASTBinOpLogicAnd:
    return "ASTBinOpLogicAnd";
  case ASTBinOpBinAnd:
    return "ASTBinOpBinAnd";
  case ASTBinOpBinOr:
    return "ASTBinOpBinOr";
  case ASTBinOpBinShr:
    return "ASTBinOpBinShr";
  case ASTBinOpBinShl:
    return "ASTBinOpBinShl";
  default:
    assert(0 && "Unknown binary operator enum element");
  }
}

const char *astUnOpToString(enum ASTUnOp UnOp) {
  switch (UnOp) {
  case ASTUnOpArithInc:
    return "ASTUnOpArithInc";
  case ASTUnOpArithDec:
    return "ASTUnOpArithDec";
  case ASTUnOpArithMinus:
    return "ASTUnOpArithMinus";
  case ASTUnOpBinNot:
    return "ASTUnOpBinNot";
  case ASTUnOpLogicNot:
    return "ASTUnOpLogicNot";
  default:
    assert(0 && "Unknown unary operator enum element");
  }
}

static void dumpAstToDot(ASTNode *Node, FILE *File, int *Counter) {
  int CurrId = (*Counter)++;

  fprintf(File, "  node%d [label=\"%s", CurrId, astTypeToString(Node->Type));

  if (Node->Type == ASTExpressionBinary) {
    fprintf(File, "\\n%s", astBinOpToString(Node->Data.BinOp.OpType));
  } else if (Node->Type == ASTExpressionUnary) {
    fprintf(File, "\\n%s", astUnOpToString(Node->Data.UnOp.OpType));
  } else if (Node->Type == ASTClassDef) {
    fprintf(File, "\\n%s", Node->Data.ClassDef.ClassName);
  } else if (Node->Type == ASTMethSignature) {
    fprintf(File, "\\n%s", Node->Data.MethodSignature.MethodName);
  } else if (Node->Type == ASTFuncSignature) {
    fprintf(File, "\\n%s", Node->Data.FunctionSignature.FunctionName);
  } else if (Node->Type == ASTTypeRefUser) {
    fprintf(File, "\\n%s", Node->Data.UserType.TypeName);
  } else if (Node->Type == ASTError) {
    fprintf(File, "\\n%s", Node->Data.Error.Message);
  } else if (Node->Type == ASTExpressionIndexer) {
    fprintf(File, "\\n%s", Node->Data.Indexer.IndexedName);
  } else if (Node->Type == ASTExpressionValuePlace) {
    fprintf(File, "\\n%ld<%s.%s>", Node->Data.ValuePlace.Id, Node->Data.ValuePlace.ObjName, Node->Data.ValuePlace.Name);
  } else if (Node->Type == ASTExpressionLiteralBits ||
             Node->Type == ASTExpressionLiteralBool ||
             Node->Type == ASTExpressionLiteralChar ||
             Node->Type == ASTExpressionLiteralDec ||
             Node->Type == ASTExpressionLiteralHex ||
             Node->Type == ASTExpressionLiteralStr) {
    fprintf(File, "\\n%s", Node->Data.Literal.Rawstr);
  } else if (Node->Type == ASTImportSpec) {
    fprintf(File, "\\n%s %s", Node->Data.ImportSpec.EntryName, Node->Data.ImportSpec.DllName);
  } else if (Node->Type == ASTCallee) {
    fprintf(File, "\\n%s", Node->Data.Callee.MethodName);
    if (Node->Type == CalleeTypeClass)
      fprintf(File, " %s", Node->Data.Callee.ObjName);
  }
  fprintf(File, "\"];\n");

  for (unsigned i = 0; i < Node->ChildrenCount; i++) {
    int ChildId = *Counter;
    dumpAstToDot(Node->Children[i], File, Counter);
    fprintf(File, "  node%d -> node%d;\n", CurrId, ChildId);
  }
}

void exportAstToDot(const char *OutputDir) {
  assert(ProgramRoot != NULL && "Attempt to dump empty tree");
  char FilenameBuff[PATH_MAX];
  strcpy(FilenameBuff, OutputDir);
  strcat(FilenameBuff, "/ast.dot");
  FILE *File = fopen(FilenameBuff, "w");

  if (!File)
    fprintf(stderr, "Error while opening file %s for write: %s", FilenameBuff,
            strerror(errno));

  fprintf(File, "digraph AST {\n");
  fprintf(File, "  node [shape=box, fontname=\"Courier\"];\n");

  int Counter = 0;
  dumpAstToDot(ProgramRoot, File, &Counter);

  fprintf(File, "}\n");
  fclose(File);
}

bool isType(enum ASTType Type) {
  return Type == ASTTypeRefArray || Type == ASTTypeRefUser ||
         Type == ASTTypeRefBuiltinBool || Type == ASTTypeRefBuiltinByte ||
         Type == ASTTypeRefBuiltinInt || Type == ASTTypeRefBuiltinUint ||
         Type == ASTTypeRefBuiltinLong || Type == ASTTypeRefBuiltinUlong ||
         Type == ASTTypeRefBuiltinChar || Type == ASTTypeRefBuiltinString;
}

bool isUserType(enum ASTType Type) {
  return Type == ASTTypeRefUser;
}

bool isBuiltinType(enum ASTType Type) {
  return Type == ASTTypeRefBuiltinBool || Type == ASTTypeRefBuiltinByte ||
         Type == ASTTypeRefBuiltinInt || Type == ASTTypeRefBuiltinUint ||
         Type == ASTTypeRefBuiltinLong || Type == ASTTypeRefBuiltinUlong ||
         Type == ASTTypeRefBuiltinChar || Type == ASTTypeRefBuiltinString;
}

bool isArray(enum ASTType Type) {
  return Type == ASTTypeRefArray;
}

const char *typeToString(enum ASTType Type) {
  switch (Type) {
  case ASTTypeRefArray:
    return "Array";
  case ASTTypeRefUser:
    return "User";
  case ASTTypeRefBuiltinBool:
    return "BuiltinBool";
  case ASTTypeRefBuiltinByte:
    return "BuiltinByte";
  case ASTTypeRefBuiltinInt:
    return "BuiltinInt";
  case ASTTypeRefBuiltinUint:
    return "BuiltinUint";
  case ASTTypeRefBuiltinLong:
    return "BuiltinLong";
  case ASTTypeRefBuiltinUlong:
    return "BuiltinUlong";
  case ASTTypeRefBuiltinChar:
    return "BuiltinChar";
  case ASTTypeRefBuiltinString:
    return "BuiltinString";
  default:
    UNREACHABLE();
  }
}
