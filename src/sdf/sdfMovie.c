#include "common.h"
#include "libsdf.h"

struct sdfMovieCb_t;
struct sdfIpuInitParm_t;
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
    struct sdfIpuDecodeCb_t *prev;
    struct sdfIpuDecodeCb_t *streamNext;
    struct sdfIpuDecodeCb_t *next;
    u8 active;
    u8 queueLinked;
    u8 ipuCycleInitialized;
    u8 drained;
    u8 playbackPhase;
    u8 inputFeedEnabled;
    u8 inputDmaStartPending;
    u8 imageTrans;
    u8 rgbMode;
    u8 loopMode;
    u8 fps;
    u8 playbackCadenceRemainder;
    u8 bufferIndex;
    u8 getBuffId;
    u8 activeBuffCount;
    u8 pad1B;
    s32 buffSize;
    void *frameBuffers[2];
    void *transferPacketBuffers[2];
    u32 transferPacketBytes;
    s32 resourceWord;
    struct sdfGsMemSlice_t *textureHead;
    u16 width;
    u16 height;
    u32 frameCount;
    u32 tickCount;
    s32 frame;
    struct SdfStreamInputDmaTag *inputDmaChain;
    u8 headerReady;
    u8 done;
    u8 filledSlots;
    u8 firstSlot;
    u8 *scratchBuffer;
    u8 pad58[4];
    s32 (*read)(struct sdfIpuDecodeCb_t *, struct sdfMovieCb_t *, s32, void *, s32);
    struct sdfMovieCb_t *strmCallbackParm;
    u8 inputFeedDmaInFlight;
    u8 unk65;
    u8 pad66[2];
    IpuDmaState dma;
} sdfIpuDecodeCb_t;

typedef struct sdfIpuInitParm_t
{
    u8 rgbMode;
    u8 ditherMode;
    u8 loopMode;
    u8 fps;
} sdfIpuInitParm_t;

typedef struct sdfMovieInitParm_t
{
    u16 x;
    u16 y;
    u32 z;
    u16 width;
    u16 height;
    s32 resourceWord;
    u8 rgbMode;
    u8 ditherMode;
    u8 fps;
} sdfMovieInitParm_t;

typedef s32 (*SdfStreamRead)(sdfIpuDecodeCb_t *, struct sdfMovieCb_t *, s32, void *, s32);

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

typedef union movieDecodeCtx_t
{
    MovLinearStream *linear;
    MovPacStream *pac;
} movieDecodeCtx_t;

typedef struct sdfMovieCb_t
{
    u8 stat;
    u8 readStat;
    u8 cancel;
    u8 mode;
    u16 x;
    u16 y;
    u32 z;
    u16 width;
    u16 height;
    void *fileHandle;
    s32 fileSize;
    s32 readRemain;
    movieDecodeCtx_t readCb;
    u8 sndActive;
    u8 sndPlaying;
    u8 sndVolume;
    u8 reserve;
    sdfIpuDecodeCb_t decodeCb;
} sdfMovieCb_t;

void puts(const char *aStr);
s32 sdfFileCmdRead(void *aHandle, void *aBuffer, s32 aLength);
s32 sdfFileCmdClose(void *aHandle);
void sdfDummyFunc(void);
void sdfMemFreeDirect(sdfMemHandle_t *aHandle);
void sdfChipFreeDirect(void *aPtr);
void sdfIpuFree(sdfIpuDecodeCb_t *cb);
s32 p2sdSyncMovie(void);
void *memcpy(void *a, const void *b, u32 c);
s32 sdfFileCmdFileSize(void *aHandle);
s32 sdfFileCmdFree(void *aHandle);
s32 DIntr(void);
s32 EIntr(void);
void *memset(void *a, s32 b, u32 c);
char *strrchr(const char *aStr, s32 aChar);
s32 strcasecmp(const char *aLeft, const char *aRight);
void *sdfChipAllocZeroDirect(s32 aSize);
sdfMemHandle_t *sdfMemAllocDirect(s32 aSize);
void *sdfMemLock(sdfMemHandle_t *aHandle);
void *sdfFileCmdReadOpen(const char *aFileName, SdfStreamRead aCallback, sdfMovieCb_t *aUserData);
void movieAttachDecodeTask(sdfIpuDecodeCb_t *aNode, sdfIpuInitParm_t *aFormat, SdfStreamRead aRead, sdfMovieCb_t *aContext, s32 aResourceWord);
s32 sdfCreateSema(s32 initCount, s32 maxCount, s32 option);
s32 WaitSema(s32 sema);
s32 SignalSema(s32 sema);
s32 movieReadChunk(void *aHandle, s32 aOp, void *aBuf, s32 aSize, sdfMovieCb_t *stream);
s32 movieCopyChunk(sdfIpuDecodeCb_t *aNode, sdfMovieCb_t *stream, s32 aOp, void *aBuf, s32 aSize);
s32 movieParseHeader(sdfIpuDecodeCb_t *aNode, sdfMovieCb_t *stream, s32 aOp, void *aBuf, s32 aSize);
extern s32 movieStreamSema;
extern u32 movieSemaCount;
extern sdfMovieCb_t *movieSemaId;
extern const char D_00551B50[];

