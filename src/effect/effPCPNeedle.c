#include "common.h"

/* One needle instance: position plus the colour the draw loop stamps. */
typedef struct effNeedleEntry_t
{
    f32 position[3];
    u8 unk0C[4];
    u32 color;
} effNeedleEntry_t;

typedef struct effNeedleWork_t
{
    f32 matrix[16];
    effNeedleEntry_t *entries;
    u32 entryCount;
    u32 mode;
    f32 scale[3];
    u32 vertexCount;
    f32 (*positions)[4];
    f32 (*normals)[4];
    u32 *colors;
    void *streamAllocation;
    void *drawAsset;
    void *backingAllocation;
} effNeedleWork_t;

/* Argument block effPkt3DPrimMakePoly reads. */
typedef struct effPrimParam_t
{
    u16 primCount;
    u16 vertexCount;
    u16 primitive;
    u16 mask;
    u32 color;
    void *parameters;
    f32 (*positions)[4];
    f32 (*normals)[4];
    u8 unk18[8];
    u32 *colors;
    u8 unk24[8];
} effPrimParam_t;

typedef struct effDrawSurface_t
{
    u8 unk00[0x10];
    void (*append)(struct effDrawSurface_t *surface, void *packet);
} effDrawSurface_t;

extern effPrimParam_t D_0056AD88;   /* effPkt3DPrimMakePoly parameter block */
extern f32 D_0031F460[][4];         /* default triangle points */
extern u32 D_0031F520[];            /* default triangle colours */
extern effDrawSurface_t *D_0031F550[];

extern void *sdfGrAllocPacketBuff(s32 size);
extern void sdfDrawTagInit(void *tag);
extern void sdfDrawTagLink(void *tag, u32 packet);
extern void sdf3DPrimLinkMat(void *tag, void *asset, s32 arg);
extern void sdf3DPrimLinkTm(void *tag, s32 arg);
extern void *effPkt3DPrimMakePoly(effPrimParam_t *param);
extern void MultMatrixNoArg(void);
extern void sdfMatFreeDelayed(void *asset);
extern void sdfMemFreeDirect(void *ptr);
extern void dds3NeedleModelVNCListDelete(effNeedleWork_t *work);

/* sceVu0UnitMatrix as libvu0 expands it; gcc 2.96 never emits qmfc2/pextuw. */
#define EFF_UNIT_MATRIX(dst) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "qmfc2.ni $5, $vf0\n\t" \
    "pextuw $4, $0, $5\n\t" \
    "pextuw $2, $0, $4\n\t" \
    "pextuw $3, $4, $0\n\t" \
    "sq $2, 0(%0)\n\t" \
    "sq $3, 0x10(%0)\n\t" \
    "sq $4, 0x20(%0)\n\t" \
    "sq $5, 0x30(%0)\n\t" \
    ".set reorder" \
    : : "r" (dst) : "$2", "$3", "$4", "$5", "memory")

#define VU0_LOAD_MATRIX(src) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "lqc2 vf28, 0(%0)\n\t" \
    "lqc2 vf29, 16(%0)\n\t" \
    "lqc2 vf30, 32(%0)\n\t" \
    "lqc2 vf31, 48(%0)\n\t" \
    ".set reorder" \
    : : "r" (src))

#define VU0_LOAD_MATRIX_B(src) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "lqc2 vf24, 0(%0)\n\t" \
    "lqc2 vf25, 16(%0)\n\t" \
    "lqc2 vf26, 32(%0)\n\t" \
    "lqc2 vf27, 48(%0)\n\t" \
    ".set reorder" \
    : : "r" (src) : "memory")

#define VU0_STORE_MATRIX(dst) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "sqc2 vf28, 0(%0)\n\t" \
    "sqc2 vf29, 16(%0)\n\t" \
    "sqc2 vf30, 32(%0)\n\t" \
    "sqc2 vf31, 48(%0)\n\t" \
    ".set reorder" \
    : : "r" (dst) : "memory")

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralCreate);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralDelete);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralCopy);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralDraw);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralSetRotateMatrix);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", effPCPNeedleSpiralSetColor);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", dds3NeedleModelCreate);

