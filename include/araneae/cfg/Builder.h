#ifndef ARANEAE_CFG_BUILDER_H
#define ARANEAE_CFG_BUILDER_H

#include "Module.h"
#include "araneae/ast/Ast.h"

#include <stdbool.h>
#include <stddef.h>

// Function builds control flow graph for parsed abstract syntax tree.
struct Module *buildModule(ASTNode *ProgramRoot, const char* SourceName);

// Apply F for all Node children
typedef void visitChildrenF(const ASTNode *ChildNode, struct Module *M);
void visitChildren(const ASTNode *Node, struct Module *M, visitChildrenF F);

#endif /* ARANEAE_CFG_BUILDER_H */
