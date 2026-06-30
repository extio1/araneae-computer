#include "araneae/cfg/Function.h"

#include <assert.h>

#define V struct Argument *
#define PFX al
#define SNAME argument_l
#include "cmc/list.h"

#define V struct Function *
#define PFX fl
#define SNAME function_l
#include "cmc/list.h"

struct Argument *argumentCreate(const char *Name) {
  struct Argument *Arg = argumentCreateEmpty();
  Arg->Name = Name;
  return Arg;
}

struct Argument *argumentCreateEmpty() {
  struct Argument *Arg = (struct Argument *)malloc(sizeof(struct Argument));
  Arg->Name = NULL;
  return Arg;
}


bool argumentPrint(FILE *OS, struct Argument *A) {
  return fprintf(OS, "{argument %s of type:", A->Name);
}

void argumentDump(struct Argument *A) {
  argumentPrint(stdout, A);
}

struct Function *functionCreateEmpty() {
  struct Function *F = (struct Function *)(malloc(sizeof(struct Function)));
  F->Args = al_new(4, NULL);
  F->CFG = NULL;
  F->Type = MethDecl;
  F->SignatureText = NULL;
  F->Name = NULL;
  F->Class = NULL;
  F->IsConstructor = false;
}

bool functionPrint(FILE *OS, struct Function *F) {
  return fprintf(OS, "{function pretty \"%s\"}", F->SignatureText) > 0;
}

void functionDump(struct Function *F) {
  functionPrint(stdout, F);
}

void exportFunctionToDot(struct Function *F, const char *Outf) {
  char CfgFileNameBuff[2048];
  sprintf(CfgFileNameBuff, "%s/cfg-%s.dot", Outf, F->SignatureText);
  
  exportCFGToDot(F->CFG, CfgFileNameBuff);
}

int functionEq(struct Function *Arg1, struct Function *Arg2) {
  return strcmp(Arg1->SignatureText, Arg2->SignatureText);
}
