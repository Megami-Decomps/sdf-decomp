#include "common.h"
#include "ps2asm.h"
#include "model.h"
#include "draw3d.h"

#define HW_READ(reg) (*(reg))
#define HW_WRITE(reg, value) (*(reg) = (value))
#define SDF_DMA_QWORD_SHIFT 4
#define SDF_DMA_CHCR_TTE 0x40

extern u32 corePktCurrLight;
extern f32 corePktZBias;
extern u32 corePktCurrFog;
extern u128 corePktFogParm;
extern u8 vu0MicroPacket[];
extern u8 vu1MicroPacket[];
extern s32 corePktPointBuff;
extern sdfMemHandle_t *sdfCoreDrawBuffHandle;

extern s32 sceDmaSync(void *, s32, s32);
extern void *sceDmaGetChan(s32);
extern void sceDmaSendN(void *, void *, s32);
extern sdfMemHandle_t *sdfMemAllocDirect(s32 size);
extern void sdfMemFreeHandleDirect(sdfMemHandle_t **handle);
extern void *sdfMemLock(sdfMemHandle_t *handle);
extern u8 *sdfModelAllocTmPacket(sdfModelNode_t *node);

/* NON_MATCHING: broad register/code-layout residual; see the unit draft guide. */
#ifdef NON_MATCHING
typedef struct sdfCorePacketPointBuff_tag sdfCorePacketPointBuff_tag;
typedef struct sdfCorePacketFaceDivParm_t sdfCorePacketFaceDivParm_t;
#define CORE_ALIGN16(p) ((u8 *)(((u32)(p) + 15) & ~15))
extern sdfCorePacketFaceDivParm_t corePktFaceDivParm;
extern sdfMatrix4_t sdf3DScreenTm;
extern u128 coreLightConst1, coreLightConst2;
void MultMatrixRev(void *matrix);
void corePktInitCb(corePktCb_t *cb, u16 *header, u32 attrMask);
void corePktSetFaceDivParm(sdfCorePacketFaceDivParm_t *, sdfMatPacket_t *, u32, f32, f32);
void corePktLoadWorldLightMtx(void *matrix);
u8 *corePktVu0Run(sdfCorePacketPointBuff_tag *dst, const void *src, s32 count, s32 offset, s32 stride);
void corePktOpenAndEmit(corePktCb_t *cb);
u64 *corePktClosePacket(corePktCb_t *cb);
void corePktSkinPointsFirst(sdfCorePacketPointBuff_tag *, s32, const f32 *, const f32 *, const u32 *);
void corePktSkinPointsAccum(sdfCorePacketPointBuff_tag *, s32, const f32 *, const f32 *, const u32 *);
void corePktSkinVertFirst(sdfCorePacketPointBuff_tag *, s32, const f32 *, const f32 *, const u32 *);
void corePktSkinVertAccum(sdfCorePacketPointBuff_tag *, s32, const f32 *, const f32 *, const u32 *);
void corePktNormalizeNorms(sdfCorePacketPointBuff_tag *, s32);
void corePktLoadVerts(u8 *, s32, const u32 *);
void corePktLoadVertsNorms(sdfCorePacketPointBuff_tag *, s32, const u32 *, const u32 *);
void corePktMorphVerts(sdfCorePacketPointBuff_tag *, s32, const u32 *, f32);
void corePktMorphVertsNorms(sdfCorePacketPointBuff_tag *, s32, const u32 *, const u32 *, f32);
void corePktXfmLightClip(sdfCorePacketPointBuff_tag *, s32, const u32 *, const u32 *);
void corePktXfmLightClipUV(sdfCorePacketPointBuff_tag *, s32, const u32 *, const u32 *, const f32 *, const f32 *);
void corePktXfmLightClipPB(sdfCorePacketPointBuff_tag *, s32);
void corePktXfmLightClipUVPB(sdfCorePacketPointBuff_tag *, s32, const f32 *, const f32 *);


/* Only the two low float lanes of these VU0 UV rows are consumed.
 * SQ stores the GPR directly; this is not a C 64-to-128-bit extension. */
#define CORE_STORE_UV_PAIR(dst, value) __asm__ volatile ( \
    ".set noreorder\n\tsq %1, %0\n\t.set reorder" \
    : "=m" (*(volatile u128 *)(dst)) : "r" ((u64)(value)))

