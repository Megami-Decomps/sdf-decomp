#include "common.h"
#include "ps2asm.h"

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
extern f32 sqrtf(f32 x);

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

INCLUDE_ASM(const s32, "effect/effMath", effPCPBlurScreenOffset);

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

INCLUDE_ASM(const s32, "effect/effMath", dds3Bezier4PCreate);

INCLUDE_ASM(const s32, "effect/effMath", dds3Bezier4PDelete);

INCLUDE_ASM(const s32, "effect/effMath", dds3Bezier4PPartRun);

INCLUDE_ASM(const s32, "effect/effMath", nbCameraLerpVec);

INCLUDE_ASM(const s32, "effect/effMath", func_001AC068);

INCLUDE_ASM(const s32, "effect/effMath", dds3Bezier4PGetPart);
