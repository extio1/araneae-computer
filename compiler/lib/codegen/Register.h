#ifndef ARANEAE_LIB_CODEGEN_REGISTER_H
#define ARANEAE_LIB_CODEGEN_REGISTER_H

#include "EmitCtx.h"
#include "araneae/aux/Aux.h"

#include <stdbool.h>

typedef struct Symbol Symbol;

#define NGPRRegs 8
typedef enum Register { r0, r1, r2, r3, r4, r5, r6, r7, rr, noreg } Register;

const char *register2str(Register Reg);

// void movSymToStack(Symbol *Sym, EmitCtx *Ctx);
// void movRegToStack(Register Reg, EmitCtx *Ctx);
// void movSymStackToReg(Register Reg, Symbol *Sym, EmitCtx *Ctx);
// void movToReg(Register R, Symbol *Sym, EmitCtx *Ctx);
// // Store SymSrc value to heap. ResRef will hold reference to it value and
// SymDst
// // updated to hold just stored value
// void store(Register ResRef, Symbol *SymDst, Symbol *SymSrc, EmitCtx *Ctx);
// // Copy reference to SymDst location from SymSrc location
// void movSymToSym(Symbol *SymDst, Symbol *SymSrc, EmitCtx *Ctx);

void spillRegisters(EmitCtx *Ctx);

bool isRegFree(Register Reg);

Symbol *getReg(Register Reg);
void setRegister(Register RegForArg, Symbol *SymForArg);

Register allocRegister(EmitCtx *Ctx);
void freeAllGPR();
void freeReg(Register Reg);

#endif /* ARANEAE_LIB_CODEGEN_REGISTER_H */