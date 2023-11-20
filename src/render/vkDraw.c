#include <render/vkDraw.h>

#include <global.h>

#include <stdio.h>

static void recordCommandBuffer(uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    if (vkBeginCommandBuffer(globalState.vulkanState.commandBuffer, &beginInfo) != VK_SUCCESS) {
        printf("%s - failed to begin recording command buffer!\n", __FUNCTION__);
    }

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = globalState.vulkanState.renderPass,
        .framebuffer = globalState.vulkanState.pSwapChainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = globalState.vulkanState.swapChainExtent,
    };

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(globalState.vulkanState.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(globalState.vulkanState.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, globalState.vulkanState.graphicsPipeline);

    VkBuffer vertexBuffers[] = { globalState.vulkanState.vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(globalState.vulkanState.commandBuffer, 0, 1, vertexBuffers, offsets);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) globalState.vulkanState.swapChainExtent.width,
        .height = (float) globalState.vulkanState.swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(globalState.vulkanState.commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = globalState.vulkanState.swapChainExtent,
    };
    vkCmdSetScissor(globalState.vulkanState.commandBuffer, 0, 1, &scissor);

    vkCmdDraw(globalState.vulkanState.commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(globalState.vulkanState.commandBuffer);

    if (vkEndCommandBuffer(globalState.vulkanState.commandBuffer) != VK_SUCCESS) {
        printf("%s - failed to record command buffer!\n", __FUNCTION__);
    }
}

void drawFrame(void) {
    vkWaitForFences(globalState.vulkanState.device, 1, &globalState.vulkanState.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(globalState.vulkanState.device, 1, &globalState.vulkanState.inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(globalState.vulkanState.device, globalState.vulkanState.swapChain, UINT64_MAX, globalState.vulkanState.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(globalState.vulkanState.commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo = {
        .sType = submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };

    VkSemaphore waitSemaphores[] = {globalState.vulkanState.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &globalState.vulkanState.commandBuffer;

    VkSemaphore signalSemaphores[] = {globalState.vulkanState.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(globalState.vulkanState.queue, 1, &submitInfo, globalState.vulkanState.inFlightFence) != VK_SUCCESS) {
        printf("%s - failed to submit draw command buffer!\n", __FUNCTION__);
    }

    VkPresentInfoKHR presentInfo = {
        .sType = presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
    };

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {globalState.vulkanState.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(globalState.vulkanState.queue, &presentInfo);
}