void movie_2C5080(const char *aStr)
{
    puts(aStr);
}

/* NON_MATCHING: 39 of 53 words differ (first at +0xC). */
#ifdef NON_MATCHING
u8 *movieWaitSema(s32 aSize)
{
    sdfMovieCb_t *mov = movieSemaId;
    MovPacStream *obj = mov->readCb.pac;
    u8 *ptr = NULL;

    WaitSema(movieStreamSema);
    if (obj->scratchSize > 0) {
        ptr = obj->scratch;
        obj->scratchSize -= aSize;
        obj->scratch = ptr + aSize;
    } else if (obj->ringLength < aSize) {
        if (movieSemaCount != mov->decodeCb.frame) {
            movieSemaCount = mov->decodeCb.frame;
        }
    } else {
        ptr = obj->ringBuffer + obj->ringOffset;
        obj->ringLength -= aSize;
        obj->ringOffset += aSize;
        if (obj->ringOffset > 0xFFFF) {
            obj->ringOffset -= 0x10000;
        }
    }
    SignalSema(movieStreamSema);
    return ptr;
}
#else
INCLUDE_ASM(const s32, "sdf/sdfMovie", movieWaitSema);
#endif

INCLUDE_ASM(const s32, "sdf/sdfMovie", release_movieobj2);

void movieReadNextChunk(sdfMovieCb_t *aMov)
{
    s32 remaining = aMov->readRemain;
    MovLinearStream *stream = aMov->readCb.linear;
    s32 size;
    u8 *dst;
    u8 *next;

    if (remaining != 0) {
        if (0x20000 - stream->bufferedBytes < 0x4000) {
            aMov->readStat = 5;
        } else {
            aMov->readStat = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            dst = stream->writeCursor;
            next = dst + size;
            if (next >= stream->bufferStart + 0x20000) {
                next = stream->bufferStart;
            }
            stream->writeCursor = next;
            sdfFileCmdRead(aMov->fileHandle, dst, size);
        }
    }
}

s32 movieGetStreamSize(void *aHandle, s32 aOp, void *aBuf, s32 aSize, sdfMovieCb_t *aMov)
{
    MovLinearStream *stream = aMov->readCb.linear;
    s32 enabled;

    aMov->fileHandle = aHandle;
    if (aMov->cancel != 0) {
        if (aMov->readStat != 6 && aMov->readStat != 7) {
            aMov->readStat = 7;
            sdfFileCmdClose(aHandle);
            return 0;
        }
    }
    switch (aMov->readStat) {
    case 0:
        if (aOp == 2) {
            aMov->readStat = 1;
            sdfFileCmdFileSize(aHandle);
        }
        break;
    case 1:
        if (aOp == 4) {
            aMov->fileSize = aSize;
            aMov->readRemain = aSize;
            movieReadNextChunk(aMov);
        }
        break;
    case 4:
        if (aOp == 5) {
            enabled = DIntr();
            stream->bufferedBytes += aSize;
            aMov->readRemain -= aSize;
            if (enabled != 0) {
                EIntr();
            }
            if (aMov->readRemain == 0) {
                aMov->readStat = 7;
                sdfFileCmdClose(aHandle);
            } else {
                movieReadNextChunk(aMov);
            }
        }
        break;
    case 7:
        if (aOp == 7) {
            aMov->fileHandle = NULL;
            aMov->readStat = 6;
            sdfFileCmdFree(aHandle);
        }
        break;
    }
    return 0;
}

