#include "common.h"
#include "libsdf.h"

extern sdfTexHandle_t *sdfTexHandleLink;

extern void *sdfChipAllocDirect(s32 size);
extern void *sdfChipAllocZeroDirect(s32 size);
extern void sdfChipFreeDirect(void *ptr);
extern void *sdfAllocAuto(s32 size);
extern void sdfFreeAuto(void *ptr);
extern void sdfGsMemFree(sdfGsMemSlice_t *slice);
extern sdfGsMemSlice_t *sdfGsMemAllocFromTail(s32 size, s32 type);
extern void *memcpy(void *dst, const void *src, u32 len);

/* The texture file header sdfTexSetEx parses; only its address is needed here. */
typedef struct sdfTexFile_t sdfTexFile_t;
extern sdfTexHandle_t *sdfTexSetEx(sdfTexFile_t *aTex, s32 aMemMode);

extern void *sdfTexTransData(s32 aGsAddr, s32 aWidth, s32 aHeight, s32 aPixelFormat, void *aData,
                             s32 aMemMode);
extern void sdfTexSetPacket(sdfTexPacket_t *aPacket, s32 aWidth, s32 aHeight, s32 aGsAddr,
                            s32 aPixelFormat, s32 aClutAddr, s32 aClutFormat, s32 aColorComp,
                            s32 aNumOfMipmap, s32 aMipmapParm, s32 aWrapMode, s32 aContext);

sdfGsMemSlice_t *sdfTexAllocClut(s32 aPixelFormat, s32 aCLUTFormat, s32 aNumOfCLUT);
void sdfTexAllocClutBuff(sdfTexHandle_t *aHandle);
void sdfTexCopyClutData(sdfTexHandle_t *aHandle, void *aData);
void sdfTexMakeGIFPacket(sdfTexHandle_t *aHandle);
void sdfTexMakeGIFPacketCtx2(sdfTexHandle_t *aHandle);
void sdfTexRebuildPacket(sdfTexHandle_t *aHandle);

sdfTexPacket_t *sdfTexGetPacket(sdfTexHandle_t *aHandle)
{
    return aHandle->packet;
}

s32 sdfTexGetPacketLength(sdfTexHandle_t *aHandle)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packet;
    if (packet == NULL) {
        return 0;
    }
    return (((u32)packet->giftag.parm & 0x7FFF) + 1) << 4;
}

sdfTexPacket_t *sdfTexGetPacketCtx2(sdfTexHandle_t *aHandle)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packetCtx2;
    if (packet == NULL) {
        sdfTexMakeGIFPacketCtx2(aHandle);
        packet = aHandle->packetCtx2;
    }
    return packet;
}

s32 sdfTexGetPacketCtx2Length(sdfTexHandle_t *aHandle)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packetCtx2;
    if (packet == NULL) {
        return 0;
    }
    return (((u32)packet->giftag.parm & 0x7FFF) + 1) << 4;
}

s32 sdfTexGetCLUTCount(sdfTexHandle_t *aHandle)
{
    return aHandle->numOfCLUT;
}

u32 sdfTexGetCLUTAddr(sdfTexHandle_t *aHandle)
{
    sdfGsMemSlice_t *slice;

    slice = aHandle->clutGsHandle;
    if (slice == NULL) {
        return 0;
    }
    return slice->addr;
}

u32 sdfTexGetPixelAddr(sdfTexHandle_t *aHandle)
{
    return aHandle->pixelGsHandle->addr;
}

s32 sdfTexGetPixelBitCount(s32 aPixelFormat)
{
    switch (aPixelFormat) {
    case 0:
        return 32;
    case 1:
        return 24;
    case 2:
    case 10:
        return 16;
    case 19:
    case 27:
        return 8;
    default:
        return 4;
    }
}

s32 sdfTexGetTexelBitCount(s32 aPixelFormat)
{
    s32 bitCount;

    switch (aPixelFormat) {
    case 19:
        bitCount = 8;
        break;
    case 20:
        bitCount = 4;
        break;
    case 2:
    case 10:
        bitCount = 16;
        break;
    default:
        bitCount = 32;
        break;
    }
    return bitCount;
}

u64 sdfTexGetTex0Reg(sdfTexHandle_t *aHandle)
{
    return aHandle->packet->tex0.data;
}

u64 sdfTexGetTex1Reg(sdfTexHandle_t *aHandle)
{
    return aHandle->packet->tex1.data;
}

