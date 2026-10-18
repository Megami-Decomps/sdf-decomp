#include "common.h"
#include "ps2asm.h"
#include "fpu.h"

/* The owner is stored after its contiguous cubic segments. */
typedef struct
{
    f32 controlPoints[4][3];
    f32 t;
    f32 step;
} dds3Bezier4PPart_t;

typedef struct
{
    void *slots;
    u32 count;
    void *allocation;
} dds3Bezier4P_t;

extern void *sdfMemAllocDirect(u32 size);
extern void *sdfMemLock(void *handle);
extern void sdfMemFreeDirect(void *handle);


/* 1/128 as binary32: the colour unpack scales 0..255 integer lanes to 0..2. */
#define EFF_RGBA_UNIT 0x3C000000

/* The colour inlines read and write a 16-byte slot, one word of it used. */
typedef struct
{
    u32 word;
    u8 pad04[12];
} EffColorWork_t;

/* Only the projection block from +0x30 on is read here: a 4x4 matrix, then the
 * viewport half-extent and centre as one quadword each. */
typedef struct
{
    u8 pad00[0x30];
    f32 matrix[16];
    QuadF32 scale;
    QuadF32 offset;
    u8 pad90[0x10];
} Dds3ScreenParam_t;

extern Dds3ScreenParam_t dds3ScreenParam;
extern f32 D_002FA240[16];
extern QuadF32 aViewEye;
extern QuadF32 aViewDown;
extern void MultMatrixRev(f32 *matrix);

/* vu0 routine: project the world point in vf10 to screen space */
void effScreenOffset(void)
{
    f32 *matrix;

    VU0_LOAD_MATRIX(D_002FA240);
    matrix = dds3ScreenParam.matrix;
    MultMatrixRev(matrix);
    VU0_TRANSFORM_POINT_VF10();
    VU0_PERSPECTIVE_DIVIDE_VF10();
    VU0_LOAD_VF11(matrix + 16);
    VU0_MUL_VF10_VF11();
    VU0_LOAD_VF11(&dds3ScreenParam.offset);
    VU0_ADD_VF10_VF11();
}

/* vu0 routine: measure the projected camera-right offset. */
s32 effPCPBlurScreenOffset(f32 scale) {
    f32 scaleVector[4];
    f32 projectedEnd[4];
    f32 projectedStart[4];
    f32 original[4];
    f32 dx;
    f32 dy;
    f32 distance;

    VU0_STORE_VF10_UNCLOBBERED(original);
    effScreenOffset();
    VU0_STORE_VF10(projectedStart);

    scaleVector[0] = scaleVector[1] = scaleVector[2] = scale;
    VU0_LOAD_VF10(original);
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF11(&aViewEye);
    VU0_SUB_VF10_VF11();
    VU0_LOAD_VF11(&aViewDown);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_LOAD_VF11_MEMORY(scaleVector);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF12_TO_VF11();
    VU0_ADD_VF10_VF11();

    effScreenOffset();
    VU0_STORE_VF10(projectedEnd);
    dx = projectedEnd[0] - projectedStart[0];
    dy = projectedEnd[1] - projectedStart[1];
    VU0_LOAD_VF10(projectedStart);
    distance = fsqrtf(dx * dx + dy * dy);
    return (s32)distance;
}

/* vu0 routine: blend two colours, `amount` of `from` */
u32 effColorBlend(u32 to, u32 from, f32 amount)
{
    EffColorWork_t work[3];
    u32 blended;
    u32 unit;

    if (amount >= 1.0f) {
        return from;
    }
    unit = EFF_RGBA_UNIT;
    work[0].word = from;
    MMI_RGBA_UNPACK(&work[0], unit);
    VU0_COPY_VF10_TO_VF11();
    work[1].word = to;
    MMI_RGBA_UNPACK(&work[1], unit);
    VU0_SCALE_VF10(1.0f - amount);
    VU0_SCALE_VF11(amount);
    VU0_ADD_VF10_VF11();
    MMI_RGBA_PACK(blended, 128.0f);
    work[2].word = blended;
    return blended;
}

/* vu0 routine: unpack two colours, multiply and pack */
u32 effMagaCylinderCalcColor(u32 color, u32 scale)
{
    EffColorWork_t work[3];
    u32 unit = EFF_RGBA_UNIT;
    u32 out;

    work[0].word = color;
    MMI_RGBA_UNPACK(&work[0], unit);
    VU0_COPY_VF10_TO_VF11();
    work[1].word = scale;
    MMI_RGBA_UNPACK(&work[1], unit);
    VU0_MUL_VF10_VF11();
    MMI_RGBA_PACK(out, 128.0f);
    return out;
}

