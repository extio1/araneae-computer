#include "araneae/ast/Builder.h"
#include "araneae/aux/Aux.h"
#include "araneae/cfg/Builder.h"
#include "araneae/codegen/BeamCodegen.h"
#include "araneae/error/ErrorCollector.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Usage: araneae-beam <source.ae> <ModuleName> <output.S> */
int main(int Argc, char **Argv) {
    if (Argc != 4) {
        fprintf(stderr, "Usage: %s <source.ae> <ModuleName> <output.S>\n", Argv[0]);
        return 1;
    }

    errorCollectorStaticInit();

    char *SrcPath = Argv[1];
    char *ModName = Argv[2];
    char *OutPath = Argv[3];

    char AbsSrc[PATH_MAX];
    if (!realpath(SrcPath, AbsSrc)) {
        perror("Cannot resolve source path");
        return 1;
    }

    parseFile(AbsSrc);
    if (errorCollectorHasErrors()) {
        errorCollectorDumpErrors();
        return 1;
    }
    if (!ProgramRoot) {
        fprintf(stderr, "Parse error: ProgramRoot is NULL\n");
        return 1;
    }

    struct Module *M = buildModule(ProgramRoot, AbsSrc);

    return generateBeam(M, ModName, OutPath);
}