/* Native driver for generated, skinned and morphing model packets. */
void *sdfCorePacketGen(sdfModelNode_t *node, void *packetData, u32 attrMask,
                       u32 color, f32 soften, f32 bias, s32 frame, s32 reverse)
{
    corePktCb_t cb;
    sdfCorePacket_t *packet = packetData;
    s32 frameOffset = frame << 7;
    u8 *matrixPacket = (u8 *)node->tmPacket + frameOffset;
    s32 faceCount, pointCount, packetBits;
    sdfCorePacketPointBuff_tag *points;
    u8 *cursor;
    volatile u128 *vu;
    s32 inputHeaderBytes, inputPointStride;

    __asm__ volatile (".set noreorder\n\tvaddw.xyz vf1, vf0, vf0w\n\t"
                      "vsubw.w vf1, vf0, vf0w\n\t.set reorder");
    corePktInitCb(&cb, (u16 *)&packet->prim, attrMask);
    if (reverse == 0) {
        cb.backfaceSign = 1.0f;
    } else {
        cb.backfaceSign = -1.0f;
    }
    cb.color = color;
    cb.fdSoften = soften;
    cb.fdBias = bias;
    cb.matPacketPtr = node->root->matList->items[packet->header.matIndex]->packet[frame];
    cb.matPacket2 = cb.matPacketPtr;
    cb.savedAttrBits = cb.attrBits;
    faceCount = cb.faceCount;
    pointCount = cb.pointCount;
    packetBits = cb.packetBits;
    points = cb.pointBuff;
    corePktSetFaceDivParm(&corePktFaceDivParm, cb.matPacketPtr, color, soften, bias);

    switch (packet->header.packetType) {
    case 7: {
        u16 *indices = cb.dataPtr;
        u32 count = *indices++;
        sdfModelNode_t **nodes = node->root->nodeList->items;
        volatile u128 *destination = (volatile u128 *)0x11004C00;
        s32 i;
        cursor = CORE_ALIGN16(indices + count);
        cb.facePtr = cursor;
        cursor = CORE_ALIGN16(cursor + faceCount * 6);
        vu = (volatile u128 *)0x11004000;
        vu[0xDC] = ((const u128 *)&sdf3DScreenTm)[0];
        vu[0xDD] = ((const u128 *)&sdf3DScreenTm)[1];
        vu[0xDE] = ((const u128 *)&sdf3DScreenTm)[2];
        vu[0xDF] = ((const u128 *)&sdf3DScreenTm)[3];
        vu[0xE0] = ((const u128 *)corePktCurrLight)[1];
        vu[0xE1] = ((const u128 *)corePktCurrLight)[2];
        vu[0xE2] = ((const u128 *)corePktCurrLight)[3];
        CORE_STORE_UV_PAIR(&vu[0xE6], cb.matPacket2->texTm.q[0]);
        CORE_STORE_UV_PAIR(&vu[0xE7], cb.matPacket2->texTm.q[1]);
        CORE_STORE_UV_PAIR(&vu[0xE8], cb.matPacket2->texTm.q[2]);
        for (i = 0; i != count; i++) {
            const u128 *source = (const u128 *)((u8 *)nodes[*indices++]->tmPacket + frameOffset);
            destination[0] = source[0];
            destination[1] = source[1];
            destination[2] = source[2];
            destination[3] = source[3];
            destination[4] = source[4];
            destination[5] = source[5];
            destination[6] = source[6];
            destination += 7;
        }
        vu[0xE3] = ((const u128 *)&corePktFaceDivParm)[0];
        vu[0xE4] = ((const u128 *)&corePktFaceDivParm)[1];
        vu[0xE5] = ((const u128 *)&corePktFaceDivParm)[2];
        inputHeaderBytes = count * 12 + 0x14;
        inputPointStride = count * 28 + 8;
    } break;
    case 8: {
        SDF_VU0_LOADMATRIX(matrixPacket);
        MultMatrixRev(&sdf3DScreenTm);
        corePktLoadWorldLightMtx(matrixPacket);
        SDF_VU0_STOREMATRIX(&cb.tmSave);
        cursor = CORE_ALIGN16(cb.dataPtr);
        cb.facePtr = cursor;
        cursor = CORE_ALIGN16(cursor + faceCount * 6);
        vu = (volatile u128 *)0x11004000;
        CORE_STORE_UV_PAIR(&vu[0xE6], cb.matPacket2->texTm.q[0]);
        CORE_STORE_UV_PAIR(&vu[0xE7], cb.matPacket2->texTm.q[1]);
        CORE_STORE_UV_PAIR(&vu[0xE8], cb.matPacket2->texTm.q[2]);
        vu[0xE3] = ((const u128 *)&corePktFaceDivParm)[0];
        vu[0xE4] = coreLightConst1;
        vu[0xE5] = coreLightConst2;
        inputHeaderBytes = 0x1C;
        inputPointStride = 0x20;
    } break;
    default:
        goto ordinaryGeometry;
    }

    cursor = corePktVu0Run(points, cursor, pointCount, inputHeaderBytes, inputPointStride);
    if (cb.orgAttrBits & SDF_ATTRBIT_MULTITEX) {
        cb.uv2Ptr = cursor;
        cursor = CORE_ALIGN16(cursor + pointCount * 8);
    }
    cb.vertColPtr = cursor;
    corePktOpenAndEmit(&cb);
    return corePktClosePacket(&cb);

ordinaryGeometry:
    if (packetBits & SDF_PACKETBIT_BLENDVERT) {
        u16 *indices = cb.dataPtr;
        u32 count = *indices++;
        sdfModelNode_t **nodes = node->root->nodeList->items;
        s32 normals = packetBits & SDF_PACKETBIT_NORMAL;
        s32 i;
        cursor = CORE_ALIGN16(indices + count);
        if (packetBits & SDF_PACKETBIT_FACE) {
            cb.facePtr = cursor;
            cursor = CORE_ALIGN16(cursor + faceCount * 6);
        }
        for (i = 0; i != count; i++) {
            u8 *vertices = cursor;
            u8 *normalData;
            u8 *transform;
            cursor = CORE_ALIGN16(cursor + pointCount * 16);
            normalData = cursor;
            transform = (u8 *)nodes[*indices++]->tmPacket + frameOffset;
            if (normals) {
                cursor = CORE_ALIGN16(cursor + pointCount * 12);
                if (i == 0) {
                    corePktSkinPointsFirst(points, pointCount, (const f32 *)transform,
                                           (const f32 *)vertices, (const u32 *)normalData);
                } else {
                    corePktSkinPointsAccum(points, pointCount, (const f32 *)transform,
                                           (const f32 *)vertices, (const u32 *)normalData);
                }
            } else if (i == 0) {
                corePktSkinVertFirst(points, pointCount, (const f32 *)transform,
                                     (const f32 *)vertices, (const u32 *)normalData);
            } else {
                corePktSkinVertAccum(points, pointCount, (const f32 *)transform,
                                     (const f32 *)vertices, (const u32 *)normalData);
            }
        }
        if (normals) corePktNormalizeNorms(points, pointCount);
        if (packetBits & SDF_PACKETBIT_ST) {
            cb.uvPtr = cursor;
            cursor = CORE_ALIGN16(cursor + pointCount * 8);
            if (cb.orgAttrBits & SDF_ATTRBIT_MULTITEX) {
                cb.uv2Ptr = cursor;
                cursor = CORE_ALIGN16(cursor + pointCount * 8);
            }
        }
        cb.vertColPtr = cursor;
        SDF_VU0_LOADMATRIX(&sdf3DScreenTm);
        __asm__ volatile (".set noreorder\n\tlqc2 vf24, 16(%0)\n\t"
                          "lqc2 vf25, 32(%0)\n\tlqc2 vf26, 48(%0)\n\t.set reorder"
                          : : "r" (corePktCurrLight));
    } else if (packetBits & SDF_PACKETBIT_BLENDGRP) {
        u32 blendCount = packet->header.extData[0] & 0xFFFF;
        u32 entry = packet->header.extData[0] >> 16;
        sdfModelMorphList_t *morphs = node->root->morphList;
        sdfModelMorph_t *morph;
        s32 normals = packetBits & SDF_PACKETBIT_NORMAL;
        s32 stride, i;
        cursor = CORE_ALIGN16(cb.dataPtr);
        if (morphs == NULL) return NULL;
        if (packetBits & SDF_PACKETBIT_FACE) {
            cb.facePtr = cursor;
            cursor = CORE_ALIGN16(cursor + faceCount * 6);
        }
        if ((s32)entry >= morphs->count) return NULL;
        morph = morphs->items + entry;
        stride = (pointCount * 12 + 15) & ~15;
        if (normals) {
            corePktLoadVertsNorms(points, pointCount, (const u32 *)cursor,
                                 (const u32 *)CORE_ALIGN16(cursor + pointCount * 12));
            stride *= 2;
        } else {
            corePktLoadVerts((u8 *)points, pointCount, (const u32 *)cursor);
        }
        for (i = 0; i != 2; i++) {
            f32 weight = morph->target[i].blend;
            if (SDF_CORE_MORPH_EPSILON < weight) {
                s32 target = morph->target[i].targetId;
                if (target < (s32)blendCount) {
                    u8 *delta = cursor + (target + 1) * stride;
                    if (normals) {
                        corePktMorphVertsNorms(points, pointCount, (const u32 *)delta,
                                              (const u32 *)CORE_ALIGN16(delta + pointCount * 12), weight);
                    } else {
                        corePktMorphVerts(points, pointCount, (const u32 *)delta, weight);
                    }
                }
            }
        }
        cursor += stride * blendCount;
        if (normals) corePktNormalizeNorms(points, pointCount);
        if (packetBits & SDF_PACKETBIT_ST) {
            cb.uvPtr = cursor;
            cursor = CORE_ALIGN16(cursor + pointCount * 8);
            if (cb.orgAttrBits & SDF_ATTRBIT_MULTITEX) {
                cb.uv2Ptr = cursor;
                cursor = CORE_ALIGN16(cursor + pointCount * 8);
            }
        }
        cb.vertColPtr = cursor;
        SDF_VU0_LOADMATRIX(matrixPacket);
        MultMatrixRev(&sdf3DScreenTm);
        corePktLoadWorldLightMtx(matrixPacket);
    } else {
        u8 *vertices;
        cursor = CORE_ALIGN16(cb.dataPtr);
        if (packetBits & SDF_PACKETBIT_FACE) {
            cb.facePtr = cursor;
            cursor = CORE_ALIGN16(cursor + faceCount * 6);
        }
        vertices = cursor;
        cursor = CORE_ALIGN16(cursor + pointCount * 12);
        if (packetBits & SDF_PACKETBIT_NORMAL) {
            cb.normalPtr = cursor;
            cursor = CORE_ALIGN16(cursor + pointCount * 12);
        }
        if (packetBits & SDF_PACKETBIT_ST) {
            cb.uvPtr = cursor;
            cursor = CORE_ALIGN16(cursor + pointCount * 8);
            if (cb.orgAttrBits & SDF_ATTRBIT_MULTITEX) {
                cb.uv2Ptr = cursor;
                cursor = CORE_ALIGN16(cursor + pointCount * 8);
            }
        }
        cb.vertColPtr = cursor;
        SDF_VU0_LOADMATRIX(matrixPacket);
        MultMatrixRev(&sdf3DScreenTm);
        corePktLoadWorldLightMtx(matrixPacket);
        if (cb.savedAttrBits & SDF_ATTRBIT_TEXMAP) {
            corePktXfmLightClipUV(points, pointCount, (const u32 *)vertices,
                                 cb.normalPtr, cb.uvPtr, cb.matPacket2->texTm.m);
        } else {
            corePktXfmLightClip(points, pointCount, (const u32 *)vertices, cb.normalPtr);
        }
        corePktOpenAndEmit(&cb);
        return corePktClosePacket(&cb);
    }

    if (cb.savedAttrBits & SDF_ATTRBIT_TEXMAP) {
        corePktXfmLightClipUVPB(points, pointCount, cb.uvPtr, cb.matPacket2->texTm.m);
    } else {
        corePktXfmLightClipPB(points, pointCount);
    }
    corePktOpenAndEmit(&cb);
    return corePktClosePacket(&cb);
}
#undef CORE_ALIGN16
#undef CORE_STORE_UV_PAIR
#else
INCLUDE_ASM(const s32, "sdf/sdfCoreDraw", sdfCorePacketGen);
#endif

