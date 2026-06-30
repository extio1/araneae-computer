#include "araneae/cfg/Builder.h"
#include "araneae/cfg/Cfg.h"
#include "araneae/cfg/Module.h"
#include "araneae/cfg/Function.h"
#include "araneae/cfg/Dag.h"
#include "araneae/cfg/Class.h"
#include "araneae/aux/Aux.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define V struct OpDAGNode *
#define SNAME local_types_l
#define PFX ltl
#include "cmc/list.h"

#include "cmc/utl/futils.h"

/// Build CFG functions {

typedef struct CFGBuildDAGContext {
  struct local_types_l *LocalTypes;
} CFGBuildDAGContext;

static struct OpDAGNode *proccessInstruction(ASTNode *FN, CFGBuildDAGContext *Ctx) {
  // Create new DAG node and link it with ParentDAGNode
  struct OpDAGNode *NewNode = opDAGNodeCreateEmpty();
  NewNode->Operands = dnl_create(2);

  switch (FN->Type) {
  case ASTExpressionAssign: {
    NewNode->Type = OpDAGAssign;
    assert(FN->ChildrenCount == 2);

    struct OpDAGNode *AssignTo = proccessInstruction(FN->Children[0], Ctx);
    struct OpDAGNode *AssignFrom = proccessInstruction(FN->Children[1], Ctx);
    dnl_push_back(NewNode->Operands, AssignTo);
    dnl_push_back(NewNode->Operands, AssignFrom);

    break;
  }
  case ASTExpressionBinary: {
    assert(FN->ChildrenCount == 2);
    switch (FN->Data.BinOp.OpType) {
    case ASTBinOpArithSum: {
      NewNode->Type = OpDAGAdd;
      break;
    }
    case ASTBinOpArithSub: {
      NewNode->Type = OpDAGSub;
      break;
    }
    case ASTBinOpArithDiv: {
      NewNode->Type = OpDAGDiv;
      break;
    }
    case ASTBinOpArithPercent: {
      NewNode->Type = OpDAGMod;
      break;
    }
    case ASTBinOpArithMul: {
      NewNode->Type = OpDAGMul;
      break;
    }
    case ASTBinOpLogicEq: {
      NewNode->Type = OpDAGEq;
      break;
    }
    case ASTBinOpLogicNeq: {
      NewNode->Type = OpDAGNeq;
      break;
    }
    case ASTBinOpLogicGt: {
      NewNode->Type = OpDAGGt;
      break;
    }
    case ASTBinOpLogicGte: {
      NewNode->Type = OpDAGGte;
      break;
    }
    case ASTBinOpLogicLt: {
      NewNode->Type = OpDAGLt;
      break;
    }
    case ASTBinOpLogicLte: {
      NewNode->Type = OpDAGLte;
      break;
    }
    case ASTBinOpLogicOr: {
      NewNode->Type = OpDAGOr;
      break;
    }
    case ASTBinOpLogicAnd: {
      NewNode->Type = OpDAGAnd;
      break;
    }
    case ASTBinOpBinAnd: {
      NewNode->Type = OpDAGBitAnd;
      break;
    }
    case ASTBinOpBinOr: {
      NewNode->Type = OpDAGBitOr;
      break;
    }
    case ASTBinOpBinShr: {
      NewNode->Type = OpDAGBitShr;
      break;
    }
    case ASTBinOpBinShl: {
      NewNode->Type = OpDAGBitShl;
      break;
    }
    default:
      UNREACHABLE();
    }
    struct OpDAGNode *LHS = proccessInstruction(FN->Children[0], Ctx);
    struct OpDAGNode *RHS = proccessInstruction(FN->Children[1], Ctx);
    dnl_push_back(NewNode->Operands, LHS);
    dnl_push_back(NewNode->Operands, RHS);
    break;
  }
  case ASTExpressionUnary: {
    assert(FN->ChildrenCount == 1);
    switch (FN->Data.UnOp.OpType) {
    case ASTUnOpArithInc: {
      NewNode->Type = OpDAGInc;
      break;
    }
    case ASTUnOpArithDec: {
      NewNode->Type = OpDAGDec;
      break;
    }
    case ASTUnOpArithMinus: {
      NewNode->Type = OpDAGNeg;
      break;
    }
    case ASTUnOpBinNot: {
      NewNode->Type = OpDAGBitNot;
      break;
    }
    case ASTUnOpLogicNot: {
      NewNode->Type = OpDAGNot;
      break;
    }
    default:
      UNREACHABLE();
    }
    struct OpDAGNode *Opnd = proccessInstruction(FN->Children[0], Ctx);
    dnl_push_back(NewNode->Operands, Opnd);
    break;
  }
  case ASTExpressionCall: {
    // call nodes are
    //    0: Callee
    //    1: AST list of operands
    assert(FN->ChildrenCount == 2);
    struct ASTNode *CalleeN = FN->Children[0];
    assert(CalleeN->Type == ASTCallee);

    switch (CalleeN->Data.Callee.Type)
    {
    case CalleeTypeFree: {
      // call of some function outside of object.
      NewNode->Type = OpDAGCall;
      NewNode->TerminalData.Call.FuncName = CalleeN->Data.Callee.MethodName;
      break;
    }
    case CalleeTypeClass: {
      // call of object function via dot, e.g. obj.func(...)
      NewNode->Type = OpDAGObjectCall;
      NewNode->TerminalData.ObjectCall.FuncName = CalleeN->Data.Callee.MethodName;
      NewNode->TerminalData.ObjectCall.ObjectName = CalleeN->Data.Callee.ObjName;
      break;
    }
    default: UNREACHABLE("Unknown type of call");
    }

    struct OpDAGNode *Params = proccessInstruction(FN->Children[1], Ctx);
    dnl_push_back(NewNode->Operands, Params);
    break;
  }
  case ASTExpressionList: {
    NewNode->Type = OpDAGList;

    // Add all (which are in function signature) parameters.
    for (size_t I = 0; I < FN->ChildrenCount; ++I) {
      struct OpDAGNode *Param = proccessInstruction(FN->Children[I], Ctx);
      dnl_push_back(NewNode->Operands, Param);
    }

    break;
  }
  case ASTExpressionIndexer: {
    NewNode->Type = OpDAGIndexer;
    assert(FN->ChildrenCount == 2);
    struct OpDAGNode *VPName = proccessInstruction(FN->Children[0], Ctx);
    dnl_push_back(NewNode->Operands, VPName);

    ASTNode *DimsN = FN->Children[1];
    unsigned int NumOfDims = DimsN->ChildrenCount;
    // TODO: to implement more than 1 dimension should read them all and calculate
    // size of array by multiplication.
    assert(NumOfDims == 1 && "More than one dimension for array NYI");
    for (size_t I = 0; I < NumOfDims; ++I) {
      struct OpDAGNode *Idx = proccessInstruction(DimsN->Children[I], Ctx);
      dnl_push_back(NewNode->Operands, Idx);
    }

    break;
  }
  case ASTExpressionCreateArray: {
    NewNode->Type = OpDAGCreateArray;
    ASTNode *DimsN = FN->Children[0];
    unsigned int NumOfDims = DimsN->ChildrenCount;
    // TODO: to implement more than 1 dimension should read them all and calculate
    // size of array by multiplication.
    assert(NumOfDims == 1 && "More than one dimension for array NYI");
    const char *SizeOfArrayStr = DimsN->Children[0]->Data.Literal.Rawstr;
    char *Endptr;
    uint64_t SizeOfArray = strtoull(SizeOfArrayStr, &Endptr, 10);
    NewNode->TerminalData.CreateArray.SizeOf = SizeOfArray;
    break;
  }
  case ASTExpressionValuePlace: {
    const char *ValueName = FN->Data.ValuePlace.Name;
    const char *ObjectName = FN->Data.ValuePlace.ObjName; 

    if (ObjectName) {
      NewNode->Type = OpDAGFieldAccess;
      NewNode->TerminalData.FieldAccess.ObjectName = ObjectName;
      NewNode->TerminalData.FieldAccess.FieldName = ValueName;
    } else {
      NewNode->Type = OpDAGVP;
      NewNode->TerminalData.ValuePlace.PlaceName = ValueName;
    }    

    break;
  }
  case ASTExpressionLiteralStr: {
    NewNode->Type = OpDAGLiteralStr;
    NewNode->TerminalData.Literal.Str = FN->Data.Literal.Rawstr;
    break;
  }
  case ASTExpressionLiteralChar: {
    NewNode->Type = OpDAGLiteralByte;
    NewNode->TerminalData.Literal.Char = FN->Data.Literal.Rawstr[0];
    break;
  }
  case ASTExpressionLiteralBool: {
    NewNode->Type = OpDAGLiteralBool;
    const char *BoolStr = FN->Data.Literal.Rawstr;
    if (strcmp("true", BoolStr) == 0)
      NewNode->TerminalData.Literal.Bool = true;
    else if (strcmp("false", BoolStr) == 0)
      NewNode->TerminalData.Literal.Bool = false;
    else
      UNREACHABLE("Unknown value for boolean literal");
    break;
  }
  case ASTExpressionLiteralHex: {;
    const char *HexStr = FN->Data.Literal.Rawstr;
    if (strncmp(HexStr, "0x", 2) == 0 || strncmp(HexStr, "0X", 2) == 0)
      HexStr += 2;
    char *endptr;
    uint64_t value = strtoull(HexStr, &endptr, 16);
    
    if (*endptr != '\0')
      UNREACHABLE("uncorrect hex literal");

    if (value <= UINT32_MAX) {
      NewNode->Type = OpDAGLiteralDoubleWord;
      NewNode->TerminalData.Literal.UInt = (uint32_t)value;
    } else {
      NewNode->Type = OpDAGLiteralQuadWord;
      NewNode->TerminalData.Literal.ULong = value;
    }
    break;
  }
  case ASTExpressionLiteralBits: {
    const char *BitsStr = FN->Data.Literal.Rawstr;
    
    if (strncmp(BitsStr, "0b", 2) == 0 || strncmp(BitsStr, "0B", 2) == 0)
      BitsStr += 2;
    
    uint64_t value = 0;
    for (const char *p = BitsStr; *p; p++) {
      if (*p == '0') {
        value = (value << 1) | 0;
      } else if (*p == '1') {
        value = (value << 1) | 1;
      } else {
        UNREACHABLE("uncorrect binary literal: not 0 or 1");
      }
    
      if ((value >> 63) > 1 && *(p+1) != '\0') {
        UNREACHABLE("uncorrect binary literal: value too big");
      }
    }
    
    if (value <= UINT32_MAX) {
      NewNode->Type = OpDAGLiteralDoubleWord;
      NewNode->TerminalData.Literal.UInt = (uint32_t)value;
    } else {
      NewNode->Type = OpDAGLiteralQuadWord;
      NewNode->TerminalData.Literal.ULong = value;
    }
    break;
  }
  case ASTExpressionLiteralDec: {
    const char *DecStr = FN->Data.Literal.Rawstr;
    
    char *endptr;
    int sign = 1;
    
    if (*DecStr == '-') {
      sign = -1;
      DecStr++;
    } else if (*DecStr == '+') {
      DecStr++;
    }
    
    uint64_t uvalue = strtoull(DecStr, &endptr, 10);
    
    if (*endptr != '\0') {
      UNREACHABLE("Uncorrect decimal literal");
    }
    
    if (sign == -1) {
      if (uvalue <= (uint64_t)INT32_MAX + 1) {
        NewNode->Type = OpDAGLiteralDoubleWord;
        NewNode->TerminalData.Literal.Signed = true;
        NewNode->TerminalData.Literal.Int = -(int32_t)uvalue;
      } else if (uvalue <= (uint64_t)INT64_MAX + 1) {
        NewNode->Type = OpDAGLiteralQuadWord;
        NewNode->TerminalData.Literal.Signed = true;
        NewNode->TerminalData.Literal.Long = -(int64_t)uvalue;
      } else {
        UNREACHABLE("Uncorrect decimal literal: overflow");
      }
    } else {
      if (uvalue <= UINT32_MAX) {
        NewNode->Type = OpDAGLiteralDoubleWord;
        NewNode->TerminalData.Literal.Signed = false;
        NewNode->TerminalData.Literal.UInt = (uint32_t)uvalue;
      } else if (uvalue <= INT64_MAX) {
        NewNode->Type = OpDAGLiteralQuadWord;
        NewNode->TerminalData.Literal.Signed = true;
        NewNode->TerminalData.Literal.ULong = uvalue;
      } else {
        UNREACHABLE("Uncorrect decimal literal: overflow");
      }
    }
    break;
  }
  default:
    UNREACHABLE();
  }
  return NewNode;
}