void movieReadHeaderChunk(sdfMovieCb_t *aMov)
{
    s32 remaining = aMov->readRemain;
    MovPacStream *stream = aMov->readCb.pac;
    s32 size;

    if (remaining != 0) {
        if (0x10000 - stream->pacBufferedBytes < 0x4000) {
            aMov->readStat = 5;
        } else if (0x10000 + (stream->unk6C - stream->ringLength) < 0x4000) {
            aMov->readStat = 5;
        } else {
            aMov->readStat = 4;
            size = remaining < 0x4001 ? remaining : 0x4000;
            sdfFileCmdRead(aMov->fileHandle, stream->pacInputBuffer, size);
        }
    }
}

s32 movieReadChunk(void *file, s32 operation, void *readBuffer, s32 bytesRead, sdfMovieCb_t *movie) {
    MovPacStream *stream;

    movie->fileHandle = file;
    stream = movie->readCb.pac;

    if (movie->cancel != 0 &&
        (movie->readStat < 6 ||
         movie->readStat > 7)) {
        movie->readStat = 7;
        sdfFileCmdClose(file);
        return 0;
    }

    switch (movie->readStat) {
    case 0:
        if (operation == 2) {
            movie->readStat = 1;
            sdfFileCmdFileSize(file);
        }
        break;
    case 1:
        if (operation == 4) {
            movie->fileSize = bytesRead;
            movie->readRemain = bytesRead;
            movie->readStat = 2;
            sdfFileCmdRead(file, stream, 0x40);
        }
        break;
    case 2:
        if (operation == 5) {
            s32 payloadBytes = stream->packetBytes - 0x40;
            stream->payloadAllocation = sdfMemAllocDirect(payloadBytes);
            stream->blockMask = (u8 *)sdfMemLock(stream->payloadAllocation);
            movie->readStat = 3;
            sdfFileCmdRead(file, stream->blockMask, payloadBytes);
            movie->readRemain -= stream->packetBytes;
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

            movie->readRemain -= bytesRead;
            if (movie->readRemain == 0) {
                movie->readStat = 7;
                sdfFileCmdClose(file);
            } else {
                movieReadHeaderChunk(movie);
            }
        }
        break;
    }
    case 7:
        if (operation == 7) {
            movie->fileHandle = NULL;
            movie->readStat = 6;
            sdfFileCmdFree(file);
        }
        break;
    }

    return 0;
}