void sdfCoreDrawScanOneSkinNodes(sdfModelRootNode_t *root, sdfCorePacket_t *p) {
    sdfModelNode_t **objects = root->nodeList->items;
    u16 *indices = (u16 *)(p + 1);
    if ((p->prim.packetBits & SDF_PACKETBIT_BLENDVERT) != 0) {
        s32 count = *indices;
        if (count != 0) {
            indices++;
            do {
                sdfModelAllocTmPacket(objects[*indices++]);
            } while (--count != 0);
        }
    }
}

void sdfCoreDrawSetLight(u8 *light) {
    corePktCurrLight = (u32)light;
}

void sdfCoreDrawSetFog(u8 *fog) {
    if (corePktCurrFog != (u32)fog) {
        corePktCurrFog = (u32)fog;
        SDF_VU0_COPYVECTOR(&corePktFogParm, fog + 0x10);
    }
}

void sdfCoreDrawSetZBias(f32 z) {
    corePktZBias = z;
}

void sdfCoreDrawReset(void) {
    corePktCurrLight = 0;
    corePktCurrFog = 0;
}

void sdfCoreDrawInit(s32 buffSize) {
    u32 *dmaChannel = sceDmaGetChan(0);
    HW_WRITE(dmaChannel, HW_READ(dmaChannel) & ~SDF_DMA_CHCR_TTE);
    sceDmaSendN(dmaChannel, vu0MicroPacket, (vu1MicroPacket - vu0MicroPacket) >> SDF_DMA_QWORD_SHIFT);
    sceDmaSync(dmaChannel, 0, 0);
    sdfMemFreeHandleDirect(&sdfCoreDrawBuffHandle);
    sdfCoreDrawBuffHandle = sdfMemAllocDirect(buffSize);
    corePktPointBuff = (u32)sdfMemLock(sdfCoreDrawBuffHandle);
}
