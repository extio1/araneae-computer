#ifndef ARANEAE_LIB_CODEGEN_SCOPE_H
#define ARANEAE_LIB_CODEGEN_SCOPE_H

#include "EmitCtx.h"

struct symbol_l;

typedef struct Scope {
  struct Scope *Parent;
  struct symbol_l *Symbols;
} Scope;
struct scope_l_fval;
struct scope_l *scl_new(size_t Capacity, struct scope_l_fval *);
void scl_free(struct scope_l *L);
bool scl_push_back(struct scope_l *L, Scope *V);
size_t scl_count(struct scope_l *L);
Scope *scl_get(struct scope_l *L, size_t I);
bool scl_pop_back(struct scope_l *);
Scope *scl_back(struct scope_l *);

struct Scope *scopeCreate(struct Scope *Parent);
void scopeFree(struct Scope *S);

void scopePush(EmitCtx *Ctx);
void scopePop(EmitCtx *Ctx);

#endif /* ARANEAE_LIB_CODEGEN_SCOPE_H */
