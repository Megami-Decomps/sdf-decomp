#include "common.h"
#include "draw3d.h"
#include "ps2asm.h"

/* The native end message is padded to a 0x20-byte rodata block. */
extern const char D_00532E00[]; /* "dds3CrossfadeEnd\n" */
extern s8 dds3CrossfadeRunFlag;
extern u32 D_003176C0[];
extern s32 sdf2DGetPrimPacketSize(s32 registerCount, s32 loopCount);
extern void *sdfGrAllocPacketBuff(s32 size);
extern sdfGrPacketHeader_t *sdf2DMakePrimPacketHeader(sdfGrPacketHeader_t *packet, s32 flags, s32 registerCount, s64 registers, s32 loopCount);
extern void dds3RenderTextureOFF(void);
extern void dds3FreeRenderTexture(void);
extern void *memcpy(void *dst, const void *src, u32 size);
extern s32 printf(const char *fmt, ...);

void *effPkt2DGetPrimPtr(void *packet) {
    return (void *)((u8 *)packet + 0x20);
}

sdfGrPacketHeader_t *effPkt2DAllocTriStrip(s32 pointCount, s32 flags) {
    sdfGrPacketHeader_t *packet = (sdfGrPacketHeader_t *)sdfGrAllocPacketBuff(sdf2DGetPrimPacketSize(9, pointCount));

    sdf2DMakePrimPacketHeader(packet, flags | 0x54, 9, 0x525252521, pointCount);
    return packet;
}

u64 *effPkt2DAllocF4(u32 flags) {
    u64 *packet = (u64 *)sdfGrAllocPacketBuff(0x80);

    packet[0] = 7;
    packet[1] = 0x5000000700000000ULL;
    packet[2] = 0xB400000000008001ULL;
    packet[3] = 0xFF515151510ULL;
    packet[4] = flags | 0x14C;
    packet[15] = 0;
    packet[14] = 0;
    packet[13] = 0;
    return packet;
}

void *effPkt3DPrimMakePoly(sdf3DPrimPolyParm_t *params) {
    s32 parameterCount = params->faceCount;
    s32 vertexCount = params->pointCount;
    u32 primitive = params->attr | 0x48;
    u32 mode = 0x60;
    s32 stride = 3;
    s32 overhead = 16;
    s32 allocationSize;
    s32 i;
    s32 offset;
    void *packet;
    void *(*allocate)(s32);
    u32 *cursor;
    u32 *stream;
    const u128 *vector;
    u32 countCode;
    u32 *stream2;

    if (params->normal != NULL) {
        mode = 0xE0;
        overhead = 17;
        stride = 6;
    }
    if (params->uv != NULL) {
        mode |= 0x100;
        primitive |= 0x10;
        overhead++;
        stride += 2;
        if (params->multiUv != NULL) {
            primitive |= 0x1000;
            stride += 2;
        }
    }
    if (params->vertexColor != NULL) {
        mode |= 0x200;
        primitive |= 0x800;
        overhead++;
        stride++;
    }
    allocationSize = (overhead + parameterCount + stride * vertexCount) * 4;
    allocationSize = (allocationSize + 15) & ~15;
    allocate = (void *(*)(s32))params->allocator;
    if (allocate == NULL) {
        allocate = sdfGrAllocPacketBuff;
    }
    packet = allocate(allocationSize);

    *(u64 *)packet = (u64)(u16)((allocationSize >> 4) - 1) | 0x20000000ULL;
    ((u32 *)packet)[2] = 0x6102C000;
    ((u32 *)packet)[3] = (~params->attrInhibit) & 0xFFFF;
    ((u32 *)packet)[4] = 0x6E01C002;
    ((u32 *)packet)[5] = params->color;
    ((u32 *)packet)[6] = 0x6001C003;
    cursor = (u32 *)packet + 7;
    *cursor++ = (u32)params->zBias;
    *cursor++ = 0x04000004;
    *cursor++ = 0x14000000;
    *cursor++ = 0x6D01C000;
    *cursor++ = (u16)parameterCount | ((u32)(u16)vertexCount << 16);
    *cursor++ = (primitive & 0xFFFF) | (mode << 16);
    *cursor++ = 0x6E00C001 | ((u32)parameterCount << 16);

    if (params->faceIndex == NULL) {
        memcpy(cursor, D_003176C0, parameterCount * 4);
    } else {
        memcpy(cursor, params->faceIndex, parameterCount * 4);
    }
    cursor += parameterCount;
    offset = parameterCount + 1;
    countCode = vertexCount << 16;
    *cursor++ = countCode | offset | 0x6800C000;
    vector = (u128 *)params->vertex;
    i = 0;
    do {
        MMI_STORE_VEC3_VALUE(cursor, *vector);
        i++;
        cursor += 3;
        vector++;
    } while (i != vertexCount);
    offset += vertexCount;

    vector = (u128 *)params->normal;
    if (vector != NULL) {
        *cursor++ = countCode | offset | 0x6800C000;
        i = 0;
        do {
            MMI_STORE_VEC3_VALUE(cursor, *vector);
            i++;
            cursor += 3;
            vector++;
        } while (i != vertexCount);
        offset += vertexCount;
    }
    stream = (u32 *)params->uv;
    if (stream != NULL) {
        stream2 = (u32 *)params->multiUv;
        if (stream2 == NULL) {
            *cursor++ = countCode | offset | 0x6400C000;
            memcpy(cursor, stream, vertexCount * 8);
            cursor += vertexCount * 2;
        } else {
            *cursor++ = countCode | offset | 0x6C00C000;
            i = 0;
            do {
                *cursor++ = *stream++;
                *cursor++ = *stream++;
                *cursor++ = *stream2++;
                *cursor++ = *stream2++;
                i++;
            } while (i != vertexCount);
        }
        offset += vertexCount;
    }
    stream = params->vertexColor;
    if (stream != NULL) {
        *cursor++ = countCode | offset | 0x6E00C000;
        memcpy(cursor, stream, vertexCount * 4);
        cursor += vertexCount;
    }
    *cursor = 0x1400000C;
    while ((u32)++cursor & 15) {
        *cursor = 0;
    }
    return packet;
}

/* Each framebuffer has two GS clear-colour writes, 0x10 bytes apart. */
INCLUDE_ASM(const s32, "effect/effPacket", dds3CrossfadeStart);

INCLUDE_ASM(const s32, "effect/effPacket", dds3CrossfadeRun);

void dds3CrossfadeEnd(void) {
    dds3RenderTextureOFF();
    dds3FreeRenderTexture();
    dds3CrossfadeRunFlag = 0;
    printf(D_00532E00);
}
INCLUDE_RODATA(const s32, "effect/effPacket", D_00532E00);

INCLUDE_SDATA(const s32, "effect/effPacket", dds3CrossfadeRunFlag);

INCLUDE_SDATA(const s32, "effect/effPacket", D_0054F0C1);

INCLUDE_SDATA(const s32, "effect/effPacket", dds3CrossfadeFrame);

INCLUDE_SDATA(const s32, "effect/effPacket", dds3CrossfadeFrameMax);

