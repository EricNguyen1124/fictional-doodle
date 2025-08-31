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
    uint32_t graphicsFamily{};
    bool hasPresentFamily = false;
    uint32_t presentFamily{};
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    VkSurfaceFormatKHR* formats = nullptr;
    uint32_t formatCount = 0;
    VkPresentModeKHR* presentModes = nullptr;
    uint32_t presentModeCount = 0;
};

static SDL_Renderer *renderer = nullptr;

SwapchainSupportDetails FindSwapChainDetails(const VkPhysicalDevice *device, const VkSurfaceKHR *surface) {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, *surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats = (VkSurfaceFormatKHR*) malloc(sizeof(VkSurfaceKHR) * formatCount);
        details.formatCount = formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(*device, *surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes = (VkPresentModeKHR*) malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        details.presentModeCount = presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(*device, *surface, &presentModeCount, details.presentModes);
    }

    return details;
}

QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice *device, const VkSurfaceKHR *surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(*device, &deviceProperties);

    VkQueueFamilyProperties *queueFamilies = (VkQueueFamilyProperties *) malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        VkQueueFamilyProperties queueFamily = queueFamilies[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.hasGraphicsFamily = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, *surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            indices.hasPresentFamily = true;
        }
    }

    free(queueFamilies);
    return indices;
}


VkPhysicalDevice ChoosePhysicalDevice(const VkPhysicalDevice *physicalDevices, const uint32_t deviceCount, const VkSurfaceKHR *surface) {
    if (physicalDevices == nullptr) return nullptr;

    for (int i = 0; i < deviceCount; i++) {
        VkPhysicalDevice device = physicalDevices[i];
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        const QueueFamilyIndices queueIndices = FindQueueFamilies(&device, surface);
        const bool queueFamiliesComplete = queueIndices.hasGraphicsFamily && queueIndices.hasPresentFamily;

        // need to check if device supports swapchain extension

        const SwapchainSupportDetails swapChainSupport = FindSwapChainDetails(&device, surface);
        const bool swapChainAdequate = swapChainSupport.formats != nullptr && swapChainSupport.presentModes != nullptr;

        if (queueFamiliesComplete && swapChainAdequate) {
            return device;
        }
    }

    return physicalDevices[0];
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const VkSurfaceFormatKHR* formats, const uint32_t formatCount) {
    for (int i = 0; i < formatCount; i++) {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const VkPresentModeKHR* presentModes, const uint32_t presentModeCount) {
    for (int i = 0; i < presentModeCount; i++) {
        VkPresentModeKHR presentMode = presentModes[0];
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, SDL_Window* window) {
    if (capabilities->currentExtent.width != 0xFFFFFFFF) {
        return capabilities->currentExtent;
    }
    else {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        VkExtent2D actualExtent = {
            (uint32_t) width,
            (uint32_t) height
        };

        // Clamp the width and height to the min and max extents
        if (actualExtent.width < capabilities->minImageExtent.width) {
            actualExtent.width = capabilities->minImageExtent.width;
        }
        else if (actualExtent.width > capabilities->maxImageExtent.width) {
            actualExtent.width = capabilities->maxImageExtent.width;
        }

        if (actualExtent.height < capabilities->minImageExtent.height) {
            actualExtent.height = capabilities->minImageExtent.height;
        }
        else if (actualExtent.height > capabilities->maxImageExtent.height) {
            actualExtent.height = capabilities->maxImageExtent.height;
        }

        return actualExtent;
    }
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

    const char **validationLayers = (const char **) malloc(sizeof(char *) * 1);
    validationLayers[0] = "VK_LAYER_KHRONOS_validation";

    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validationLayers,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions
    };

#ifdef __APPLE__
    const char **allExts = (const char **) malloc(sizeof(char *) * (extensionCount + 1));
    memcpy(allExts, extensions, sizeof(*extensions) * extensionCount);
    allExts[extensionCount] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;

    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceCreateInfo.enabledExtensionCount = extensionCount + 1;
    instanceCreateInfo.ppEnabledExtensionNames = allExts;
#endif


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

    VkPhysicalDevice physicalDevice = ChoosePhysicalDevice(physicalDevices, deviceCount, &surface);

    QueueFamilyIndices queueFamilies = FindQueueFamilies(&physicalDevice, &surface);

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo *queueCreateInfos = (VkDeviceQueueCreateInfo *) malloc(sizeof(VkDeviceQueueCreateInfo));
    queueCreateInfos[0] = VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilies.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    if (queueFamilies.presentFamily != queueFamilies.graphicsFamily) {
        queueCreateInfos = (VkDeviceQueueCreateInfo *) realloc(queueCreateInfos, sizeof(VkDeviceQueueCreateInfo) * 2);
        queueCreateInfos[1] = VkDeviceQueueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamilies.presentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        SDL_Log("Present and Graphics are on different Queue Families");
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    const char **deviceExtensions = (const char **) malloc(sizeof(char*));
    deviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 2,
        .pQueueCreateInfos = queueCreateInfos,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
        .pEnabledFeatures = &deviceFeatures
    };


#ifdef __APPLE__
    deviceExtensions = (const char **) realloc(deviceExtensions, sizeof(char*) * 2);
    deviceCreateInfo.enabledExtensionCount = 2;
    deviceExtensions[1] = "VK_KHR_portability_subset";
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
#endif

    VkDevice device;
    result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

    if (result != VK_SUCCESS) {
        SDL_Log("CREATE DEVICE FAILED");
        return SDL_APP_FAILURE;
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, queueFamilies.graphicsFamily, 0, &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(device, queueFamilies.presentFamily, 0, &presentQueue);

    SwapchainSupportDetails swapChainSupport = FindSwapChainDetails(&physicalDevice, &surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
    VkExtent2D extent = ChooseSwapExtent(&swapChainSupport.capabilities, state->Window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha =  VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (queueFamilies.presentFamily != queueFamilies.graphicsFamily) {
        uint32_t* queueFamilyIndicies = (uint32_t*) malloc(sizeof(uint32_t) * 2);
        queueFamilyIndicies[0] = queueFamilies.graphicsFamily;
        queueFamilyIndicies[1] = queueFamilies.presentFamily;

        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicies;
    }
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
        SDL_Log("CREATE SWAPCHAIN FAILED");
        return SDL_APP_FAILURE;
    }

    //free(allExts);
    free(validationLayers);
    free(physicalDevices);
    free(deviceExtensions);
    free(queueCreateInfos);

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
