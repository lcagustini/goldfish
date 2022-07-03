#define MAX_OBJ_SIZE 2000

struct TNPVertex { // Texture, Normal, Position
    SceFVector2 texture;
    SceFVector3 normal;
    SceFVector3 position;
};
#define TNP_VERTEX_FORMAT (GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_VERTEX_32BITF)

struct NPVertex { // Normal, Position
    SceFVector3 normal;
    SceFVector3 position;
};
#define NP_VERTEX_FORMAT (GU_NORMAL_32BITF | GU_VERTEX_32BITF)

struct PVertex { // Position
    SceFVector3 position;
};
#define P_VERTEX_FORMAT (GU_VERTEX_32BITF)

enum faceType {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_ALL
};

struct model {
    enum faceType face_type;

    void *texture_vram;
    int texture_size;

    struct TNPVertex *vertices;
    int num_vertices;

    GLuint vertexBuffer;
};

struct model loaded_models[10];
int loaded_models_n;

static void *loadTexture(const char *texture_filename, enum faceType face_type) {
    return NULL;
}

/*
static void *loadTexture(const char *texture_filename, enum faceType face_type) {
    FILE *f = fopen(texture_filename, "r");

    fseek(f, 0, SEEK_END);
    int sz = ftell(f) - 128;
    fseek(f, 128, SEEK_SET);

    void *texture = malloc(sz);
    fread(texture, 1, sz, f);
    PSPconvert_dxt1(texture, sz);
    sceGuTexSync();

    fclose(f);

    return texture;
}

void drawModel(int model, SceFVector3 *pos, SceFVector3 *rot, SceFVector3 *scale) {
    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();
    sceGumTranslate(pos);
    sceGumRotateXYZ(rot);
    sceGumScale(scale);

    sceGuTexMode(GU_PSM_DXT1, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
    sceGuTexLevelMode(GU_TEXTURE_AUTO, 0);
    sceGuTexFilter(GU_LINEAR, GU_LINEAR);
    sceGuTexImage(0, loaded_models[model].texture_size, loaded_models[model].texture_size, loaded_models[model].texture_size, loaded_models[model].texture_vram);

    sceGuColor(0xffffff);
    sceGumDrawArray(GU_TRIANGLES, TNP_VERTEX_FORMAT | GU_TRANSFORM_3D, loaded_models[model].num_vertices, NULL, loaded_models[model].vertices);
}
*/

void destroyModel(int model) {
    free(loaded_models[model].vertices);
    glDeleteBuffers(1, &loaded_models[model].vertexBuffer);
    memset(&loaded_models[model], 0, sizeof(loaded_models[model]));
}

void updateModelVertices(int model) {
    //glBindBuffer(GL_ARRAY_BUFFER, vertexID);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

int loadModel(const char *obj_filename, const char *texture_filename, enum faceType face_type, int texture_size) {
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

    if (face_type == VERTEX_ALL) {
        model->texture_vram = loadTexture(texture_filename, face_type);
    }
    model->texture_size = texture_size;
    model->face_type = face_type;
    model->vertices = malloc(3 * file.num_faces * sizeof(struct TNPVertex));
    model->num_vertices = 3 * file.num_faces;

    glGenBuffers(1, &model->vertexBuffer);

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

    loaded_models_n++;
    return loaded_models_n - 1;
}
