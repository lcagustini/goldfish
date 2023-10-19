#include <render/framebuffer.h>

#include <string.h>

struct framebuffer activeFramebuffers[MAX_FRAMEBUFFERS];

static framebufferId createFramebuffer(bool scaleToWindow, int width, int height) {
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

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &fb.colorBuffer);

    glGenFramebuffers(1, &fb.FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);
	glBindTexture(GL_TEXTURE_2D, fb.colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorBuffer, 0); 

	glGenRenderbuffers(1, &fb.RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, fb.RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.RBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	activeFramebuffers[id] = fb;

	return id;
}

framebufferId createFixedFramebuffer(int width, int height) {
	return createFramebuffer(false, width, height);
}

framebufferId createWindowFramebuffer() {
	return createFramebuffer(true, 0, 0);
}

void deleteFramebuffer(framebufferId id) {
	glDeleteTextures(1, &activeFramebuffers[id].colorBuffer);
	glDeleteRenderbuffers(1, &activeFramebuffers[id].RBO);
	glDeleteFramebuffers(1, &activeFramebuffers[id].FBO);

	activeFramebuffers[id].valid = false;
}

void updateFramebuffersWindowSize() {
	for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
		if (!activeFramebuffers[i].valid) continue;
		if (!activeFramebuffers[i].scaleToWindow) continue;

		deleteFramebuffer(i);
		framebufferId newBuffer = createWindowFramebuffer();

		if (i != newBuffer) {
			activeFramebuffers[i] = activeFramebuffers[newBuffer];
			activeFramebuffers[newBuffer].valid = false;
		}
	}
}
