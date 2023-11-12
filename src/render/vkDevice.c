#include <render/vkDevice.h>

#include <stdio.h>

static bool findQueueFamilies(void) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(globalState.vulkanState.physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(globalState.vulkanState.physicalDevice, &queueFamilyCount, (VkQueueFamilyProperties *) &queueFamilies);

    if (queueFamilyCount == 0) {
        printf("%s - Failed to get queue properties.\n", __FUNCTION__);
    }

    // Taking a cue from SteamVR Vulkan example and just assuming queue that supports both graphics and present is the only one we want. Don't entirely know if that's right.
    for (int i = 0; i < queueFamilyCount; ++i) {
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
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(globalState.vulkanState.instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        printf("%s - failed to find GPUs with Vulkan support!\n", __FUNCTION__);
    }

    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(globalState.vulkanState.instance, &deviceCount, devices);

    // Todo Implement Query OpenVR for the physical device to use
    // If no OVR fallback to first one. OVR Vulkan used this logic, its much simpler than vulkan example, is it correct? Seemed to be on my 6950xt
    globalState.vulkanState.physicalDevice = devices[0];
}

bool createLogicalDevice(void) {
    if (!findQueueFamilies()) {
        return false;
    }

    const uint32_t queueFamilyCount = 1;
    VkDeviceQueueCreateInfo queueCreateInfos[queueFamilyCount];
    uint32_t uniqueQueueFamilies[] = {globalState.vulkanState.graphicsQueueFamilyIndex};

    float queuePriority = 1.0f;
    for (int i = 0; i < queueFamilyCount; ++i) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = queueFamilyCount,
        .pQueueCreateInfos = queueCreateInfos,
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
        return false;
    }

    vkGetDeviceQueue(globalState.vulkanState.device, globalState.vulkanState.graphicsQueueFamilyIndex, 0, &globalState.vulkanState.queue);

    return true;
}