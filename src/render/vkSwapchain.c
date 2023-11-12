#include <render/vkSwapchain.h>

#include <stdio.h>
#include <stdlib.h>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR *availableFormats, uint32_t formatCount) {
    // Favor sRGB if it's available
    for (int i = 0; i < formatCount; ++i) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    // Default to the first one if no sRGB
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, uint32_t presentModeCount) {
    // This logic taken from OVR Vulkan Example
    // VK_PRESENT_MODE_FIFO_KHR - equivalent of eglSwapInterval(1).  The presentation engine waits for the next vertical blanking period to update
    // the current image. Tearing cannot be observed. This mode must be supported by all implementations.
    VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (int i = 0; i < presentModeCount; ++i) {
        // Order of preference for no vsync:
        // 1. VK_PRESENT_MODE_IMMEDIATE_KHR - The presentation engine does not wait for a vertical blanking period to update the current image,
        //                                    meaning this mode may result in visible tearing
        // 2. VK_PRESENT_MODE_MAILBOX_KHR - The presentation engine waits for the next vertical blanking period to update the current image. Tearing cannot be observed.
        //                                  An internal single-entry queue is used to hold pending presentation requests.
        // 3. VK_PRESENT_MODE_FIFO_RELAXED_KHR - equivalent of eglSwapInterval(-1).
        if (availablePresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            // The presentation engine does not wait for a vertical blanking period to update the
            // current image, meaning this mode may result in visible tearing
            swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            break;
        }
        else if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        else if ((swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
                 (availablePresentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)) {
            // VK_PRESENT_MODE_FIFO_RELAXED_KHR - equivalent of eglSwapInterval(-1)
            swapChainPresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        }
    }

    return swapChainPresentMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
    // Logic from OVR Vulkan sample. Logic little different from vulkan tutorial
    VkExtent2D extents;
    if (capabilities.currentExtent.width == -1) {
        // If the surface size is undefined, the size is set to the size of the images requested.
        glfwGetWindowSize(globalState.window, &extents.width, &extents.height);
    }
    else {
        // If the surface size is defined, the swap chain size must match
        extents = capabilities.currentExtent;
    }

    return extents;
}

void createSwapChain(void) {
    // Logic from OVR Vulkan example
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(globalState.vulkanState.physicalDevice, globalState.vulkanState.surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(globalState.vulkanState.physicalDevice, globalState.vulkanState.surface, &formatCount, NULL);
    VkSurfaceFormatKHR formats[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(globalState.vulkanState.physicalDevice, globalState.vulkanState.surface, &formatCount, (VkSurfaceFormatKHR *) &formats);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(globalState.vulkanState.physicalDevice, globalState.vulkanState.surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[presentModeCount];
    vkGetPhysicalDeviceSurfacePresentModesKHR(globalState.vulkanState.physicalDevice, globalState.vulkanState.surface, &presentModeCount, (VkPresentModeKHR *) &presentModes);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats, formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, presentModeCount);
    VkExtent2D extent = chooseSwapExtent(capabilities);

    // Have a swap queue depth of at least three frames
    globalState.vulkanState.swapChainImageCount = capabilities.minImageCount;
    if (globalState.vulkanState.swapChainImageCount < 2) {
        globalState.vulkanState.swapChainImageCount = 2;
    }
    if ((capabilities.maxImageCount > 0) && (globalState.vulkanState.swapChainImageCount > capabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        globalState.vulkanState.swapChainImageCount = capabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else {
        preTransform = capabilities.currentTransform;
    }

    VkImageUsageFlags nImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if ((capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        nImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    else {
        printf("Vulkan swapchain does not support VK_IMAGE_USAGE_TRANSFER_DST_BIT. Some operations may not be supported.\n");
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = globalState.vulkanState.surface,
        .minImageCount = globalState.vulkanState.swapChainImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageUsage = nImageUsageFlags,
        .preTransform = preTransform,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .presentMode = presentMode,
        .clipped = VK_TRUE
    };
    if ((capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0) {
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if ((capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0) {
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else {
        printf("Unexpected value for VkSurfaceCapabilitiesKHR.compositeAlpha: %x\n", capabilities.supportedCompositeAlpha);
    }

    if (vkCreateSwapchainKHR(globalState.vulkanState.device, &createInfo, NULL, &globalState.vulkanState.swapChain) != VK_SUCCESS) {
        printf("%s - failed to create swap chain!\n", __FUNCTION__);
    }

    vkGetSwapchainImagesKHR(globalState.vulkanState.device, globalState.vulkanState.swapChain, &globalState.vulkanState.swapChainImageCount, NULL);
    globalState.vulkanState.pSwapChainImages = malloc(sizeof(VkImage) * globalState.vulkanState.swapChainImageCount);
    vkGetSwapchainImagesKHR(globalState.vulkanState.device, globalState.vulkanState.swapChain, &globalState.vulkanState.swapChainImageCount, globalState.vulkanState.pSwapChainImages);

    globalState.vulkanState.swapChainImageFormat = surfaceFormat.format;
    globalState.vulkanState.swapChainExtent = extent;
}

void createImageViews(void) {
    globalState.vulkanState.pSwapChainImageViews = malloc(sizeof(VkImageView) * globalState.vulkanState.swapChainImageCount);

    for (size_t i = 0; i < globalState.vulkanState.swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = globalState.vulkanState.pSwapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = globalState.vulkanState.swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        if (vkCreateImageView(globalState.vulkanState.device, &createInfo, NULL, &globalState.vulkanState.pSwapChainImageViews[i]) != VK_SUCCESS) {
            printf("%s - failed to create image views!\n", __FUNCTION__);
        }
    }
}

void createFramebuffers(void) {
    globalState.vulkanState.pSwapChainFramebuffers = malloc(sizeof(VkFramebuffer) * globalState.vulkanState.swapChainImageCount);

    for (size_t i = 0; i < globalState.vulkanState.swapChainImageCount; i++) {
        VkImageView attachments[] = {
            globalState.vulkanState.pSwapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = globalState.vulkanState.renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = globalState.vulkanState.swapChainExtent.width,
            .height = globalState.vulkanState.swapChainExtent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(globalState.vulkanState.device, &framebufferInfo, NULL, &globalState.vulkanState.pSwapChainFramebuffers[i]) != VK_SUCCESS) {
            printf("%s - failed to create framebuffer!\n", __FUNCTION__);
        }
    }
}