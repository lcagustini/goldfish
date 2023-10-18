#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define MAX_FRAMEBUFFERS 10

#include <global.h>

#include <stdbool.h>

typedef unsigned int framebufferId;

struct framebuffer {
    bool valid;
    bool scaleToWindow;

    GLuint colorBuffer;
    GLuint FBO, RBO;
};

extern struct framebuffer activeFramebuffers[MAX_FRAMEBUFFERS];

framebufferId createFixedFramebuffer(int width, int height);
framebufferId createWindowFramebuffer();

void updateFramebuffersWindowSize();

#endif
