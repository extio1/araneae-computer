#ifndef ARANEAE_AUX_AUX_H
#define ARANEAE_AUX_AUX_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

// Always false assert
#define UNREACHABLE(msg) { assert(false && "" msg); abort(); }

// Safely concatenate strings with dynamivally reallocating @String.
// @With must ends with '\0'
void concat(char **String, const char *With);

void uint642Binary(uint64_t Value, char *Output);

uint64_t hash(const unsigned char *str);

void mangle(char *OutBuffer, const char* FName, size_t NArgs);
void mangleClass(char *OutBuffer, const char* FName, const char* CName, size_t NArgs);

#endif // ARANEAE_AUX_AUX_H