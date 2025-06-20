#include "common.h"
#include <SDL3/SDL.h>
#include <fcntl.h>
#include <cstdlib>
#include "utility.h"

void LoadShaders(const char* shaderFileName, SDL_GPUDevice* device, SDL_GPUShaderFormat supportedShaders) {
    SDL_GPUShaderStage shaderStage;

    if (StringContains(shaderFileName, "vert")) {
        shaderStage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (StringContains(shaderFileName, "frag")) {
        shaderStage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Invalid shader stage!");
        return;
    }

    constexpr char shaderDirectory[] = "shaders/";
    const size_t dirLength = strlen(shaderDirectory);
    const size_t nameLength = strlen(shaderFileName);

    char* directory;
    const char *entrypoint;
    SDL_GPUShaderFormat format;

    if (supportedShaders & SDL_GPU_SHADERFORMAT_DXIL) {
        directory = (char*)malloc(dirLength + nameLength + 5);
        strcpy(directory, shaderDirectory);
        strcat(directory, shaderFileName);
        strcat(directory, ".dxil");

        entrypoint = "main";
        format = SDL_GPU_SHADERFORMAT_DXIL;
    }
    else if (supportedShaders & SDL_GPU_SHADERFORMAT_MSL) {
        directory = (char*)malloc(dirLength + nameLength + 4);
        strcpy(directory, shaderDirectory);
        strcat(directory, shaderFileName);
        strcat(directory, ".msl");

        entrypoint = "main0";
        format = SDL_GPU_SHADERFORMAT_MSL;
    }
    else {
        SDL_Log("Shader Formats Unsupported!");
        return;
    }

    SDL_Log(directory);
    size_t length;
    char *shaderCode = LoadFile(directory, length);

    free(directory);

    SDL_GPUShaderCreateInfo shaderInfo = {
        .code_size = length,
        .code = reinterpret_cast<const Uint8*>(shaderCode),
        .entrypoint = entrypoint,
        .format = format,
        .stage = shaderStage,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 0,
    };

    free(shaderCode);

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
    if (!shader)
    {
        SDL_Log("Failed to create shader!");
    }

    SDL_Log((char*) shaderCode);
}