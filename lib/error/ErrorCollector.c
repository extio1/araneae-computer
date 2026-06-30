#include "araneae/error/ErrorCollector.h"
#include "araneae/aux/Aux.h"
#include <stdlib.h>
#include <string.h>

static ErrorCollector EC;

void errorCollectorStaticInit() {
  EC.First = NULL;
  EC.Last = NULL;
  EC.Count = 0;
}

void errorCollectorDestroy() {
  Error *Curr = EC.First;
  while (Curr != NULL) {
    Error *Next = Curr->Next;
    free(Curr->Message);
    free(Curr);
    Curr = Next;
  }
  errorCollectorStaticInit();
}

void errorCollectorAddError(ErrorStage ES, int Line, int Column,
                            const char *Msg) {
  Error *Err = (Error *)malloc(sizeof(Error));
  if (!Err) {
    perror("errorCollectorAddError: malloc error");
    return;
  }

  Err->Stage = ES;
  Err->Line = Line;
  Err->Column = Column;
  Err->Message = strdup(Msg);
  Err->Next = NULL;

  if (EC.Last == NULL && EC.First == NULL) {
    EC.First = Err;
    EC.Last = Err;
  } else {
    EC.Last->Next = Err;
    EC.Last = Err;
  }

  ++EC.Count;
}

void errorCollectorAddErrorNoPos(const char *Msg) {
  errorCollectorAddError(ErrorStageNoSpec, -1, -1, Msg);
}

bool errorCollectorHasErrors() { return EC.Count > 0; }

int errorCollectorGetCount() { return EC.Count; }

void errorCollectorPrintErrors(FILE *output) {
  Error *Curr = EC.First;
  while (Curr != NULL) {
    fprintf(output, "%s: Error at line %d, column %d: %s\n",
            errorState2Str(Curr->Stage), Curr->Line, Curr->Column,
            Curr->Message);
    Curr = Curr->Next;
  }
}

void errorCollectorDumpErrors() { errorCollectorPrintErrors(stderr); }

const char *errorState2Str(ErrorStage ES) {
  switch (ES) {
  case ErrorStageNoSpec:
    return "ErrorStageNoSpec";
  case ErrorStageLexer:
    return "ErrorStageLexer";
  case ErrorStageParser:
    return "ErrorStageParser";
  case ErrorStageCfg:
    return "ErrorStageCfg";
  default:
    UNREACHABLE();
  }
}
