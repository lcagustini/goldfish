#ifndef GOLDFISH_SHADOW_H
#define GOLDFISH_SHADOW_H

#include <render/material.h>

#include <math/matrix.h>
#include <math/vector.h>

enum shadowKind {
    SHADOW_KIND_2D,
    SHADOW_KIND_CUBE,
};

// Clears the per-frame slot usage. Call once before the shadow passes.
void shadowBeginFrame(void);

// Claims a map for this frame. Returns -1 when the pool for the kind is full.
// The map is created or recreated when the kind or resolution changes.
int shadowAcquire(enum shadowKind kind, int resolution);

GLuint shadowTexture2D(int slot);
GLuint shadowTextureCube(int slot);

void shadowBegin2D(int slot, union mat4 *lightSpaceMatrix);
void shadowBeginCube(int slot, int face, union mat4 *faceMatrix, struct vec3 lightPosition, float farPlane);
void shadowDraw(struct meshRenderData *mesh);
void shadowEnd(void);

#endif
