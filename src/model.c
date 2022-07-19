#define MAX_OBJ_SIZE 2000

struct TNPVertex { // Texture, Normal, Position
    SceFVector3 position;
    SceFVector3 normal;
    SceFVector2 texture;
};

enum faceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_ALL
};

struct model {
    enum faceType face_type;

    struct TNPVertex *vertices;
    int num_vertices;

    GLuint vertexBuffer;
    GLuint textureBuffer;
};

struct model loaded_models[10];
int loaded_models_n;

void drawModel(int model, SceFVector3 *pos, SceFVector3 *rot, SceFVector3 *scale) {
    glBindBuffer(GL_ARRAY_BUFFER, loaded_models[model].vertexBuffer);
    
    glBindTexture(GL_TEXTURE_2D, loaded_models[model].textureBuffer);

    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(normalLoc);
    glVertexAttribPointer(textCoordLoc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(textCoordLoc);

    glUniformMatrix4fv(modelLoc, 1, false, &modelMat->mat[0][0]);
    glUniformMatrix4fv(viewLoc, 1, false, &viewMat->mat[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, false, &projectionMat->mat[0][0]);

    glDrawArrays(GL_TRIANGLES, 0 , loaded_models[model].num_vertices);

    glDisableVertexAttribArray(positionLoc);
    glDisableVertexAttribArray(normalLoc);
    glDisableVertexAttribArray(textCoordLoc);
}

void destroyModel(int model) {
    free(loaded_models[model].vertices);
    glDeleteBuffers(1, &loaded_models[model].vertexBuffer);
    memset(&loaded_models[model], 0, sizeof(loaded_models[model]));
}

void updateModelVertices(int model) {
    glBindBuffer(GL_ARRAY_BUFFER, loaded_models[model].vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, loaded_models[model].num_vertices * sizeof(struct TNPVertex), loaded_models[model].vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int loadModel(const char *obj_filename, const char *texture_filename, enum faceType face_type) {
    struct {
        struct face {
            int vertices[3];
            int normals[3];
            int texture_coords[3];
        } faces[MAX_OBJ_SIZE];
        int num_faces;
        SceFVector3 vertices[MAX_OBJ_SIZE];
        int num_vertices;
        SceFVector3 normals[MAX_OBJ_SIZE];
        int num_normals;
        SceFVector2 texture_coords[MAX_OBJ_SIZE];
        int num_texture_coords;
    } file = {0};

    FILE *f = fopen(obj_filename, "r");

    char buffer[40] = {0};
    while ((fscanf(f, " %s", buffer)) != EOF) {
        if (!strcmp(buffer, "v")) {
            SceFVector3 v = {};
            fscanf(f, " %f %f %f", &v.x, &v.y, &v.z);
            file.vertices[++file.num_vertices] = v;
        }
        else if (!strcmp(buffer, "vn")) {
            SceFVector3 n = {};
            fscanf(f, " %f %f %f", &n.x, &n.y, &n.z);
            file.normals[++file.num_normals] = n;
        }
        else if (!strcmp(buffer, "vt")) {
            SceFVector2 t = {};
            fscanf(f, " %f %f", &t.x, &t.y);
            t.y = 1 - t.y;
            file.texture_coords[++file.num_texture_coords] = t;
        }
        else if (!strcmp(buffer, "f")) {
            struct face face = {};
            switch (face_type) {
                case VERTEX_ONLY:
                    fscanf(f, " %d %d %d",
                            &face.vertices[0], &face.vertices[1], &face.vertices[2]
                          );
                    break;
                case VERTEX_NORMAL:
                    fscanf(f, " %d//%d %d//%d %d//%d",
                            &face.vertices[0], &face.normals[0], &face.vertices[1],
                            &face.normals[1], &face.vertices[2], &face.normals[2]
                          );
                    break;
                case VERTEX_ALL:
                    fscanf(f, " %d/%d/%d %d/%d/%d %d/%d/%d",
                            &face.vertices[0], &face.texture_coords[0], &face.normals[0],
                            &face.vertices[1], &face.texture_coords[1], &face.normals[1],
                            &face.vertices[2], &face.texture_coords[2], &face.normals[2]
                          );
                    break;
                default:
                    assert(false);
            }

            file.faces[file.num_faces++] = face;
        }
    }
    fclose(f);

    struct model *model = &loaded_models[loaded_models_n];

    model->face_type = face_type;
    model->vertices = malloc(3 * file.num_faces * sizeof(struct TNPVertex));
    model->num_vertices = 3 * file.num_faces;

    glGenBuffers(1, &model->vertexBuffer);
    glGenTextures(1, &model->textureBuffer);

    qoi_desc desc;
    void *rgb_pixels = qoi_read(texture_filename, &desc, 3);

    glBindTexture(GL_TEXTURE_2D, model->textureBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, desc.width, desc.height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb_pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    free(rgb_pixels);

    int k = 0;
    for (int i = 0; i < file.num_faces; i++) {
        model->vertices[k].normal = file.normals[file.faces[i].normals[0]];
        model->vertices[k+1].normal = file.normals[file.faces[i].normals[1]];
        model->vertices[k+2].normal = file.normals[file.faces[i].normals[2]];

        model->vertices[k].position = file.vertices[file.faces[i].vertices[0]];
        model->vertices[k+1].position = file.vertices[file.faces[i].vertices[1]];
        model->vertices[k+2].position = file.vertices[file.faces[i].vertices[2]];

        model->vertices[k].texture = file.texture_coords[file.faces[i].texture_coords[0]];
        model->vertices[k+1].texture = file.texture_coords[file.faces[i].texture_coords[1]];
        model->vertices[k+2].texture = file.texture_coords[file.faces[i].texture_coords[2]];

        k += 3;
    }

    updateModelVertices(loaded_models_n);

    loaded_models_n++;
    return loaded_models_n - 1;
}
