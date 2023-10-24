#include <render/framebuffer.h>

#include <string.h>

struct framebuffer activeFramebuffers[MAX_FRAMEBUFFERS];

static void createFramebufferColorBuffer(struct framebuffer *fb, int width, int height) {
    glGenTextures(1, &fb->colorBuffer);
    glBindTexture(GL_TEXTURE_2D, fb->colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->colorBuffer, 0);
}

static void createFramebufferDepthBuffer(struct framebuffer *fb, int width, int height) {
    glGenTextures(1, &fb->depthBuffer);
    glBindTexture(GL_TEXTURE_2D, fb->depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb->depthBuffer, 0);
}

static framebufferId createFramebuffer(enum framebufferType type, bool scaleToWindow, int width, int height) {
    framebufferId id = -1;

    for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
        if (!activeFramebuffers[i].valid) {
            id = i;
            break;
        }
    }

    if (id == -1) return -1;

    struct framebuffer fb = { 0 };
    fb.scaleToWindow = scaleToWindow;
    fb.valid = true;

    if (scaleToWindow) {
        glfwGetWindowSize(globalState.window, &width, &height);
    }

    glGenFramebuffers(1, &fb.FBO);
    glActiveTexture(GL_TEXTURE0);

    glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);

    switch (type) {
        case FRAMEBUFFER_COLOR_DEPTH:
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            createFramebufferColorBuffer(&fb, width, height);
            createFramebufferDepthBuffer(&fb, width, height);
            break;
        case FRAMEBUFFER_COLOR_ONLY:
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            createFramebufferColorBuffer(&fb, width, height);
            break;
        case FRAMEBUFFER_DEPTH_ONLY:
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            createFramebufferDepthBuffer(&fb, width, height);
            break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    activeFramebuffers[id] = fb;

    return id;
}

framebufferId createFixedFramebuffer(enum framebufferType type, int width, int height) {
    return createFramebuffer(type, false, width, height);
}

framebufferId createWindowFramebuffer(enum framebufferType type) {
    return createFramebuffer(type, true, 0, 0);
}

void deleteFramebuffer(framebufferId id) {
    glDeleteTextures(1, &activeFramebuffers[id].colorBuffer);
    glDeleteTextures(1, &activeFramebuffers[id].depthBuffer);
    glDeleteFramebuffers(1, &activeFramebuffers[id].FBO);

    memset(&activeFramebuffers[id], 0, sizeof(struct framebuffer));
    activeFramebuffers[id].valid = false;
}

void updateFramebuffersWindowSize() {
    for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
        if (!activeFramebuffers[i].valid) continue;
        if (!activeFramebuffers[i].scaleToWindow) continue;

        enum framebufferType type = activeFramebuffers[i].type;

        deleteFramebuffer(i);
        framebufferId newBuffer = createWindowFramebuffer(type);

        if (i != newBuffer) {
            activeFramebuffers[i] = activeFramebuffers[newBuffer];
            activeFramebuffers[newBuffer].valid = false;
        }
    }
}
