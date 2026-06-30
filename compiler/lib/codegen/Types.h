#ifndef ARANEAE_LIB_CODEGEN_TYPES_H
#define ARANEAE_LIB_CODEGEN_TYPES_H

#include <stdint.h>

typedef uint64_t ReferenceT;

#define MakeRef(type, flags, payload)                                          \
  MakeRefInternal((ReferenceT), type, flags, payload)

#endif /* ARANEAE_LIB_CODEGEN_TYPES_H */
