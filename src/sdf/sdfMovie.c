#include "common.h"
#include "libsdf.h"

struct movieStream_t;
struct SoundFormat;
struct sdfGsMemSlice_t;
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

typedef struct sdfIpuDecodeCb_t
{
    struct sdfIpuDecodeCb_t *streamPrev;
    struct sdfIpuDecodeCb_t *streamNext;
    struct sdfIpuDecodeCb_t *next;
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
    struct sdfGsMemSlice_t *textureHead;
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
    s32 (*read)(struct sdfIpuDecodeCb_t *, struct movieStream_t *, s32, void *, s32);
    struct movieStream_t *readContext;
    u8 inputFeedDmaInFlight;
    u8 unk65;
    u8 pad66[2];
    IpuDmaState dma;
} sdfIpuDecodeCb_t;

typedef struct SoundFormat
{
    u8 output16Bit;
    u8 dither;
    u8 loopMode;
    u8 playbackCadenceStep;
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

typedef s32 (*SdfStreamRead)(sdfIpuDecodeCb_t *, struct movieStream_t *, s32, void *, s32);

/* Ring buffer of raw file data for a plain (non PAC) movie stream. */
typedef struct MovLinearStream
{
    sdfMemHandle_t *allocation;
    u8 *bufferStart;
    u8 *readCursor;
    u8 *writeCursor;
    s32 bufferedBytes;
} MovLinearStream;

typedef struct MovPacStream
{
    u8 pad00[0x18];
    s32 packetBytes;
    s32 blockBytes;
    u8 scratchBuffer[0x20];
    sdfMemHandle_t *payloadAllocation;
    u8 *blockMask;
    s32 blockIndex;
    sdfMemHandle_t *allocation;
    u8 *pacInputBuffer;
    u8 *pacBuffer;
    s32 pacReadOffset;
    s32 pacBufferedBytes;
    u8 *ringBuffer;
    s32 ringOffset;
    s32 ringLength;
    s32 unk6C;
    s32 scratchSize;
    u8 *scratch;
} MovPacStream;

typedef union MovObjStream
{
    MovLinearStream *linear;
    MovPacStream *pac;
} MovObjStream;

typedef struct movieStream_t
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
    void *file;
    s32 totalBytes;
    s32 remainingBytes;
    MovObjStream stream;
    u8 pacEnabled;
    u8 pad21;
    u8 packetLimit;
    u8 pad23;
    sdfIpuDecodeCb_t soundNode;
} movieStream_t;

void puts(const char *aStr);
s32 sdfFileCmdRead(void *aHandle, void *aBuffer, s32 aSize);
s32 sdfFileCmdClose(void *aHandle);
void sdfDummyFunc(void);
void sdfMemFreeDirect(sdfMemHandle_t *aHandle);
void sdfChipFreeDirect(void *aPtr);
void sdfIpuFree(sdfIpuDecodeCb_t *aNode);
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
void *sdfFileCmdReadOpen(const char *aName, SdfStreamRead aRead, movieStream_t *aContext);
void movieAttachDecodeTask(sdfIpuDecodeCb_t *aNode, SoundFormat *aFormat, SdfStreamRead aRead, movieStream_t *aContext, s32 aResourceWord);
s32 sdfCreateSema(s32 aInit, s32 aMax, s32 aAttr);
s32 WaitSema(s32 sema);
s32 SignalSema(s32 sema);
s32 movieReadChunk(void *aHandle, s32 aOp, void *aBuf, s32 aSize, movieStream_t *aMov);
s32 movieCopyChunk(sdfIpuDecodeCb_t *aNode, movieStream_t *aMov, s32 aOp, void *aBuf, s32 aSize);
s32 movieParseHeader(sdfIpuDecodeCb_t *aNode, movieStream_t *aMov, s32 aOp, void *aBuf, s32 aSize);
extern s32 movieStreamSema;
extern s32 movieSemaCount;
extern const char D_00551B50[];

void movie_2C5080(const char *aStr)
{
    puts(aStr);
}

INCLUDE_ASM(const s32, "sdf/sdfMovie", movieWaitSema);

INCLUDE_ASM(const s32, "sdf/sdfMovie", release_movieobj2);

void movieReadNextChunk(movieStream_t *aMov)
{
    s32 remaining = aMov->remainingBytes;
    MovLinearStream *stream = aMov->stream.linear;
    s32 size;
    u8 *dst;
    u8 *next;

    if (remaining != 0) {
        if (0x20000 - stream->bufferedBytes < 0x4000) {
            aMov->state = 5;
        } else {
            aMov->state = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            dst = stream->writeCursor;
            next = dst + size;
            if (next >= stream->bufferStart + 0x20000) {
                next = stream->bufferStart;
            }
            stream->writeCursor = next;
            sdfFileCmdRead(aMov->file, dst, size);
        }
    }
}

