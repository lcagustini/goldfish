float vectorLenSquared(SceFVector3 v) {
    float len = v.x * v.x + v.y * v.y + v.z * v.z;
    return len;
}

float vectorLen(SceFVector3 v) {
    return sqrt(vectorLenSquared(v));
}

void vectorNormalize2D(SceFVector3 *v) {
    float len = v->x * v->x + v->y * v->y;
    if (!len) return;
    float div = sqrt(len);
    v->x *= div;
    v->y *= div;
}

void vectorNormalize(SceFVector3 *v) {
    float len = v->x * v->x + v->y * v->y + v->z * v->z;
    if (!len) return;
    float div = 1.0f/sqrt(len);
    v->x *= div;
    v->y *= div;
    v->z *= div;
}

float vectorDot(SceFVector3 a, SceFVector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

SceFVector3 vectorCross(SceFVector3 a, SceFVector3 b) {
    SceFVector3 n = {
        .x = a.y*b.z - a.z*b.y,
        .y = a.z*b.x - a.x*b.z,
        .z = a.x*b.y - a.y*b.x
    };
    return n;
}

SceFVector3 vectorScale(float a, SceFVector3 v) {
    v.x *= a;
    v.y *= a;
    v.z *= a;
    return v;
}

SceFVector3 vectorAdd(SceFVector3 a, SceFVector3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

SceFVector3 vectorSubtract(SceFVector3 a, SceFVector3 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
SceFVector3 vectorRotate(SceFVector3 v, SceFQuaternion q) {
    SceFVector3 u = {.x = q.x, .y = q.y, .z = q.z};
    float s = q.w;

    return vectorAdd(vectorScale(2.0f * vectorDot(u, v), u),
            vectorAdd(vectorScale(s*s - vectorDot(u, u), v),
                vectorScale(2.0f * s, vectorCross(u, v))));
}

SceFQuaternion getRotationQuat(SceFVector3 from, SceFVector3 to) {
    SceFQuaternion result;
    SceFVector3 H;

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

SceFQuaternion quatMult(SceFQuaternion a, SceFQuaternion b) {
    SceFQuaternion ret;

    ret.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
    ret.x = a.w*b.x + a.x*b.w - a.y*b.z + a.z*b.y;
    ret.y = a.w*b.y + a.x*b.z + a.y*b.w - a.z*b.x;
    ret.z = a.w*b.z - a.x*b.y + a.y*b.x + a.z*b.w;

    return ret;
}

SceFVector3 quatToEuler(SceFQuaternion q) {
    SceFVector3 angles;

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

