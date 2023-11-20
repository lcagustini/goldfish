#ifndef GOLDFISH_VKBASE_H
#define GOLDFISH_VKBASE_H

#include <stdbool.h>

extern const uint32_t validationLayersCount;
extern const char *validationLayers[];

extern const uint32_t requiredExtensionCount;
extern const char *requiredExtensions[];

struct vulkanState {
    bool enableValidationLayers;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkInstance instance;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkQueue queue;
    uint32_t graphicsQueueFamilyIndex;

    VkSwapchainKHR swapChain;
    uint32_t swapChainImageCount;
    VkImage *pSwapChainImages;
    VkImageView *pSwapChainImageViews;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkFramebuffer *pSwapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
};

void initVulkan(void);
void destroyVulkan(void);

#endif //GOLDFISH_VKBASE_H
