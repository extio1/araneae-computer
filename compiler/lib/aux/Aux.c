#include "araneae/aux/Aux.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void concat(char **String, const char *With) {
  assert(String && With);

  size_t OldLen = *String ? strlen(*String) : 0;
  size_t WithLen = strlen(With);

  char *NewString = realloc(*String, OldLen + WithLen + 1);
  if (!NewString) {
    perror("realloc failed");
    return;
  }

  strncpy(NewString + OldLen, With, WithLen + 1);
  *String = NewString;
}

void uint642Binary(uint64_t Value, char *Output) {
  Output[0] = '0';
  Output[1] = 'b';
  uint64_t Mask = 1ULL << 63;
  for (int i = 2; i < 66; i++) {
    Output[i] = (Value & Mask) ? '1' : '0';
    Mask >>= 1;
  }
  Output[66] = '\0';
}

uint64_t hash(const unsigned char *str){
  uint64_t hash = 5381;
  int c;
  while (c = *str++)
    hash = ((hash << 5) + hash) + c;
  return hash;
}

void mangle(char *OutBuffer, const char* FName, size_t NArgs) {
  sprintf(OutBuffer, "%s__%ld", FName, NArgs);
}

void mangleClass(char *OutBuffer, const char* FName, const char* CName, size_t NArgs) {
  sprintf(OutBuffer, "%s__%s__%ld", FName, CName, NArgs);
}
