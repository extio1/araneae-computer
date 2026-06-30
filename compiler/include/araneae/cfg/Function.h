#ifndef ARANEAE_CFG_FUNCTION_H
#define ARANEAE_CFG_FUNCTION_H

#include "Cfg.h"

#include <stddef.h>
#include <stdio.h>

struct Type;

struct argument_l;
struct Argument {
  // Name of the argument.
  const char *Name;
};
struct argument_l_fval;
struct argument_l *al_new(size_t Capacity, struct argument_l_fval *);
void al_free(struct argument_l *L);
bool al_push_back(struct argument_l *L, struct Argument *V);
bool al_push_front(struct argument_l *L, struct Argument *V);
size_t al_count(struct argument_l *L);
struct Argument *al_get(struct argument_l *L, size_t I);
bool al_pop_back(struct argument_l *);
struct Argument *al_back(struct argument_l *);

void argumentDump(struct Argument *A);
bool argumentPrint(FILE *OS, struct Argument *DAG);
struct Argument *argumentCreate(const char *Name);
struct Argument *argumentCreateEmpty();

struct function_l;
struct Function {
  enum { 
    FuncDecl, 
    FuncDef, 
    MethDecl, 
    MethDef
  } Type;
  // Name of the function.
  char *Name;
  // Arguments of the function.
  struct argument_l *Args;
  // CFG for the function body.
  struct CFG *CFG;
  // Pretty name of the function with arguments (for printing).
  char *SignatureText;
  // Is this function part of object.
  struct ClassType *Class;
  // Is this function constructor for some object.
  bool IsConstructor;
};
struct function_l_fval;
struct function_l *fl_new(size_t Capacity, struct function_l_fval *);
void fl_free(struct function_l *L);
bool fl_push_back(struct function_l *L, struct Function *V);
size_t fl_count(struct function_l *L);
struct Function *fl_get(struct function_l *L, size_t I);
bool fl_pop_back(struct function_l *);
struct Function *fl_back(struct function_l *);

int functionEq(struct Function *Arg1, struct Function *Arg2);
bool functionPrint(FILE *OS, struct Function *F);
void functionDump(struct Function *F);
struct Function *functionCreateEmpty();

void exportFunctionToDot(struct Function *F, const char *Outf);

#endif