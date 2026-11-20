#include <render/shader.h>

#include <data/dynarray.h>
#include <goldfish/print.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

static struct dynarray shaderTracks = { 0 };

static const char *const pinnedAttributes[] = { "aPos", "aNormal", "aTexCoord", "aTangent" };

static const struct { const char *name; int slot; } defaultSamplers[] = {
    { "textureMap", TEXTURE_DIFFUSE },
    { "normalMap", TEXTURE_NORMAL },
    { "specularMap", TEXTURE_SPECULAR },
    { "reflectanceMap", TEXTURE_REFLECTANCE },
    { "skybox", TEXTURE_CUBEMAP },
};

static char *copyString(const char *source) {
    size_t length = strlen(source);
    char *copy = malloc(length + 1);
    memcpy(copy, source, length + 1);
    return copy;
}

static char *readFile(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(length + 1);
    if (fread(content, 1, length, file) != (size_t)length) {
        print("[shader] short read on %s\n", path);
        free(content);
        fclose(file);
        return NULL;
    }
    content[length] = '\0';
    fclose(file);

    return content;
}

static void appendText(struct dynarray *out, const char *text) {
    for (const char *c = text; *c != '\0'; c++) dynarrayAdd(out, (void *)c);
}

static bool pathInList(struct dynarray *list, const char *path) {
    for (unsigned int i = 0; i < list->bufferCount; i++) {
        if (strcmp(*(char **)dynarrayGet(list, i), path) == 0) return true;
    }
    return false;
}

static char *findInclude(const char *fromPath, const char *name) {
    char candidate[512];
    const char *slash = strrchr(fromPath, '/');

    if (slash != NULL) {
        snprintf(candidate, sizeof(candidate), "%.*s/%s", (int)(slash - fromPath), fromPath, name);
        if (fopen(candidate, "rb") != NULL) return copyString(candidate);
    }
    snprintf(candidate, sizeof(candidate), GOLDFISH_ENGINE_ASSETS "/shaders/include/%s", name);
    if (fopen(candidate, "rb") != NULL) return copyString(candidate);
    snprintf(candidate, sizeof(candidate), GOLDFISH_ENGINE_ASSETS "/shaders/%s", name);
    if (fopen(candidate, "rb") != NULL) return copyString(candidate);

    return NULL;
}

static bool resolveInto(const char *path, struct dynarray *out, struct dynarray *visited, struct dynarray *includes) {
    char *content = readFile(path);
    if (content == NULL) {
        print("[shader] cannot read %s\n", path);
        return false;
    }

    char *pathCopy = copyString(path);
    dynarrayAdd(visited, &pathCopy);

    char name[256];
    char *line = content;
    while (line != NULL) {
        char *next = strchr(line, '\n');
        if (next != NULL) *next = '\0';

        if (sscanf(line, " #include \" %255[^\"]\"", name) == 1) {
            char *includePath = findInclude(path, name);
            if (includePath == NULL) {
                print("[shader] %s: include \"%s\" not found\n", path, name);
                free(content);
                return false;
            }

            if (!pathInList(visited, includePath)) {
                char *listCopy = copyString(includePath);
                dynarrayAdd(includes, &listCopy);

                if (!resolveInto(includePath, out, visited, includes)) {
                    free(includePath);
                    free(content);
                    return false;
                }
            }
            free(includePath);
        }
        else {
            appendText(out, line);
            char newline = '\n';
            dynarrayAdd(out, &newline);
        }

        line = next != NULL ? next + 1 : NULL;
    }

    free(content);
    return true;
}

static bool resolveShader(const char *path, char **sourceOut, struct dynarray *includes) {
    struct dynarray visited = DYNARRAY_CREATE(8, char *);
    struct dynarray out = DYNARRAY_CREATE(4096, char);

    bool ok = resolveInto(path, &out, &visited, includes);

    char terminator = '\0';
    if (ok) dynarrayAdd(&out, &terminator);

    for (unsigned int i = 0; i < visited.bufferCount; i++) free(*(char **)dynarrayGet(&visited, i));
    dynarrayDestroy(&visited);

    if (ok) {
        *sourceOut = malloc(out.bufferCount);
        memcpy(*sourceOut, out.buffer, out.bufferCount);
    }
    dynarrayDestroy(&out);

    return ok;
}

