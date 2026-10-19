#include "common.h"
#include "ps2asm.h"
#include "sdf_lighting.h"

extern f32 cosf(f32 angle);

extern f32 sinf(f32 angle);

void LoadIdentity(void);

void DiagMatrix(void);

void ReverseMatrix(void);

void InversMatrix(void);

void SetRotateMatrixX(f32 angle);

void SetRotateMatrixY(f32 angle);

void SetRotateMatrixZ(f32 angle);

void StoreRotateMatrixAxis(f32 angle, const sdfVector3_t *axis, sdfMatrix4_t *matrix);

void SetRotateMatrixAxis(f32 angle, const sdfVector3_t* axis);

void SetRotateMatrixX_B(f32 angle);

void SetRotateMatrixY_B(f32 angle);

void SetRotateMatrixZ_B(f32 angle);

void SetRotateMatrixAxis_B(const sdfVector3_t* axis, f32 angle);

void RotateMatrixX(f32 angle);

void RotateMatrixY(f32 angle);

void RotateMatrixZ(f32 angle);

void SetRotateMatrixXYZ(const sdfVector3_t *rot);

void MultMatrixNoArg(void);

void MultMatrixRevNoArg(void);

void MultMatrix(void *matrix);

void MultMatrixRev(const void *matrix);

void LookAtMatrix(const void *directionPoint, const void *basisOrigin, const void *up);

void LoadMatrix(const void *matrix) {
    SDF_VU0_LOADMATRIX(matrix);
}

void LoadMatrix_B(const void *matrix) {
    SDF_VU0_LOADMATRIX_B(matrix);
}

void StoreMatrix(void *matrix) {
    SDF_VU0_STOREMATRIX(matrix);
}

void StoreMatrix_B(void *matrix) {
    SDF_VU0_STOREMATRIX_B(matrix);
}

void CopyMatrixAtoB(void) {
    SDF_VU0_COPYMATRIX_A_TO_B();
}

void CopyMatrixAtoC(void) {
    SDF_VU0_COPYMATRIX_A_TO_C();
}

void CopyMatrixBtoA(void) {
    SDF_VU0_COPYMATRIX_B_TO_A();
}

void CopyMatrixBtoC(void) {
    SDF_VU0_COPYMATRIX_B_TO_C();
}

void CopyMatrixCtoA(void) {
    SDF_VU0_COPYMATRIX_C_TO_A();
}

void CopyMatrixCtoB(void) {
    SDF_VU0_COPYMATRIX_C_TO_B();
}

void LoadIdentity(void) {
    SDF_VU0_LOADIDENTITY();
}

void LoadIdentity_B(void) {
    SDF_VU0_LOADIDENTITY_B();
}

/* vu0 routine: store the unit matrix built from vf0 by MMI lane shuffles */
void StoreIdentity(f32 *matrix) {
    SDF_VU0_STOREIDENTITY(matrix);
}

/* libvu0: sceVu0TransposeMatrix, register form (vf28-vf31 in and out) */
void DiagMatrix(void) {
    SDF_VU0_TRANSMATRIX();
}

/* vu0 routine: rigid inverse of vf28-vf31 (transpose the 3x3, translation = -(R^T * t)) */
void ReverseMatrix(void) {
    VU0_MATRIX4_INVERT_RIGID();
}

/* vu0 routine: inverse of vf28-vf31 with per-axis scale removed (transpose, rows / |row|^2, translation = -(R^T * t)) */
void InversMatrix(void) {
    VU0_MATRIX4_INVERT_SCALED();
}

void SetRotateMatrixX(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[1][1] = c;
    m[1][2] = s;
    m[2][1] = -s;
    m[2][2] = c;
    SDF_VU0_LOADMATRIX((f32 *)m);
}

void SetRotateMatrixY(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[0][0] = c;
    m[0][2] = -s;
    m[2][0] = s;
    m[2][2] = c;
    SDF_VU0_LOADMATRIX((f32 *)m);
}

void SetRotateMatrixZ(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[0][0] = c;
    m[0][1] = s;
    m[1][0] = -s;
    m[1][1] = c;
    SDF_VU0_LOADMATRIX((f32 *)m);
}

void StoreRotateMatrixAxis(f32 angle, const sdfVector3_t *axis, sdfMatrix4_t *matrix) {
    f32 normalizedAxis[4];
    f32 x;
    f32 y;
    f32 z;
    f32 cosine = cosf(angle);
    f32 sine = sinf(angle);

    SDF_VU0_LOADVECTOR(axis);
    SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(normalizedAxis);

    x = normalizedAxis[0];
    y = normalizedAxis[1];
    z = normalizedAxis[2];

    matrix->m[0] = x * x + (1.0f - x * x) * cosine;
    matrix->m[1] = x * y * (1.0f - cosine) + z * sine;
    matrix->m[2] = x * z * (1.0f - cosine) - y * sine;
    matrix->m[3] = 0.0f;
    matrix->m[4] = x * y * (1.0f - cosine) - z * sine;
    matrix->m[5] = y * y + (1.0f - y * y) * cosine;
    matrix->m[6] = y * z * (1.0f - cosine) + x * sine;
    matrix->m[7] = 0.0f;
    matrix->m[8] = x * z * (1.0f - cosine) + y * sine;
    matrix->m[9] = y * z * (1.0f - cosine) - x * sine;
    matrix->m[10] = z * z + (1.0f - z * z) * cosine;
    matrix->m[11] = 0.0f;
    SDF_VU0_STOREIDENTITYVECTOR(&matrix->m[12]);
}

