#ifndef ARANEAE_CFG_CFG_H
#define ARANEAE_CFG_CFG_H

#include "Dag.h"

#include "araneae/ast/Ast.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <bits/types/FILE.h>

struct CFG;

// Basic block of the code.
struct basic_block_l;
struct BasicBlock {
  // Id of this basic block.
  uint64_t Id;
  // Several types of basic blocks exist.
  enum {
    // Unconditional branch to the another basic block.
    BBTypeUncondBranch,
    // Conditional branch to the another basic block.
    // Should specify IfTrue and IfFalse basic blocks.
    BBTypeCondBranch,
    // Basic block - terminator.
    BBTypeRet
  } Type;
  // Label of this basic block. Usually consist of id and type.
  const char *Label;

  struct basic_block_l *Predecessors;
  struct basic_block_l *Successors;

  // Collection of DAGs for instructions
  struct dag_l *Instructions;

  // Data for each type of flow
  union {
    struct {
      struct BasicBlock *Target;
    } Uncond;
    struct {
      struct OpDAG *Condition;
      struct BasicBlock *IfTrue;
      struct BasicBlock *IfFalse;
    } Cond;
    struct {
      struct OpDAG *RetVal;
    } Ret;
  } Flow;
};
struct basic_block_l_fval;
struct basic_block_l *bbl_create(size_t Capacity);
void bbl_free(struct basic_block_l *L);
bool bbl_push_back(struct basic_block_l *L, struct BasicBlock *V);
size_t bbl_count(struct basic_block_l *L);
struct BasicBlock *bbl_get(struct basic_block_l *L, size_t I);
bool bbl_pop_back(struct basic_block_l *);
struct BasicBlock *bbl_back(struct basic_block_l *);
bool bbl_empty(struct basic_block_l *);
bool bbl_contains(struct basic_block_l *, struct BasicBlock *);

struct BasicBlock *basicBlockCreate(struct CFG *GFG, const char *Label);
int basicBlockEq(struct BasicBlock *C1, struct BasicBlock *C2);

// Control flow graph of the function.
struct CFG {
  // Entry of CFG. Exactly one entry can graph have.
  struct BasicBlock *Entry;
  // List of basic blocks inside this function.
  struct basic_block_l *Blocks;
  // List of returns from this function.
  struct dag_l *Returns;
};
struct CFG *CFGCreate();

void exportCFGToDot(struct CFG *CFG, const char *OutFile);

#endif /* ARANEAE_CFG_CFG_H */