static bool compileShader(GLuint *out, GLenum type, const char *source, const char *path) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 1) {
            char *infoLog = malloc(infoLength);
            glGetShaderInfoLog(shader, infoLength, NULL, infoLog);
            print("[shader] compile error in %s:\n%s\n", path, infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return false;
    }

    *out = shader;
    return true;
}

static bool buildShader(struct shader *result, const char *vertexPath, const char *fragmentPath, char ***includesOut, unsigned int *includesLengthOut) {
    struct dynarray includes = DYNARRAY_CREATE(8, char *);
    struct shader built = { 0 };

    char *vertexSource = NULL;
    char *fragmentSource = NULL;
    GLuint vshader = 0;
    GLuint fshader = 0;
    GLuint program = 0;

    if (!resolveShader(vertexPath, &vertexSource, &includes) || !resolveShader(fragmentPath, &fragmentSource, &includes)) {
        goto fail;
    }

    if (!compileShader(&vshader, GL_VERTEX_SHADER, vertexSource, vertexPath) ||
        !compileShader(&fshader, GL_FRAGMENT_SHADER, fragmentSource, fragmentPath)) {
        goto fail;
    }

    program = glCreateProgram();
    if (program == 0) {
        print("[shader] glCreateProgram failed\n");
        goto fail;
    }

    glAttachShader(program, vshader);
    glAttachShader(program, fshader);

    for (unsigned int i = 0; i < 4; i++) glBindAttribLocation(program, i, pinnedAttributes[i]);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLchar log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        print("[shader] link error in %s/%s:\n%s\n", vertexPath, fragmentPath, log);
        goto fail;
    }

    glDeleteShader(vshader);
    glDeleteShader(fshader);
    free(vertexSource);
    free(fragmentSource);

    built.uniforms = malloc(sizeof(struct hashtable));
    *built.uniforms = HASHTABLE_CREATE(64, GLint);
    built.program = program;

    *result = built;

    glUseProgram(program);
    for (unsigned int i = 0; i < sizeof(defaultSamplers) / sizeof(defaultSamplers[0]); i++) {
        GLint location = glGetUniformLocation(program, defaultSamplers[i].name);
        if (location != -1) glUniform1i(location, defaultSamplers[i].slot);
    }

    *includesOut = malloc(includes.bufferCount * sizeof(char *));
    memcpy(*includesOut, includes.buffer, includes.bufferCount * sizeof(char *));
    *includesLengthOut = includes.bufferCount;
    dynarrayDestroy(&includes); // ownership moves to includesOut

    return true;

fail:
    glDeleteShader(vshader); // 0 is silently ignored
    glDeleteShader(fshader);
    glDeleteProgram(program);
    free(vertexSource);
    free(fragmentSource);
    for (unsigned int i = 0; i < includes.bufferCount; i++) free(*(char **)dynarrayGet(&includes, i));
    dynarrayDestroy(&includes);
    return false;
}

struct shaderTrack {
    struct shader *shader;

    char *vertexPath;
    char *fragmentPath;
    char **includes;
    unsigned int includesLength;

    // mtimes: vertex, fragment, then includes.
    time_t *mtimes;
};

static void statTrack(struct shaderTrack *track) {
    track->mtimes = malloc((2 + track->includesLength) * sizeof(time_t));

    struct stat st;
    if (stat(track->vertexPath, &st) == 0) track->mtimes[0] = st.st_mtime;
    if (stat(track->fragmentPath, &st) == 0) track->mtimes[1] = st.st_mtime;
    for (unsigned int i = 0; i < track->includesLength; i++) {
        if (stat(track->includes[i], &st) == 0) track->mtimes[2 + i] = st.st_mtime;
    }
}

static void registerTrack(struct shader *shader, const char *vertexPath, const char *fragmentPath, char **includes, unsigned int includesLength) {
    if (shaderTracks.typeSize == 0) shaderTracks = DYNARRAY_CREATE(8, struct shaderTrack);

    for (unsigned int i = 0; i < shaderTracks.bufferCount; i++) {
        struct shaderTrack *track = dynarrayGet(&shaderTracks, i);
        if (track->shader != shader) continue;

        free(track->vertexPath);
        free(track->fragmentPath);
        for (unsigned int j = 0; j < track->includesLength; j++) free(track->includes[j]);
        free(track->includes);
        free(track->mtimes);

        track->vertexPath = copyString(vertexPath);
        track->fragmentPath = copyString(fragmentPath);
        track->includes = includes;
        track->includesLength = includesLength;
        statTrack(track);
        return;
    }

    struct shaderTrack track = {
        .shader = shader,
        .vertexPath = copyString(vertexPath),
        .fragmentPath = copyString(fragmentPath),
        .includes = includes,
        .includesLength = includesLength,
    };
    statTrack(&track);
    dynarrayAdd(&shaderTracks, &track);
}

