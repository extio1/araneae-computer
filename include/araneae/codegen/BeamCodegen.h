#ifndef ARANEAE_BEAM_CODEGEN_H
#define ARANEAE_BEAM_CODEGEN_H

#include "araneae/cfg/Module.h"

/* Generate BEAM assembly (.S) for the given module.
 * The output file can be assembled with: erlc <OutputFile>
 * Returns 0 on success. */
int generateBeam(struct Module *M, const char *ModuleName, const char *OutputFile);

#endif /* ARANEAE_BEAM_CODEGEN_H */
