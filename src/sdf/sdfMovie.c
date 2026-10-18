#include "common.h"
#include "libsdf.h"

struct MovObj;
struct SoundFormat;
struct SdfTexResource;
struct SdfStreamInputDmaTag;

typedef struct IpuDmaState
{
    u32 inputAddress;
    u32 inputTagAddress;
    u32 inputQwords;
    u32 inputControl;
    u32 outputAddress;
    u32 outputQwords;
    u32 outputControl;
    u32 bitPointer;
    u32 ipuControl;
} IpuDmaState;

typedef struct SdfStreamFrameNode
{
    struct SdfStreamFrameNode *streamPrev;
    struct SdfStreamFrameNode *streamNext;
    struct SdfStreamFrameNode *next;
    u8 active;
    u8 queued;
    u8 ipuCycleInitialized;
    u8 drained;
    u8 playbackPhase;
    u8 inputFeedEnabled;
    u8 inputDmaStartPending;
    u8 pendingPlaybackSubmissions;
    u8 audioMode;
    u8 loopMode;
    u8 playbackCadenceStep;
    u8 playbackCadenceRemainder;
    u8 bufferIndex;
    u8 transferPacketIndex;
    u8 completedBufferCount;
    u8 pad1B;
    s32 bufferSize;
    void *frameBuffers[2];
    void *transferPacketBuffers[2];
    u32 transferPacketBytes;
    s32 resourceWord;
    struct SdfTexResource *textureHead;
    u16 width;
    u16 height;
    u32 cycleLength;
    u32 tickCount;
    s32 playbackFrameIndex;
    struct SdfStreamInputDmaTag *inputDmaChain;
    u8 headerReady;
    u8 done;
    u8 filledSlots;
    u8 firstSlot;
    u8 *scratchBuffer;
    u8 pad58[4];
    s32 (*read)(struct SdfStreamFrameNode *, struct MovObj *, s32, void *, s32);
    struct MovObj *readContext;
    u8 inputFeedDmaInFlight;
    u8 unk65;
    u8 pad66[2];
    IpuDmaState dma;
} SdfStreamFrameNode;

typedef struct SoundFormat
{
    u8 unk00;
    u8 unk01;
    u8 unk02;
    u8 unk03;
} SoundFormat;

typedef struct MovOpenParam
{
    u16 unk00;
    u16 unk02;
    u32 unk04;
    u16 unk08;
    u16 unk0A;
    s32 resourceWord;
    u8 unk10;
    u8 unk11;
    u8 unk12;
} MovOpenParam;

typedef s32 (*SdfStreamRead)(SdfStreamFrameNode *, struct MovObj *, s32, void *, s32);

/* Ring buffer of raw file data for a plain (non PAC) movie stream. */
typedef struct MovLinearStream
{
    sdfMemHandle_t *memHandle;
    u8 *base;
    u8 *readPtr;
    u8 *writePtr;
    s32 pending;
} MovLinearStream;

typedef struct MovPacStream
{
    u8 unk00[0x18];
    s32 chunkSize;
    s32 unitSize;
    u8 scratch[0x20];
    sdfMemHandle_t *bitmapHandle;
    u8 *bitmap;
    s32 bitIndex;
    sdfMemHandle_t *dataHandle;
    u8 *dataBase;
    u8 *dataBody;
    s32 writeOffset;
    s32 pending;
    u8 *ringBase;
    s32 ringRead;
    s32 ringFill;
    s32 ringWritten;
    s32 unk70;
    u8 *scratchPtr;
} MovPacStream;

typedef union MovObjStream
{
    MovLinearStream *linear;
    MovPacStream *pac;
} MovObjStream;

typedef struct MovObj
{
    u8 active;
    u8 state;
    u8 stopRequested;
    u8 isPac;
    u16 unk04;
    u16 unk06;
    u32 unk08;
    u16 unk0C;
    u16 unk0E;
    void *fileHandle;
    s32 totalBytes;
    s32 remainingBytes;
    MovObjStream stream;
    u8 pacEnabled;
    u8 pad21;
    u8 packetLimit;
    u8 pad23;
    SdfStreamFrameNode soundNode;
} MovObj;

