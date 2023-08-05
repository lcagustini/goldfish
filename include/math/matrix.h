#ifndef MATRIX_H
#define MATRIX_H

#include <math/vector.h>

union mat4 {
    float mat[4][4];

    struct {
        float a1, a2, a3, a4;
        float b1, b2, b3, b4;
        float c1, c2, c3, c4;
        float d1, d2, d3, d4;
    };

    struct {
        float a[4];
        float b[4];
        float c[4];
        float d[4];
    };
};

void multMatrix(union mat4 *result, union mat4 *srcA, union mat4 *srcB);

void loadIdentity(union mat4 *result);

void translationMatrix(union mat4 *result, struct vec3 vec);
void scalingMatrix(union mat4 *result, struct vec3 vec);
void rotationMatrix(union mat4 *result, struct quat quat);

struct quat rotationMatrixToQuat(union mat4 src);

void lookAt(union mat4 *result, struct vec3 position, struct vec3 dir, struct vec3 worldUp);
void createProjectionMatrix(union mat4 *result, int yFOV, float aspect);

#endif