u64 sdfTexGetClampReg(sdfTexHandle_t *aHandle)
{
    return aHandle->packet->clamp.data;
}

void sdfTexSetFilter(sdfTexHandle_t *aHandle, s32 aMagMode, s32 aMinMode)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packet;
    packet->tex1.data = (packet->tex1.data & ~0x1E0) | (aMagMode << 5) | (aMinMode << 6);
}

void sdfTexSetFilterCtx2(sdfTexHandle_t *aHandle, s32 aMagMode, s32 aMinMode)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packetCtx2;
    if (packet == NULL) {
        sdfTexMakeGIFPacketCtx2(aHandle);
        packet = aHandle->packetCtx2;
    }
    packet->tex1.data = (packet->tex1.data & ~0x1E0) | (aMagMode << 5) | (aMinMode << 6);
}

void sdfTexSetWrapMode(sdfTexHandle_t *aHandle, s32 aWrapMode)
{
    aHandle->texWrapMode = aWrapMode;
    sdfTexRebuildPacket(aHandle);
}

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexTransData);

void *sdfTexTransCLUT(sdfTexHandle_t *aHandle, s32 aGsAddr, void *aData, s32 aMemMode)
{
    s32 width;
    s32 height;

    if (aHandle->pixelFormat == 19 || aHandle->pixelFormat == 27) {
        width = 16;
        height = 16;
    } else {
        width = 8;
        height = 2;
    }
    return sdfTexTransData(aGsAddr, width, height, aHandle->CLUTFormat, aData, aMemMode);
}

void sdfTexClutToGs(sdfTexHandle_t *aHandle)
{
    if (aHandle->clutGsHandle != NULL) {
        sdfTexTransCLUT(aHandle, sdfTexGetCLUTAddr(aHandle), aHandle->clutData, 0);
    }
}

void sdfTexLink(sdfTexHandle_t *aHandle)
{
    aHandle->prev = NULL;
    if (sdfTexHandleLink != NULL) {
        aHandle->next = sdfTexHandleLink;
        sdfTexHandleLink->prev = aHandle;
    } else {
        aHandle->next = NULL;
    }
    sdfTexHandleLink = aHandle;
}

sdfTexHandle_t *sdfTexNew(s32 aWidth, s32 aHeight, s32 aPixelFormat, s32 aNumOfMipmap,
                          sdfGsMemSlice_t *aPixelGsHandle, s32 aCLUTFormat, s32 aNumOfCLUT,
                          sdfGsMemSlice_t *aCLUTGsHandle)
{
    sdfTexHandle_t *handle;

    handle = sdfChipAllocZeroDirect(64);
    handle->numOfCLUT = aNumOfCLUT;
    handle->CLUTFormat = aCLUTFormat;
    handle->width = aWidth;
    handle->height = aHeight;
    handle->pixelFormat = aPixelFormat;
    handle->numOfMipmap = aNumOfMipmap;
    handle->clutGsHandle = aCLUTGsHandle;
    handle->pixelGsHandle = aPixelGsHandle;
    sdfTexLink(handle);
    handle->clutColor = 0x80808080;
    return handle;
}

sdfTexHandle_t *sdfTexDup(sdfTexHandle_t *aHandle)
{
    sdfTexHandle_t *dup;
    sdfTexHandle_t *origin;

    dup = sdfChipAllocDirect(64);
    *dup = *aHandle;
    origin = dup->origin;
    if (origin == NULL) {
        origin = aHandle;
        dup->origin = aHandle;
    }
    dup->packet = NULL;
    dup->packetCtx2 = NULL;
    dup->clutData = NULL;
    dup->monoClutData = NULL;
    sdfTexLink(dup);
    if (dup->clutGsHandle != NULL) {
        dup->clutGsHandle = sdfTexAllocClut(dup->pixelFormat, dup->CLUTFormat, dup->numOfCLUT);
        sdfTexAllocClutBuff(dup);
        sdfTexCopyClutData(dup, origin->clutData);
        dup->clutColor = 0x80808080;
    }
    sdfTexMakeGIFPacket(dup);
    sdfTexClutToGs(dup);
    return dup;
}

