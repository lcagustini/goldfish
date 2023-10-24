#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define MAX_FRAMEBUFFERS 10

#include <global.h>

#include <stdbool.h>

typedef unsigned int framebufferId;

enum framebufferType {
    FRAMEBUFFER_COLOR_DEPTH,
    FRAMEBUFFER_COLOR_ONLY,
    FRAMEBUFFER_DEPTH_ONLY,
};

struct framebuffer {
    bool valid;
    bool scaleToWindow;

    enum framebufferType type;

    GLuint colorBuffer;
    GLuint depthBuffer;
    GLuint FBO;
};

extern struct framebuffer activeFramebuffers[MAX_FRAMEBUFFERS];

framebufferId createFixedFramebuffer(enum framebufferType type, int width, int height);
framebufferId createWindowFramebuffer(enum framebufferType type);

void updateFramebuffersWindowSize();

#endif