typedef struct CFGBuildContext {
  struct CFG *CFG;
  // Pointer to current analysed basic block.
  struct BasicBlock *CurrentBlock;
  // Pointer to the block we should branch inside of cycle or if-statement.
  struct BasicBlock *FallthrouhBB;
  // Pointer to current fallthrouh block for cycles (to use in break statement).
  struct BasicBlock *BreakBB;
  // e.g. can be set true if there is some code after if-else flow.
  bool Unreachable;
  // Types of local variables. Now this map used to support object function calls.
  struct local_types_l *LocalTypes;
} CFGBuildContext;

static int CfgNodeId = 0;

static void makeBlocksCondConnected(struct BasicBlock *FromBB, struct OpDAG *OpDAG,
                                    struct BasicBlock *IfTrueBB,
                                    struct BasicBlock *IfFalseBB) {
  FromBB->Type = BBTypeCondBranch;
  FromBB->Flow.Cond.Condition = OpDAG;
  FromBB->Flow.Cond.IfTrue = IfTrueBB;
  FromBB->Flow.Cond.IfFalse = IfFalseBB;
  bbl_push_back(FromBB->Successors, IfTrueBB);
  bbl_push_back(FromBB->Successors, IfFalseBB);
  bbl_push_back(IfTrueBB->Predecessors, FromBB);
  bbl_push_back(IfFalseBB->Predecessors, FromBB);
}

