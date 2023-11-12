#include <render/vkCommandBuffer.h>

#include <stdio.h>

void createCommandPool(void) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = globalState.vulkanState.graphicsQueueFamilyIndex,
    };

    if (vkCreateCommandPool(globalState.vulkanState.device, &poolInfo, NULL, &globalState.vulkanState.commandPool) != VK_SUCCESS) {
        printf("%s - failed to create command pool!\n", __FUNCTION__);
    }
}

void createCommandBuffer(void) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = globalState.vulkanState.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(globalState.vulkanState.device, &allocInfo, &globalState.vulkanState.commandBuffer) != VK_SUCCESS) {
        printf("%s - failed to allocate command buffers!\n", __FUNCTION__);
    }
}