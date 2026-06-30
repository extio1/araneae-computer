#include "araneae/codegen/Codegen.h"
#include "araneae/abi/Reference.h"
#include "araneae/abi/Types.h"
#include "araneae/aux/Aux.h"
#include "araneae/aux/CodeBuffer.h"
#include "araneae/cfg/Builder.h"
#include "araneae/cfg/Class.h"

#include "EmitCtx.h"
#include "Register.h"
#include "Scope.h"
#include "Symbol.h"
#include "Types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MakeBinOp(Ctx, Mnemonic)                                               \
  Symbol *Src2 = sl_back(Ctx->SymbolStack);                                    \
  assert(sl_pop_back(Ctx->SymbolStack));                                       \
  Symbol *Src1 = sl_back(Ctx->SymbolStack);                                    \
  assert(sl_pop_back(Ctx->SymbolStack));                                       \
  char Src1LocStr[128], Src2LocStr[128];                                       \
  symbolGetLocationStr(Src1, Src1LocStr);                                      \
  symbolGetLocationStr(Src2, Src2LocStr);                                      \
  Register RegToHoldRef = allocRegister(Ctx);                                  \
  codeBufferAddFormatted(Ctx->CodeBuff, "\t" #Mnemonic " %s, %s, %s\n",        \
                         register2str(RegToHoldRef), Src1LocStr, Src2LocStr);  \
  Symbol *ResSym =                                                             \
      symbolDataCreateLocReg(NULL, RegToHoldRef, Ctx->CurrentScope, true);     \
  sl_push_back(Ctx->SymbolStack, ResSym);                                      \
  setRegister(RegToHoldRef, ResSym);                                           \
                                                                               \
  if (Src1->Tmp)                                                               \
    symbolFree(Src1);                                                          \
  if (Src2->Tmp)                                                               \
    symbolFree(Src2);

#define MakeUnOp(Ctx, Mnemonic)                                                \
  Symbol *Src = sl_back(Ctx->SymbolStack);                                     \
  assert(sl_pop_back(Ctx->SymbolStack));                                       \
  char SrcLocStr[128];                                                         \
  symbolGetLocationStr(Src, SrcLocStr);                                        \
  Register RegToHoldRef = allocRegister(Ctx);                                  \
  codeBufferAddFormatted(Ctx->CodeBuff, "\t" #Mnemonic " %s, %s\n",            \
                         register2str(RegToHoldRef), SrcLocStr);               \
  Symbol *ResSym =                                                             \
      symbolDataCreateLocReg(NULL, RegToHoldRef, Ctx->CurrentScope, true);     \
  sl_push_back(Ctx->SymbolStack, ResSym);                                      \
  setRegister(RegToHoldRef, ResSym);                                           \
                                                                               \
  if (Src->Tmp)                                                                \
    symbolFree(Src);

static struct basic_block_l *getAllBasicBlocks(struct BasicBlock *Entry) {
  struct basic_block_l *AllBBs = bbl_create(32);
  struct basic_block_l *Stack = bbl_create(32);
  struct basic_block_l *Visited = bbl_create(32);

  bbl_push_back(Stack, Entry);

  while (!bbl_empty(Stack)) {
    struct BasicBlock *BB = bbl_back(Stack);
    assert(bbl_pop_back(Stack));

    if (bbl_contains(Visited, BB))
      continue;

    bbl_push_back(Visited, BB);
    bbl_push_back(AllBBs, BB);

    for (size_t I = 0; I < bbl_count(BB->Successors); ++I) {
      struct BasicBlock *Succ = bbl_get(BB->Successors, I);
      if (!bbl_contains(Visited, Succ)) {
        bbl_push_back(Stack, Succ);
      }
    }
  }

  bbl_free(Visited);
  bbl_free(Stack);
  return AllBBs;
}

static void findVariablesInDAG(EmitCtx *Ctx, struct OpDAGNode *Node,
                               int32_t *CurrentOffset) {
  if (!Node)
    return;

  if (Node->Type == OpDAGVP) {
    const char *VarName = Node->TerminalData.ValuePlace.PlaceName;
    assert(VarName);
    if (lookupSymbol(Ctx, VarName) == NULL) {
      Symbol *Sym =
          symbolDataCreateLocStack(VarName, *CurrentOffset + ReferenceByteSize,
                                   Ctx->CurrentScope, false);
      sl_push_back(Ctx->CurrentScope->Symbols, Sym);
      *CurrentOffset += ReferenceByteSize;
    }
  }

  for (size_t I = 0; I < dnl_count(Node->Operands); ++I) {
    struct OpDAGNode *Operand = dnl_get(Node->Operands, I);
    findVariablesInDAG(Ctx, Operand, CurrentOffset);
  }
}