s32 movieGetStreamSize(void *aHandle, s32 aOp, void *aBuf, s32 aSize, movieStream_t *aMov)
{
    MovLinearStream *stream = aMov->stream.linear;
    s32 enabled;

    aMov->file = aHandle;
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
            stream->bufferedBytes += aSize;
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
            aMov->file = NULL;
            aMov->state = 6;
            sdfFileCmdFree(aHandle);
        }
        break;
    }
    return 0;
}

void movieReadHeaderChunk(movieStream_t *aMov)
{
    s32 remaining = aMov->remainingBytes;
    MovPacStream *stream = aMov->stream.pac;
    s32 size;

    if (remaining != 0) {
        if (0x10000 - stream->pacBufferedBytes < 0x4000) {
            aMov->state = 5;
        } else if (0x10000 + (stream->unk6C - stream->ringLength) < 0x4000) {
            aMov->state = 5;
        } else {
            aMov->state = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            sdfFileCmdRead(aMov->file, stream->pacInputBuffer, size);
        }
    }
}

s32 movieReadChunk(void *file, s32 operation, void *readBuffer, s32 bytesRead, movieStream_t *movie) {
    MovPacStream *stream;

    movie->file = file;
    stream = movie->stream.pac;

    if (movie->stopRequested != 0 &&
        (movie->state < 6 ||
         movie->state > 7)) {
        movie->state = 7;
        sdfFileCmdClose(file);
        return 0;
    }

    switch (movie->state) {
    case 0:
        if (operation == 2) {
            movie->state = 1;
            sdfFileCmdFileSize(file);
        }
        break;
    case 1:
        if (operation == 4) {
            movie->totalBytes = bytesRead;
            movie->remainingBytes = bytesRead;
            movie->state = 2;
            sdfFileCmdRead(file, stream, 0x40);
        }
        break;
    case 2:
        if (operation == 5) {
            s32 payloadBytes = stream->packetBytes - 0x40;
            stream->payloadAllocation = sdfMemAllocDirect(payloadBytes);
            stream->blockMask = (u8 *)sdfMemLock(stream->payloadAllocation);
            movie->state = 3;
            sdfFileCmdRead(file, stream->blockMask, payloadBytes);
            movie->remainingBytes -= stream->packetBytes;
        }
        break;
    case 3:
        if (operation == 5) {
            movieReadHeaderChunk(movie);
        }
        break;
    case 4: {
        if (operation == 5) {
            u8 *source = readBuffer;
            s32 unitSize = stream->blockBytes;
            s32 blocksRemaining = bytesRead / unitSize;
            do {
                s32 blockIndex = stream->blockIndex;
                s32 byteIndex = blockIndex / 8;
                s32 bitIndex = blockIndex - byteIndex * 8;
                s32 blockFlag = 1 << bitIndex;
                s32 writeOffset;
                if ((stream->blockMask[byteIndex] & blockFlag) != 0) {
                    s32 ringFill;
                    s32 wrappedBytes;

                    WaitSema(movieStreamSema);
                    ringFill = stream->ringLength;
                    writeOffset = stream->ringOffset + ringFill;
                    writeOffset += (writeOffset > 0xFFFF) * -0x10000;
                    wrappedBytes = writeOffset + unitSize - 0x10000;
                    if (wrappedBytes > 0) {
                        memcpy(stream->ringBuffer + writeOffset, source, unitSize - wrappedBytes);
                        memcpy(stream->ringBuffer, source + unitSize - wrappedBytes, wrappedBytes);
                    } else {
                        memcpy(stream->ringBuffer + writeOffset, source, unitSize);
                    }
                    wrappedBytes = ringFill + unitSize;
                    if (wrappedBytes > 0x10000) {
                        stream->ringOffset += wrappedBytes - 0x10000;
                        if (stream->ringOffset > 0xFFFF) {
                            stream->ringOffset -= 0x10000;
                        }
                        stream->ringLength = 0x10000;
                    } else {
                        stream->ringLength = wrappedBytes;
                    }
                    SignalSema(movieStreamSema);
                } else {
                    s32 restoreInterrupts = DIntr();

                    writeOffset = stream->pacReadOffset + stream->pacBufferedBytes;
                    writeOffset += (writeOffset > 0xFFFF) * -0x10000;
                    memcpy(stream->pacBuffer + writeOffset, source, unitSize);
                    stream->pacBufferedBytes += unitSize;
                    if (restoreInterrupts != 0) {
                        EIntr();
                    }
                }
                source += unitSize;
                stream->blockIndex++;
            } while (--blocksRemaining != 0);

            movie->remainingBytes -= bytesRead;
            if (movie->remainingBytes == 0) {
                movie->state = 7;
                sdfFileCmdClose(file);
            } else {
                movieReadHeaderChunk(movie);
            }
        }
        break;
    }
    case 7:
        if (operation == 7) {
            movie->file = NULL;
            movie->state = 6;
            sdfFileCmdFree(file);
        }
        break;
    }

    return 0;
}

