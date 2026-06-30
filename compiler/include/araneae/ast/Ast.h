#ifndef ARANEAE_AST_AST_H
#define ARANEAE_AST_AST_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum ASTMemberVisibility {
  ASTMemberVisibilityPublic,
  ASTMemberVisibilityPrivate
};

enum ASTBinOp {
  ASTBinOpArithSum,
  ASTBinOpArithSub,
  ASTBinOpArithDiv,
  ASTBinOpArithMul,
  ASTBinOpArithPercent,
  ASTBinOpLogicEq,
  ASTBinOpLogicNeq,
  ASTBinOpLogicGt,
  ASTBinOpLogicGte,
  ASTBinOpLogicLt,
  ASTBinOpLogicLte,
  ASTBinOpLogicOr,
  ASTBinOpLogicAnd,
  ASTBinOpBinAnd,
  ASTBinOpBinOr,
  ASTBinOpBinShr,
  ASTBinOpBinShl
};
const char *astBinOpToString(enum ASTBinOp BinOp);
enum ASTUnOp {
  ASTUnOpArithInc,
  ASTUnOpArithDec,
  ASTUnOpArithMinus,
  ASTUnOpBinNot,
  ASTUnOpLogicNot
};
const char *astUnOpToString(enum ASTUnOp UnOp);

enum ASTType {
  ASTProgram,

  ASTClassDef,
  ASTClassMemberList,
  ASTClassMember,
  ASTFuncDecl,
  ASTFuncDef,
  ASTFuncImported,
  ASTFuncSignature,
  ASTMethSignature,
  ASTFuncArguments,
  ASTFuncBody,
  ASTFuncBodyProlog,
  ASTFuncBodyPrologList,
  ASTPrologItemList,
  ASTImportSpec,

  ASTTypeRefArray,
  ASTTypeRefUser,
  ASTTypeRefBuiltinBool,
  ASTTypeRefBuiltinByte,
  ASTTypeRefBuiltinInt,
  ASTTypeRefBuiltinUint,
  ASTTypeRefBuiltinLong,
  ASTTypeRefBuiltinUlong,
  ASTTypeRefBuiltinChar,
  ASTTypeRefBuiltinString,
  ASTTypeVarDecl,

  ASTStatementIf,
  ASTStatementIfElse,
  ASTStatementBlock,
  ASTStatementWhile,
  ASTStatementDoWhile,
  ASTStatementDoUntil,
  ASTStatementBreak,
  ASTStatementReturn,
  ASTStatementReturnVoid,
  ASTStatementExpr,
  ASTStatementList,

  ASTExpressionAssign,
  ASTExpressionBinary,
  ASTExpressionUnary,
  ASTExpressionCall,
  ASTExpressionIndexer,
  ASTExpressionValuePlace,
  ASTExpressionLiteralStr,
  ASTExpressionLiteralChar,
  ASTExpressionLiteralHex,
  ASTExpressionLiteralBits,
  ASTExpressionLiteralDec,
  ASTExpressionLiteralBool,
  ASTExpressionList,
  ASTExpressionCreateArray,

  ASTCallee,

  ASTError,

  ASTSizeList,
  ASTIdentifierList
};
const char *astTypeToString(enum ASTType Type);

typedef struct ASTNode {
  enum ASTType Type;
  unsigned ChildrenCount;
  struct ASTNode **Children;
  union {
    struct {
      enum ASTBinOp OpType;
    } BinOp;
    struct {
      enum ASTUnOp OpType;
    } UnOp;
    struct {
      const char *ClassName;
      const char *BaseClassName;
    } ClassDef;
    struct {
      const char *MethodName;
    } MethodSignature;
    struct {
      const char *FunctionName;
    } FunctionSignature;
    struct {
      const char *TypeName;
    } UserType;
    struct {
      const char *Message;
    } Error;
    struct {
      const char *IndexedName;
      size_t Indexes[8];
    } Indexer;
    struct {
      enum PlaceType {
        PlaceTypeFree,
        PlaceTypeClass,
        PlaceTypeArrayElement
      } Type;
      const char *Name;
      const char *ObjName;
      uint64_t Id;
    } ValuePlace;
    struct {
      const char *Rawstr;
    } Literal;
    enum ASTMemberVisibility MemberVisibility;
    struct {
      const char *EntryName;
      const char *DllName;
    } ImportSpec;
    struct {
      enum CalleeType {
        CalleeTypeFree,
        CalleeTypeClass
      } Type;
      const char *MethodName;
      const char *ObjName;
    } Callee;
    struct {
      size_t Indexes[8];
    } CreateArray;
  } Data;

} ASTNode;

extern ASTNode *ProgramRoot;

// Add child to node "To" and return pointer to updated node
ASTNode *addChild(ASTNode *To, ASTNode *NewChild);

ASTNode *createAstProgramEmpty();

