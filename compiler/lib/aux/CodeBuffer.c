#include "araneae/aux/CodeBuffer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODEBUFFER_INITIAL_SIZE 1024
#define CODEBUFFER_GROW_FACTOR 2

CodeBuffer *codeBufferCreate() {
  CodeBuffer *buffer = malloc(sizeof(CodeBuffer));
  if (!buffer)
    return NULL;

  buffer->Data = malloc(CODEBUFFER_INITIAL_SIZE);
  if (!buffer->Data) {
    free(buffer);
    return NULL;
  }

  buffer->Data[0] = '\0';
  buffer->Size = CODEBUFFER_INITIAL_SIZE;
  return buffer;
}

void codeBufferFree(CodeBuffer *buffer) {
  if (!buffer)
    return;

  free(buffer->Data);
  free(buffer);
}

void codeBufferAddString(CodeBuffer *buffer, const char *string) {
  if (!buffer || !string)
    return;

  size_t stringLen = strlen(string);
  size_t currentLen = strlen(buffer->Data);

  if (currentLen + stringLen + 1 >= buffer->Size) {
    size_t newSize = buffer->Size;
    while (currentLen + stringLen + 1 >= newSize) {
      newSize *= CODEBUFFER_GROW_FACTOR;
    }

    char *newData = realloc(buffer->Data, newSize);
    if (!newData) {
      return;
    }

    buffer->Data = newData;
    buffer->Size = newSize;
  }

  strcat(buffer->Data, string);
}

void codeBufferAddLine(CodeBuffer *buffer, const char *line) {
  if (!buffer || !line)
    return;

  codeBufferAddString(buffer, line);
  codeBufferAddString(buffer, "\n");
}

void codeBufferAddFormatted(CodeBuffer *buffer, const char *format, ...) {
  if (!buffer || !format)
    return;

  va_list args;
  va_start(args, format);

  va_list argsCopy;
  va_copy(argsCopy, args);
  int needed = vsnprintf(NULL, 0, format, argsCopy);
  va_end(argsCopy);

  if (needed < 0) {
    va_end(args);
    return;
  }

  size_t currentLen = strlen(buffer->Data);
  size_t requiredSize = currentLen + needed + 1;

  if (requiredSize > buffer->Size) {
    size_t newSize = buffer->Size;
    while (requiredSize > newSize) {
      newSize *= CODEBUFFER_GROW_FACTOR;
    }

    char *newData = realloc(buffer->Data, newSize);
    if (!newData) {
      va_end(args);
      return;
    }

    buffer->Data = newData;
    buffer->Size = newSize;
  }

  vsprintf(buffer->Data + currentLen, format, args);
  va_end(args);
}

void codeBufferClear(CodeBuffer *buffer) {
  if (!buffer)
    return;
  buffer->Data[0] = '\0';
}

const char *codeBufferGetData(const CodeBuffer *buffer) {
  return buffer ? buffer->Data : NULL;
}

size_t codeBufferGetLength(const CodeBuffer *buffer) {
  return buffer ? strlen(buffer->Data) : 0;
}

int codeBufferDumpToFile(const CodeBuffer *buffer, FILE *file) {
  if (!buffer || !file) {
    return -1;
  }

  size_t length = strlen(buffer->Data);
  size_t written = fwrite(buffer->Data, 1, length, file);

  return (written == length) ? 0 : -1;
}

int codeBufferDump(const CodeBuffer *Buffer) {
  for (size_t I = 0; I < Buffer->Size; ++I)
    printf("%c", Buffer->Data[I]);
}