void puts(const char *aStr);
void sdfFileCmdRead(void *aHandle, void *aBuffer, s32 aSize);
s32 sdfFileCmdClose(void *aHandle);
void sdfDummyFunc(void);
void sdfMemFreeDirect(sdfMemHandle_t *aHandle);
void sdfChipFreeDirect(void *aPtr);
void sdfIpuFree(SdfStreamFrameNode *aNode);
s32 p2sdSyncMovie(void);
void *memcpy(void *aDst, const void *aSrc, u32 aSize);
s32 sdfFileCmdFileSize(void *aHandle);
s32 sdfFileCmdFree(void *aHandle);
s32 DIntr(void);
s32 EIntr(void);
void *memset(void *aDst, s32 aValue, u32 aSize);
char *strrchr(const char *aStr, s32 aChar);
s32 strcasecmp(const char *aLeft, const char *aRight);
void *sdfChipAllocZeroDirect(s32 aSize);
sdfMemHandle_t *sdfMemAllocDirect(s32 aSize);
void *sdfMemLock(sdfMemHandle_t *aHandle);
void *sdfFileCmdReadOpen(const char *aName, SdfStreamRead aRead, MovObj *aContext);
void movieAttachDecodeTask(SdfStreamFrameNode *aNode, SoundFormat *aFormat, SdfStreamRead aRead, MovObj *aContext, s32 aResourceWord);
s32 sdfCreateSema(s32 aInit, s32 aMax, s32 aAttr);
s32 movieReadChunk(void *aHandle, s32 aOp, void *aBuf, s32 aSize, MovObj *aMov);
s32 movieCopyChunk(SdfStreamFrameNode *aNode, MovObj *aMov, s32 aOp, void *aBuf, s32 aSize);
s32 movieParseHeader(SdfStreamFrameNode *aNode, MovObj *aMov, s32 aOp, void *aBuf, s32 aSize);
extern s32 movieStreamSema;
extern s32 movieSemaCount;
extern const char D_00551B50[];

void movie_2C5080(const char *aStr)
{
    puts(aStr);
}

INCLUDE_ASM(const s32, "sdf/sdfMovie", movieWaitSema);

INCLUDE_ASM(const s32, "sdf/sdfMovie", release_movieobj2);

void movieReadNextChunk(MovObj *aMov)
{
    s32 remaining = aMov->remainingBytes;
    MovLinearStream *stream = aMov->stream.linear;
    s32 size;
    u8 *dst;
    u8 *next;

    if (remaining != 0) {
        if (0x20000 - stream->pending < 0x4000) {
            aMov->state = 5;
        } else {
            aMov->state = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            dst = stream->writePtr;
            next = dst + size;
            if (next >= stream->base + 0x20000) {
                next = stream->base;
            }
            stream->writePtr = next;
            sdfFileCmdRead(aMov->fileHandle, dst, size);
        }
    }
}

s32 movieGetStreamSize(void *aHandle, s32 aOp, void *aBuf, s32 aSize, MovObj *aMov)
{
    MovLinearStream *stream = aMov->stream.linear;
    s32 enabled;

    aMov->fileHandle = aHandle;
    if (aMov->stopRequested != 0) {
        if (aMov->state != 6 && aMov->state != 7) {
            aMov->state = 7;
            sdfFileCmdClose(aHandle);
            return 0;
        }
    }
    switch (aMov->state) {
    case 0:
        if (aOp == 2) {
            aMov->state = 1;
            sdfFileCmdFileSize(aHandle);
        }
        break;
    case 1:
        if (aOp == 4) {
            aMov->totalBytes = aSize;
            aMov->remainingBytes = aSize;
            movieReadNextChunk(aMov);
        }
        break;
    case 4:
        if (aOp == 5) {
            enabled = DIntr();
            stream->pending += aSize;
            aMov->remainingBytes -= aSize;
            if (enabled != 0) {
                EIntr();
            }
            if (aMov->remainingBytes == 0) {
                aMov->state = 7;
                sdfFileCmdClose(aHandle);
            } else {
                movieReadNextChunk(aMov);
            }
        }
        break;
    case 7:
        if (aOp == 7) {
            aMov->fileHandle = NULL;
            aMov->state = 6;
            sdfFileCmdFree(aHandle);
        }
        break;
    }
    return 0;
}