s32 movieCopyChunk(sdfIpuDecodeCb_t *aNode, sdfMovieCb_t *aMov, s32 aOp, void *aBuf, s32 aSize)
{
    MovLinearStream *stream = aMov->readCb.linear;
    u8 *src;
    u8 *base;
    s32 over;
    s32 first;

    switch (aOp) {
    case 0:
        if (aMov->cancel != 0) {
            *(u8 *)aBuf = 1;
            return 0;
        }
        if (aMov->readRemain == 0) {
            *(u8 *)aBuf = 1;
        }
        return stream->bufferedBytes;
    case 1:
        if (aMov->cancel != 0) {
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
        return aMov->readRemain;
    case 2:
        if (aMov->cancel != 0) {
            return 0;
        }
        if (aMov->readStat == 5) {
            movieReadNextChunk(aMov);
        }
        return 0;
    }
    return 0;
}

s32 movieParseHeader(sdfIpuDecodeCb_t *aNode, sdfMovieCb_t *aMov, s32 aOp, void *aBuf, s32 aSize)
{
    /* Keep the retail helper symbol while retaining GCC's nested static chain. */
    auto void movieMemcpy(void *, const void *, s32) __asm__("movieMemcpy");
    void movieMemcpy(void *aDst, const void *aSrc, s32 aCount)
    {
        memcpy(aDst, aSrc, aCount);
    }
    MovPacStream *stream = aMov->readCb.pac;
    u8 *body;
    s32 off;
    s32 over;
    s32 first;

    switch (aOp) {
    case 0:
        if (aMov->cancel != 0) {
            *(u8 *)aBuf = 1;
            return 0;
        }
        if (aMov->readRemain == 0) {
            *(u8 *)aBuf = 1;
        }
        return stream->pacBufferedBytes;
    case 1:
        if (aMov->cancel != 0) {
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
        return aMov->readRemain;
    case 2:
        if (aMov->cancel != 0) {
            return 0;
        }
        if (aMov->readStat == 5) {
            movieReadHeaderChunk(aMov);
        }
        return 0;
    }
    return 0;
}

void movieOpenStream(sdfMovieCb_t *stream, sdfMovieInitParm_t *desc, const char *path)
{
    sdfIpuInitParm_t fmt;
    MovLinearStream *linear;
    MovPacStream *pac;
    char *ext;
    u8 isPac;

    memset(stream, 0, sizeof(sdfMovieCb_t));
    stream->stat = 1;
    ext = strrchr(path, '.');
    isPac = 0;
    if (ext != NULL) {
        isPac = strcasecmp(ext, D_00551B50) == 0;
    }
    stream->mode = isPac;
    fmt.loopMode = 0;
    stream->readRemain = 0x7FFFFFFF;
    stream->x = desc->x;
    stream->z = desc->z;
    stream->y = desc->y;
    stream->width = desc->width;
    stream->height = desc->height;
    fmt.rgbMode = desc->rgbMode;
    fmt.ditherMode = desc->ditherMode;
    fmt.fps = desc->fps;
    if (isPac == 0) {
        sdfMemHandle_t *handle;
        u8 *base;

        linear = sdfChipAllocZeroDirect(sizeof(MovLinearStream));
        stream->readCb.linear = linear;
        handle = sdfMemAllocDirect(0x20000);
        linear->allocation = handle;
        base = sdfMemLock(handle);
        linear->bufferStart = base;
        linear->readCursor = base;
        linear->writeCursor = base;
        stream->readStat = 0;
        stream->fileHandle = sdfFileCmdReadOpen(path, (SdfStreamRead)movieGetStreamSize, stream);
        movieAttachDecodeTask(&stream->decodeCb, &fmt, movieCopyChunk, stream, desc->resourceWord);
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
        stream->readCb.pac = pac;
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
        stream->readStat = 0;
        stream->fileHandle = sdfFileCmdReadOpen(path, (SdfStreamRead)movieReadChunk, stream);
        stream->sndActive = 1;
        stream->sndVolume = 0x7F;
        movieAttachDecodeTask(&stream->decodeCb, &fmt, movieParseHeader, stream, desc->resourceWord);
    }
}

void movieCloseStream(sdfMovieCb_t *aMov)
{
    u8 state;
    MovLinearStream *stream;
    MovPacStream *pac;

    if (aMov->stat == 0) {
        return;
    }
    state = aMov->readStat;
    aMov->cancel = 1;
    if (state == 5) {
        aMov->readStat = 7;
        sdfFileCmdClose(aMov->fileHandle);
        state = aMov->readStat;
    }
    if (state != 6) {
        do {
            sdfDummyFunc();
        } while (aMov->readStat != 6);
    }
    if (aMov->mode == 0) {
        stream = aMov->readCb.linear;
        sdfMemFreeDirect(stream->allocation);
        sdfChipFreeDirect(stream);
    } else {
        pac = aMov->readCb.pac;
        sdfMemFreeDirect(pac->payloadAllocation);
        sdfMemFreeDirect(pac->allocation);
        sdfChipFreeDirect(pac);
    }
    sdfIpuFree(&aMov->decodeCb);
    if (aMov->sndActive != 0) {
        p2sdSyncMovie();
    }
    aMov->stat = 0;
}

s32 sdfMovieIsFinished(sdfMovieCb_t *aMov)
{
    if (aMov->decodeCb.drained == 0) {
        return 0;
    }
    return aMov->decodeCb.activeBuffCount == 0;
}
INCLUDE_SDATA(const s32, "sdf/sdfMovie", movieStreamSema);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B4C);

INCLUDE_SDATA(const s32, "sdf/sdfMovie", D_00551B50);

