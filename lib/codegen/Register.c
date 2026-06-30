#include "Register.h"

#include "EmitCtx.h"
#include "Symbol.h"

#include "araneae/abi/Reference.h"
#include "araneae/abi/Types.h"
#include "araneae/aux/Aux.h"

#include <stdbool.h>
#include <stdlib.h>

const char *register2str(Register Reg) {
  switch (Reg) {
  case r0:
    return "r0";
    break;
  case r1:
    return "r1";
    break;
  case r2:
    return "r2";
    break;
  case r3:
    return "r3";
    break;
  case r4:
    return "r4";
    break;
  case r5:
    return "r5";
    break;
  case r6:
    return "r6";
    break;
  case r7:
    return "r7";
    break;
  case rr:
    return "rr";
    break;
  default:
    UNREACHABLE("unknown register");
  }
}

#define NRegs 9
static Symbol *regs2symbol[NRegs] = {NULL, NULL, NULL, NULL, NULL,
                                     NULL, NULL, NULL, NULL};

void movSymToStack(Symbol *Sym, EmitCtx *Ctx) {
  Ctx->SPOffset += sizeof(ReferenceT);

  char SymLocStr[128];
  symbolGetLocationStr(Sym, SymLocStr);
  codeBufferAddFormatted(Ctx->CodeBuff, "\tpush %s\n", SymLocStr);

  regs2symbol[Sym->Reg] = NULL;

  Sym->RefLoc = Stack;
}

void movRegToStack(Register R, EmitCtx *Ctx) {
  Symbol *Sym = regs2symbol[R];

  Ctx->SPOffset += sizeof(ReferenceT);

  codeBufferAddFormatted(Ctx->CodeBuff, "\tpush %s\n", register2str(R));

  regs2symbol[R] = NULL;

  Sym->RefLoc = Stack;
  Sym->SPOffset = Ctx->SPOffset;
}

void movSymStackToReg(Register R, Symbol *Sym, EmitCtx *Ctx) {
  codeBufferAddFormatted(Ctx->CodeBuff, "\tpop %s\n", register2str(R));

  regs2symbol[R] = Sym;

  Sym->RefLoc = Reg;
  Sym->Reg = R;

  Ctx->SPOffset -= sizeof(ReferenceT);
}

void movToReg(Register R, Symbol *Sym, EmitCtx *Ctx) {
  // here we can override and lost value in register if there was someting
  char SymLocStr[128];
  symbolGetLocationStr(Sym, SymLocStr);
  codeBufferAddFormatted(Ctx->CodeBuff, "\tmv %s, %s\n", register2str(R),
                         SymLocStr);

  regs2symbol[R] = Sym;
  Sym->RefLoc = Reg;
  Sym->Reg = R;
}

void movSymToSym(Symbol *SymDst, Symbol *SymSrc, EmitCtx *Ctx) {
  assert(SymDst->RefLoc != Imm && SymDst->RefLoc != NoSpec);
  char SymSrcLocStr[128], SymDstLocStr[128];
  symbolGetLocationStr(SymSrc, SymSrcLocStr);
  symbolGetLocationStr(SymDst, SymDstLocStr);

  codeBufferAddFormatted(Ctx->CodeBuff, "\tmv %s, %s\n", SymDstLocStr,
                         SymSrcLocStr);
}

void store(Register ResRef, Symbol *SymDst, Symbol *SymSrc, EmitCtx *Ctx) {
  char SymSrcLocStr[128];
  symbolGetLocationStr(SymSrc, SymSrcLocStr);
  codeBufferAddFormatted(Ctx->CodeBuff, "\tst %s, %s\n", register2str(ResRef),
                         SymSrcLocStr);

  SymDst->RefLoc = Reg;
  SymDst->Reg = ResRef;

  regs2symbol[ResRef] = SymDst;
}

bool isRegFree(Register Reg) { return (regs2symbol[Reg] == NULL); }

Symbol *getReg(Register R) {
  assert(regs2symbol[R]);
  assert(regs2symbol[R]->RefLoc == Reg && regs2symbol[R]->Reg == R);
  return regs2symbol[R];
}

void setRegister(Register RegForArg, Symbol *SymForArg) {
  regs2symbol[RegForArg] = SymForArg;
  SymForArg->RefLoc = Reg;
  SymForArg->Reg = RegForArg;
}

void spillRegisters(EmitCtx *Ctx) {
  for (size_t I = 0; I < NGPRRegs; ++I)
    movRegToStack((Register)I, Ctx);
}

Register allocRegister(EmitCtx *Ctx) {
  for (size_t I = 0; I < NGPRRegs; ++I)
    if (isRegFree(I))
      return (Register)I;
  spillRegisters(Ctx);
  allocRegister(Ctx);
}

void freeReg(Register Reg) { regs2symbol[Reg] = NULL; }

void freeAllGPR() {
  for (int I = 0; I < NGPRRegs; ++I)
    regs2symbol[I] = NULL;
}