void SetRotateMatrixAxis(f32 angle, const sdfVector3_t* axis)
{
    sdfMatrix4_t matrix;
    StoreRotateMatrixAxis(angle, axis, &matrix);
    SDF_VU0_LOADMATRIX(&matrix);
}

void SetRotateMatrixX_B(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[1][1] = c;
    m[1][2] = s;
    m[2][1] = -s;
    m[2][2] = c;
    SDF_VU0_LOADMATRIX_B((f32 *)m);
}

void SetRotateMatrixY_B(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[0][0] = c;
    m[0][2] = -s;
    m[2][0] = s;
    m[2][2] = c;
    SDF_VU0_LOADMATRIX_B((f32 *)m);
}

void SetRotateMatrixZ_B(f32 angle) {
    volatile f32 m[4][4] __attribute__((aligned(16)));
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    SDF_VU0_STOREIDENTITY((f32 *)m);
    m[0][0] = c;
    m[0][1] = s;
    m[1][0] = -s;
    m[1][1] = c;
    SDF_VU0_LOADMATRIX_B((f32 *)m);
}

void SetRotateMatrixAxis_B(const sdfVector3_t* axis, f32 angle)
{
    sdfMatrix4_t matrix;
    StoreRotateMatrixAxis(angle, axis, &matrix);
    SDF_VU0_LOADMATRIX_B(&matrix);
}

/* vu0 routine: rotate the vf29/vf30 pair by angle (vf29 = vf29*c + vf30*s, vf30 = vf29*s - vf30*c) */
void RotateMatrixX(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    VU0_ROTATE_BASIS_X(c, s);
}

/* vu0 routine: rotate the vf28/vf30 pair by angle (vf28 = vf30*s - vf28*c, vf30 = vf30*c + vf28*s) */
void RotateMatrixY(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    VU0_ROTATE_BASIS_Y(c, s);
}

/* vu0 routine: rotate the vf28/vf29 pair by angle (vf28 = vf29*s + vf28*c, vf29 = vf28*s - vf29*c) */
void RotateMatrixZ(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    VU0_ROTATE_BASIS_Z(c, s);
}

void SetRotateMatrixXYZ(const sdfVector3_t *rot)
{
    SetRotateMatrixX(rot->x);
    SetRotateMatrixY_B(rot->y);
    MultMatrixRevNoArg();
    SetRotateMatrixZ_B(rot->z);
    MultMatrixRevNoArg();
}

/* vu0 routine: vf28-vf31 = vf24-vf27 * vf28-vf31 (4x4 product) */
void MultMatrixNoArg(void)
{
    VU0_MATRIX4_MUL_BANK_B_RIGHT();
}

/* vu0 routine: vf28-vf31 = vf28-vf31 * vf24-vf27 (4x4 product) */
void MultMatrixRevNoArg(void)
{
        VU0_MATRIX4_MUL_BANK_B_LEFT();
}

/* vu0 routine: vf28-vf31 = vf20-vf23 * vf28-vf31 (4x4 product) */
void MultMatrixNoArg_C(void)
{
    VU0_MATRIX4_MUL_BANK_C_RIGHT();
}

/* vu0 routine: vf28-vf31 = vf28-vf31 * vf20-vf23 (4x4 product) */
void MultMatrixRevNoArg_C(void)
{
    VU0_MATRIX4_MUL_BANK_C_LEFT();
}

void MultMatrix(void *matrix) {
    SDF_VU0_LOADMATRIX_B(matrix);
    MultMatrixNoArg();
}

void MultMatrixRev(const void *matrix) {
    SDF_VU0_LOADMATRIX_B(matrix);
    MultMatrixRevNoArg();
}

void ApplyMatrix(void *dst, const void *src) {
    SDF_VU0_LOADVECTOR(src);
    SDF_VU0_APPLYMATRIX();
    SDF_VU0_STOREVECTOR(dst);
}

f32 InnerProduct(const void *a, const void *b) {
    f32 product;

    SDF_VU0_LOADVECTOR(a);
    SDF_VU0_LOADVECTOR_B(b);
    SDF_VU0_INNERPRODUCT(product);
    return product;
}

void OuterProduct(void *dst, const void *a, const void *b) {
    SDF_VU0_LOADVECTOR(a);
    SDF_VU0_LOADVECTOR_B(b);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_STOREVECTOR(dst);
}

/* vu0 routine: look-at basis in vf28-vf31 (forward, right, up, eye), then its rigid inverse */
void LookAtMatrix(const void *directionPoint, const void *basisOrigin, const void *up) {
    SDF_VU0_LOADVECTOR(basisOrigin);
    SDF_VU0_SET_ROW3();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(directionPoint);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_NORMALIZE();
    SDF_VU0_SET_ROW2();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(up);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_SET_ROW0();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_GET_ROW2();
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_SET_ROW1();
    ReverseMatrix();
}
INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518B8);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", sdf3DSpecularTex);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", sdf3DRefMapParam);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518C8);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", sdfPadPortSlotPairs);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", sdfPadManagerMode);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518D5);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518D8);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518E0);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518E1);

INCLUDE_SDATA(const s32, "sdf/sdfVu0", D_005518E2);

