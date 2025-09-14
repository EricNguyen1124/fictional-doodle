#include "utility.h"
#include <cstdio>
#include "SDL3/SDL_stdinc.h"
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_log.h"
#include <limits.h>

char* Uint32ToBinary(Uint32 num) {
    char* result = new char[33];
    result[32] = '\0';

    for (int i = 31; i >= 0; i--) {
        if (num & 1) {
            result[i] = '1';
        }
        else {
            result[i] = '0';
        }
        num >>= 1;
    }

    return result;
}

char *LoadFile(const char *directory, size_t &length) {
    FILE* shaderFile = fopen(directory, "rb");
    if (!shaderFile) {
        perror("fopen failed");
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Tried: %s", directory);
        return nullptr;
    }
    fseek(shaderFile, 0, SEEK_END);
    length = ftell(shaderFile);
    rewind(shaderFile);
    char* buffer = (char*)malloc(length);
    fread(buffer, length, 1, shaderFile);
    fclose(shaderFile);

    return buffer;
}

bool StringContains(const char* searchString, const char* target) {
    size_t searchIndex = 0;
    size_t targetIndex = 0;
    const size_t targetLength = strlen(target);

    for (int i = 0; searchString[i] != '\0'; i++) {
        char c = searchString[i];
        if (c == target[targetIndex]) {
            if (targetIndex + 1 == targetLength) {
                return true;
            }
            targetIndex++;
        }
        else {
            targetIndex = c == target[0];
        }
    }
    return false;
}