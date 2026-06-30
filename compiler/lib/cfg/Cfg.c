#include "araneae/cfg/Cfg.h"

#include "araneae/aux/Aux.h"
#include "araneae/cfg/Dag.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

#define V struct BasicBlock *
#define PFX bbl
#define SNAME basic_block_l
#include "cmc/list.h"

struct CFG *CFGCreate() {
  struct CFG *CFG = (struct CFG *)malloc(sizeof(struct CFG));
  CFG->Entry = NULL;
  CFG->Blocks = bbl_create(16);
  CFG->Returns = dl_create(2);
  return CFG;
}

struct BasicBlock *basicBlockCreate(struct CFG *CFG, const char *Label) {
  static uint64_t BBIdCounter = 0;
  struct BasicBlock *BB = (struct BasicBlock *)malloc(sizeof(struct BasicBlock));

  BB->Id = BBIdCounter++;
  BB->Label = Label ? strdup(Label) : NULL;

  const int InitPredsCount = 2;
  const int InitSuccCount = 2;
  BB->Predecessors = bbl_create(InitPredsCount);
  BB->Successors = bbl_create(InitSuccCount);

  BB->Instructions = dl_create(16);

  BB->Type = BBTypeRet;

  bbl_push_back(CFG->Blocks, BB);

  return BB;
}

int basicBlockEq(struct BasicBlock *C1, struct BasicBlock *C2) {
  return (C1->Id == C2->Id) ? 0 : 1;
}

struct basic_block_l *bbl_create(size_t Capacity) {
  static struct basic_block_l_fval fval = {.cmp = basicBlockEq};
  return bbl_new(Capacity, &fval);
}
