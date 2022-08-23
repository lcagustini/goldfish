#ifndef MATERIAL_H
#define MATERIAL_H

#include <PVR_PSP2/GLES2/gl2.h>

struct shader {
    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;

    GLuint positionLoc;
    GLuint normalLoc;
    GLuint textCoordLoc;
    GLuint tangentLoc;

    GLuint cameraPosLoc;

    GLuint program;
};

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR,
    
    TEXTURE_MAX
};

struct texture {
    unsigned int textureBuffer;
    enum textureType type;
    const char *path;
};

struct material {
    struct texture textures[TEXTURE_MAX];
    unsigned int texturesLength;

    struct shader shader;
};

void createMaterial(struct material *material, const char *diffusePath, const char *normalPath, const char *specularPath);

#endif
