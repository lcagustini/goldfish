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

void translationMatrix(glMatrix *result, float x, float y, float z) {
    glMatrix m1;

    loadIdentity(&m1);

    m1.mat[3][0] = x;
    m1.mat[3][1] = y;
    m1.mat[3][2] = z;

    multMatrix(result, &m1, result);
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

void createProjectionMatrix(int yFOV) {
    memset(projectionMat, 0, sizeof(glMatrix));

    float aspect = (float)surface_width/(float)surface_height;
    float near = 0.1f;
    float far  = 100.0f;
    yFOV = yFOV * M_PI / 180;

    float tanHalfFovy = tan(yFOV / 2.0f);

    projectionMat->mat[0][0] = 1 / (aspect * tanHalfFovy);
    projectionMat->mat[1][1] = 1 / (tanHalfFovy);
    projectionMat->mat[2][2] = far / (near - far);
    projectionMat->mat[2][3] = -1;
    projectionMat->mat[3][2] = -(far * near) / (far - near);
}

