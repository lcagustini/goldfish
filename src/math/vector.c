#include <math/vector.h>
#include <math.h>

float vectorLenSquared(struct vec3 v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    return len;
}

float vectorLen(struct vec3 v) {
    return sqrt(vectorLenSquared(v));
}

void vectorNormalize2D(struct vec3 *v) {
    float len = v->x * v->x + v->y * v->y;
    if (!len) return;
    float div = sqrt(len);
    v->x *= div;
    v->y *= div;
}

void vectorNormalize(struct vec3 *v) {
    float len = v->x * v->x + v->y * v->y + v->z * v->z;
    if (!len) return;
    float div = 1.0f/sqrt(len);
    v->x *= div;
    v->y *= div;
    v->z *= div;
}

float vectorDot(struct vec3 a, struct vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct vec3 vectorCross(struct vec3 a, struct vec3 b) {
    struct vec3 n = {
        .x = a.y*b.z - a.z*b.y,
        .y = a.z*b.x - a.x*b.z,
        .z = a.x*b.y - a.y*b.x
    };
    return n;
}

struct vec3 vectorScale(float a, struct vec3 v) {
    v.x *= a;
    v.y *= a;
    v.z *= a;
    return v;
}

struct vec3 vectorAdd(struct vec3 a, struct vec3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

struct vec3 vectorSubtract(struct vec3 a, struct vec3 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
struct vec3 vectorRotate(struct vec3 v, struct quat q) {
    struct vec3 u = {.x = q.x, .y = q.y, .z = q.z};
    float s = q.w;

    return vectorAdd(vectorScale(2.0f * vectorDot(u, v), u),
            vectorAdd(vectorScale(s*s - vectorDot(u, u), v),
                vectorScale(2.0f * s, vectorCross(u, v))));
}

struct quat getRotationQuat(struct vec3 from, struct vec3 to) {
    struct quat result;
    struct vec3 H;

    H.x = from.x + to.x;
    H.y = from.y + to.y;
    H.z = from.z + to.z;
    vectorNormalize(&H);

    result.w = vectorDot(from, H);
    result.x = from.y*H.z - from.z*H.y;
    result.y = from.z*H.x - from.x*H.z;
    result.z = from.x*H.y - from.y*H.x;

    return result;
}

struct quat quatMult(struct quat a, struct quat b) {
    struct quat ret;

    ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
    ret.x = a.w*b.x + a.x*b.w - a.y*b.z + a.z*b.y;
    ret.y = a.w*b.y + a.x*b.z + a.y*b.w - a.z*b.x;
    ret.z = a.w*b.z - a.x*b.y + a.y*b.x + a.z*b.w;

    return ret;
}

struct vec3 quatToEuler(struct quat q) {
    struct vec3 angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.x = atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = 2 * (q.w * q.y - q.z * q.x);
    if (fabs(sinp) >= 1)
        angles.y = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        angles.y = asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.z = atan2(siny_cosp, cosy_cosp);

    return angles;
}

