#ifndef ARANEAE_CFG_MODULE_H
#define ARANEAE_CFG_MODULE_H

#include "Function.h"

typedef struct Module {
  // List of functions inside the module.
  struct function_l *Functions;
  // List of classes inside the module.
  struct class_type_l *Classes;
  // Path to source for this module.
  const char *SourceFile;
} Module;
// bool modulePrint(FILE *OS, struct Module *DAG);
struct Module *moduleCreateEmpty(const char *SourceFile);

#endif /* ARANEAE_CFG_MODULE_H */
