#define MAX_OBJ_SIZE 2000

struct vertex {
    struct vec3 position;
    struct vec3 normal;
    struct vec2 texture;
};

enum faceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_ALL
};

enum textureType {
    TEXTURE_DIFFUSE,
    TEXTURE_NORMAL,
    TEXTURE_SPECULAR
};

struct texture {
    unsigned int textureBuffer;
    enum textureType type;
};

struct mesh {
    struct vertex *vertices;
    unsigned int verticesLength;

    unsigned int *indices;
    unsigned int indicesLength;

    struct texture *textures;
    unsigned int texturesLength;

    unsigned int VAO, VBO, EBO;
};

void setupMesh(struct mesh *mesh) {
    glGenBuffers(1, &mesh->VBO);
    GLERROR()
    glGenBuffers(1, &mesh->EBO);

    GLERROR()
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    GLERROR()
    glBufferData(GL_ARRAY_BUFFER, mesh->verticesLength * sizeof(struct vertex), mesh->vertices, GL_STATIC_DRAW);

    GLERROR()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    GLERROR()
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesLength * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

    GLERROR()
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLERROR()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLERROR()
}

void drawMesh(struct mesh *mesh, unsigned int shader) {
    glUseProgram(program);

    // draw mesh
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);

    GLERROR()

    // vertex positions
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(textCoordLoc);
    glVertexAttribPointer(textCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, texture));

    GLERROR()

    glDrawElements(GL_TRIANGLES, mesh->indicesLength, GL_UNSIGNED_INT, 0);

    GLERROR()

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

struct mesh *processMesh(struct mesh *mesh, struct aiMesh *aiMesh, const struct aiScene *scene) {
    print("--Processing Mesh (%d vertices, ", aiMesh->mNumVertices);

    mesh->vertices = malloc(aiMesh->mNumVertices * sizeof(struct vertex));

    mesh->indicesLength = 0;
    for (int i = 0; i < aiMesh->mNumFaces; i++) {
        struct aiFace face = aiMesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            mesh->indicesLength++;
        }
    }
    mesh->indices = malloc(mesh->indicesLength * sizeof(unsigned int));
    print("%d indices, %d faces)\n", mesh->indicesLength, aiMesh->mNumFaces);

    for (int i = 0; i < aiMesh->mNumVertices; i++) {
        mesh->vertices[mesh->verticesLength].position.x = aiMesh->mVertices[i].x;
        mesh->vertices[mesh->verticesLength].position.y = aiMesh->mVertices[i].y;
        mesh->vertices[mesh->verticesLength].position.z = aiMesh->mVertices[i].z;

        mesh->vertices[mesh->verticesLength].normal.x = aiMesh->mNormals[i].x;
        mesh->vertices[mesh->verticesLength].normal.y = aiMesh->mNormals[i].y;
        mesh->vertices[mesh->verticesLength].normal.z = aiMesh->mNormals[i].z;

        if (aiMesh->mTextureCoords[0]) {
            mesh->vertices[mesh->verticesLength].texture.x = aiMesh->mTextureCoords[0][i].x;
            mesh->vertices[mesh->verticesLength].texture.y = aiMesh->mTextureCoords[0][i].y;
        }
        else {
            mesh->vertices[mesh->verticesLength].texture.x = 0;
            mesh->vertices[mesh->verticesLength].texture.y = 0;
        }

        mesh->verticesLength++;
    }

    int k = 0;
    for (int i = 0; i < aiMesh->mNumFaces; i++) {
        struct aiFace face = aiMesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            mesh->indices[k] = face.mIndices[j];
            k++;
        }
    }

    setupMesh(mesh);

    return mesh;
}

struct model {
    struct mesh *meshes;
    unsigned int meshesLength;

    const char *path;
};

void processNode(struct model *model, struct aiNode *node, const struct aiScene *scene) {
    print("- Processing Node (%d meshes, %d children)\n", node->mNumMeshes, node->mNumChildren);
    for (int i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(&model->meshes[model->meshesLength], mesh, scene);
        model->meshesLength++;
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        processNode(model, node->mChildren[i], scene);
    }
}

struct model *loadModel(const char *path) {
    print("[Model load start]\n");

    const struct aiScene *scene = aiImportFile(path, aiProcessPreset_TargetRealtime_Fast);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        print("Error loading model %s: %s\n", path, aiGetErrorString());
        return NULL;
    }

    struct model *model = malloc(sizeof(struct model));
    memset(model, 0, sizeof(struct model));

    model->path = path;
    model->meshes = malloc(scene->mNumMeshes * sizeof(struct mesh));
    memset(model->meshes, 0, sizeof(struct mesh));

    print("File has %d meshes\n", scene->mNumMeshes);
    processNode(model, scene->mRootNode, scene);

    aiReleaseImport(scene);

    print("[Model load end]\n\n");

    return model;
}

void drawModel(struct model *model, unsigned int shader) {
    for (int i = 0; i < model->meshesLength; i++) {
        drawMesh(&model->meshes[i], shader);
    }
}

void printModel(struct model *model) {
    print("[Model %s]\n", model->path);
    print("Total meshes: %u\n", model->meshesLength);
    for (int i = 0; i < model->meshesLength; i++) {
        struct mesh *mesh = &model->meshes[i];

        print("-Mesh %d\n", i);
        print("-Vertices %u\n", mesh->verticesLength);
        for (int j = 0; j < mesh->verticesLength; j++) {
            print("--v(%f, %f, %f) vn(%f, %f, %f) vt(%f, %f)\n", mesh->vertices[j].position.x, mesh->vertices[j].position.y, mesh->vertices[j].position.z, mesh->vertices[j].normal.x, mesh->vertices[j].normal.y, mesh->vertices[j].normal.z, mesh->vertices[j].texture.x, mesh->vertices[j].texture.y);
        }

        print("-Indices %u\n", mesh->indicesLength);
        for (int j = 0; j < mesh->indicesLength; j++) {
            print("--%u\n", mesh->indices[j]);
        }

        print("-VBO %d EBO %d\n", mesh->VBO, mesh->EBO);
    }
    print("\n");
}