void movieReadHeaderChunk(MovObj *aMov)
{
    s32 remaining = aMov->remainingBytes;
    MovPacStream *stream = aMov->stream.pac;
    s32 size;

    if (remaining != 0) {
        if (0x10000 - stream->pending < 0x4000) {
            aMov->state = 5;
        } else if (0x10000 + (stream->ringWritten - stream->ringFill) < 0x4000) {
            aMov->state = 5;
        } else {
            aMov->state = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            sdfFileCmdRead(aMov->fileHandle, stream->dataBase, size);
        }
    }
}

INCLUDE_ASM(const s32, "sdf/sdfMovie", movieReadChunk);

s32 movieCopyChunk(SdfStreamFrameNode *aNode, MovObj *aMov, s32 aOp, void *aBuf, s32 aSize)
{
    MovLinearStream *stream = aMov->stream.linear;
    u8 *src;
    u8 *base;
    s32 over;
    s32 first;

    switch (aOp) {
    case 0:
        if (aMov->stopRequested != 0) {
            *(u8 *)aBuf = 1;
            return 0;
        }
        if (aMov->remainingBytes == 0) {
            *(u8 *)aBuf = 1;
        }
        return stream->pending;
    case 1:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        src = stream->readPtr;
        base = stream->base;
        over = (src - base) + aSize - 0x20000;
        if (over >= 0) {
            first = aSize - over;
            memcpy(aBuf, src, first);
            if (over > 0) {
                memcpy((u8 *)aBuf + first, base, over);
            }
            stream->readPtr = base + over;
        } else {
            memcpy(aBuf, src, aSize);
            stream->readPtr = src + aSize;
        }
        stream->pending -= aSize;
        return aMov->remainingBytes;
    case 2:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        if (aMov->state == 5) {
            movieReadNextChunk(aMov);
        }
        return 0;
    }
    return 0;
}

s32 movieParseHeader(SdfStreamFrameNode *aNode, MovObj *aMov, s32 aOp, void *aBuf, s32 aSize)
{
    void movieMemcpy(void *aDst, const void *aSrc, s32 aCount)
    {
        memcpy(aDst, aSrc, aCount);
    }
    MovPacStream *stream = aMov->stream.pac;
    u8 *body;
    s32 off;
    s32 over;
    s32 first;

    switch (aOp) {
    case 0:
        if (aMov->stopRequested != 0) {
            *(u8 *)aBuf = 1;
            return 0;
        }
        if (aMov->remainingBytes == 0) {
            *(u8 *)aBuf = 1;
        }
        return stream->pending;
    case 1:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        off = stream->writeOffset;
        body = stream->dataBody;
        over = off + aSize - 0x10000;
        if (over >= 0) {
            first = aSize - over;
            movieMemcpy(aBuf, body + off, first);
            if (over > 0) {
                movieMemcpy((u8 *)aBuf + first, body, over);
            }
            stream->writeOffset = over;
        } else {
            movieMemcpy(aBuf, body + off, aSize);
            stream->writeOffset = off + aSize;
        }
        stream->pending -= aSize;
        return aMov->remainingBytes;
    case 2:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        if (aMov->state == 5) {
            movieReadHeaderChunk(aMov);
        }
        return 0;
    }
    return 0;
}

