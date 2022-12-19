#ifndef VECTOR_H
#define VECTOR_H

struct vec2 {
    float x, y;
};

struct vec3 {
    float x, y, z;
};

struct quat {
    float x, y, z, w;
};

float vectorLenSquared2D(struct vec2 v);
float vectorLen2D(struct vec2 v);

float vectorLenSquared(struct vec3 v);
float vectorLen(struct vec3 v);

struct vec2 vectorNormalize2D(struct vec2 v);
struct vec3 vectorNormalize(struct vec3 v);

float vectorDot(struct vec3 a, struct vec3 b);
struct vec3 vectorCross(struct vec3 a, struct vec3 b);

struct vec2 vectorScale2D(float a, struct vec2 v);
struct vec3 vectorScale(float a, struct vec3 v);

struct vec3 vectorAdd(struct vec3 a, struct vec3 b);
struct vec3 vectorSubtract(struct vec3 a, struct vec3 b);

// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
struct vec3 vectorRotate(struct vec3 v, struct quat q);

struct quat getRotationQuat(struct vec3 from, struct vec3 to);
struct quat quatMult(struct quat a, struct quat b);
struct vec3 quatToEuler(struct quat q);
struct quat eulerToQuat(struct vec3 v);
struct quat quatFromAxisAngle(struct vec3 axis, float angle);

#endif