// Firstly we traverse all basic block of function and
// allocate all locals on stack to avoid creating phi-nodes and stuff like this
// for mergin basic blocks.
// This function modifies EmitCtx->SymbolStack
static void allocateAllLocalsOnStack(EmitCtx *Ctx, struct BasicBlock *EntryBB) {
  int32_t CurrentOffset = 0;

  struct basic_block_l *AllBBs = getAllBasicBlocks(EntryBB);

  for (size_t BBIdx = 0; BBIdx < bbl_count(AllBBs); ++BBIdx) {
    struct BasicBlock *BB = bbl_get(AllBBs, BBIdx);

    for (size_t InstrIdx = 0; InstrIdx < dl_count(BB->Instructions); ++InstrIdx) {
      struct OpDAG *DAG = dl_get(BB->Instructions, InstrIdx);
      struct OpDAGNode *Root = DAG->Root;
      findVariablesInDAG(Ctx, Root, &CurrentOffset);
    }
  }

  Ctx->SPOffset = CurrentOffset;
}

static struct Function *getMainFunction(struct Module *M) {
  static const char *MainFuncName = "main";
  for (size_t I = 0; I < fl_count(M->Functions); ++I) {
    struct Function *FI = fl_get(M->Functions, I);
    if (strcmp(MainFuncName, FI->Name) == 0)
      return FI;
  }
  return NULL;
}

// Push all gpr to stack.
static void flushRegs(Symbol **SavedSymbol2Reg, EmitCtx *Ctx) {
  memset(SavedSymbol2Reg, 0, sizeof(Symbol *) * NGPRRegs);
  for (Register R = r0; R <= r7; R++)
    if (!isRegFree(R)) {
      Symbol *RegSym = getReg(R);
      assert(RegSym);
      SavedSymbol2Reg[R] = RegSym;

      codeBufferAddFormatted(Ctx->CodeBuff, "\tpush %s\n", register2str(R));
    }
}

// Pop all gpr from stack.
static void fetchRegs(Symbol **SavedSymbol2Reg, EmitCtx *Ctx) {
  for (int I = NGPRRegs - 1; I >= 0; I--)
    if (SavedSymbol2Reg[I]) {
      Symbol *SymbolRestore = SavedSymbol2Reg[I];
      codeBufferAddFormatted(Ctx->CodeBuff, "\tpop %s\n", register2str(I));
    }
}

// Emit passing argument reference to function via register.
static void emitFuncArgument(Symbol *Arg, Register ArgReg, EmitCtx *Ctx) {
  char ArgLocStr[128];
  symbolGetLocationStr(Arg, ArgLocStr);

  if (Arg->RefLoc == Imm)
    codeBufferAddFormatted(Ctx->CodeBuff, "\tst %s, %s\n",
                            register2str(ArgReg), ArgLocStr);
  else
    codeBufferAddFormatted(Ctx->CodeBuff, "\tmv %s, %s\n",
                            register2str(ArgReg), ArgLocStr);

  if (Arg->Tmp)
    free(Arg);
}

static void emitCall(struct OpDAGNode *DAGNode, EmitCtx *Ctx, bool ObjectCall) {
  Symbol *SavedSymbol2Reg[NGPRRegs];
  flushRegs(SavedSymbol2Reg, Ctx);

  size_t NArgs = sl_count(Ctx->OperandList);
  if (ObjectCall) ++NArgs;
  if (NArgs > NGPRRegs) {
    fprintf(stderr, "Too many arguments for function call (max %d)\n",
            NGPRRegs);
    abort();
  }

  if (ObjectCall) {
    Symbol *ObjectSym = lookupSymbol(Ctx, DAGNode->TerminalData.ObjectCall.ObjectName);
    emitFuncArgument(ObjectSym, r0, Ctx);
  }

  // Emit load arguments into registers (r0[r1] is the first argument)
  size_t StartReg = ObjectCall ? 1 : 0;
  for (size_t I = StartReg; I < NArgs; I++) {
    Register ArgReg = (Register)I;
    Symbol *Arg = sl_back(Ctx->OperandList);
    assert(sl_pop_back(Ctx->OperandList));
    emitFuncArgument(Arg, ArgReg, Ctx);
  }

  char MangledFuncName[2048];
  mangle(MangledFuncName, DAGNode->TerminalData.ObjectCall.FuncName, NArgs);
  if (ObjectCall) {
    uint64_t hashFuncName = hash(MangledFuncName);
    // If emitting vcall then object symbol is in r0 register already.
    codeBufferAddFormatted(Ctx->CodeBuff, "\tvcall r0, 0x%016zx\n", hashFuncName);
  } else { 
    codeBufferAddFormatted(Ctx->CodeBuff, "\tcall %s\n", MangledFuncName);
  }

  // Emit restore registers from stack in reverse order
  fetchRegs(SavedSymbol2Reg, Ctx);

  Symbol *ResultSym = symbolDataCreateLocReg(NULL, rr, Ctx->CurrentScope, true);
  sl_push_back(Ctx->SymbolStack, ResultSym);
}