sdfGsMemSlice_t *sdfTexAllocClut(s32 aPixelFormat, s32 aCLUTFormat, s32 aNumOfCLUT)
{
    sdfGsMemSlice_t *slice;
    s32 bitsPerEntry;
    s32 size;
    s16 height;

    slice = NULL;
    switch (aCLUTFormat) {
    case 0:
        bitsPerEntry = 32;
        break;
    case 2:
    case 10:
        bitsPerEntry = 16;
        break;
    default:
        return NULL;
    }

    switch (aPixelFormat) {
    case 19:
    case 27:
        size = bitsPerEntry * 8;
        height = 32;
        break;
    case 20:
    case 36:
    case 44:
        size = 64;
        height = 8;
        break;
    default:
        return slice;
    }

    slice = sdfGsMemAllocFromTail(size, 3);
    slice->w = 8;
    slice->h = height;
    slice->pixelFormat = aCLUTFormat;
    return slice;
}

s32 __attribute__((const)) sdfTexGetClutDataSize(s32 aPixelFormat, s32 aCLUTFormat, s32 aNumOfCLUT)
{
    s32 entries;
    s32 bytesPerEntry;

    bytesPerEntry = (aCLUTFormat == 0) ? 4 : 2;
    entries = (aPixelFormat == 19 || aPixelFormat == 27) ? 256 : 16;
    return entries * bytesPerEntry * aNumOfCLUT;
}

void sdfTexAllocClutBuff(sdfTexHandle_t *aHandle)
{
    s32 size;

    size = sdfTexGetClutDataSize(aHandle->pixelFormat, aHandle->CLUTFormat, aHandle->numOfCLUT);
    aHandle->clutDataSize = size;
    aHandle->clutData = sdfAllocAuto(size);
}

void sdfTexCopyClutData(sdfTexHandle_t *aHandle, void *aData)
{
    memcpy(aHandle->clutData, aData, aHandle->clutDataSize);
}

sdfTexHandle_t *sdfTexAlloc(s32 aWidth, s32 aHeight, s32 aPixelFormat, s32 aNumOfMipmap,
                            s32 aCLUTFormat, s32 aNumOfCLUT)
{
    s32 indexed;
    s32 bitsPerPixel;
    sdfGsMemSlice_t *clutSlice;
    sdfGsMemSlice_t *pixelSlice;
    s32 mipWidth;
    s32 mipHeight;
    s32 words;
    s32 mipLevel;

    indexed = 0;
    switch (aPixelFormat) {
    case 0:
    case 1:
        bitsPerPixel = 32;
        break;
    case 2:
        bitsPerPixel = 16;
        break;
    case 10:
        bitsPerPixel = 16;
        break;
    case 19:
        indexed = 1;
        bitsPerPixel = 8;
        break;
    case 20:
        indexed = 1;
        bitsPerPixel = 4;
        break;
    default:
        return NULL;
    }

    clutSlice = NULL;
    if (indexed) {
        clutSlice = sdfTexAllocClut(aPixelFormat, aCLUTFormat, aNumOfCLUT);
    }

    mipWidth = aWidth;
    mipHeight = aHeight;
    words = 0;
    mipLevel = 0;
    do {
        words += (mipWidth * mipHeight * bitsPerPixel) >> 5;
        mipWidth >>= 1;
        mipHeight >>= 1;
    } while (mipLevel++ != aNumOfMipmap);

    pixelSlice = sdfGsMemAllocFromTail(words, 2);
    pixelSlice->w = aWidth;
    pixelSlice->h = aHeight;
    pixelSlice->pixelFormat = aPixelFormat;
    return sdfTexNew(aWidth, aHeight, aPixelFormat, aNumOfMipmap, pixelSlice, aCLUTFormat,
                     aNumOfCLUT, clutSlice);
}

void sdfTexFree(sdfTexHandle_t *aHandle)
{
    sdfTexHandle_t *next;
    sdfTexHandle_t *prev;

    if (aHandle != NULL) {
        if (aHandle->origin == NULL) {
            sdfGsMemFree(aHandle->pixelGsHandle);
        }
        sdfGsMemFree(aHandle->clutGsHandle);
        sdfChipFreeDirect(aHandle->packet);
        sdfChipFreeDirect(aHandle->packetCtx2);
        prev = aHandle->prev;
        next = aHandle->next;
        if (next != NULL) {
            next->prev = prev;
        }
        if (prev != NULL) {
            prev->next = next;
        } else {
            sdfTexHandleLink = next;
        }
        sdfFreeAuto(aHandle->clutData);
        sdfFreeAuto(aHandle->monoClutData);
        sdfChipFreeDirect(aHandle);
    }
}

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexFreeDelayed);

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexSetPacket);