static void makeBlocksUncondConnected(struct BasicBlock *FromBB,
                                      struct BasicBlock *ToBB) {
  FromBB->Type = BBTypeUncondBranch;
  FromBB->Flow.Uncond.Target = ToBB;
  bbl_push_back(FromBB->Successors, ToBB);
  bbl_push_back(ToBB->Predecessors, FromBB);
}

static void proccessAstToCfg(ASTNode *FN, CFGBuildContext *Ctx) {
  switch (FN->Type) {
  case ASTStatementBlock: {
    // Create new basic block and jump to it.
    assert(FN->ChildrenCount == 1);
    struct BasicBlock *BB = basicBlockCreate(Ctx->CFG, "entry");
    makeBlocksUncondConnected(Ctx->CurrentBlock, BB);
    Ctx->CurrentBlock = BB;
    proccessAstToCfg(FN->Children[0], Ctx);
    break;
  }
  case ASTStatementList: {
    for (size_t I = 0; I < FN->ChildrenCount; ++I) {
      if (Ctx->Unreachable) {
        Ctx->Unreachable = false;
        break;
      }
      proccessAstToCfg(FN->Children[I], Ctx);
    }
    break;
  }
  case ASTStatementIf: {
    // some_bb:
    //  ...
    //  br if_cond
    // if_cond:
    //  cond_value = calc_cond_value()
    //  br cond_value, if_true, if_false
    // if_true:
    //  do_smth
    // if_fallthrough:
    //  do_smth
    assert(FN->ChildrenCount == 2);
    struct BasicBlock *BBIfCond = basicBlockCreate(Ctx->CFG, "ifCond");
    struct BasicBlock *BBIfTrue = basicBlockCreate(Ctx->CFG, "ifTrue");
    struct BasicBlock *BBFallthrough = basicBlockCreate(Ctx->CFG, "ifFallthrough");

    makeBlocksUncondConnected(Ctx->CurrentBlock, BBIfCond);

    // Process predicate expression to some_bb
    Ctx->CurrentBlock = BBIfCond;
    proccessAstToCfg(FN->Children[0], Ctx);
    struct OpDAG *CondDAG = dl_back(Ctx->CurrentBlock->Instructions);
    // Connect conditionaly result of condition calculate with true or fallthrough paths.
    makeBlocksCondConnected(Ctx->CurrentBlock, CondDAG, BBIfTrue, BBFallthrough);

    // Continue building cfg with if-true branch
    Ctx->CurrentBlock = BBIfTrue;
    proccessAstToCfg(FN->Children[1], Ctx);
    // If if-true block doesn't have terminator, connect it with fallthrough block.
    if (!Ctx->Unreachable)
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBFallthrough);
    Ctx->Unreachable = false;

    // When finising analysing if-true branch continue with fallthough branch.
    // After changing current block we are going to continue ASTStatementList node.
    Ctx->CurrentBlock = BBFallthrough;
    break;
  }
  case ASTStatementIfElse: {
    // some_bb:
    //  ...
    //  br if_cond
    // if_cond:
    //  cond_value = calc_cond_value()
    //  br cond_value, if_true, if_false
    // if_true:
    //  ...
    // if_false:
    //  ...
    assert(FN->ChildrenCount == 3);
    struct BasicBlock *BBIfCond = basicBlockCreate(Ctx->CFG, "ifCond");
    struct BasicBlock *BBIfTrue = basicBlockCreate(Ctx->CFG, "ifTrue");
    struct BasicBlock *BBIfElse = basicBlockCreate(Ctx->CFG, "ifElse");
    struct BasicBlock *BBMerge = basicBlockCreate(Ctx->CFG, "ifMerge");

    makeBlocksUncondConnected(Ctx->CurrentBlock, BBIfCond);
    // Process predicate expression.
    Ctx->CurrentBlock = BBIfCond;
    proccessAstToCfg(FN->Children[0], Ctx);
    struct OpDAG *CondDAG = dl_back(Ctx->CurrentBlock->Instructions);
    // Connect result of predicate calculation with if-true and if-else block conditionaly.
    makeBlocksCondConnected(Ctx->CurrentBlock, CondDAG, BBIfTrue, BBIfElse);

    // Continue building cfg with if-true branch
    Ctx->CurrentBlock = BBIfTrue;
    proccessAstToCfg(FN->Children[1], Ctx);
    // If if-true block doesn't have terminator, connect it with merge block.
    if (!Ctx->Unreachable)
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBMerge);
    Ctx->Unreachable = false;

    // When finising analysing if-true branch continue with if-else
    Ctx->CurrentBlock = BBIfElse;
    proccessAstToCfg(FN->Children[2], Ctx);
    // If if-true block doesn't have terminator, connect it with merge block.
    if (!Ctx->Unreachable)
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBMerge);
    Ctx->Unreachable = false;

    // Continue processing to merge block
    Ctx->CurrentBlock = BBMerge;
    break;
  }
  case ASTStatementWhile: {
    // some_bb:
    //  br while_cond
    // while_cond:
    //  cond_value = calc_cond_value()
    //  br cond_value, while_body, while_fallthrough
    // while_body:
    //  ...
    //  br while_cond
    // while_fallthrough:
    //  ...
    struct BasicBlock *BBWhileCond = basicBlockCreate(Ctx->CFG, "whileCond");
    struct BasicBlock *BBWhileBody = basicBlockCreate(Ctx->CFG, "whileBody");
    struct BasicBlock *BBWhileFallthrough =
        basicBlockCreate(Ctx->CFG, "whileFallthrough");

    makeBlocksUncondConnected(Ctx->CurrentBlock, BBWhileCond);

    // Process predicate expression to whileCond basic block
    Ctx->CurrentBlock = BBWhileCond;
    proccessAstToCfg(FN->Children[0], Ctx);
    struct OpDAG *CondDAG = dl_back(BBWhileCond->Instructions);
    // Connect result calculation of branch predicate with fallthrough or body block.
    makeBlocksCondConnected(Ctx->CurrentBlock, CondDAG, BBWhileBody, BBWhileFallthrough);

    // Continue building cfg with while-body branch
    Ctx->CurrentBlock = BBWhileBody;
    struct BasicBlock *OldBreakBB = Ctx->BreakBB;
    Ctx->BreakBB = BBWhileFallthrough;
    proccessAstToCfg(FN->Children[1], Ctx);
    if (!Ctx->Unreachable)
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBWhileCond);
    Ctx->Unreachable = false;

    // When finising analysing while-body branch continue with fallthough
    // branch and restore previous basic block for break. 
    Ctx->CurrentBlock = BBWhileFallthrough;
    Ctx->BreakBB = OldBreakBB;
    break;
  }
  case ASTStatementDoWhile: {
    // some_bb:
    //  br doWhileBody
    // doWhileBody:
    //  ...
    //  br doWhileCond
    // doWhileCond:
    //  cond_value = calc_cond_value()
    //  br cond_value, doWhileBody, doWhileFallthrough
    // doWhileFallthrough:
    //  ...

    struct BasicBlock *BBDoWhileBody = basicBlockCreate(Ctx->CFG, "doWhileBody");
    struct BasicBlock *BBDoWhileCond = basicBlockCreate(Ctx->CFG, "doWhileCond");
    struct BasicBlock *BBDoWhileFallthrough = basicBlockCreate(Ctx->CFG, "doWhileFallthrough");

    struct BasicBlock *OldBreakBB = Ctx->BreakBB;
    Ctx->BreakBB = BBDoWhileFallthrough;
    
    makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoWhileCond);

    // Process do-while cond
    Ctx->CurrentBlock = BBDoWhileCond;
    proccessAstToCfg(FN->Children[1], Ctx);
    struct OpDAG *CondDAG = dl_back(BBDoWhileCond->Instructions);
    makeBlocksCondConnected(Ctx->CurrentBlock, CondDAG, BBDoWhileBody,
                            BBDoWhileFallthrough);

    // Process do-while body
    Ctx->CurrentBlock = BBDoWhileBody;
    proccessAstToCfg(FN->Children[0], Ctx);
    if (!Ctx->Unreachable)
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoWhileCond);
    Ctx->Unreachable = false;

    // When finising analysing do-while branch continue with fallthough branch.
    Ctx->CurrentBlock = BBDoWhileFallthrough;
    Ctx->BreakBB = OldBreakBB;
    break;
  }
  case ASTStatementDoUntil: {
    // All the same as do while, but we change order of while-fallthrough and
    // while-body in while-cond block some_bb:
    //  br doUntilBody
    // doUntilBody:
    //  ...
    //  br doUntilCond
    // doUntilCond:
    //  cond_value = calc_cond_value()
    //  br cond_value, doWhileFallthrough, doWhileBody
    // doWhileFallthrough:
    //  ...

    struct BasicBlock *BBDoWhileBody = basicBlockCreate(Ctx->CFG, "doUntilBody");
    struct BasicBlock *BBDoWhileCond = basicBlockCreate(Ctx->CFG, "doUntilCond");
    struct BasicBlock *BBDoWhileFallthrough = basicBlockCreate(Ctx->CFG, "doUntilFallthrough");

    struct BasicBlock *OldBreakBB = Ctx->BreakBB;
    Ctx->BreakBB = BBDoWhileFallthrough;
    
    makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoWhileBody);

    // Process do-while body
    Ctx->CurrentBlock = BBDoWhileBody;
    proccessAstToCfg(FN->Children[0], Ctx);
    if (!Ctx->Unreachable) {
      // If do-while has return or break, we dont even need to create basic block
      // for condition.
      makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoWhileCond);

      // Process do-while cond
      Ctx->CurrentBlock = BBDoWhileCond;
      proccessAstToCfg(FN->Children[1], Ctx);
      struct OpDAG *CondDAG = dl_back(BBDoWhileCond->Instructions);
      makeBlocksCondConnected(Ctx->CurrentBlock, CondDAG, BBDoWhileFallthrough,
                              BBDoWhileBody);
    }
    Ctx->Unreachable = false;

    // When finising analysing do-while branch continue with fallthough branch.
    Ctx->CurrentBlock = BBDoWhileFallthrough;
    Ctx->BreakBB = OldBreakBB;
    break;
  }
  case ASTStatementBreak: {
    assert(Ctx->BreakBB);
    makeBlocksUncondConnected(Ctx->CurrentBlock, Ctx->BreakBB);
    Ctx->Unreachable = true;
    break;
  }
  case ASTStatementReturn: {
    struct BasicBlock *BBDoRet = basicBlockCreate(Ctx->CFG, "ret");
    
    makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoRet);
    Ctx->CurrentBlock = BBDoRet;
    proccessAstToCfg(FN->Children[0], Ctx);

    struct OpDAG *RetDAG = dl_back(BBDoRet->Instructions);
    BBDoRet->Flow.Ret.RetVal = RetDAG;
    dl_push_back(Ctx->CFG->Returns, RetDAG);

    Ctx->Unreachable = true;
    break;
  }
  case ASTStatementReturnVoid: {
    struct BasicBlock *BBDoRet = basicBlockCreate(Ctx->CFG, "ret");
    makeBlocksUncondConnected(Ctx->CurrentBlock, BBDoRet);

    BBDoRet->Flow.Ret.RetVal = NULL;

    Ctx->Unreachable = true;
    break;
  }
  case ASTStatementExpr: {
    assert(FN->ChildrenCount == 1);
    CFGBuildDAGContext DAGCtx = {.LocalTypes = Ctx->LocalTypes};
    struct OpDAGNode *DAGRoot = proccessInstruction(FN->Children[0], &DAGCtx);
    struct OpDAG *DAG = opDAGCreateEmpty();
    DAG->Root = DAGRoot;
    dl_push_back(Ctx->CurrentBlock->Instructions, DAG);
    break;
  }
  default:
    UNREACHABLE();
  }
}

