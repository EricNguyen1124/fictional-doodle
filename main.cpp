#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <queue>
#include "common.h"
#include "utility.h"
#include <vulkan/vulkan.h>
#include "SDL3/SDL_vulkan.h"

typedef struct {
    SDL_Window *Window;
    SDL_Renderer *Renderer;
    SDL_GPUDevice *Device;
    SDL_GPUShaderFormat SupportedShaders;
} AppState;

struct QueueFamilyIndices {
    bool hasGraphicsFamily = false;
    uint32_t graphicsFamily;
    bool hasPresentFamily = false;
    uint32_t presentFamily;
};

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;

VkPhysicalDevice ChoosePhysicalDevice(const VkPhysicalDevice *physicalDevices, const uint32_t deviceCount) {
    if (physicalDevices == nullptr) return nullptr;

    for (int i = 0; i < deviceCount; i++) {
        const VkPhysicalDevice device = physicalDevices[i];
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        return device;
    }

    return physicalDevices[0];
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties *) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        VkQueueFamilyProperties queueFamily = queueFamilies[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }
    }

    free(queueFamilies);
    return indices;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    uint32_t version = 0;
    if (vkEnumerateInstanceVersion(&version) == VK_SUCCESS) {
        std::cout << "Vulkan Version: "
                << VK_VERSION_MAJOR(version) << "."
                << VK_VERSION_MINOR(version) << "."
                << VK_VERSION_PATCH(version) << "\n";
    } else {
        std::cerr << "Failed to query Vulkan version.\n";
    }

    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto *state = (AppState *) SDL_calloc(1, sizeof(AppState));
    if (!state) {
        return SDL_APP_FAILURE;
    }

    *appstate = state;

    state->Window = SDL_CreateWindow("Hi", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "my engine",
        .apiVersion = VK_API_VERSION_1_4
    };

    uint32_t extensionCount;
    char const *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    const char **allExts = (const char **) malloc(sizeof(*allExts) * (extensionCount + 1)); // MAC MOMENT
    memcpy(allExts, extensions, sizeof(*extensions) * extensionCount); // MAC MOMENT
    allExts[extensionCount] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;

    const char **validationLayers = (const char **) malloc(sizeof(*validationLayers) * 1);
    validationLayers[0] = "VK_LAYER_KHRONOS_validation";

    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = extensionCount + 1, // MAC MOMENT
        .ppEnabledExtensionNames = allExts,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validationLayers,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
    };

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        SDL_Log("Create Instance Failed");
        return SDL_APP_FAILURE;
    }

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(state->Window, instance, nullptr, &surface)) {
        SDL_Log("Create Surface Failed");
        return SDL_APP_FAILURE;
    }

    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (result != VK_SUCCESS) {
        SDL_Log("EnumeratePhysicalDevices Failed");
        return SDL_APP_FAILURE;
    }

    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *) malloc(sizeof(VkPhysicalDevice) * deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);

    if (result != VK_SUCCESS) {
        SDL_Log("EnumeratePhysicalDevices Failed 2");
        return SDL_APP_FAILURE;
    }

    VkPhysicalDevice physicalDevice = ChoosePhysicalDevice(physicalDevices, deviceCount);

    QueueFamilyIndices queueFamilies = FindQueueFamilies(physicalDevice, surface);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilies.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceFeatures deviceFeatures{};
    const char **deviceExtensions = (const char **)malloc(sizeof(*deviceExtensions));
    deviceExtensions[0] = "VK_KHR_portability_subset";

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queueCreateInfo,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions
    };

    VkDevice device;
    result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

    if (result != VK_SUCCESS) {
        SDL_Log("CREATE DEVICE FAILED");
        return SDL_APP_FAILURE;
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, queueFamilies.graphicsFamily, 0, &graphicsQueue);

    free(allExts);
    free(validationLayers);
    free(physicalDevices);
    free(deviceExtensions);

    // state->Device = SDL_CreateGPUDevice(
    //     SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
    //     true,
    //     nullptr);
    //
    // state->SupportedShaders = SDL_GetGPUShaderFormats(state->Device);
    //
    // LoadShaders("PositionColor.vert", state->Device, state->SupportedShaders);
    //
    // SDL_Log(Uint32ToBinary(SDL_GetGPUShaderFormats(state->Device)));
    // if (state->Device == nullptr) {
    //     SDL_Log("GPUCreateDevice failed");
    //     return SDL_APP_FAILURE;
    // }
    //
    // if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480, 0, &window, &renderer)) {
    //     SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    //     return SDL_APP_FAILURE;
    // }

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {
    const double now = ((double) SDL_GetTicks()) / 1000.0; /* convert from milliseconds to seconds. */
    /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    const float red = (float) (0.5 + 0.5 * SDL_sin(now));
    const float green = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
    const float blue = (float) (0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
    SDL_SetRenderDrawColorFloat(renderer, red, green, blue, SDL_ALPHA_OPAQUE_FLOAT); /* new color, full alpha. */

    /* clear the window to the draw color. */
    SDL_RenderClear(renderer);

    /* put the newly-cleared rendering on the screen. */
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    /* SDL will clean up the window/renderer for us. */
}
