#include <render/vkBuffer.h>

#include <global.h>
#include <mesh/mesh.h>

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>

const struct vertex vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(globalState.vulkanState.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    printf("%s - failed to find suitable memory type!", __FUNCTION__);
    return -1;
}

static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(globalState.vulkanState.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        printf("%s - failed to create vertex buffer!\n", __FUNCTION__);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(globalState.vulkanState.device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(globalState.vulkanState.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        printf("%s - failed to allocate vertex buffer memory!\n", __FUNCTION__);
    }

    vkBindBufferMemory(globalState.vulkanState.device, *buffer, *bufferMemory, 0);
}

static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = globalState.vulkanState.commandPool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(globalState.vulkanState.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(globalState.vulkanState.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(globalState.vulkanState.queue);

    vkFreeCommandBuffers(globalState.vulkanState.device, globalState.vulkanState.commandPool, 1, &commandBuffer);
}

void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &globalState.vulkanState.stagingBuffer,
                 &globalState.vulkanState.stagingBufferMemory);

    void* data;
    vkMapMemory(globalState.vulkanState.device, globalState.vulkanState.stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(globalState.vulkanState.device, globalState.vulkanState.stagingBufferMemory);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &globalState.vulkanState.vertexBuffer,
                 &globalState.vulkanState.vertexBufferMemory);

    copyBuffer(globalState.vulkanState.stagingBuffer, globalState.vulkanState.vertexBuffer, bufferSize);

    vkDestroyBuffer(globalState.vulkanState.device, globalState.vulkanState.stagingBuffer, NULL);
    vkFreeMemory(globalState.vulkanState.device, globalState.vulkanState.stagingBufferMemory, NULL);
}