void movieOpenStream(MovObj *aMov, MovOpenParam *aParam, const char *aName)
{
    SoundFormat fmt;
    MovLinearStream *linear;
    MovPacStream *pac;
    char *ext;
    u8 isPac;

    memset(aMov, 0, sizeof(MovObj));
    aMov->active = 1;
    ext = strrchr(aName, '.');
    isPac = 0;
    if (ext != NULL) {
        isPac = strcasecmp(ext, D_00551B50) == 0;
    }
    aMov->isPac = isPac;
    fmt.unk02 = 0;
    aMov->remainingBytes = 0x7FFFFFFF;
    aMov->unk04 = aParam->unk00;
    aMov->unk08 = aParam->unk04;
    aMov->unk06 = aParam->unk02;
    aMov->unk0C = aParam->unk08;
    aMov->unk0E = aParam->unk0A;
    fmt.unk00 = aParam->unk10;
    fmt.unk01 = aParam->unk11;
    fmt.unk03 = aParam->unk12;
    if (isPac == 0) {
        sdfMemHandle_t *handle;
        u8 *base;

        linear = sdfChipAllocZeroDirect(sizeof(MovLinearStream));
        aMov->stream.linear = linear;
        handle = sdfMemAllocDirect(0x20000);
        linear->memHandle = handle;
        base = sdfMemLock(handle);
        linear->base = base;
        linear->readPtr = base;
        linear->writePtr = base;
        aMov->state = 0;
        aMov->fileHandle = sdfFileCmdReadOpen(aName, (SdfStreamRead)movieGetStreamSize, aMov);
        movieAttachDecodeTask(&aMov->soundNode, &fmt, movieCopyChunk, aMov, aParam->resourceWord);
    } else {
        sdfMemHandle_t *handle;
        u8 *base;
        u8 *ring;
        u8 *body;
        u8 *scr;
        s32 n;

        if (movieStreamSema < 0) {
            movieStreamSema = sdfCreateSema(1, 1, 0);
        }
        movieSemaCount = -1;
        pac = sdfChipAllocZeroDirect(sizeof(MovPacStream));
        aMov->stream.pac = pac;
        handle = sdfMemAllocDirect(0x24000);
        pac->dataHandle = handle;
        base = sdfMemLock(handle);
        n = 0x20;
        ring = base + 0x14000;
        body = base + 0x4000;
        scr = pac->scratch;
        pac->dataBody = body;
        pac->scratchPtr = scr;
        pac->ringBase = ring;
        pac->unk70 = n;
        pac->dataBase = base;
        aMov->state = 0;
        aMov->fileHandle = sdfFileCmdReadOpen(aName, (SdfStreamRead)movieReadChunk, aMov);
        aMov->pacEnabled = 1;
        aMov->packetLimit = 0x7F;
        movieAttachDecodeTask(&aMov->soundNode, &fmt, movieParseHeader, aMov, aParam->resourceWord);
    }
}

void movieCloseStream(MovObj *aMov)
{
    u8 state;
    MovLinearStream *stream;
    MovPacStream *pac;

    if (aMov->active == 0) {
        return;
    }
    state = aMov->state;
    aMov->stopRequested = 1;
    if (state == 5) {
        aMov->state = 7;
        sdfFileCmdClose(aMov->fileHandle);
        state = aMov->state;
    }
    if (state != 6) {
        do {
            sdfDummyFunc();
        } while (aMov->state != 6);
    }
    if (aMov->isPac == 0) {
        stream = aMov->stream.linear;
        sdfMemFreeDirect(stream->memHandle);
        sdfChipFreeDirect(stream);
    } else {
        pac = aMov->stream.pac;
        sdfMemFreeDirect(pac->bitmapHandle);
        sdfMemFreeDirect(pac->dataHandle);
        sdfChipFreeDirect(pac);
    }
    sdfIpuFree(&aMov->soundNode);
    if (aMov->pacEnabled != 0) {
        p2sdSyncMovie();
    }
    aMov->active = 0;
}

s32 sdfMovieIsFinished(MovObj *aMov)
{
    if (aMov->soundNode.drained == 0) {
        return 0;
    }
    return aMov->soundNode.completedBufferCount == 0;
}
INCLUDE_SDATA(const s32, "sdf/sdfMovie", movieStreamSema);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B4C);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B50);

