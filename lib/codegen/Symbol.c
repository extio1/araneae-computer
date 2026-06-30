#include "Symbol.h"

#include "Register.h"
#include "araneae/abi/Reference.h"
#include "araneae/abi/Types.h"

#include <stddef.h>

#define V Symbol *
#define PFX sl
#define SNAME symbol_l
#include "cmc/list.h"

void symbolFree(Symbol *Sym) {
  if (Sym->RefLoc == Reg)
    freeReg(Sym->Reg);
  free(Sym);
}

Symbol *symbolExeCreate(const char *SymName, bool Tmp) {
  Symbol *S = malloc(sizeof(Symbol));
  S->SType = Executable;
  S->Name = SymName;
  S->Tmp = Tmp;
  return S;
}

Symbol *symbolDataCreateNoLoc(const char *SymName, Scope *DefScope, bool Tmp) {
  Symbol *S = malloc(sizeof(Symbol));
  S->SType = Data;
  S->RefLoc = NoSpec;
  S->Name = SymName;
  S->Ref = 0;
  S->Tmp = Tmp;
  S->DefinedIn = DefScope;
  return S;
}

Symbol *symbolDataCreateLocImm(const char *SymName, ReferenceT Ref,
                               Scope *DefScope, bool Tmp) {
  Symbol *S = malloc(sizeof(Symbol));
  S->SType = Data;
  S->RefLoc = Imm;
  S->Ref = Ref;
  S->Name = SymName;
  S->Tmp = Tmp;
  S->SPOffset = 0;
  S->Reg = r0;
  S->DefinedIn = DefScope;
  return S;
}

Symbol *symbolDataCreateLocStack(const char *SymName, size_t Offset,
                                 Scope *DefScope, bool Tmp) {
  Symbol *S = malloc(sizeof(Symbol));
  S->SType = Data;
  S->RefLoc = Stack;
  S->Name = SymName;
  S->SPOffset = Offset;
  S->Tmp = Tmp;
  S->DefinedIn = DefScope;
  return S;
}

Symbol *symbolDataCreateLocReg(const char *SymName, Register R, Scope *DefScope,
                               bool Tmp) {
  Symbol *S = malloc(sizeof(Symbol));
  S->SType = Data;
  S->RefLoc = Reg;
  S->Reg = R;
  S->Name = SymName;
  S->Tmp = Tmp;
  S->DefinedIn = DefScope;
  S->Ref = 0;
  return S;
}

void symbolGetLocationStr(Symbol *Sym, char *Buff) {
  switch (Sym->SType) {
  case Executable: {
    memcpy(Buff, Sym->Name, strlen(Sym->Name));
    break;
  }
  case Data: {
    switch (Sym->RefLoc) {
    case Imm: {
      uint642Binary(Sym->Ref, Buff);
      break;
    }
    case Reg: {
      strcpy(Buff, register2str(Sym->Reg));
      break;
    }
    case Stack: {
      sprintf(Buff, "fp[%ld]", Sym->SPOffset);
      break;
    }
    default:
      UNREACHABLE("Attempt to get locations of symbol with unknown location");
    }
    break;
  }
  default:
    UNREACHABLE("Unknown symbol type.")
  }
}

Symbol *lookupSymbolInScope(Scope *Scope, const char *SymName) {
  for (size_t I = 0; I < sl_count(Scope->Symbols); ++I) {
    Symbol *S = sl_get(Scope->Symbols, I);
    if (strcmp(S->Name, SymName) == 0)
      return S;
  }

  if (Scope->Parent)
    return lookupSymbolInScope(Scope->Parent, SymName);

  return NULL;
}

Symbol *lookupSymbol(EmitCtx *Ctx, const char *SymName) {
  Symbol *S = lookupSymbolInScope(Ctx->CurrentScope, SymName);
  if (S)
    return S;

  // TODO: if adding global variables, when here should be lookup by global
  // variables

  return NULL;
}

bool symbolPrint(FILE *OS, struct Symbol *S) {
  return fprintf(OS, "Symbol <%s>", S->Name) > 0;
}
