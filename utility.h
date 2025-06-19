#ifndef UTILITY_H
#define UTILITY_H

#include "SDL3/SDL_stdinc.h"

char* Uint32ToBinary(Uint32 num);
char *LoadFile(const char *directory, size_t &length);
bool StringContains(const char* searchString, const char* target);

#endif //UTILITY_H
