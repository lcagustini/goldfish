#ifndef SHADER_H
#define SHADER_H

#include <PVR_PSP2/GLES2/gl2.h>

struct shader {
    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;

    GLuint positionLoc;
    GLuint normalLoc;
    GLuint textCoordLoc;

    GLuint cameraPosLoc;

    GLuint program;
};

int initShaders(struct shader *shader);

#endif
