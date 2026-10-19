#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

/* GS pixel storage modes, the values of sdfGsMemSlice_t.pixelFormat. */
enum sdf_psm
{
    SDF_PSMCT32 = 0x00,
    SDF_PSMCT24 = 0x01,
    SDF_PSMCT16 = 0x02,
    SDF_PSMCT16S = 0x0A,
    SDF_PSMT8 = 0x13,
    SDF_PSMT4 = 0x14,
    SDF_PSMT8H = 0x1B,
    SDF_PSMT4HL = 0x24,
    SDF_PSMT4HH = 0x2C,
    SDF_PSMZ32 = 0x30,
    SDF_PSMZ24 = 0x31,
    SDF_PSMZ16 = 0x32,
    SDF_PSMZ16S = 0x3A
};

extern void *sdfChipAllocDirect(u32 size);

extern void sdfChipFreeDirect(void *ptr);

extern s32 iWakeupThread(s32 threadId);

extern void (*sdfGsErrorHandler)(s32 size, s32 type);

/* sdfImageSetRelease_t.freeMode: which heap owns the uploaded pixels. */
enum SdfImageSetMem
{
    SDF_IMAGESET_MEM_IGNORE = 0,
    SDF_IMAGESET_MEM_HANDLE = 1,
    SDF_IMAGESET_MEM_CHIP = 2
};

typedef struct sdfImageSetRelease
{
    struct sdfImageSetRelease *next;
    void *chip;
    sdfMemHandle_t *memHandle;
    u8 freeMode;
} sdfImageSetRelease_t;

/* Terminal DMA tag of a queued upload packet. Linking the next packet writes
 * its address into tag and clears the two VIF codes behind it. */
typedef struct sdfImageSetTag
{
    u64 tag;
    u64 vifCodes;
} sdfImageSetTag_t;

typedef struct sdfImageSetCB
{
    s32 semaId;
    sdfImageSetRelease_t *releaseHead;
    sdfImageSetRelease_t *releaseTail;
    u32 dmaAddr;
    sdfImageSetTag_t *tagTail;
} sdfImageSetCB_t;

typedef struct sdfImageSetReqParam
{
    void *data;
    sdfMemHandle_t *memHandle;
    u8 freeMode;
    u8 pixelFormat;
    u16 bufferWidth;
    u32 addr;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} sdfImageSetReqParam_t;

extern s32 WaitSema(s32 semaId);

extern s32 SignalSema(s32 semaId);

extern s32 sdfCreateSema(s32 initCount, s32 maxCount, s32 option);

extern void *sdfChipAllocZeroDirect(u32 size);

extern s32 sdfChipCheck(void *ptr);

extern sdfMemHandle_t *sdfMemGetHandleByPtr(void *ptr);

extern void sdfMemFreeDirect(sdfMemHandle_t *handle);

extern s32 GetThreadId(void);

extern s32 AddDmacHandler2(s32 channel, s32 (*handler)(s32, s32), s32 next, s32 arg);

extern s32 RemoveDmacHandler(s32 channel, s32 handlerId);

extern void *sceDmaGetChan(s32 channel);

extern s32 sceDmaSend(void *chan, void *packet);

extern void FlushCache(s32 mode);

extern void SleepThread(void);

extern sdfImageSetCB_t sdfImageSetCb;

extern u64 D_005293A0[];

s32 sdfImageDataSize(u32 pixelFormat, s32 w, s32 h);

s32 sdfImageSetDmacHandler(s32 channel, s32 threadId);

void sdfImageSetClearCb(sdfImageSetCB_t *cb);