bool createShader(struct shader *shader, const char *vertexPath, const char *fragmentPath) {
    char **includes = NULL;
    unsigned int includesLength = 0;

    struct shader built = { 0 };
    if (!buildShader(&built, vertexPath, fragmentPath, &includes, &includesLength)) return false;
    *shader = built;

    registerTrack(shader, vertexPath, fragmentPath, includes, includesLength);
    return true;
}

bool shaderReplace(struct shader *shader, const char *vertexPath, const char *fragmentPath) {
    char **includes = NULL;
    unsigned int includesLength = 0;

    struct shader built = { 0 };
    if (!buildShader(&built, vertexPath, fragmentPath, &includes, &includesLength)) {
        return false;
    }

    if (shader->uniforms != NULL) {
        hashtableDestroy(shader->uniforms);
        free(shader->uniforms);
    }
    *shader = built;

    registerTrack(shader, vertexPath, fragmentPath, includes, includesLength);
    return true;
}

void shaderHotReload(void) {
    for (unsigned int i = 0; i < shaderTracks.bufferCount; i++) {
        struct shaderTrack *track = dynarrayGet(&shaderTracks, i);

        bool changed = false;
        struct stat st;
        if (stat(track->vertexPath, &st) == 0 && st.st_mtime != track->mtimes[0]) changed = true;
        if (stat(track->fragmentPath, &st) == 0 && st.st_mtime != track->mtimes[1]) changed = true;
        for (unsigned int j = 0; j < track->includesLength && !changed; j++) {
            if (stat(track->includes[j], &st) == 0 && st.st_mtime != track->mtimes[2 + j]) changed = true;
        }
        if (!changed) continue;

        // Store the new mtimes either way: failures wait for the next save.
        statTrack(track);

        print("[shader] source changed, reloading %s\n", track->fragmentPath);

        struct shader reloaded = { 0 };
        char **includes = NULL;
        unsigned int includesLength = 0;
        if (buildShader(&reloaded, track->vertexPath, track->fragmentPath, &includes, &includesLength)) {
            glDeleteProgram(track->shader->program);
            if (track->shader->uniforms != NULL) {
                hashtableDestroy(track->shader->uniforms);
                free(track->shader->uniforms);
            }
            *track->shader = reloaded;

            for (unsigned int j = 0; j < track->includesLength; j++) free(track->includes[j]);
            free(track->includes);
            free(track->mtimes);
            track->includes = includes;
            track->includesLength = includesLength;
            statTrack(track);
        }
        else {
            print("[shader] reload failed, keeping the previous program\n");
        }
    }
}

GLint shaderUniformLocation(struct shader *shader, const char *name) {
    if (shader->uniforms == NULL) {
        shader->uniforms = malloc(sizeof(struct hashtable));
        *shader->uniforms = HASHTABLE_CREATE(64, GLint);
    }

    GLint *cached = hashtableGet(shader->uniforms, name);
    if (cached != NULL) return *cached;

    GLint location = glGetUniformLocation(shader->program, name);
    hashtableSet(shader->uniforms, name, &location);
    return location;
}

void destroyShader(struct shader *shader) {
    if (shader->uniforms == NULL && shader->program == 0) return;

    for (unsigned int i = 0; i < shaderTracks.bufferCount; i++) {
        struct shaderTrack *track = dynarrayGet(&shaderTracks, i);
        if (track->shader != shader) continue;

        free(track->vertexPath);
        free(track->fragmentPath);
        for (unsigned int j = 0; j < track->includesLength; j++) free(track->includes[j]);
        free(track->includes);
        free(track->mtimes);
        dynarrayRemove(&shaderTracks, i);
        break;
    }

    if (shader->uniforms != NULL) {
        hashtableDestroy(shader->uniforms);
        free(shader->uniforms);
    }
    glDeleteProgram(shader->program);

    shader->uniforms = NULL;
    shader->program = 0;
}