static void emitDAG(struct OpDAGNode *DAGNode, EmitCtx *Ctx) {
  // Probably WA, here is special handling of assign to object field and to array. 
  if (DAGNode->Type == OpDAGAssign) {
    struct OpDAGNode *LHS = dnl_get(DAGNode->Operands, 0);
    struct OpDAGNode *RHS = dnl_get(DAGNode->Operands, 1);
    
    if (LHS->Type == OpDAGFieldAccess) {
      // Emit rhs of asignment
      emitDAG(RHS, Ctx);
      Symbol *Src = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));

      const char *ObjName = LHS->TerminalData.FieldAccess.ObjectName;
      const char *FieldName = LHS->TerminalData.FieldAccess.FieldName;
      uint64_t FieldNameHash = hash(FieldName);
      
      Symbol *ObjSym = lookupSymbol(Ctx, ObjName);
      if (!ObjSym) {
        fprintf(stderr, "Object '%s' not found\n", ObjName);
        abort();
      }
      
      char ObjLocStr[128], SrcLocStr[128];
      symbolGetLocationStr(ObjSym, ObjLocStr);
      symbolGetLocationStr(Src, SrcLocStr);
      
      codeBufferAddFormatted(Ctx->CodeBuff, "\tstf %s, %s, 0x%016zx\n",
          ObjLocStr, SrcLocStr, FieldNameHash);
      
      if (Src->Tmp) symbolFree(Src);
      return;
    } else if (LHS->Type == OpDAGIndexer) {
      // Emit value place for indexed value place, it can be class field as well
      emitDAG(dnl_get(LHS->Operands, 0), Ctx);
      Symbol *ArrSym = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));

      // Emit symbols fot indexes. WA: only one-dimentional array supported
      emitDAG(dnl_get(LHS->Operands, 1), Ctx);
      Symbol *IndexSym = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));

      // Emit rhs of asignment
      emitDAG(RHS, Ctx);
      Symbol *SrcSym = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));

      char ArrLocStr[128], IndxLocStr[128], SrcLocStr[128];
      symbolGetLocationStr(ArrSym, ArrLocStr);
      symbolGetLocationStr(SrcSym, SrcLocStr);
      symbolGetLocationStr(IndexSym, IndxLocStr);
      
      codeBufferAddFormatted(Ctx->CodeBuff, "\tars %s, %s, %s\n",
          ArrLocStr, IndxLocStr, SrcLocStr);
      
      if (SrcSym->Tmp) symbolFree(SrcSym);
      if (IndexSym->Tmp) symbolFree(IndexSym);
      return;
    }
  }

  for (size_t I = 0; I < dnl_count(DAGNode->Operands); ++I)
    emitDAG(dnl_get(DAGNode->Operands, I), Ctx);

  switch (DAGNode->Type) {
  case OpDAGAssign: {
    Symbol *Src = sl_back(Ctx->SymbolStack);
    assert(sl_pop_back(Ctx->SymbolStack));
    Symbol *Dst = sl_back(Ctx->SymbolStack);
    assert(sl_pop_back(Ctx->SymbolStack));
    assert(Src->RefLoc != NoSpec);

    char SrcLocStr[128];
    symbolGetLocationStr(Src, SrcLocStr);

    switch (Dst->RefLoc) {
    case Reg: {
      char DstLocStr[128];
      symbolGetLocationStr(Dst, DstLocStr);
      codeBufferAddFormatted(Ctx->CodeBuff, "\tmv %s, %s\n", DstLocStr,
                             SrcLocStr);

      if (Src->Tmp && Src->RefLoc == Reg)
        freeReg(Src->Reg);
      setRegister(Dst->Reg, Src);
      break;
    }
    case Imm: {
      UNREACHABLE("Store to imm ref NYI");
    }
    case Stack: {
      char DstLocStr[128];
      symbolGetLocationStr(Dst, DstLocStr);
      codeBufferAddFormatted(Ctx->CodeBuff, "\tmv %s, %s\n", DstLocStr,
                             SrcLocStr);
      if (Src->Tmp && Src->RefLoc == Reg)
        freeReg(Src->Reg);
      break;
    }
    case NoSpec: {
      // We are assign value to some symbol with no location, so
      // we will get location of stored symbol in some register which
      // will be associated with the Dst symbol reference.
      // Location of Dst symbol are goinf to be changed.
      Register DstLocReg = allocRegister(Ctx);
      codeBufferAddFormatted(Ctx->CodeBuff, "\tst %s, %s\n",
                             register2str(DstLocReg), SrcLocStr);
      if (Src->Tmp && Src->RefLoc == Reg)
        freeReg(Src->Reg);
      setRegister(DstLocReg, Src);
      break;
    }
    default:
      UNREACHABLE("Unknown dst assign location");
    }

    break;
  }
  case OpDAGVP: {
    const char *ValuePlaceName = DAGNode->TerminalData.ValuePlace.PlaceName;
    Symbol *Sym = lookupSymbol(Ctx, ValuePlaceName);
    // Creating new symbol if it was not declared before
    if (!Sym) {
      assert(false && "WA: now we allocate all locals on stack before generating code for function");
    }
    sl_push_back(Ctx->SymbolStack, Sym);
    break;
  }
  case OpDAGFieldAccess: {
    const char *ObjName = DAGNode->TerminalData.FieldAccess.ObjectName;
    const char *FieldName = DAGNode->TerminalData.FieldAccess.FieldName;
    uint64_t FieldNameHash = hash(FieldName);
    
    Symbol *ObjSym = lookupSymbol(Ctx, ObjName);
    if (!ObjSym) {
      fprintf(stderr, "Object '%s' not found for field access\n", ObjName);
      abort();
    }
    
    char ObjLocStr[128];
    symbolGetLocationStr(ObjSym, ObjLocStr);
    
    Register DestReg = allocRegister(Ctx);

    codeBufferAddFormatted(Ctx->CodeBuff, "\tldf %s, %s, 0x%016zx\n", 
        register2str(DestReg), ObjLocStr, FieldNameHash);
    
    Symbol *ResultSym = symbolDataCreateLocReg(NULL, DestReg, 
                                               Ctx->CurrentScope, true);
    setRegister(DestReg, ResultSym);
    sl_push_back(Ctx->SymbolStack, ResultSym);
    break;
  }
  case OpDAGObjectCall: {
    emitCall(DAGNode, Ctx, true);
    break;
  }
  case OpDAGCall: {
    emitCall(DAGNode, Ctx, false);
    break;
  }
  case OpDAGIndexer: {
    // TODO only one dimentional index implemented.
    // Emit value place for indexed value place, it can be class field as well
    // Emit symbols fot indexes. WA: only one-dimentional array supported
    Symbol *IndexSym = sl_back(Ctx->SymbolStack);
    assert(sl_pop_back(Ctx->SymbolStack));
    Symbol *ArrSym = sl_back(Ctx->SymbolStack);
    assert(sl_pop_back(Ctx->SymbolStack));

    char ArrLocStr[128], IndexLocStr[128];
    symbolGetLocationStr(ArrSym, ArrLocStr);
    symbolGetLocationStr(IndexSym, IndexLocStr);

    Register DestReg = allocRegister(Ctx);

    codeBufferAddFormatted(Ctx->CodeBuff, "\targ %s, %s, %s\n", 
        register2str(DestReg), ArrLocStr, IndexLocStr);
    
    Symbol *ResultSym = symbolDataCreateLocReg(NULL, DestReg, 
                                               Ctx->CurrentScope, true);
    setRegister(DestReg, ResultSym);
    sl_push_back(Ctx->SymbolStack, ResultSym);

    break;
  }
  case OpDAGList: {
    for (size_t I = 0; I < dnl_count(DAGNode->Operands); ++I) {
      Symbol *S = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));
      sl_push_back(Ctx->OperandList, S);
    }
    break;
  }
  case OpDAGNeg: { // neg
    MakeUnOp(Ctx, neg);
    break;
  }
  case OpDAGInc: { // inc
    assert(false && "NYI");
    break;
  }
  case OpDAGDec: { // dec
    assert(false && "NYI");
    break;
  }
  case OpDAGNot: { // lnot
    MakeUnOp(Ctx, lnot);
    break;
  }
  case OpDAGBitNot: { // bnot
    MakeUnOp(Ctx, bnot);
    break;
  }
  case OpDAGAdd: {
    MakeBinOp(Ctx, add);
    break;
  }
  case OpDAGSub: {
    MakeBinOp(Ctx, sub);
    break;
  }
  case OpDAGMul: {
    MakeBinOp(Ctx, mul);
    break;
  }
  case OpDAGDiv: {
    MakeBinOp(Ctx, div);
    break;
  }
  case OpDAGMod: {
    MakeBinOp(Ctx, mod);
    break;
  }
  case OpDAGEq: {
    MakeBinOp(Ctx, eq);
    break;
  }
  case OpDAGNeq: {
    MakeBinOp(Ctx, neq);
    break;
  }
  case OpDAGLt: {
    MakeBinOp(Ctx, lt);
    break;
  }
  case OpDAGLte: {
    MakeBinOp(Ctx, lte);
    break;
  }
  case OpDAGGt: {
    MakeBinOp(Ctx, gt);
    break;
  }
  case OpDAGGte: {
    MakeBinOp(Ctx, gte);
    break;
  }
  case OpDAGAnd: {
    MakeBinOp(Ctx, land);
    break;
  }
  case OpDAGOr: {
    MakeBinOp(Ctx, lor);
    break;
  }
  case OpDAGBitAnd: {
    MakeBinOp(Ctx, band);
    break;
  }
  case OpDAGBitOr: {
    MakeBinOp(Ctx, bor);
    break;
  }
  case OpDAGBitXor: {
    MakeBinOp(Ctx, bxor);
    break;
  }
  case OpDAGBitShl: {
    MakeBinOp(Ctx, bshl);
    break;
  }
  case OpDAGBitShr: {
    MakeBinOp(Ctx, bshr);
    break;
  }
  case OpDAGLiteralStr: {
    assert(false && "NYI");
    break;
  }
  case OpDAGLiteralByte: {
    uint64_t Ref = MakeRef(TYPE_CHAR, F_IMM, DAGNode->TerminalData.Literal.Char);
    Symbol *Sym = symbolDataCreateLocImm(NULL, Ref, Ctx->CurrentScope, true);
    sl_push_back(Ctx->SymbolStack, Sym);
    break;
  }
  case OpDAGLiteralDoubleWord: {
    uint32_t Val = DAGNode->TerminalData.Literal.UInt;
    int Type = DAGNode->TerminalData.Literal.Signed ? TYPE_INT : TYPE_UINT;
    uint64_t Ref = MakeRef(Type, F_IMM, Val);
    Symbol *Sym = symbolDataCreateLocImm(NULL, Ref, Ctx->CurrentScope, true);
    sl_push_back(Ctx->SymbolStack, Sym);
    break;
  }
  case OpDAGLiteralBool: {
    bool Val = DAGNode->TerminalData.Literal.Bool;

    Symbol *Sym = symbolDataCreateLocImm(NULL, MakeRef(TYPE_BOOL, F_IMM, Val),
                                         Ctx->CurrentScope, true);
    sl_push_back(Ctx->SymbolStack, Sym);
    break;
  }
  case OpDAGCreateArray: {
    size_t SizeOf = DAGNode->TerminalData.CreateArray.SizeOf;
    Register DestReg = allocRegister(Ctx);

    // Emit creationg of array with storing reference to DestReg
    char SizeOfBuff[128];
    uint642Binary(MakeRef(TYPE_ULONG, F_IMM, SizeOf), SizeOfBuff);
    codeBufferAddFormatted(Ctx->CodeBuff, "\tarn %s, %s\n", register2str(DestReg), SizeOfBuff);

    Symbol *Sym = symbolDataCreateLocReg(NULL, DestReg, Ctx->CurrentScope, true);
    sl_push_back(Ctx->SymbolStack, Sym);
    break;
  }
  default:
    UNREACHABLE("Unknown dag node type");
  }
}

