#ifndef ARANEAE_LIB_CODEGEN_SYMBOL_H
#define ARANEAE_LIB_CODEGEN_SYMBOL_H

#include "Register.h"
#include "Scope.h"
#include "Types.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct Symbol {
  const char *Name;
  bool Tmp;
  enum { Data, Executable, Argument } SType;
  // Location of the symbol - reference to the memory can be located
  // in immediate (then Ref field is valid) or
  // in a register (then Reg field is valid )
  enum RefLocation { NoSpec, Imm, Reg, Stack } RefLoc;
  ReferenceT Ref;
  Register Reg;
  size_t SPOffset;
  Scope *DefinedIn;
} Symbol;
struct symbol_l_fval;
struct symbol_l *sl_new(size_t Capacity, struct symbol_l_fval *);
void sl_free(struct symbol_l *L);
bool sl_push_back(struct symbol_l *L, Symbol *V);
size_t sl_count(struct symbol_l *L);
Symbol *sl_get(struct symbol_l *L, size_t I);
bool sl_pop_back(struct symbol_l *);
Symbol *sl_back(struct symbol_l *);
bool sl_empty(struct symbol_l *);
void sl_clear(struct symbol_l *);

Symbol *symbolExeCreate(const char *SymName, bool Tmp);
Symbol *symbolDataCreateNoLoc(const char *SymName, Scope *DefScope, bool Tmp);
Symbol *symbolDataCreateLocImm(const char *SymName, ReferenceT Ref,
                               Scope *DefScope, bool Tmp);
Symbol *symbolDataCreateLocStack(const char *SymName, size_t Offset,
                                 Scope *DefScope, bool Tmp);
Symbol *symbolDataCreateLocReg(const char *SymName, Register R, Scope *DefScope,
                               bool Tmp);
void symbolFree(Symbol *Sym);

void symbolGetLocationStr(Symbol *Sym, char *Buff);

Symbol *lookupSymbol(EmitCtx *Ctx, const char *SymName);

#endif /* ARANEAE_LIB_CODEGEN_SYMBOL_H */
