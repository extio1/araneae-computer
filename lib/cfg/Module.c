#include "araneae/cfg/Module.h"
#include "araneae/cfg/Function.h"
#include "araneae/cfg/Class.h"

#include <stdlib.h>

// bool modulePrint(FILE *OS, struct Module *M) {
//   size_t PrintedN = 0;
//   PrintedN += fprintf(OS, "{module for %s}", M->SourceFile);
//   for (size_t I = 0; I < fl_count(M->Functions); ++I) {
//     struct Function *F = fl_get(M->Functions, I);
//     PrintedN += fprintf(OS, "\t");
//     PrintedN += functionPrint(OS, F);
//   }
//   return PrintedN > 0;
// }

struct Module *moduleCreateEmpty(const char *SourceFile) {
  struct Module *M = (struct Module *)malloc(sizeof(struct Module));
  M->Functions = fl_new(16, NULL);
  M->Classes = cltl_create(16);
  M->SourceFile = SourceFile;
}