static void emitExpression(struct OpDAG *DAG, EmitCtx *Ctx) {
  emitDAG(DAG->Root, Ctx);
}

static void emitLabel(struct BasicBlock *BB, EmitCtx *Ctx) {
  codeBufferAddFormatted(Ctx->CodeBuff, ".%s%ld:\n", BB->Label, BB->Id);
}

static void emitBasicBlocksStartsWith(const struct Function *F, struct BasicBlock *BB, EmitCtx *Ctx) {
  emitLabel(BB, Ctx);

  struct dag_l *Instructions = BB->Instructions;
  for (size_t I = 0; I < dl_count(Instructions); ++I)
    emitExpression(dl_get(Instructions, I), Ctx);

  switch (BB->Type) {
  case BBTypeUncondBranch: {
    codeBufferAddFormatted(Ctx->CodeBuff, "\tjmp .%s%ld\n",
                           BB->Flow.Uncond.Target->Label,
                           BB->Flow.Uncond.Target->Id);
    break;
  }
  case BBTypeCondBranch: {
    char BrCondLoc[128];
    Symbol *BrCond = sl_back(Ctx->SymbolStack);
    assert(sl_pop_back(Ctx->SymbolStack));
    symbolGetLocationStr(BrCond, BrCondLoc);
    // WA: after update of RemoteTasks got error in pdsl, 
    // see pdsl for more info
    if (BrCond->RefLoc == Imm)
      codeBufferAddFormatted(
          Ctx->CodeBuff, "\tibr %s, .%s%ld, .%s%ld\n", BrCondLoc,
          BB->Flow.Cond.IfTrue->Label, BB->Flow.Cond.IfTrue->Id,
          BB->Flow.Cond.IfFalse->Label, BB->Flow.Cond.IfFalse->Id);
    else 
      codeBufferAddFormatted(
          Ctx->CodeBuff, "\tbr %s, .%s%ld, .%s%ld\n", BrCondLoc,
          BB->Flow.Cond.IfTrue->Label, BB->Flow.Cond.IfTrue->Id,
          BB->Flow.Cond.IfFalse->Label, BB->Flow.Cond.IfFalse->Id);
    break;
  }
  case BBTypeRet: {
    if (F->IsConstructor) {
      // In return block from constructor we always return symbol for "this"
      bool ThisFinded = false;
      for (size_t I = 0; I < sl_count(Ctx->SymbolStack); ++I) {
        Symbol *S = sl_get(Ctx->SymbolStack, I);
        if (strcmp(S->Name, "this") == 0) {
          char RetSymLocStr[128];
          symbolGetLocationStr(S, RetSymLocStr);
          codeBufferAddFormatted(Ctx->CodeBuff, "\tmv rr, %s\n", RetSymLocStr);
          ThisFinded = true;
          break;
        }
      }
      assert(ThisFinded);
    } else if (!sl_empty(Ctx->SymbolStack)) {
      Symbol *RetSym = sl_back(Ctx->SymbolStack);
      assert(sl_pop_back(Ctx->SymbolStack));
      char RetSymLocStr[128];
      symbolGetLocationStr(RetSym, RetSymLocStr);
      codeBufferAddFormatted(Ctx->CodeBuff, "\tmv rr, %s\n", RetSymLocStr);
    }
    codeBufferAddFormatted(Ctx->CodeBuff, "\tret\n");
    // If this block in returning, then we cannot use symbol defined here
    // in other basic block (they are unreacheable)
    sl_clear(Ctx->SymbolStack);
    break;
  }
  default:
    UNREACHABLE();
  }

  bbl_push_back(Ctx->VisitedBBs, BB);

  struct basic_block_l *SuccBBs = BB->Successors;
  for (size_t I = 0; I < bbl_count(SuccBBs); ++I) {
    struct BasicBlock *NextBB = bbl_get(SuccBBs, I);
    if (!bbl_contains(Ctx->VisitedBBs, NextBB)) {
      for (int J = Ctx->NFuncArgs; J < NGPRRegs; ++J)
        freeReg(J);
      emitBasicBlocksStartsWith(F, NextBB, Ctx);
    }
  }
}