static void buildCfgForFuncProlog(ASTNode *FN) {
  assert(FN->Type == ASTFuncBodyProlog);
}

static struct CFG *buildCfgFor(ASTNode *FN) {
  assert(FN->Type == ASTFuncBody);

  struct CFG *CFG = CFGCreate();
  struct BasicBlock *EntryBB = basicBlockCreate(CFG, "entry");
  CFG->Entry = EntryBB;

  CFGBuildContext Ctx = {.CFG = CFG,
                         .CurrentBlock = EntryBB,
                         .FallthrouhBB = NULL,
                         .Unreachable = false,
                         .LocalTypes = NULL};

  // This map going to hold map from variable name to variable type.
  // e.g. for this piece of code:
  //    Num = 42;
  //    Calc = Calculator();
  //    Calc.add(Num)
  // we need to know type of Calc (Calculator class) to call function add#Calculator and trasfer
  // to this function "this" reference to Calc of type Calculator.

  switch (FN->Type) {
  case ASTFuncBody:
    // Always do not emit prolog. 
    proccessAstToCfg(FN->Children[1], &Ctx);
    break;
  default:
    UNREACHABLE();
  }
  
  return CFG;
}

/// } Build CFG functions
/// Collect info about functions {

static void collectReturnType(struct Function *FI, ASTNode *FRTN) {
  // Because of dynamic typing be ingore this specification for now. 
}