ASTNode *createAstFuncImported(ASTNode *FSignN, ASTNode *ImportSpecN);
ASTNode *createAstFuncDecl(ASTNode *FSignN);
ASTNode *createAstFuncDef(ASTNode *FSignatureN, ASTNode *FBodyN);

ASTNode *createAstClassDef(const char *Name, const char *BaseName, ASTNode *VarDeclsN, ASTNode *MemberListN);
ASTNode *createAstClassMemberListEmpty();
ASTNode *createAstClassMemberList(ASTNode *Node);
ASTNode *createAstClassMemberDef(enum ASTMemberVisibility Visibility, ASTNode *FuncN);

ASTNode *createAstImportSpec(const char* EntryName, const char *DllName);

ASTNode *createAstMethodSignature(const char *Name, ASTNode *FArgsN);
ASTNode *createAstFuncSignature(const char *Name, ASTNode *FArgsN,
                                ASTNode *RetTypeN);
ASTNode *createAstFuncBody(ASTNode *FPrologF, ASTNode *FBodyN);
ASTNode *createAstFuncArgumentsEmpty();
ASTNode *createAstFuncArguments();
ASTNode *createAstFuncProlog(ASTNode *VarDeclsN);
ASTNode *createAstFuncPrologEmpty();
ASTNode *createAstPrologItemList(ASTNode *PrologItemN);

ASTNode *createAstSizeListEmpty();
ASTNode *createAstSizeList(ASTNode *Node);
ASTNode *createAstIdentifierListEmpty();
ASTNode *createAstIdentifierList(ASTNode *Node);

ASTNode *createAstPrologItemList();

ASTNode *createAstError(const char *Msg);

ASTNode *createAstTypeRefBuiltinBool();
ASTNode *createAstTypeRefBuiltinByte();
ASTNode *createAstTypeRefBuiltinInt();
ASTNode *createAstTypeRefBuiltinUint();
ASTNode *createAstTypeRefBuiltinLong();
ASTNode *createAstTypeRefBuiltinUlong();
ASTNode *createAstTypeRefBuiltinChar();
ASTNode *createAstTypeRefBuiltinString();
ASTNode *createAstTypeRefUser(const char *Name);
ASTNode *createAstTypeRefArray(ASTNode *DimsN, ASTNode *ElementTypeN);
ASTNode *createAstTypeVarDecl(ASTNode *VarNameN, ASTNode *TypeNode);

ASTNode *createAstStatementIf(ASTNode *CondN, ASTNode *IfTrueN);
ASTNode *createAstStatementIfElse(ASTNode *CondN, ASTNode *IfTrueN,
                                  ASTNode *IfFalseN);
ASTNode *createAstStatementBlock(ASTNode *StatementListN);
ASTNode *createAstStatementWhile(ASTNode *CondN, ASTNode *BlockN);
ASTNode *createAstStatementDoWhile(ASTNode *CondN, ASTNode *BlockN);
ASTNode *createAstStatementDoUntil(ASTNode *CondN, ASTNode *BlockN);
ASTNode *createAstStatementBreak();
ASTNode *createAstStatementRetVoid();
ASTNode *createAstStatementRet(ASTNode *RetVal);
ASTNode *createAstStatementExpr(ASTNode *ExprN);
ASTNode *createAstStatementList(ASTNode *StatementN);

ASTNode *createAstExpressionAssign(ASTNode *PlaceN, ASTNode *ValueN);
ASTNode *createAstExpressionBinary(ASTNode *LHSN, enum ASTBinOp Op,
                                   ASTNode *RHSN);
ASTNode *createAstExpressionUnary(enum ASTUnOp Op, ASTNode *OpndN);
ASTNode *createAstExpressionCall(ASTNode *Calle, ASTNode *ArgsN);
ASTNode *createAstExpressionIndexer(ASTNode *IndexedValueN, ASTNode *IndexN);
ASTNode *createAstExpressionValuePlace(enum PlaceType Type, const char *Place, const char* Class);
ASTNode *createAstCreateArray(ASTNode *DimsN);
ASTNode *createAstExpressionLiteralStr(const char *Initializer);
ASTNode *createAstExpressionLiteralChar(const char *Initializer);
ASTNode *createAstExpressionLiteralHex(const char *Initializer);
ASTNode *createAstExpressionLiteralBits(const char *Initializer);
ASTNode *createAstExpressionLiteralDec(const char *Initializer);
ASTNode *createAstExpressionLiteralBool(const char *Initializer);
ASTNode *createAstExpressionListEmpty();
ASTNode *createAstExpressionList(ASTNode *ExprN);

ASTNode *createAstCallee(enum CalleeType Type, const char *FuncName, const char *ClassName);

void exportAstToDot(const char *Filename);

bool isType(enum ASTType Type);
bool isUserType(enum ASTType Type);
bool isBuiltinType(enum ASTType Type);
bool isArray(enum ASTType Type);

const char *typeToString(enum ASTType Type);

#endif /* ARANEAE_AST_AST_H */