s32 sdfImageDataSize(u32 pixelFormat, s32 w, s32 h)
{
    s32 bitCount;

    switch (pixelFormat) {
    case SDF_PSMCT24:
    case SDF_PSMZ24:
        bitCount = 0x18;
        break;
    case SDF_PSMCT16:
    case SDF_PSMCT16S:
    case SDF_PSMZ16:
    case SDF_PSMZ16S:
        bitCount = 0x10;
        break;
    case SDF_PSMT8:
    case SDF_PSMT8H:
        bitCount = 8;
        break;
    case SDF_PSMT4:
    case SDF_PSMT4HL:
    case SDF_PSMT4HH:
        bitCount = 4;
        break;
    default:
        bitCount = 0x20;
        break;
    }
    return (bitCount * w * h) >> 7;
}

void sdfImageSetReq(sdfImageSetReqParam_t *req)
{
    sdfImageSetCB_t *cb = &sdfImageSetCb;
    sdfImageSetRelease_t *release;
    sdfImageSetTag_t *tagTail;
    u64 *setup;
    u64 *packet;
    void *buf;
    u32 pixelFormat;
    u16 w;
    u16 h;
    u16 bufferWidth;
    s32 qwc;
    s32 blocks;
    s32 remaining;
    s32 widthUnits;
    s32 last;
    u32 src;
    u64 formatAndAddr;
    u64 coords;
    u64 size;

    pixelFormat = req->pixelFormat;
    w = req->w;
    h = req->h;
    bufferWidth = req->bufferWidth;
    widthUnits = bufferWidth >> 6;
    if (widthUnits == 0) {
        widthUnits = 1;
    }

    qwc = sdfImageDataSize(pixelFormat, w, h);
    blocks = (qwc + 0x7FEF) / 0x7FF0;
    buf = sdfChipAllocDirect(blocks * 0x30 + 0x80);
    release = (sdfImageSetRelease_t *)buf;
    release->next = NULL;
    release->chip = req->data;
    release->memHandle = req->memHandle;
    release->freeMode = req->freeMode;

    setup = (u64 *)((u8 *)buf + 0x10);
    packet = (u64 *)((u8 *)buf + 0x70);
    formatAndAddr = ((u64)pixelFormat << 56) | ((u64)widthUnits << 48) | ((u64)(req->addr >> 6) << 32);
    coords = ((u64)req->y << 48) | ((u64)req->x << 32);
    size = ((u64)h << 32) | w;

    setup[0] = 0x0000000010000005ULL;
    setup[1] = 0x5000000500000000ULL;
    setup[2] = 0x1000000000000004ULL;
    setup[3] = 0x000000000000000EULL;
    setup[4] = formatAndAddr;
    setup[5] = 0x50ULL;
    setup[6] = coords;
    setup[7] = 0x51ULL;
    setup[8] = size;
    setup[9] = 0x52ULL;
    setup[10] = 0;
    setup[11] = 0x53ULL;

    src = (u32)req->data & 0x0FFFFFFF;
    remaining = qwc;
    last = 0;
    do {
        s32 chunk = 0x7FF0;

        if (remaining < 0x7FF1) {
            chunk = remaining;
            last = 1;
        }

        packet[0] = 0x0000000010000001ULL;
        packet[1] = 0x5000000100000000ULL;
        packet[2] = ((u64)chunk | ((u64)last << 15)) | 0x0800000000000000ULL;
        packet[4] = ((u32)chunk & 0xFFFF) | 0x30000000ULL | ((u64)src << 32);
        packet[5] = ((u64)((u32)chunk | 0x51000000)) << 32;

        src += (u32)chunk << 4;
        remaining -= chunk;
        packet += 6;
    } while (last == 0);

    WaitSema(cb->semaId);

    if (cb->releaseTail != NULL) {
        cb->releaseTail->next = release;
    } else {
        cb->releaseHead = release;
    }
    cb->releaseTail = release;

    setup = (u64 *)((u32)setup & 0x0FFFFFFF);
    tagTail = cb->tagTail;
    if (tagTail != NULL) {
        tagTail->vifCodes = 0;
        tagTail->tag = ((u64)(u32)setup << 32) | 0x20000000ULL;
    } else {
        cb->dmaAddr = (u32)setup;
    }
    cb->tagTail = (sdfImageSetTag_t *)packet;
    SignalSema(cb->semaId);
}

