#include <math/matrix.h>
#include <string.h>
#include <math.h>

void multMatrix(union mat4 *result, union mat4 *srcA, union mat4 *srcB) {
    union mat4 tmp;

    for (int i = 0; i < 4; i++) {
        tmp.mat[i][0] = (srcA->mat[i][0] * srcB->mat[0][0]) +
            (srcA->mat[i][1] * srcB->mat[1][0]) +
            (srcA->mat[i][2] * srcB->mat[2][0]) +
            (srcA->mat[i][3] * srcB->mat[3][0]) ;

        tmp.mat[i][1] = (srcA->mat[i][0] * srcB->mat[0][1]) +
            (srcA->mat[i][1] * srcB->mat[1][1]) +
            (srcA->mat[i][2] * srcB->mat[2][1]) +
            (srcA->mat[i][3] * srcB->mat[3][1]) ;

        tmp.mat[i][2] = (srcA->mat[i][0] * srcB->mat[0][2]) +
            (srcA->mat[i][1] * srcB->mat[1][2]) +
            (srcA->mat[i][2] * srcB->mat[2][2]) +
            (srcA->mat[i][3] * srcB->mat[3][2]) ;

        tmp.mat[i][3] = (srcA->mat[i][0] * srcB->mat[0][3]) +
            (srcA->mat[i][1] * srcB->mat[1][3]) +
            (srcA->mat[i][2] * srcB->mat[2][3]) +
            (srcA->mat[i][3] * srcB->mat[3][3]) ;
    }

    memcpy(result, &tmp, sizeof(union mat4));
}

void loadIdentity(union mat4 *result) {
    memset(result, 0x0, sizeof(union mat4));

    result->mat[0][0] = 1.0f;
    result->mat[1][1] = 1.0f;
    result->mat[2][2] = 1.0f;
    result->mat[3][3] = 1.0f;
}

void translationMatrix(union mat4 *result, float x, float y, float z) {
    union mat4 m1;

    loadIdentity(&m1);

    m1.mat[3][0] = x;
    m1.mat[3][1] = y;
    m1.mat[3][2] = z;

    multMatrix(result, &m1, result);
}

void lookAt(union mat4 *result, struct vec3 position, struct vec3 target, struct vec3 worldUp) {
    struct vec3 dir = vectorSubtract(target, position);
    vectorNormalize(&dir);

    struct vec3 right = vectorCross(dir, worldUp);
    vectorNormalize(&right);

    struct vec3 up = vectorCross(right, dir);

    loadIdentity(result);
    result->mat[0][0] = right.x;
    result->mat[1][0] = right.y;
    result->mat[2][0] = right.z;
    result->mat[0][1] = up.x;
    result->mat[1][1] = up.y;
    result->mat[2][1] = up.z;
    result->mat[0][2] = -dir.x;
    result->mat[1][2] = -dir.y;
    result->mat[2][2] = -dir.z;
    result->mat[3][0] = -vectorDot(right, position);
    result->mat[3][1] = -vectorDot(up, position);
    result->mat[3][2] = vectorDot(dir, position);
}

void createProjectionMatrix(union mat4 *result, int yFOV, float aspect) {
    memset(result, 0, sizeof(union mat4));

    float near = 0.1f;
    float far  = 100.0f;
    yFOV = yFOV * M_PI / 180;

    float tanHalfFovy = tan(yFOV / 2.0f);

    result->mat[0][0] = 1 / (aspect * tanHalfFovy);
    result->mat[1][1] = 1 / (tanHalfFovy);
    result->mat[2][2] = far / (near - far);
    result->mat[2][3] = -1;
    result->mat[3][2] = -(far * near) / (far - near);
}