static void collectArguments(struct Function *F, ASTNode *FAN) {
  assert(FAN->Type == ASTFuncArguments);

  assert(FAN->ChildrenCount >= 0);
  if (FAN->ChildrenCount == 0) {
    concat(&F->SignatureText, "()");
    return;
  }

  concat(&F->SignatureText, "(");
  for (size_t I = 0; I < FAN->ChildrenCount; ++I) {
    ASTNode *Arg = FAN->Children[I];

    struct Argument *FArg = argumentCreateEmpty();
    switch (Arg->Type) {
    case ASTTypeVarDecl:
    case ASTTypeRefUser: {
      FArg->Name = strdup(Arg->Data.UserType.TypeName);
      break;
    }
    default:
      UNREACHABLE();
    }

    concat(&F->SignatureText, FArg->Name);
    if (I != FAN->ChildrenCount - 1)
      concat(&F->SignatureText, ", ");

    al_push_back(F->Args, FArg);
  }

  assert(FAN->ChildrenCount == al_count(F->Args));
  concat(&F->SignatureText, ")");
}

static void collectFunctionName(struct Function *F, ASTNode *FSN) {
  assert(FSN->Type == ASTFuncSignature || FSN->Type == ASTMethSignature);
  F->Name = strdup(FSN->Data.FunctionSignature.FunctionName);
  F->SignatureText = strdup(FSN->Data.FunctionSignature.FunctionName);
}

