#include "Scope.h"

#include "Symbol.h"

#define V Scope *
#define PFX scl
#define SNAME scope_l
#include "cmc/list.h"

Scope *scopeCreate(Scope *Parent) {
  Scope *S = malloc(sizeof(Scope));
  S->Parent = Parent;
  S->Symbols = sl_new(16, NULL);
  return S;
}

void scopeFree(Scope *S) {
  for (size_t I = 0; I < sl_count(S->Symbols); ++I) {
    Symbol *Sym = sl_get(S->Symbols, I);
    symbolFree(Sym);
  }
  free(S);
}

void scopePush(EmitCtx *Ctx) {
  Scope *NewScope = scopeCreate(Ctx->CurrentScope);
  if (Ctx->CurrentScope)
    scl_push_back(Ctx->ScopeStack, Ctx->CurrentScope);
  Ctx->CurrentScope = NewScope;
}

void scopePop(EmitCtx *Ctx) {
  if (scl_count(Ctx->ScopeStack) > 0) {
    Scope *OldScope = Ctx->CurrentScope;
    Ctx->CurrentScope = scl_back(Ctx->ScopeStack);
    assert(scl_pop_back(Ctx->ScopeStack));
    scopeFree(OldScope);
  }
}

