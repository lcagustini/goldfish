#include <math/matrix.h>

#include <math/float.h>

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

void translationMatrix(union mat4 *result, struct vec3 vec) {
    union mat4 m1;

    loadIdentity(&m1);

    m1.mat[3][0] = vec.x;
    m1.mat[3][1] = vec.y;
    m1.mat[3][2] = vec.z;

    multMatrix(result, &m1, result);
}

void scalingMatrix(union mat4 *result, struct vec3 vec) {
    union mat4 m1;

    loadIdentity(&m1);

    m1.mat[0][0] = vec.x;
    m1.mat[1][1] = vec.y;
    m1.mat[2][2] = vec.z;

    multMatrix(result, &m1, result);
}

void rotationMatrix(union mat4 *result, struct quat quat) {
    union mat4 m1;

    loadIdentity(&m1);

    m1.mat[0][0] = 1 - (2 * quat.y * quat.y) - (2 * quat.z * quat.z);
    m1.mat[1][0] = (2 * quat.x * quat.y) - (2 * quat.w * quat.z);
    m1.mat[2][0] = (2 * quat.x * quat.z) + (2 * quat.w * quat.y);

    m1.mat[0][1] = (2 * quat.x * quat.y) + (2 * quat.w * quat.z);
    m1.mat[1][1] = 1 - (2 * quat.x * quat.x) - (2 * quat.z * quat.z);
    m1.mat[2][1] = (2 * quat.y * quat.z) - (2 * quat.w * quat.x);

    m1.mat[0][2] = (2 * quat.x * quat.z) - (2 * quat.w * quat.y);
    m1.mat[1][2] = (2 * quat.y * quat.z) + (2 * quat.w * quat.x);
    m1.mat[2][2] = 1 - (2 * quat.x * quat.x) - (2 * quat.y * quat.y);

    multMatrix(result, &m1, result);
}

// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
struct quat rotationMatrixToQuat(union mat4 src) {
    struct quat q;
    float t;

    if (src.mat[2][2] < 0)
    {
        if (src.mat[0][0] > src.mat[1][1])
        {
            t = 1 + src.mat[0][0] - src.mat[1][1] - src.mat[2][2];
            q.x = t;
            q.y = src.mat[0][1] + src.mat[1][0];
            q.z = src.mat[2][0] + src.mat[0][2];
            q.w = src.mat[1][2] - src.mat[2][1];
        }
        else
        {
            t = 1 - src.mat[0][0] + src.mat[1][1] - src.mat[2][2];
            q.x = src.mat[0][1] + src.mat[1][0];
            q.y = t;
            q.z = src.mat[1][2] + src.mat[2][1];
            q.w = src.mat[2][0] - src.mat[0][2];
        }
    }
    else
    {
        if (src.mat[0][0] < -src.mat[1][1])
        {
            t = 1 - src.mat[0][0] - src.mat[1][1] + src.mat[2][2];
            q.x = src.mat[2][0] + src.mat[0][2];
            q.y = src.mat[1][2] + src.mat[2][1];
            q.z = t;
            q.w = src.mat[0][1] - src.mat[1][0];
        }
        else
        {
            t = 1 + src.mat[0][0] + src.mat[1][1] + src.mat[2][2];
            q.x = src.mat[1][2] - src.mat[2][1];
            q.y = src.mat[2][0] - src.mat[0][2];
            q.z = src.mat[0][1] - src.mat[1][0];
            q.w = t;
        }
    }
    t = 0.5f / sqrt(t);

    q.x *= t;
    q.y *= t;
    q.z *= t;
    q.w *= t;

    return q;
}

void lookAt(union mat4 *result, struct vec3 position, struct vec3 dir, struct vec3 worldUp) {
    struct vec3 right = vectorNormalize(vectorCross(dir, worldUp));
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

void createPerspProjectionMatrix(union mat4 *result, float yFOV, float aspect, float near, float far) {
    memset(result, 0, sizeof(union mat4));

    yFOV = yFOV * M_PI / 180.0f;

    float tanHalfFovy = tan(yFOV / 2.0f);

    result->mat[0][0] = 1 / (aspect * tanHalfFovy);
    result->mat[1][1] = 1 / (tanHalfFovy);
    result->mat[2][2] = - ((far + near) / (far - near));
    result->mat[2][3] = -1;
    result->mat[3][2] = (-2 * far * near) / (far - near);
}

void createOrthoProjectionMatrix(union mat4 *result, float size, float aspect, float near, float far) {
    memset(result, 0, sizeof(union mat4));

    float top = size;
    float bottom = -size;

    float right = size * aspect;
    float left = -size * aspect;

    result->mat[0][0] = 2 / (right - left);
    result->mat[1][1] = 2 / (top - bottom);
    result->mat[2][2] = -2 / (far - near);
    result->mat[3][0] = - (right + left) / (right - left);
    result->mat[3][1] = - (top + bottom) / (top - bottom);
    result->mat[3][2] = - (far + near) / (far - near);
    result->mat[3][3] = 1;
}
