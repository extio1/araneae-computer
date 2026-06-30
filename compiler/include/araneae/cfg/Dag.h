#ifndef ARANEAE_CFG_DAG_H
#define ARANEAE_CFG_DAG_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum OpDAGType {
  OpDAGAssign,      // x = y
  OpDAGVP,          // x
  OpDAGFieldAccess, // obj.field
  OpDAGCall,        // call func(args)
  OpDAGObjectCall,
  OpDAGCallObj,     // call obj.func(args)
  OpDAGIndexer,     // x[1, 2, 3]
  OpDAGCreateArray,
  // Arith
  OpDAGAdd,
  OpDAGSub,
  OpDAGMul,
  OpDAGDiv,
  OpDAGMod,
  OpDAGNeg,
  OpDAGInc, // x++ or ++x
  OpDAGDec, // x-- or --x
  // Logical
  OpDAGEq,
  OpDAGNeq,
  OpDAGLt,
  OpDAGLte,
  OpDAGGt,
  OpDAGGte,
  OpDAGAnd,
  OpDAGOr,
  OpDAGNot,
  // Binary
  OpDAGBitAnd,
  OpDAGBitOr,
  OpDAGBitXor,
  OpDAGBitNot,
  OpDAGBitShl,
  OpDAGBitShr,
  // Literals
  OpDAGLiteralStr,
  OpDAGLiteralBool,
  OpDAGLiteralByte,
  OpDAGLiteralDoubleWord,
  OpDAGLiteralQuadWord,
  // Misc
  OpDAGList,
  OpDAGNoSpec = -1
};
const char *cfgOpType2Str(enum OpDAGType T);

struct dag_node_l;
// Node of directed acyclic graph.
struct OpDAGNode {
  // Id of this dag node.
  uint64_t Id;
  // Children of this node, aka operands.
  struct dag_node_l *Operands;
  // Type of this dag node.
  enum OpDAGType Type;
  // Payload corresponing to Type.
  union {
    struct {
      const char *FuncName;
    } Call;
    struct {
      const char *FuncName;
      const char *ObjectName;
    } ObjectCall;
    struct {
      const char *IndexedName;
      size_t Index[8];
    } Indexer;
    struct {
      const char *PlaceName;
    } ValuePlace;
    struct {
      const char *FieldName;
      const char *ObjectName;
    } FieldAccess;
    struct {
      bool Signed;
      union
      {
        const char* Str;
        char Char;
        bool Bool;
        uint32_t UInt;
        int32_t Int;
        uint64_t ULong;
        int64_t Long;
      };
    } Literal;
    struct {
      size_t SizeOf;
    } CreateArray;
  } TerminalData;
};
struct dag_node_l_fval;
struct dag_node_l *dnl_create(size_t Capacity);
void dnl_free(struct dag_node_l *L);
bool dnl_push_back(struct dag_node_l *L, struct OpDAGNode *V);
size_t dnl_count(struct dag_node_l *L);
struct OpDAGNode *dnl_get(struct dag_node_l *L, size_t I);
bool dnl_pop_back(struct dag_node_l *);
struct OpDAGNode *dnl_back(struct dag_node_l *);

bool opDAGNodePrint(FILE *OS, struct OpDAGNode *N);
struct OpDAGNode *opDAGNodeCreateEmpty();
int opDAGNodeEq(struct OpDAGNode *Arg1, struct OpDAGNode *Arg2);
void opDAGNodeFree(struct OpDAGNode *N);

// Operation directed acyclic graph.
struct dag_l;
struct OpDAG {
  uint64_t Id;
  // Root node of operation dag.
  struct OpDAGNode *Root;
  // List of all nodes in operation dag.
  struct dag_node_l *Nodes;
};
struct dag_l_fval;
struct dag_l *dl_create(size_t Capacity);
void dl_free(struct dag_l *L);
bool dl_push_back(struct dag_l *L, struct OpDAG *V);
size_t dl_count(struct dag_l *L);
struct OpDAG *dl_get(struct dag_l *L, size_t I);
bool dl_pop_back(struct dag_l *);
struct OpDAG *dl_back(struct dag_l *);

struct OpDAG *opDAGCreateEmpty();

void exportDAGToDot(struct OpDAG *DAG, const char *OutFile);
int opDAGEq(struct OpDAG *Arg1, struct OpDAG *Arg2);

#endif /* ARANEAE_CFG_DAG_H */