static void createFunctionArgSymbol(struct Argument *Arg, EmitCtx *Ctx, Register *Reg) {
  Symbol *SymForArg = symbolDataCreateLocReg(Arg->Name, *Reg, Ctx->CurrentScope, false);
  setRegister(*Reg, SymForArg);
  SymForArg->DefinedIn = Ctx->CurrentScope;
  sl_push_back(Ctx->CurrentScope->Symbols, SymForArg);
  (*Reg)++;
}

void *emitFunction(const struct Function *F, EmitCtx *Ctx) {
  // Create scope for the function
  scopePush(Ctx);
  assert(sl_empty(Ctx->CurrentScope->Symbols));

  // Create symbols for function input arguments
  Register RegForArg = r0;
  for (int I = 0; I < al_count(F->Args); ++I) {
    struct Argument *Arg = al_get(F->Args, I);
    createFunctionArgSymbol(Arg, Ctx, &RegForArg);
    assert(RegForArg <= r7 && "Passing arguments to function via stack NYI");
  }

  Ctx->NFuncArgs = RegForArg;

  allocateAllLocalsOnStack(Ctx, F->CFG->Entry);
  char UpdateSPStr[128];
  sprintf(UpdateSPStr, "%ld", Ctx->SPOffset);

  // Emit function which one is constructor.
  if (F->IsConstructor) {
    // Emit function constructor with parameter - creating object.
    // Constuctor consisnt of 2 functions: 
    // - one of them initialized object
    //   and accepts reference to object to initialize.
    // - second part calls first part and pass reference to object created
    //   via new instruction.
    assert(F->Class);
    // Emit "initializing part" of constructor.
    struct Symbol *ThisSym = symbolDataCreateLocReg("this", r0, Ctx->CurrentScope, false);
    sl_push_back(Ctx->SymbolStack, ThisSym);
    codeBufferAddFormatted(Ctx->CodeBuff, "__init_%s:\n", F->Name);
    codeBufferAddFormatted(Ctx->CodeBuff, "\tusp %s\n", UpdateSPStr);
    emitBasicBlocksStartsWith(F, F->CFG->Entry, Ctx);
    // Emit "new part" of constuctor.
    codeBufferAddFormatted(Ctx->CodeBuff, "%s__%ld:\n", F->Name, al_count(F->Args)-1);
    codeBufferAddFormatted(Ctx->CodeBuff, "\tnew r0, %d\n", F->Class->Id);
    codeBufferAddFormatted(Ctx->CodeBuff, "\tcall __init_%s\n", F->Name);
    struct ClassType *Base = F->Class->BaseClass;
    while (Base) {
      codeBufferAddFormatted(Ctx->CodeBuff, "\tcall __init_%s\n", Base->Name);
      Base = Base->BaseClass;
    }
    codeBufferAddFormatted(Ctx->CodeBuff, "\tmv rr, r0\n");
    codeBufferAddFormatted(Ctx->CodeBuff, "\tret\n");
  } else {
    // Emit non constructor function body.
    size_t NArgs = al_count(F->Args);
    char MangledFuncName[2048];
    if (F->Class) {
      mangleClass(MangledFuncName, F->Name, F->Class->Name, NArgs);
    } else {
      mangle(MangledFuncName, F->Name, NArgs);
    }
    codeBufferAddFormatted(Ctx->CodeBuff, "%s:\n", MangledFuncName);

    codeBufferAddFormatted(Ctx->CodeBuff, "\tusp %s\n", UpdateSPStr);
    emitBasicBlocksStartsWith(F, F->CFG->Entry, Ctx);
  }

  // Leave function scope
  scopePop(Ctx);
  emitCtxReset(Ctx);
  freeAllGPR();

  codeBufferAddLine(Ctx->CodeBuff, "");
}

