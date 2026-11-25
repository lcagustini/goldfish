#include <render/shadow.h>

#include <render/shader.h>

#include <goldfish/print.h>

#include <stdbool.h>

struct shadowMap {
    bool valid;
    bool used;

    enum shadowKind kind;
    int resolution;

    GLuint fbo;
    GLuint texture;
};

static struct shadowMap shadowMaps2D[MAX_SHADOW_2D];
static struct shadowMap shadowMapsCube[MAX_SHADOW_CUBE];

static struct shader shader2D;
static struct shader shaderCube;
static bool shadersReady;
static bool shadersAttempted;

static bool inPass;
static struct shader *activeShader;
static int savedViewport[4];

static bool ensureShaders(void) {
    if (shadersReady) return true;
    if (shadersAttempted) return false;
    shadersAttempted = true;

    if (!createShader(&shader2D, GOLDFISH_ENGINE_ASSETS "/shaders/shadow_v.glsl", GOLDFISH_ENGINE_ASSETS "/shaders/shadow_f.glsl")) {
        print("[shadow] 2D shadow shader failed to build\n");
        return false;
    }
    if (!createShader(&shaderCube, GOLDFISH_ENGINE_ASSETS "/shaders/shadow_v.glsl", GOLDFISH_ENGINE_ASSETS "/shaders/point_shadow_f.glsl")) {
        print("[shadow] cube shadow shader failed to build\n");
        return false;
    }

    shadersReady = true;
    return true;
}

static void destroyMap(struct shadowMap *map) {
    if (map->fbo != 0) glDeleteFramebuffers(1, &map->fbo);
    if (map->texture != 0) glDeleteTextures(1, &map->texture);
    *map = (struct shadowMap) { 0 };
}

static bool createMap(struct shadowMap *map, enum shadowKind kind, int resolution) {
    destroyMap(map);

    map->kind = kind;
    map->resolution = resolution;

    glGenTextures(1, &map->texture);
    glGenFramebuffers(1, &map->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);

    if (kind == SHADOW_KIND_2D) {
        glBindTexture(GL_TEXTURE_2D, map->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Fragments outside the map must not be shadowed.
        GLfloat border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map->texture, 0);
    } else {
        glBindTexture(GL_TEXTURE_CUBE_MAP, map->texture);
        for (int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, map->texture, 0);
    }

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!complete) {
        print("[shadow] framebuffer is incomplete (%d, %d)\n", kind, resolution);
        destroyMap(map);
        return false;
    }

    map->valid = true;
    print("[shadow] %s map %d ready\n", kind == SHADOW_KIND_2D ? "2D" : "cube", resolution);
    return true;
}

static struct shadowMap *shadowPool(enum shadowKind kind, int *count) {
    if (kind == SHADOW_KIND_2D) {
        *count = MAX_SHADOW_2D;
        return shadowMaps2D;
    }
    *count = MAX_SHADOW_CUBE;
    return shadowMapsCube;
}

void shadowBeginFrame(void) {
    for (int i = 0; i < MAX_SHADOW_2D; i++) shadowMaps2D[i].used = false;
    for (int i = 0; i < MAX_SHADOW_CUBE; i++) shadowMapsCube[i].used = false;
}

int shadowAcquire(enum shadowKind kind, int resolution) {
    if (resolution < 256) resolution = 256;
    if (resolution > 4096) resolution = 4096;

    if (!ensureShaders()) return -1;

    int count;
    struct shadowMap *pool = shadowPool(kind, &count);

    for (int i = 0; i < count; i++) {
        struct shadowMap *map = &pool[i];
        if (!map->used && map->valid && map->kind == kind && map->resolution == resolution) {
            map->used = true;
            return i;
        }
    }

    for (int i = 0; i < count; i++) {
        struct shadowMap *map = &pool[i];
        if (map->used) continue;
        if (!createMap(map, kind, resolution)) return -1;
        map->used = true;
        return i;
    }

    // Every slot is used by another light this frame.
    static bool warned2D = false;
    static bool warnedCube = false;
    bool *warned = kind == SHADOW_KIND_2D ? &warned2D : &warnedCube;
    if (!*warned) {
        print("[shadow] all %d %s shadow maps are in use; extra casters are skipped\n", count, kind == SHADOW_KIND_2D ? "2D" : "cube");
        *warned = true;
    }
    return -1;
}

GLuint shadowTexture2D(int slot) {
    if (slot < 0 || slot >= MAX_SHADOW_2D) return 0;
    return shadowMaps2D[slot].texture;
}

GLuint shadowTextureCube(int slot) {
    if (slot < 0 || slot >= MAX_SHADOW_CUBE) return 0;
    return shadowMapsCube[slot].texture;
}

static void beginPass(void) {
    if (inPass) return;

    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    inPass = true;
}

void shadowBegin2D(int slot, union mat4 *lightSpaceMatrix) {
    if (slot < 0 || slot >= MAX_SHADOW_2D || !shadowMaps2D[slot].valid) return;

    struct shadowMap *map = &shadowMaps2D[slot];

    beginPass();
    glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
    glViewport(0, 0, map->resolution, map->resolution);
    glClear(GL_DEPTH_BUFFER_BIT);

    activeShader = &shader2D;
    glUseProgram(shader2D.program);
    glUniformMatrix4fv(shaderUniformLocation(&shader2D, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix->a1);
}

void shadowBeginCube(int slot, int face, union mat4 *faceMatrix, struct vec3 lightPosition, float farPlane) {
    if (slot < 0 || slot >= MAX_SHADOW_CUBE || !shadowMapsCube[slot].valid) return;
    if (face < 0 || face > 5) return;

    struct shadowMap *map = &shadowMapsCube[slot];

    beginPass();
    glBindFramebuffer(GL_FRAMEBUFFER, map->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, map->texture, 0);
    glViewport(0, 0, map->resolution, map->resolution);
    glClear(GL_DEPTH_BUFFER_BIT);

    activeShader = &shaderCube;
    glUseProgram(shaderCube.program);
    glUniformMatrix4fv(shaderUniformLocation(&shaderCube, "lightSpaceMatrix"), 1, GL_FALSE, &faceMatrix->a1);
    glUniform3fv(shaderUniformLocation(&shaderCube, "lightPosition"), 1, &lightPosition.x);
    glUniform1f(shaderUniformLocation(&shaderCube, "farPlane"), farPlane);
}

void shadowDraw(struct meshRenderData *mesh) {
    if (activeShader == NULL) return;

    glUniformMatrix4fv(shaderUniformLocation(activeShader, "model"), 1, GL_FALSE, &mesh->modelMatrix.a1);
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->indicesLength, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void shadowEnd(void) {
    if (!inPass) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
    inPass = false;
    activeShader = NULL;
}