void sdfImageSetChainTag(u32 dmaAddr, sdfImageSetTag_t *tagTail)
{
    sdfImageSetCB_t *cb = &sdfImageSetCb;
    sdfImageSetTag_t *last;

    WaitSema(cb->semaId);
    last = cb->tagTail;
    if (last != NULL) {
        last->vifCodes = 0;
        last->tag = ((u64)(dmaAddr & 0x0FFFFFFF) << 32) | 0x20000000;
    } else {
        cb->dmaAddr = dmaAddr;
    }
    cb->tagTail = tagTail;
    SignalSema(cb->semaId);
}

void sdfImageSetChainReleaseMem(s32 addr)
{
    sdfImageSetCB_t *cb = &sdfImageSetCb;
    sdfImageSetRelease_t *release;

    if (addr != 0) {
        release = sdfChipAllocZeroDirect(0xA0);
        if (sdfChipCheck((void *)addr) != 0) {
            release->chip = (void *)addr;
            release->freeMode = SDF_IMAGESET_MEM_CHIP;
        } else {
            release->freeMode = SDF_IMAGESET_MEM_HANDLE;
            release->memHandle = sdfMemGetHandleByPtr((void *)addr);
        }
        WaitSema(cb->semaId);
        if (cb->releaseTail != NULL) {
            cb->releaseTail->next = release;
        } else {
            cb->releaseHead = release;
        }
        cb->releaseTail = release;
        SignalSema(cb->semaId);
    }
}

void sdfImageSetClearCb(sdfImageSetCB_t *cb)
{
    cb->releaseHead = NULL;
    cb->releaseTail = NULL;
    cb->dmaAddr = 0;
    cb->tagTail = NULL;
}

s32 sdfImageSetDmacHandler(s32 channel, s32 threadId)
{
    iWakeupThread(threadId);
    EE_SYNC_EI();
    return -1;
}

void sdfImageSetTrans(void)
{
    sdfImageSetCB_t *cb = &sdfImageSetCb;
    sdfImageSetRelease_t *release;
    sdfImageSetTag_t *tagTail;
    u32 dmaAddr;

    WaitSema(cb->semaId);
    release = cb->releaseHead;
    dmaAddr = cb->dmaAddr;
    tagTail = cb->tagTail;
    sdfImageSetClearCb(cb);
    SignalSema(cb->semaId);

    if (dmaAddr != 0) {
        s32 threadId = GetThreadId();
        s32 handlerId = AddDmacHandler2(1, sdfImageSetDmacHandler, 0, threadId);
        u64 tag = ((u64)((u32)D_005293A0 & 0x0FFFFFFF) << 32) | 0x20000000;
        u32 *chan;

        tagTail->vifCodes = 0;
        tagTail->tag = tag;
        chan = sceDmaGetChan(1);
        *chan = *chan | 0x40;
        FlushCache(0);
        sceDmaSend(chan, (void *)dmaAddr);
        SleepThread();
        RemoveDmacHandler(1, handlerId);
    }

    while (release != NULL) {
        sdfImageSetRelease_t *next = release->next;

        switch (release->freeMode) {
        case SDF_IMAGESET_MEM_HANDLE:
            sdfMemFreeDirect(release->memHandle);
            break;
        case SDF_IMAGESET_MEM_CHIP:
            sdfChipFreeDirect(release->chip);
            break;
        }
        sdfChipFreeDirect(release);
        release = next;
    }
}

void sdfImageSetInit(void)
{
    sdfImageSetCB_t *cb;

    cb = &sdfImageSetCb;
    cb->semaId = sdfCreateSema(1, 0x7F, 0);
    sdfImageSetClearCb(cb);
}

