#include "utility.h"
#include "SDL3/SDL_stdinc.h"
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

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

char *LoadFile(const char *directory) {
    const int fileDescriptor = open(directory, O_RDONLY);

    size_t capacity = 1024;
    char* buffer = (char*)malloc(capacity);
    size_t length = 0;

    while (true) {
        const size_t bytesRead = read(fileDescriptor, buffer + length, capacity);
        if (bytesRead == 0) break;
        length += bytesRead;
        if  (length == capacity) {
            capacity *= 2;
            char* tempBuffer = (char*)realloc(buffer, capacity);
            buffer = tempBuffer;
        }
    }

    close(fileDescriptor);

    if (length > 0) {
        char* trimmedBuffer = (char*)realloc(buffer, length + 1);
        buffer = trimmedBuffer;
        buffer[length] = '\0';
    }
    else {
        free(buffer);
    }

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