#include <stdlib.h>

#include <GL/glew.h>

#include <render/model.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include <print.h>

static void setupMesh(struct mesh *mesh) {
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->verticesLength * sizeof(struct vertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesLength * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(mesh->material.shader.positionLoc);
    glVertexAttribPointer(mesh->material.shader.positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(mesh->material.shader.normalLoc);
    glVertexAttribPointer(mesh->material.shader.normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(mesh->material.shader.textCoordLoc);
    glVertexAttribPointer(mesh->material.shader.textCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, texture));
    // vertex tangents
    glEnableVertexAttribArray(mesh->material.shader.tangentLoc);
    glVertexAttribPointer(mesh->material.shader.tangentLoc, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, tangent));

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void drawMesh(struct mesh *mesh) {
}

static struct mesh *processMesh(struct mesh *mesh, struct aiMesh *aiMesh, const struct aiScene *scene, struct material material) {
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

        mesh->vertices[mesh->verticesLength].tangent.x = aiMesh->mTangents[i].x;
        mesh->vertices[mesh->verticesLength].tangent.y = aiMesh->mTangents[i].y;
        mesh->vertices[mesh->verticesLength].tangent.z = aiMesh->mTangents[i].z;

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

    mesh->material = material;

    setupMesh(mesh);

    return mesh;
}

static void processNode(struct model *model, struct aiNode *node, const struct aiScene *scene, struct material material) {
    print("- Processing Node (%d meshes, %d children)\n", node->mNumMeshes, node->mNumChildren);
    for (int i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(&model->meshes[model->meshesLength], mesh, scene, material);
        model->meshesLength++;
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        processNode(model, node->mChildren[i], scene, material);
    }
}

struct model loadModel(const char *modelPath, const char *diffusePath, const char *normalPath, const char *specularPath) {
    print("[Model load start]\n");

    const struct aiScene *scene = aiImportFile(modelPath,
            aiProcess_CalcTangentSpace |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_Triangulate |
            aiProcess_GenUVCoords |
            aiProcess_SortByPType |
            aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        print("Error loading model %s: %s\n", modelPath, aiGetErrorString());
        return (struct model) {0};
    }

    struct model model = {0};

    model.path = modelPath;
    model.meshes = malloc(scene->mNumMeshes * sizeof(struct mesh));
    memset(model.meshes, 0, sizeof(struct mesh));

    struct material material = {0};
    createMaterial(&material, diffusePath, normalPath, specularPath);

    print("File has %d meshes\n", scene->mNumMeshes);
    processNode(&model, scene->mRootNode, scene, material);

    aiReleaseImport(scene);

    print("[Model load end]\n\n");

    return model;
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