static struct Function *buildFunctionDef(const ASTNode *FunctionN, struct Module* M, struct ClassType *ClassT) {
  assert(FunctionN->Type == ASTFuncDef || FunctionN->Type == ASTFuncDecl);
  struct Function *F = functionCreateEmpty();

  ASTNode *FunctionSignatureN = FunctionN->Children[0];
  collectFunctionName(F, FunctionSignatureN);

  // If create function for class, add special class suffix.
  if (ClassT) {
    // Function is constructor, if it has the same name as a class.
    F->IsConstructor = (strcmp(F->Name, ClassT->Name) == 0);
  }

  switch (FunctionN->Type) {
  case ASTFuncDecl:
    switch (FunctionSignatureN->Type) {
    case ASTMethSignature:
      F->Type = MethDecl;
      break;
    case ASTFuncSignature:
      F->Type = FuncDecl;
      break;
    default:
      UNREACHABLE();
    }
    break;
  case ASTFuncDef:
    switch (FunctionSignatureN->Type) {
    case ASTMethSignature:
      F->Type = MethDef;
      break;
    case ASTFuncSignature:
      F->Type = FuncDef;
      break;
    default:
      UNREACHABLE();
    }
    break;
  default:
    UNREACHABLE();
  }

  assert(al_count(F->Args) == 0);
  switch (F->Type) {
  case MethDecl:
  case MethDef: {
    ASTNode *FunctionArgumentsN = FunctionSignatureN->Children[0];
    collectArguments(F, FunctionArgumentsN);
    break;
  }
  case FuncDecl:
  case FuncDef: {
    ASTNode *FunctionArgumentsN = FunctionSignatureN->Children[0];
    collectArguments(F, FunctionArgumentsN);
    ASTNode *FunctionReturnTypeN = FunctionSignatureN->Children[1];
    collectReturnType(F, FunctionReturnTypeN);
    break;
  }
  }

