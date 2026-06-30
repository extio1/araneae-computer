#ifndef ERROR_COLLECTOR_H
#define ERROR_COLLECTOR_H

#include <stdbool.h>
#include <stdio.h>

typedef enum ErrorStage {
  ErrorStageNoSpec,
  ErrorStageLexer,
  ErrorStageParser,
  ErrorStageCfg
} ErrorStage;
const char *errorState2Str(ErrorStage ES);

typedef struct Error {
  ErrorStage Stage;
  int Line;
  int Column;
  char *Message;
  struct Error *Next;
} Error;

typedef struct ErrorCollector {
  Error *First;
  Error *Last;
  int Count;
} ErrorCollector;

void errorCollectorStaticInit();
void errorCollectorDestroy();

void errorCollectorAddError(ErrorStage ES, int Line, int Column,
                            const char *Msg);
void errorCollectorAddErrorNoPos(const char *Msg);

bool errorCollectorHasErrors();
int errorCollectorGetCount();

void errorCollectorPrintErrors(FILE *output);
void errorCollectorDumpErrors();

#endif // ERROR_COLLECTOR_H