s32 movieCopyChunk(sdfIpuDecodeCb_t *aNode, movieStream_t *aMov, s32 aOp, void *aBuf, s32 aSize)
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
        return stream->bufferedBytes;
    case 1:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        src = stream->readCursor;
        base = stream->bufferStart;
        over = (src - base) + aSize - 0x20000;
        if (over >= 0) {
            first = aSize - over;
            memcpy(aBuf, src, first);
            if (over > 0) {
                memcpy((u8 *)aBuf + first, base, over);
            }
            stream->readCursor = base + over;
        } else {
            memcpy(aBuf, src, aSize);
            stream->readCursor = src + aSize;
        }
        stream->bufferedBytes -= aSize;
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

s32 movieParseHeader(sdfIpuDecodeCb_t *aNode, movieStream_t *aMov, s32 aOp, void *aBuf, s32 aSize)
{
    /* Keep the retail helper symbol while retaining GCC's nested static chain. */
    auto void movieMemcpy(void *, const void *, s32) __asm__("movieMemcpy");
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
        return stream->pacBufferedBytes;
    case 1:
        if (aMov->stopRequested != 0) {
            return 0;
        }
        off = stream->pacReadOffset;
        body = stream->pacBuffer;
        over = off + aSize - 0x10000;
        if (over >= 0) {
            first = aSize - over;
            movieMemcpy(aBuf, body + off, first);
            if (over > 0) {
                movieMemcpy((u8 *)aBuf + first, body, over);
            }
            stream->pacReadOffset = over;
        } else {
            movieMemcpy(aBuf, body + off, aSize);
            stream->pacReadOffset = off + aSize;
        }
        stream->pacBufferedBytes -= aSize;
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

void movieOpenStream(movieStream_t *aMov, MovOpenParam *aParam, const char *aName)
{
    SoundFormat fmt;
    MovLinearStream *linear;
    MovPacStream *pac;
    char *ext;
    u8 isPac;

    memset(aMov, 0, sizeof(movieStream_t));
    aMov->active = 1;
    ext = strrchr(aName, '.');
    isPac = 0;
    if (ext != NULL) {
        isPac = strcasecmp(ext, D_00551B50) == 0;
    }
    aMov->isPac = isPac;
    fmt.loopMode = 0;
    aMov->remainingBytes = 0x7FFFFFFF;
    aMov->unk04 = aParam->unk00;
    aMov->unk08 = aParam->unk04;
    aMov->unk06 = aParam->unk02;
    aMov->unk0C = aParam->unk08;
    aMov->unk0E = aParam->unk0A;
    fmt.output16Bit = aParam->unk10;
    fmt.dither = aParam->unk11;
    fmt.playbackCadenceStep = aParam->unk12;
    if (isPac == 0) {
        sdfMemHandle_t *handle;
        u8 *base;

        linear = sdfChipAllocZeroDirect(sizeof(MovLinearStream));
        aMov->stream.linear = linear;
        handle = sdfMemAllocDirect(0x20000);
        linear->allocation = handle;
        base = sdfMemLock(handle);
        linear->bufferStart = base;
        linear->readCursor = base;
        linear->writeCursor = base;
        aMov->state = 0;
        aMov->file = sdfFileCmdReadOpen(aName, (SdfStreamRead)movieGetStreamSize, aMov);
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
        pac->allocation = handle;
        base = sdfMemLock(handle);
        n = 0x20;
        ring = base + 0x14000;
        body = base + 0x4000;
        scr = pac->scratchBuffer;
        pac->pacBuffer = body;
        pac->scratch = scr;
        pac->ringBuffer = ring;
        pac->scratchSize = n;
        pac->pacInputBuffer = base;
        aMov->state = 0;
        aMov->file = sdfFileCmdReadOpen(aName, (SdfStreamRead)movieReadChunk, aMov);
        aMov->pacEnabled = 1;
        aMov->packetLimit = 0x7F;
        movieAttachDecodeTask(&aMov->soundNode, &fmt, movieParseHeader, aMov, aParam->resourceWord);
    }
}

void movieCloseStream(movieStream_t *aMov)
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
        sdfFileCmdClose(aMov->file);
        state = aMov->state;
    }
    if (state != 6) {
        do {
            sdfDummyFunc();
        } while (aMov->state != 6);
    }
    if (aMov->isPac == 0) {
        stream = aMov->stream.linear;
        sdfMemFreeDirect(stream->allocation);
        sdfChipFreeDirect(stream);
    } else {
        pac = aMov->stream.pac;
        sdfMemFreeDirect(pac->payloadAllocation);
        sdfMemFreeDirect(pac->allocation);
        sdfChipFreeDirect(pac);
    }
    sdfIpuFree(&aMov->soundNode);
    if (aMov->pacEnabled != 0) {
        p2sdSyncMovie();
    }
    aMov->active = 0;
}

s32 sdfMovieIsFinished(movieStream_t *aMov)
{
    if (aMov->soundNode.drained == 0) {
        return 0;
    }
    return aMov->soundNode.completedBufferCount == 0;
}
INCLUDE_SDATA(const s32, "sdf/sdfMovie", movieStreamSema);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B4C);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B50);

