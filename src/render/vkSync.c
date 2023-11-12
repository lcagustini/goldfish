#include <render/vkSync.h>

#include <stdio.h>

void createSyncObjects(void) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    if (vkCreateSemaphore(globalState.vulkanState.device, &semaphoreInfo, NULL, &globalState.vulkanState.imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(globalState.vulkanState.device, &semaphoreInfo, NULL, &globalState.vulkanState.renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(globalState.vulkanState.device, &fenceInfo, NULL, &globalState.vulkanState.inFlightFence) != VK_SUCCESS) {
        printf("%s - failed to create synchronization objects for a frame!\n", __FUNCTION__);
    }
}