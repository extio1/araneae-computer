// This file defines constructor and printers 
// for directed acyclic graph structures.

#include "araneae/cfg/Dag.h"

#include "araneae/aux/Aux.h"

#define V struct OpDAGNode *
#define PFX dnl
#define SNAME dag_node_l
#include "cmc/list.h"

struct dag_node_l *dnl_create(size_t Capacity) {
  static struct dag_node_l_fval FVal = {.free = opDAGNodeFree, .cmp = opDAGNodeEq}; 
  return dnl_new(Capacity, &FVal);
}

#define V struct OpDAG *
#define PFX dl
#define SNAME dag_l
#include "cmc/list.h"

struct dag_l *dl_create(size_t Capacity) {
  static struct dag_l_fval FVal = {.cmp = opDAGEq}; 
  return dl_new(Capacity, &FVal);
}

const char *cfgOpType2Str(enum OpDAGType T) {
  switch (T) {
  case OpDAGAssign:
    return "Assign";
  case OpDAGVP:
    return "ValuePlace";
  case OpDAGCall:
    return "Call";
  case OpDAGIndexer:
    return "Indexer";
  case OpDAGAdd:
    return "Add";
  case OpDAGSub:
    return "Sub";
  case OpDAGMul:
    return "Mul";
  case OpDAGDiv:
    return "Div";
  case OpDAGMod:
    return "Mod";
  case OpDAGNeg:
    return "Neg";
  case OpDAGInc:
    return "Inc";
  case OpDAGDec:
    return "Dec";
  case OpDAGEq:
    return "Eq";
  case OpDAGNeq:
    return "Neq";
  case OpDAGLt:
    return "Lt";
  case OpDAGLte:
    return "Lte";
  case OpDAGGt:
    return "Gt";
  case OpDAGGte:
    return "Gte";
  case OpDAGAnd:
    return "And";
  case OpDAGOr:
    return "Or";
  case OpDAGNot:
    return "Not";
  case OpDAGBitAnd:
    return "BitAnd";
  case OpDAGBitOr:
    return "BitOr";
  case OpDAGBitXor:
    return "BitXor";
  case OpDAGBitNot:
    return "BitNot";
  case OpDAGBitShl:
    return "BitShl";
  case OpDAGBitShr:
    return "BitShr";
  case OpDAGLiteralStr:
    return "LiteralStr";
  case OpDAGLiteralBool:
    return "LiteralBool";
  case OpDAGLiteralByte:
    return "LiteralByte";
  case OpDAGLiteralDoubleWord:
    return "LiteralDoubleWord";
  case OpDAGLiteralQuadWord:
    return "LiteralQuadWord";
  case OpDAGList:
    return "List";
  case OpDAGNoSpec:
    return "NoSpec";
  default:
    UNREACHABLE();
  }
}

void opDAGNodeFree(struct OpDAGNode *N) {
  free(N);
}

bool opDAGNodePrint(FILE *OS, struct OpDAGNode *N) {
  return fprintf(OS, "node#%ld[%s]", N->Id, cfgOpType2Str(N->Type)) > 0;
}

struct OpDAGNode *opDAGNodeCreateEmpty() {
  static uint64_t DagNodeIdCounter = 0;
  static size_t InitChildrenCount = 2;
  struct OpDAGNode *N = (struct OpDAGNode *)malloc(sizeof(struct OpDAGNode));
  N->Id = DagNodeIdCounter++;
  N->Type = OpDAGNoSpec;
  N->Operands = dnl_create(InitChildrenCount);
  return N;
}

struct OpDAG *opDAGCreateEmpty() {
  static uint64_t DagIdCounter = 0;
  static uint64_t InitNodesCount = 8;
  struct OpDAG *DAG = (struct OpDAG *)malloc(sizeof(struct OpDAG));
  DAG->Id = DagIdCounter++;
  DAG->Root = NULL;
  DAG->Nodes = dnl_create(InitNodesCount);
  return DAG;
}

int opDAGNodeEq(struct OpDAGNode *Arg1, struct OpDAGNode *Arg2) {
  return (Arg1->Id == Arg2->Id) ? 0 : 1;
}

int opDAGEq(struct OpDAG *Arg1, struct OpDAG *Arg2) {
  return (Arg1->Id == Arg2->Id) ? 0 : 1;
}
