#include <render/vkDevice.h>

#include <stdio.h>

static bool findQueueFamilies(void) {
    uint32_t queueFamilyCount = 16;
    VkQueueFamilyProperties queueFamilies[16];
    vkGetPhysicalDeviceQueueFamilyProperties(globalState.vulkanState.physicalDevice, &queueFamilyCount, (VkQueueFamilyProperties *) &queueFamilies);

    if (queueFamilyCount == 0) {
        printf("%s - Failed to get queue properties.\n", __FUNCTION__);
    }

    // Taking a cue from SteamVR Vulkan example and just assumes queue that supports both graphics and present is the only one we want. Should do for now
    for (int i = 0; i < queueFamilyCount; i++) {
        VkBool32 graphicsSupport = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(globalState.vulkanState.physicalDevice, i, globalState.vulkanState.surface, &presentSupport);

        if (graphicsSupport && presentSupport) {
            globalState.vulkanState.graphicsQueueFamilyIndex = i;
            return true;
        }
    }

    printf("%s - Failed to find a queue that supports both graphics and present!\n", __FUNCTION__);
    return false;
}

void pickPhysicalDevice(void) {
    uint32_t deviceCount = 10;
    VkPhysicalDevice devices[10];
    vkEnumeratePhysicalDevices(globalState.vulkanState.instance, &deviceCount, devices);

    if (deviceCount == 0) {
        printf("%s - failed to find GPUs with Vulkan support!\n", __FUNCTION__);
    }

    // TODO: actually choose a device logically
    globalState.vulkanState.physicalDevice = devices[0];
}

void createLogicalDevice(void) {
    if (!findQueueFamilies()) {
        return;
    }

    const uint32_t queueFamilyCount = 1;
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = globalState.vulkanState.graphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueFamilyCount,
        .pQueueCreateInfos = &queueCreateInfo,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = requiredExtensionCount,
        .ppEnabledExtensionNames = requiredExtensions,
    };

    if (globalState.vulkanState.enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayersCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(globalState.vulkanState.physicalDevice, &createInfo, NULL, &globalState.vulkanState.device) != VK_SUCCESS) {
        printf("%s - failed to create logical device!\n", __FUNCTION__);
        return;
    }

    vkGetDeviceQueue(globalState.vulkanState.device, globalState.vulkanState.graphicsQueueFamilyIndex, 0, &globalState.vulkanState.queue);
}