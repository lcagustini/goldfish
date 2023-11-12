#include <global.h>

#include <render/vkBase.h>
#include <render/vkDevice.h>
#include <render/vkSwapchain.h>
#include <render/vkPipeline.h>
#include <render/vkCommandBuffer.h>
#include <render/vkSync.h>

#include <string.h>
#include <stdio.h>

const uint32_t validationLayersCount = 1;
const char *validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t requiredExtensionCount = 1;
const char *requiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VkResult createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                                                           "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        printf("%s - validation layer: %s\n", __FUNCTION__, pCallbackData->pMessage);
    }

    return VK_FALSE;
}

static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo) {
    pCreateInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    pCreateInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    pCreateInfo->messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    pCreateInfo->pfnUserCallback = debugCallback;
}

static bool checkValidationLayerSupport(void) {
    uint32_t availableLayerCount;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, NULL);

    VkLayerProperties availableLayers[availableLayerCount];
    vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);

    for (int i = 0; i < validationLayersCount; ++i) {
        bool layerFound = false;

        for (int j = 0; j < availableLayerCount; ++j) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

static void getRequiredExtensions(uint32_t *extensionCount, const char **pExtensions) {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if (pExtensions == NULL) {
        *extensionCount = glfwExtensionCount + (globalState.vulkanState.enableValidationLayers ? 1 : 0);
        return;
    }

    for (int i = 0; i < glfwExtensionCount; ++i) {
        pExtensions[i] = glfwExtensions[i];
    }

    if (globalState.vulkanState.enableValidationLayers) {
        pExtensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
}

static void createInstance(void) {
    if (globalState.vulkanState.enableValidationLayers && !checkValidationLayerSupport()) {
        printf("%s - validation layers requested, but not available!\n", __FUNCTION__);
        globalState.vulkanState.enableValidationLayers = false;
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = NULL,
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
    };

    uint32_t extensionCount = 0;
    getRequiredExtensions(&extensionCount, NULL);

    const char *extensions[extensionCount];
    getRequiredExtensions(&extensionCount, extensions);

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if (globalState.vulkanState.enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayersCount;
        createInfo.ppEnabledLayerNames = validationLayers;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    if (vkCreateInstance(&createInfo, NULL, &globalState.vulkanState.instance) != VK_SUCCESS) {
        printf("%s - unable to initialize Vulkan!\n", __FUNCTION__);
    }
}

void setupDebugMessenger(void) {
    if (!globalState.vulkanState.enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(&createInfo);

    if (createDebugUtilsMessenger(globalState.vulkanState.instance, &createInfo, NULL, &globalState.vulkanState.debugMessenger) != VK_SUCCESS) {
        printf("%s - failed to set up debug messenger!\n", __FUNCTION__);
    }
}

bool createSurface(void) {
    if (glfwCreateWindowSurface(globalState.vulkanState.instance, globalState.window, NULL, &globalState.vulkanState.surface) != VK_SUCCESS) {
        printf("%s - failed to create window surface!\n", __FUNCTION__);
        return false;
    }

    return true;
}

void initVulkan(void) {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

void destroyVulkan(void) {
    vkDeviceWaitIdle(globalState.vulkanState.device);

    vkDestroySemaphore(globalState.vulkanState.device, globalState.vulkanState.renderFinishedSemaphore, NULL);
    vkDestroySemaphore(globalState.vulkanState.device, globalState.vulkanState.imageAvailableSemaphore, NULL);
    vkDestroyFence(globalState.vulkanState.device, globalState.vulkanState.inFlightFence, NULL);

    vkDestroyCommandPool(globalState.vulkanState.device, globalState.vulkanState.commandPool, NULL);

    for (int i = 0; i < globalState.vulkanState.swapChainImageCount; ++i) {
        vkDestroyFramebuffer(globalState.vulkanState.device, globalState.vulkanState.pSwapChainFramebuffers[i], NULL);
    }

    vkDestroyPipeline(globalState.vulkanState.device, globalState.vulkanState.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(globalState.vulkanState.device, globalState.vulkanState.pipelineLayout, NULL);
    vkDestroyRenderPass(globalState.vulkanState.device, globalState.vulkanState.renderPass, NULL);

    for (int i = 0; i < globalState.vulkanState.swapChainImageCount; ++i) {
        vkDestroyImageView(globalState.vulkanState.device, globalState.vulkanState.pSwapChainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(globalState.vulkanState.device, globalState.vulkanState.swapChain, NULL);
    vkDestroyDevice(globalState.vulkanState.device, NULL);

    if (globalState.vulkanState.enableValidationLayers) {
        destroyDebugUtilsMessenger(globalState.vulkanState.instance, globalState.vulkanState.debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(globalState.vulkanState.instance, globalState.vulkanState.surface, NULL);
    vkDestroyInstance(globalState.vulkanState.instance, NULL);
}