static void emitClassMetadata(struct Module *M, EmitCtx *Ctx) {
  codeBufferAddLine(Ctx->DataBuff, "");
  codeBufferAddLine(Ctx->DataBuff, "; ========================================");
  codeBufferAddLine(Ctx->DataBuff, "; CLASS METADATA");
  codeBufferAddLine(Ctx->DataBuff, "");
  
  uint8_t MaxClassId = 0;
  for (size_t I = 0; I < cltl_count(M->Classes); I++) {
    struct ClassType *Class = cltl_get(M->Classes, I);
    if (Class->Id > MaxClassId) MaxClassId = Class->Id;
  }
  
  codeBufferAddLine(Ctx->DataBuff, "; Class pointer table (indexed by class_id)");
  codeBufferAddLine(Ctx->DataBuff, "class_table:");
  
  for (int Id = 0; Id <= MaxClassId; Id++) {
    int found = 0;
    for (size_t I = 0; I < cltl_count(M->Classes); I++) {
      struct ClassType *Class = cltl_get(M->Classes, I);
      if (Class->Id == Id) {
        codeBufferAddFormatted(Ctx->DataBuff, 
          "\tdq .class_entry_%s  ; [%d] %s\n", 
          Class->Name, Id, Class->Name);
        found = 1;
        break;
      }
    }
    if (!found) {
      codeBufferAddFormatted(Ctx->DataBuff, "\tdq 0  ; [%d] empty\n", Id);
    }
  }
  codeBufferAddLine(Ctx->DataBuff, "");

  // CLASS ENTRIES
  codeBufferAddLine(Ctx->DataBuff, "; Class entries");
  for (size_t I = 0; I < cltl_count(M->Classes); I++) {
    struct ClassType *Class = cltl_get(M->Classes, I);
    
    codeBufferAddFormatted(Ctx->DataBuff, ".class_entry_%s:\n", Class->Name);
    
    // +0: class_id (1 byte)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdb 0x%02x  ; +0: class_id\n", Class->Id);
    // +1: base_class_id (1 byte)
    int BaseClassId = Class->BaseClass ? Class->BaseClass->Id : 0xFF;
    codeBufferAddFormatted(Ctx->DataBuff, "\tdb 0x%02x  ; +1: base_class_id\n", BaseClassId);
    // +2: num_interfaces (4 bytes)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdd %d  ; +2: num_interfaces\n", 
      (int)intfl_count(Class->Interfaces));
    // +6: num_fields (4 bytes)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdd %d  ; +6: num_fields\n", 
      (int)fldl_count(Class->Fields));
    // +10: num_methods (4 bytes)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdd %d  ; +10: num_methods\n", 
      (int)methl_count(Class->Methods));
    // +14: fields_ptr (8 byte)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdq .class_fields_%s  ; +14: fields_ptr\n", 
      Class->Name);
    // +22: methods_ptr (8 byte)
    codeBufferAddFormatted(Ctx->DataBuff, "\tdq .class_methods_%s  ; +22: methods_ptr\n", 
      Class->Name);
    // +30
    
    codeBufferAddLine(Ctx->DataBuff, "");
  }

  // FIELDS ARRAYS
  codeBufferAddLine(Ctx->DataBuff, "; Fields arrays (hash, global_offset)");
  codeBufferAddLine(Ctx->DataBuff, "; field_entry: hash(8) + offset(4) = 12 bytes");
  for (size_t I = 0; I < cltl_count(M->Classes); I++) {
    struct ClassType *Class = cltl_get(M->Classes, I);
    
    size_t baseOffset = 0;
    struct ClassType *current = Class->BaseClass;
    while (current) {
      baseOffset += fldl_count(current->Fields) * ReferenceByteSize;
      current = current->BaseClass;
    }
    
    codeBufferAddFormatted(Ctx->DataBuff, ".class_fields_%s:\n", Class->Name);
    if (fldl_count(Class->Fields) == 0) {
      codeBufferAddLine(Ctx->DataBuff, "\t; (no fields)");
    }
    for (size_t J = 0; J < fldl_count(Class->Fields); J++) {
      struct Field *F = fldl_get(Class->Fields, J);
      size_t fieldHash = hash(F->Name);

      size_t globalOffset = baseOffset + F->Offset;

      codeBufferAddFormatted(Ctx->DataBuff, 
          "\tdq 0x%016zx  ; hash of '%s'\n", fieldHash, F->Name);
      codeBufferAddFormatted(Ctx->DataBuff, 
          "\tdd %zu  ; offset (global)\n", globalOffset);
    }
    codeBufferAddLine(Ctx->DataBuff, "");
  }

  // METHODS ARRAYS
  codeBufferAddLine(Ctx->DataBuff, "; Methods arrays (hash, function_ptr)");
  codeBufferAddLine(Ctx->DataBuff, "; method_entry: hash(8) + function_ptr(8) = 16 bytes");
  for (size_t I = 0; I < cltl_count(M->Classes); I++) {
    struct ClassType *Class = cltl_get(M->Classes, I);
    
    codeBufferAddFormatted(Ctx->DataBuff, ".class_methods_%s:\n", Class->Name);
    if (methl_count(Class->Methods) == 0) {
      codeBufferAddLine(Ctx->DataBuff, "\t; (no methods)");
    }
    for (size_t J = 0; J < methl_count(Class->Methods); J++) {
      struct Method *M = methl_get(Class->Methods, J);
      char MangledFuncName[2048];
      char MangledMethodName[2048];
      mangle(MangledFuncName, M->Function->Name, al_count(M->Function->Args));
      size_t methodHash = hash(MangledFuncName);
      mangleClass(MangledMethodName, M->Function->Name, Class->Name, al_count(M->Function->Args));

      // method_entry: hash(8), function_ptr(8), is_public(4) = 20 bytes
      codeBufferAddFormatted(Ctx->DataBuff, 
        "\tdq 0x%016zx  ; hash of '%s'\n", methodHash, MangledFuncName);
      codeBufferAddFormatted(Ctx->DataBuff, 
        "\tdq %s  ; function_ptr\n", MangledMethodName);
      codeBufferAddFormatted(Ctx->DataBuff,
        "\tdd %d  ; visibility == public\n", (M->Visibility == MemberVisibilityPublic)
      );
    }
    codeBufferAddLine(Ctx->DataBuff, "");
  }
  
  codeBufferAddLine(Ctx->DataBuff, "; END OF CLASS METADATA");
  codeBufferAddLine(Ctx->DataBuff, "; ========================================");
  codeBufferAddLine(Ctx->DataBuff, "");
}

