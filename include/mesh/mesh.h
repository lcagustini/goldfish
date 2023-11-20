#ifndef GOLDFISH_MESH_H
#define GOLDFISH_MESH_H

#include <vulkan/vulkan.h>

#include <math/vector.h>

/*
struct vertex {
    struct vec3 position;
    struct vec3 normal;
    struct vec2 texture;
    struct vec3 tangent;
};
 */
struct vertex {
    struct vec2 position;
    struct vec3 color;
};

VkVertexInputBindingDescription getVertexBindingDescription();
void getVertexAttributeDescriptions(VkVertexInputAttributeDescription descriptions[2]);

#endif //GOLDFISH_MESH_H
