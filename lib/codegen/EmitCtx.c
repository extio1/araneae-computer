#include "EmitCtx.h"
#include "Scope.h"
#include "Symbol.h"

#include <stdlib.h>

EmitCtx *emitCtxCreate() {
  EmitCtx *Ctx = malloc(sizeof(EmitCtx));
  Ctx->CodeBuff = codeBufferCreate();
  Ctx->DataBuff = codeBufferCreate();
  Ctx->VisitedBBs = bbl_create(8);
  Ctx->OperandList = sl_new(8, NULL);
  Ctx->SymbolStack = sl_new(8, NULL);
  Ctx->ScopeStack = scl_new(8, NULL);
  Ctx->SPOffset = 0;
  Ctx->CurrentScope = scopeCreate(NULL);
  return Ctx;
}

void emitCtxReset(EmitCtx *Ctx) {
  sl_clear(Ctx->OperandList);
  sl_clear(Ctx->SymbolStack);
  Ctx->SPOffset = 0;
}

void emitCtxFree(EmitCtx *Ctx) {
  sl_free(Ctx->SymbolStack);
  codeBufferFree(Ctx->CodeBuff);
  codeBufferFree(Ctx->DataBuff);
  free(Ctx);
}
