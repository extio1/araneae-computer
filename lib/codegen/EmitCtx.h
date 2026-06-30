#ifndef ARANEAE_LIB_CODEGEN_EMITCTX_H
#define ARANEAE_LIB_CODEGEN_EMITCTX_H

#include "araneae/aux/Aux.h"
#include "araneae/aux/CodeBuffer.h"

#include "araneae/cfg/Cfg.h"

struct Scope;
struct scope_l;
struct symbol_l;

typedef struct EmitCtx {
  struct symbol_l *SymbolStack;

  struct basic_block_l *VisitedBBs;

  CodeBuffer *CodeBuff;
  CodeBuffer *DataBuff;

  size_t SPOffset;

  struct Scope *CurrentScope;
  struct scope_l *ScopeStack;

  size_t NFuncArgs;

  // For emitting call instr
  struct symbol_l *OperandList;
} EmitCtx;

EmitCtx *emitCtxCreate();
void emitCtxReset(EmitCtx *Ctx);
void emitCtxFree(EmitCtx *Ctx);

#endif /* ARANEAE_LIB_CODEGEN_EMITCTX_H */