  // If build this function for class then add one argument of class type in the 
  // beginning of the function argument list. This is "this" reference.
  if (ClassT) {
    struct Argument *ClassArg = argumentCreate("this");
    al_push_front(F->Args, ClassArg);
  }

  switch (F->Type) {
  case MethDef:
  case FuncDef: {
    struct ASTNode *FunctionBodyN = FunctionN->Children[1];
    struct CFG *CFG = buildCfgFor(FunctionBodyN);
    F->CFG = CFG;
    break;
  }
  case MethDecl:
  case FuncDecl: {
    F->CFG = NULL;
    break;
  }
  default:
    UNREACHABLE();
  }

  fl_push_back(M->Functions, F);
  return F;
}

static void buildFieldsForClass(const ASTNode *CN, struct ClassType *Class) {
  assert(CN->Type == ASTFuncBodyProlog);
  
  if (CN->ChildrenCount == 0)
    // No fields in class. Do not emit anything.
    return;

  // Get AST nodes for list of variables and AST node for nodes type.
  const struct ASTNode *ListOfVarDecls = CN->Children[0];
  for (size_t I = 0; I < ListOfVarDecls->ChildrenCount; ++I) {
    struct ASTNode *DeclsN = NULL;
    struct ASTNode *DeclsTypeN = NULL;

    struct ASTNode *TypedOrNotN = ListOfVarDecls->Children[I];
    switch (TypedOrNotN->Type)
    {
    case ASTIdentifierList: {
      DeclsN = TypedOrNotN;
      // Declaration of field without type.
      break;
    }
    case ASTTypeVarDecl: {
      // Declaration of typed fields.
      assert(TypedOrNotN->Children[0]->Type == ASTIdentifierList);
      assert(isType(TypedOrNotN->Children[1]->Type));
      DeclsN = TypedOrNotN->Children[0];
      DeclsTypeN = TypedOrNotN->Children[1];
      break;
    }
    default: UNREACHABLE("unknown type fo class field decl");
    }

    for (size_t I = 0; I < DeclsN->ChildrenCount; ++I) {
      struct ASTNode *Decl = DeclsN->Children[I];
      classAddField(Class, Decl->Data.ValuePlace.Name);
    }
  }
}

