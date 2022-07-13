void multMatrix(glMatrix *result, glMatrix *srcA, glMatrix *srcB) {
    glMatrix tmp;

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

    memcpy(result, &tmp, sizeof(glMatrix));
}

void loadIdentity(glMatrix *result) {
    memset(result, 0x0, sizeof(glMatrix));

    result->mat[0][0] = 1.0f;
    result->mat[1][1] = 1.0f;
    result->mat[2][2] = 1.0f;
    result->mat[3][3] = 1.0f;
}

void scaleMatrix(glMatrix *result, float sx, float sy, float sz) {
    result->mat[0][0] *= sx;
    result->mat[0][1] *= sx;
    result->mat[0][2] *= sx;
    result->mat[0][3] *= sx;

    result->mat[1][0] *= sy;
    result->mat[1][1] *= sy;
    result->mat[1][2] *= sy;
    result->mat[1][3] *= sy;

    result->mat[2][0] *= sz;
    result->mat[2][1] *= sz;
    result->mat[2][2] *= sz;
    result->mat[2][3] *= sz;
}

void translationMatrix(glMatrix *result, float x, float y, float z) {
    glMatrix m1;

    loadIdentity(&m1);

    m1.mat[0][3] = x;
    m1.mat[1][3] = y;
    m1.mat[2][3] = z;

    multMatrix(result, &m1, result);
}

void rotationMatrix(glMatrix *result, float angle, float x, float y, float z) {
    float sinAngle, cosAngle;
    float mag = sqrtf(x * x + y * y + z * z);

    sinAngle = sin(angle * M_PI / 180.0f);
    cosAngle = cos(angle * M_PI / 180.0f);

    if (mag > 0.0f) {
        float xx, yy, zz, xy, yz, zx, xs, ys, zs;
        float oneMinusCos;
        glMatrix rotMat;

        x /= mag;
        y /= mag;
        z /= mag;

        xx = x * x;
        yy = y * y;
        zz = z * z;
        xy = x * y;
        yz = y * z;
        zx = z * x;
        xs = x * sinAngle;
        ys = y * sinAngle;
        zs = z * sinAngle;
        oneMinusCos = 1.0f - cosAngle;

        rotMat.mat[0][0] = (oneMinusCos * xx) + cosAngle;
        rotMat.mat[0][1] = (oneMinusCos * xy) - zs;
        rotMat.mat[0][2] = (oneMinusCos * zx) + ys;
        rotMat.mat[0][3] = 0.0F;

        rotMat.mat[1][0] = (oneMinusCos * xy) + zs;
        rotMat.mat[1][1] = (oneMinusCos * yy) + cosAngle;
        rotMat.mat[1][2] = (oneMinusCos * yz) - xs;
        rotMat.mat[1][3] = 0.0F;

        rotMat.mat[2][0] = (oneMinusCos * zx) - ys;
        rotMat.mat[2][1] = (oneMinusCos * yz) + xs;
        rotMat.mat[2][2] = (oneMinusCos * zz) + cosAngle;
        rotMat.mat[2][3] = 0.0F;

        rotMat.mat[3][0] = 0.0F;
        rotMat.mat[3][1] = 0.0F;
        rotMat.mat[3][2] = 0.0F;
        rotMat.mat[3][3] = 1.0F;

        multMatrix(result, &rotMat, result);
    }
}

void frustumMatrix(glMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ) {
    float deltaX = right - left;
    float deltaY = top - bottom;
    float deltaZ = farZ - nearZ;
    glMatrix frust;

    if ((nearZ <= 0.0f) || (farZ <= 0.0f) || (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f)) {
        return;
    }

    frust.mat[0][0] = 2.0f * nearZ / deltaX;
    frust.mat[0][1] = frust.mat[0][2] = frust.mat[0][3] = 0.0f;

    frust.mat[1][1] = 2.0f * nearZ / deltaY;
    frust.mat[1][0] = frust.mat[1][2] = frust.mat[1][3] = 0.0f;

    frust.mat[2][0] = (right + left) / deltaX;
    frust.mat[2][1] = (top + bottom) / deltaY;
    frust.mat[2][2] = -(nearZ + farZ) / deltaZ;
    frust.mat[2][3] = -1.0f;

    frust.mat[3][2] = -2.0f * nearZ * farZ / deltaZ;
    frust.mat[3][0] = frust.mat[3][1] = frust.mat[3][3] = 0.0f;

    multMatrix(result, &frust, result);
}

void lookAt(glMatrix *result, SceFVector3 position, SceFVector3 target, SceFVector3 worldUp) {
    SceFVector3 dir = vectorSubtract(target, position);
    vectorNormalize(&dir);

    SceFVector3 right = vectorCross(dir, worldUp);
    vectorNormalize(&right);

    SceFVector3 up = vectorCross(right, dir);

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
