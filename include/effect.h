#ifndef EFFECT_H
#define EFFECT_H

#include "common.h"
#include "libsdf.h"

/* Shared effect RNG and retained slot-array header. */
typedef struct effRandState_t {
    u32 words[4];
} effRandState_t;

typedef struct effArrHdr_t {
    void *slots;
    u32 count;
    sdfMemHandle_t *allocation;
} effArrHdr_t;


/* Per-cell vector history and packed color stream (0x14 bytes). */
typedef struct parCell_t {
    u128 *history;
    u32 *colors;
    s32 vertexCount;
    s32 historyAdvanceCountdown;
    u32 color;
} parCell_t;

/* The complete 0x2C-byte cell-system owner shared by PCP effects. */
typedef struct parSystem_t {
    u16 kind;
    u16 bucket;
    s32 cellCount;
    s32 vertexWordCount;
    s32 groupDivisor;
    sdfMemHandle_t *allocation;
    parCell_t *cells;
    u128 *vertices;
    u32 *colors;
    sdfMat_t *asset;
    struct parSystem_t *next;
    s32 unk28;
} parSystem_t;

/* Four XYZ controls, evaluation position and frame step (0x38 bytes). */
typedef struct effCubicBezierSlot_t {
    f32 controlPoints[4][3];
    f32 t;
    f32 step;
} effCubicBezierSlot_t;

typedef struct effBezierPoint_t {
    f32 x, y, z;
} effBezierPoint_t;

/* Seven packed XYZ controls form two cubic segments in each 0x60-byte curve. */
typedef struct effBezierCurve_t {
    effBezierPoint_t controlPoints[7];
    u32 pointIndex;
    f32 t;
    f32 parameterStep;
} effBezierCurve_t;

/* Shared rectangle and sampling payload, copied as 0x28 bytes. */
typedef struct dds3Effect_Blur_t {
    u32 abgr;
    s32 blend;
    f32 rotate;
    f32 scale;
    s32 center[2];
    s32 xy[2][2];
} dds3Effect_Blur_t;

/* Phase-driven blur parameters and the scale-slot allocation they own. */
typedef struct effBlurScaleParams_t {
    s32 count;
    f32 phaseStep;
    f32 spacing;
    u32 color;
    s32 blendControl;
    f32 uvDisplacementAngleDegrees;
    f32 uvDisplacementAmplitude;
    f32 angleStep;
    s32 position[2];
    s32 size;
} effBlurScaleParams_t;

typedef struct effBlurScaleSlot_t {
    f32 phase;
    f32 angle;
    dds3Effect_Blur_t quad;
} effBlurScaleSlot_t;

typedef struct effBlurScaleWork_t {
    effBlurScaleParams_t params;
    sdfTexHandle_t *texture;
    sdfMemHandle_t *allocation;
    effBlurScaleSlot_t *slots;
} effBlurScaleWork_t;

/* Random-position blur parameters and their allocated slot owner. */
typedef struct effBlurScatterParams_t {
    s32 count;
    s32 delaySpread;
    f32 angleStep;
    u32 color;
    s32 blendControl;
    f32 uvDisplacementAngleDegrees;
    f32 uvDisplacementAmplitude;
    s32 position[2];
    s32 positionSpread;
    s32 size;
} effBlurScatterParams_t;

typedef struct effBlurScatterSlot_t {
    s32 delay;
    f32 angle;
    dds3Effect_Blur_t quad;
} effBlurScatterSlot_t;

typedef struct effBlurScatterWork_t {
    effBlurScatterParams_t params;
    sdfTexHandle_t *texture;
    sdfMemHandle_t *allocation;
    effBlurScatterSlot_t *slots;
} effBlurScatterWork_t;

/* A copied 0x2C-byte body followed by its separately acquired texture. */
typedef struct dds3Effect_PartialMultiBlurParam_t {
    s32 area;
    dds3Effect_Blur_t Blur;
} dds3Effect_PartialMultiBlurParam_t;

typedef struct effBlurTemplate_t {
    dds3Effect_PartialMultiBlurParam_t body;
    sdfTexHandle_t *texture;
} effBlurTemplate_t;

typedef struct {
    s32 left;
    s32 top;
    s32 right;
    s32 bottom;
} effResourceRectBounds_t;

typedef struct {
    /* Packed movie RGBA and the renderer's channel view share one word. */
    union { u8 color[4]; u32 rgba; };
    s32 blendControl;
    effResourceRectBounds_t bounds;
} effResourceRectDrawParams_t;

/* Nine copied words; the selected source texture belongs to the owner. */
typedef struct {
    s32 area;
    s32 center[2];
    effResourceRectDrawParams_t Filter;
} dds3Effect_PartialMultiFilterParam_t;

typedef struct {
    dds3Effect_PartialMultiFilterParam_t params;
    sdfTexHandle_t *texture;
} dds3Effect_PartialMultiFilter_t;

typedef struct dds3Effect_Filter_t {
    u32 abgr;
    s32 blend;
    s32 xy[2][2];
} dds3Effect_Filter_t;

#endif /* EFFECT_H */
