#ifndef ARANEAE_AST_BUILDER_H
#define ARANEAE_AST_BUILDER_H

#include "Ast.h"

// Function builds Abstract Semantic Tree for program code
// in Path. Source of the tree assigned to the Root variable in "Ast.h".
void parseFile(const char *Path);

#endif /* ARANEAE_AST_BUILDER_H */
