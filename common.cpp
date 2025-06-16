#include "common.h"
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <cstdlib>
#include "utility.h"

void LoadShaders(const char* shaderFileName) {
    SDL_GPUShaderStage shaderStage;

    if (StringContains("vejvtyveverto", "vert")) {
        SDL_Log("true");
    }
    else {
        SDL_Log("false");
    }

    if (StringContains(shaderFileName, "vert")) {
        shaderStage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (StringContains(shaderFileName, "frag")) {
        shaderStage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }

    constexpr char shaderDirectory[] = "shaders/";
    const size_t dirLength = strlen(shaderDirectory);
    const size_t nameLength = strlen(shaderFileName);

    char* directory = (char*)malloc(dirLength + nameLength);
    strcpy(directory, shaderDirectory);
    strcat(directory, shaderFileName);

    SDL_Log(directory);
    char* shaderCode = LoadFile(directory);

    SDL_Log((char*) shaderCode);
}