static void buildFunctionsForClass(const ASTNode *FLN, struct ClassType *Class, struct Module *M) {
  assert(FLN->Type == ASTClassMemberList);

  for (size_t I = 0; I < FLN->ChildrenCount; ++I) {
    struct ASTNode *ClassMemberN = FLN->Children[I];
    assert(ClassMemberN->Type == ASTClassMember && ClassMemberN->ChildrenCount == 1);
    struct ASTNode *FunctionN = ClassMemberN->Children[0];
    struct Function *NewFunction = buildFunctionDef(FunctionN, M, Class);
    enum MemberVisibility MV = (ClassMemberN->Data.MemberVisibility == ASTMemberVisibilityPrivate) 
                              ? MemberVisibilityPrivate
                              : MemberVisibilityPublic;
    struct Method *NewMethod = methodCreate(NewFunction, MV);
    if (NewFunction->IsConstructor)
      classAddConstructor(Class, NewFunction);
    else
      classAddMethod(Class, NewMethod);
    NewFunction->Class = Class;
  }
}

static void buildClassDef(const ASTNode *CN, struct Module* M) {
  assert(CN->Type == ASTClassDef);
  struct ASTNode *FieldDefs = CN->Children[0];
  struct ASTNode *FunctionDefs = CN->Children[1];
  assert(FieldDefs->Type == ASTFuncBodyProlog);
  assert(FunctionDefs->Type == ASTClassMemberList);
  
  struct ClassType *ClassType = classCreate(CN->Data.ClassDef.ClassName);
  if (CN->Data.ClassDef.BaseClassName) {
    struct ClassType *BaseClassType = classLookup(M->Classes, CN->Data.ClassDef.BaseClassName);
    if (!BaseClassType)
      fprintf(stderr, "No class named %s to be base class for %s.\n", CN->Data.ClassDef.BaseClassName, CN->Data.ClassDef.ClassName);
    ClassType->BaseClass = BaseClassType;
  }

  // Collect info about class fields.
  buildFieldsForClass(FieldDefs, ClassType);
  // Collect info about class functions.
  buildFunctionsForClass(FunctionDefs, ClassType, M);
  cltl_push_back(M->Classes, ClassType);
}

static void buildDispatch(const ASTNode *N, struct Module* M) {
  switch (N->Type)
  {
  case ASTFuncDecl:
  case ASTFuncDef:
    buildFunctionDef(N, M, NULL);
    break;
  case ASTClassDef:
    buildClassDef(N, M);
    break;
  default: UNREACHABLE("Unknown ast node type. Not a function or class.")
  }
}

void visitChildren(const ASTNode *Node, struct Module* M, visitChildrenF F) {
  for (size_t I = 0; I < Node->ChildrenCount; ++I)
    F(Node->Children[I], M);
}

struct Module *buildModule(ASTNode *ProgramRoot, const char* SourceName) {
  assert(ProgramRoot->Type == ASTProgram && "ProgramRoot is not a root of AST");

  struct Module *M = moduleCreateEmpty(SourceName);
  visitChildren(ProgramRoot, M, buildDispatch);

  return M;
}
