//
// Created by Eric Nguyen on 5/31/25.
//

#include "common.h"
#include <SDL3/SDL.h>

#include <cstdlib>

char* uint32ToBinary(Uint32 num) {
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
