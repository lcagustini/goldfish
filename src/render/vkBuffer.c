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

void createVertexBuffer() {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateBuffer(globalState.vulkanState.device, &bufferInfo, NULL, &globalState.vulkanState.vertexBuffer) != VK_SUCCESS) {
        printf("%s - failed to create vertex buffer!\n", __FUNCTION__);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(globalState.vulkanState.device, globalState.vulkanState.vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };

    if (vkAllocateMemory(globalState.vulkanState.device, &allocInfo, NULL, &globalState.vulkanState.vertexBufferMemory) != VK_SUCCESS) {
        printf("%s - failed to allocate vertex buffer memory!\n", __FUNCTION__);
    }

    vkBindBufferMemory(globalState.vulkanState.device, globalState.vulkanState.vertexBuffer, globalState.vulkanState.vertexBufferMemory, 0);

    void* data;
    vkMapMemory(globalState.vulkanState.device, globalState.vulkanState.vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices, (size_t) bufferInfo.size);
    vkUnmapMemory(globalState.vulkanState.device, globalState.vulkanState.vertexBufferMemory);
}