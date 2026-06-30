#ifndef ARANEAE_AUX_CODEBUFFER_H
#define ARANEAE_AUX_CODEBUFFER_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

typedef struct CodeBuffer {
  char *Data;
  size_t Size;
} CodeBuffer;

CodeBuffer *codeBufferCreate(void);
void codeBufferFree(CodeBuffer *Buffer);

void codeBufferAddString(CodeBuffer *Buffer, const char *String);
void codeBufferAddLine(CodeBuffer *Buffer, const char *Line);
void codeBufferAddFormatted(CodeBuffer *Buffer, const char *Format, ...);

void codeBufferClear(CodeBuffer *Buffer);
const char *codeBufferGetData(const CodeBuffer *Buffer);
size_t codeBufferGetLength(const CodeBuffer *Buffer);

int codeBufferDumpToFile(const CodeBuffer *Buffer, FILE *File);
int codeBufferDump(const CodeBuffer *Buffer);

#endif /* ARANEAE_AUX_CODEBUFFER_H */