sdfTexPacket_t *sdfTexMakeGIFPacketSub(sdfTexHandle_t *aHandle, s32 aContext)
{
    sdfTexPacket_t *packet;
    u32 pixelAddr;
    u32 clutAddr;

    packet = sdfChipAllocDirect(64);
    pixelAddr = sdfTexGetPixelAddr(aHandle);
    clutAddr = sdfTexGetCLUTAddr(aHandle);
    sdfTexSetPacket(packet, aHandle->width, aHandle->height, pixelAddr, aHandle->pixelFormat,
                    clutAddr, aHandle->CLUTFormat, 1, aHandle->numOfMipmap, aHandle->mipmapParam,
                    aHandle->texWrapMode, aContext);
    return packet;
}

void sdfTexMakeGIFPacket(sdfTexHandle_t *aHandle)
{
    aHandle->packet = sdfTexMakeGIFPacketSub(aHandle, 0);
}

void sdfTexMakeGIFPacketCtx2(sdfTexHandle_t *aHandle)
{
    aHandle->packetCtx2 = sdfTexMakeGIFPacketSub(aHandle, 1);
}

void sdfTexRebuildPacket(sdfTexHandle_t *aHandle)
{
    sdfTexPacket_t *packet;

    packet = aHandle->packet;
    if (packet != NULL) {
        sdfTexSetPacket(packet, aHandle->width, aHandle->height, sdfTexGetPixelAddr(aHandle),
                        aHandle->pixelFormat, sdfTexGetCLUTAddr(aHandle), aHandle->CLUTFormat,
                        (packet->tex0.data >> 34) & 1, aHandle->numOfMipmap, aHandle->mipmapParam,
                        aHandle->texWrapMode, 0);
    }

    packet = aHandle->packetCtx2;
    if (packet != NULL) {
        sdfTexSetPacket(packet, aHandle->width, aHandle->height, sdfTexGetPixelAddr(aHandle),
                        aHandle->pixelFormat, sdfTexGetCLUTAddr(aHandle), aHandle->CLUTFormat,
                        (packet->tex0.data >> 34) & 1, aHandle->numOfMipmap, aHandle->mipmapParam,
                        aHandle->texWrapMode, 1);
    }
}

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexSetEx);

sdfTexHandle_t *sdfTexSet(sdfTexFile_t *aTex)
{
    return sdfTexSetEx(aTex, 0);
}

sdfTexHandle_t *sdfTexSetDirect(sdfTexFile_t *aTex)
{
    return sdfTexSetEx(aTex, 1);
}

void sdfTexSetMonoClut(sdfTexHandle_t *aHandle)
{
    s32 bytesPerEntry;
    s32 entryCount;
    u8 *out;
    u8 *src;
    u32 color;
    u32 r;
    u32 g;
    u32 b;

    if (aHandle->CLUTFormat == 0) {
        bytesPerEntry = 4;
        entryCount = (u32)aHandle->clutDataSize >> 2;
    } else {
        bytesPerEntry = 2;
        entryCount = (u32)aHandle->clutDataSize >> 1;
    }
    out = aHandle->monoClutData;
    if (out == NULL) {
        out = sdfAllocAuto(entryCount);
        aHandle->monoClutData = out;
    }
    src = aHandle->clutData;
    do {
        if (bytesPerEntry == 4) {
            color = *(u32 *)src;
            src += 4;
        } else {
            u16 packed = *(u16 *)src;

            src += 2;
            /* 1555 to 8888: pext5 spreads the five-bit fields, then the top
             * three bits of each are copied down into the low bits. */
            __asm__ volatile("pext5 %0, %1" : "=r"(color) : "r"(packed));
            color |= (color >> 5) & 0x070707;
        }
        g = (color >> 8) & 0xFF;
        r = color & 0xFF;
        b = (color >> 16) & 0xFF;
        entryCount--;
        *out = ((r * 77) >> 8) + ((g * 150) >> 8) + ((b * 29) >> 8);
        out++;
    } while (entryCount != 0);
}

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexSetClutColor);

sdfTexHandle_t *sdfTexSearch(u32 aTexId)
{
    sdfTexHandle_t *handle;

    for (handle = sdfTexHandleLink; handle != NULL; handle = handle->next) {
        if (handle->texId == aTexId) {
            return handle;
        }
    }
    return NULL;
}

INCLUDE_ASM(const s32, "sdf/sdfTex", sdfTexInit);

INCLUDE_SDATA(const s32, "sdf/sdfTex", sdfTexHandleLink);
