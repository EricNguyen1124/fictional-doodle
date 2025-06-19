#ifndef COMMON_H
#define COMMON_H
#include "SDL3/SDL_gpu.h"

void LoadShaders(const char* shaderFileName, SDL_GPUDevice* device, SDL_GPUShaderFormat supportedShaders);

#endif //COMMON_H

