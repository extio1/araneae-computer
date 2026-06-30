#include "araneae/aux/Aux.h"
#include "araneae/ast/Builder.h"
#include "araneae/cfg/Builder.h"
#include "araneae/codegen/Codegen.h"
#include "araneae/error/ErrorCollector.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int Argc, char **Argv) {
  errorCollectorStaticInit();

  // TODO: exterminate hardcode, use some library to parse cli
  assert(Argc == 4);
  char *RelativeSourcePath = Argv[1];
  char *OutputAsmName = Argv[2];
  char *RelativeOutputPath = Argv[3];

  char *AbsoluteSourcePath = malloc(sizeof(char) * PATH_MAX);
  char *AbsoluteOutputPath = malloc(sizeof(char) * PATH_MAX);

  if (!realpath(RelativeSourcePath, AbsoluteSourcePath)) {
    perror("Error while resolving full path for source file.\n");
    return -1;
  }
  if (!realpath(RelativeOutputPath, AbsoluteOutputPath)) {
    perror("Error while resolving full path for output path.\n");
    return -1;
  }

  concat(&AbsoluteOutputPath, "/");
  concat(&AbsoluteOutputPath, OutputAsmName);

  // Build AST
  parseFile(AbsoluteSourcePath);
  exportAstToDot(AbsoluteOutputPath);
  if (errorCollectorHasErrors()) {
    errorCollectorDumpErrors();
    return -1;
  }
  if (ProgramRoot == NULL) {
    printf("Error while parsing program: ProgramRoot is NULL\n");
    return -2;
  }
  
  // Build CFGs
  struct Module *M = buildModule(ProgramRoot, AbsoluteSourcePath);
  for (size_t I = 0; I < fl_count(M->Functions); ++I) {
    struct Function* F = fl_get(M->Functions, I);
    exportFunctionToDot(F, AbsoluteOutputPath);
  }

  // Generate AreneaeVM code
  concat(&AbsoluteOutputPath, "/");
  concat(&AbsoluteOutputPath, OutputAsmName);
  concat(&AbsoluteOutputPath, ".s");
  generateArcode(M, AbsoluteOutputPath);

  free(AbsoluteSourcePath);
  free(AbsoluteOutputPath);

  return 0;
}