void dds3NeedleModelDelete(effNeedleWork_t *work)
{
    sdfMatFreeDelayed(work->drawAsset);
    dds3NeedleModelVNCListDelete(work);
    sdfMemFreeDirect(work->backingAllocation);
}

void dds3NeedleModelDraw(effNeedleWork_t *work)
{
    f32 matrix[16] __attribute__((aligned(16)));
    void *packet;
    effNeedleEntry_t *entry;
    u32 entryIndex;
    u32 remaining;
    u32 perPrim;

    packet = sdfGrAllocPacketBuff(0x20);
    sdfDrawTagInit(packet);
    sdf3DPrimLinkMat(packet, work->drawAsset, 0);
    EFF_UNIT_MATRIX(matrix);
    matrix[0] = work->scale[0];
    matrix[5] = work->scale[1];
    matrix[10] = work->scale[2];
    VU0_LOAD_MATRIX(matrix);
    VU0_LOAD_MATRIX_B(work->matrix);
    MultMatrixNoArg();
    VU0_STORE_MATRIX(matrix);
    D_0056AD88.parameters = NULL;
    entry = work->entries;
    for (entryIndex = 0; entryIndex < work->entryCount; entryIndex++, entry++) {
        perPrim = 3;
        matrix[12] = entry->position[0];
        matrix[13] = entry->position[1];
        matrix[14] = entry->position[2];
        VU0_LOAD_MATRIX(matrix);
        sdf3DPrimLinkTm(packet, 0);
        if (work->streamAllocation == NULL) {
            remaining = 12;
            D_0056AD88.colors = D_0031F520;
            D_0056AD88.positions = D_0031F460;
            D_0056AD88.normals = NULL;
        } else {
            D_0056AD88.positions = work->positions;
            D_0056AD88.colors = work->colors;
            D_0056AD88.normals = work->normals;
            remaining = work->vertexCount;
        }
        D_0056AD88.color = entry->color;
        D_0056AD88.primCount = 16;
        D_0056AD88.vertexCount = 48;
        while (remaining >= 48) {
            sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(&D_0056AD88));
            if (work->streamAllocation == NULL) {
                D_0056AD88.colors += 48;
                D_0056AD88.positions += 48;
            } else {
                D_0056AD88.colors += 48;
                D_0056AD88.positions += 48;
                D_0056AD88.normals += 48;
            }
            remaining -= 48;
        }
        if (remaining >= 3) {
            D_0056AD88.primCount = remaining / perPrim;
            D_0056AD88.vertexCount = remaining;
            sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(&D_0056AD88));
        }
    }
    D_0031F550[work->mode]->append(D_0031F550[work->mode], packet);
}

INCLUDE_ASM(const s32, "effect/effPCPNeedle", dds3NeedleModelVNCListCreate);

INCLUDE_ASM(const s32, "effect/effPCPNeedle", dds3NeedleModelVNCListDelete);

void dds3NeedleModelSetPosition(effNeedleWork_t *work, s32 index, f32 *vec)
{
    f32 *dst = (f32 *)(index * sizeof(effNeedleEntry_t) + (s32)work->entries);

    dst[0] = vec[0];
    dst[1] = vec[1];
    dst[2] = vec[2];
}

void dds3NeedleModelGetPosition(effNeedleWork_t *work, s32 index, f32 *vec)
{
    f32 *src = (f32 *)(index * sizeof(effNeedleEntry_t) + (s32)work->entries);

    vec[0] = src[0];
    vec[1] = src[1];
    vec[2] = src[2];
}

void dds3NeedleModelSetColor(effNeedleWork_t *work, s32 index, u32 color)
{
    work->entries[index].color = color;
}

INCLUDE_ASM(const s32, "effect/effPCPNeedle", dds3NeedleModelSetMatrix);