/* vu0 routine: distance from `to` to the line through `from` along `dir` */
f32 effLinePointLength(QuadF32 *dir, QuadF32 *from, QuadF32 *to)
{
    QuadF32 along;
    QuadF32 delta;
    f32 projection;
    f32 length;

    VU0_LOAD_VF10(to);
    VU0_LOAD_VF11(from);
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(&delta);
    VU0_LOAD_VF11(dir);
    VU0_DOT_VF10_VF11(projection);
    along.components[0] = dir->components[0] * projection;
    along.components[1] = dir->components[1] * projection;
    along.components[2] = dir->components[2] * projection;
    VU0_LOAD_VF10(&delta);
    VU0_LOAD_VF11(&along);
    VU0_SUB_VF10_VF11();
    VU0_LENGTH_VF10(length);
    return length;
}

dds3Bezier4P_t *dds3Bezier4PCreate(s32 count) {
    void *allocation = sdfMemAllocDirect(count * sizeof(dds3Bezier4PPart_t) + sizeof(dds3Bezier4P_t));
    void *retainedAddress = (void *)sdfMemLock(allocation);
    u32 slotIndex = 0;
    dds3Bezier4PPart_t *slot = retainedAddress;
    u8 *headerAddress = (u8 *)slot + count * sizeof(dds3Bezier4PPart_t);

    ((dds3Bezier4P_t *)headerAddress)->allocation = allocation;
    ((dds3Bezier4P_t *)headerAddress)->slots = retainedAddress;
    ((dds3Bezier4P_t *)headerAddress)->count = count;
    if (count != 0) {
        do {
            slotIndex++;
            slot->t = 0;
            slot->step = 0.05f;
            slot = (dds3Bezier4PPart_t *)((u8 *)slot + sizeof(dds3Bezier4PPart_t));
        } while (slotIndex < count);
    }
    return (dds3Bezier4P_t *)headerAddress;
}

void dds3Bezier4PDelete(dds3Bezier4P_t *work) {
    sdfMemFreeDirect(work->allocation);
}

s32 dds3Bezier4PPartRun(dds3Bezier4P_t *table, s32 slotIndex, f32 *out) {
    dds3Bezier4PPart_t *slot = &((dds3Bezier4PPart_t *)table->slots)[slotIndex];
    f32 w[4];
    f32 t = slot->t;
    f32 u = 1.0f - t;

    w[0] = u * u * u;
    w[1] = t * (u * u) * 3.0f;
    w[2] = t * t * u * 3.0f;
    w[3] = t * t * t;
    out[0] = slot->controlPoints[0][0] * w[0] + slot->controlPoints[1][0] * w[1] + slot->controlPoints[2][0] * w[2] + slot->controlPoints[3][0] * w[3];
    out[1] = slot->controlPoints[0][1] * w[0] + slot->controlPoints[1][1] * w[1] + slot->controlPoints[2][1] * w[2] + slot->controlPoints[3][1] * w[3];
    out[2] = slot->controlPoints[0][2] * w[0] + slot->controlPoints[1][2] * w[1] + slot->controlPoints[2][2] * w[2] + slot->controlPoints[3][2] * w[3];
    t += slot->step;
    if (t > 1.0f) {
        slot->t = 1.0f;
        return 0;
    }
    slot->t = t;
    return 1;
}

s32 nbCameraLerpVec(dds3Bezier4PPart_t *slot, f32 *out) {
    f32 w[4];
    f32 t = slot->t;
    f32 u = 1.0f - t;

    w[0] = u * u * u;
    w[1] = t * (u * u) * 3.0f;
    w[2] = t * t * u * 3.0f;
    w[3] = t * t * t;
    out[0] = slot->controlPoints[0][0] * w[0] + slot->controlPoints[1][0] * w[1] + slot->controlPoints[2][0] * w[2] + slot->controlPoints[3][0] * w[3];
    out[1] = slot->controlPoints[0][1] * w[0] + slot->controlPoints[1][1] * w[1] + slot->controlPoints[2][1] * w[2] + slot->controlPoints[3][1] * w[3];
    out[2] = slot->controlPoints[0][2] * w[0] + slot->controlPoints[1][2] * w[1] + slot->controlPoints[2][2] * w[2] + slot->controlPoints[3][2] * w[3];
    out[3] = 1.0f;
    t += slot->step;
    if (t > 1.0f) {
        slot->t = 1.0f;
        return 0;
    }
    slot->t = t;
    return 1;
}

void func_001AC068(dds3Bezier4P_t *table, s32 slotIndex) {
    dds3Bezier4PPart_t *slot = &((dds3Bezier4PPart_t *)table->slots)[slotIndex];

    slot->step = 0.05f;
    slot->t = 0;
}

dds3Bezier4PPart_t *dds3Bezier4PGetPart(dds3Bezier4P_t *table, s32 slotIndex) {
    return &((dds3Bezier4PPart_t *)table->slots)[slotIndex];
}