int generateArcode(struct Module *M, const char *OutAsmFileName) {
  printf("Emitting code to %s file\n", OutAsmFileName);
  FILE *OutF = fopen(OutAsmFileName, "w");

  EmitCtx *Ctx = emitCtxCreate();
  
  codeBufferAddLine(Ctx->DataBuff, "[section datas]");

  codeBufferAddLine(Ctx->CodeBuff, "[section code]");
  codeBufferAddLine(Ctx->CodeBuff, "start:");
  codeBufferAddLine(Ctx->CodeBuff, "\treset");
  codeBufferAddLine(Ctx->CodeBuff, "\tcall __init_timer");
  codeBufferAddLine(Ctx->CodeBuff, "\tcall __init_pic");
  codeBufferAddLine(Ctx->CodeBuff, "\tcall main__0");
  codeBufferAddLine(Ctx->CodeBuff, "\thlt");
  codeBufferAddLine(Ctx->CodeBuff, "");

  emitClassMetadata(M, Ctx);

  // Create execution symbols for each function.
  for (size_t I = 0; I < fl_count(M->Functions); ++I) {
    struct Function *F = fl_get(M->Functions, I);
    Symbol *FSym = symbolExeCreate(F->Name, /* Tmp */ false);
    sl_push_back(Ctx->CurrentScope->Symbols, FSym);
  }
  // Emit each function in module
  for (size_t I = 0; I < fl_count(M->Functions); ++I) {
    struct Function *F = fl_get(M->Functions, I);
    if (F->CFG)
      emitFunction(F, Ctx);
  }
  codeBufferDumpToFile(Ctx->DataBuff, OutF);
  codeBufferDumpToFile(Ctx->CodeBuff, OutF);
  
  fclose(OutF);
  emitCtxFree(Ctx);

  return 0;
}
