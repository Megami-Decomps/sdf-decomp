#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

enum {
    SDF_STREAM_READ_QUERY = 0,
    SDF_STREAM_READ_COPY = 1,
    SDF_STREAM_READ_RESUME = 2
};

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

typedef s32 (*SdfStreamRead)(sdfIpuDecodeCb_t *, struct movieStream_t *, s32, void *, s32);

void sdfIpuStrmDecodeInit(sdfIpuDecodeCb_t *aNode, struct SoundFormat *aFormat, SdfStreamRead aRead, struct movieStream_t *aContext);

void sdfIpuSetDecodeParams(sdfIpuDecodeCb_t *aNode, struct SoundFormat *aFormat, SdfStreamRead aRead, struct movieStream_t *aContext);

void sdfIpuAttachTask(sdfIpuDecodeCb_t *aNode);

s32 SleepThread(void);

extern s32 sdfCdTrayMode;

extern s32 sdfFileSemaId;

extern u8 sdfFileDefaultSrcType;

extern u8 sdfFileCmdDefaultDevice;

extern u8 sdfFileCmdThreadPrioCheckCounter;

extern u32 p2sdPcmCallback;

extern u32 p2sdPcmState;

extern u32 p2sdPcmBusy;

extern sdfIpuDecodeCb_t *sdfIpuTaskLink;

#define SDF_IPU_CMD (*(volatile u32 *)0x10002000)

#define SDF_IPU_CTRL (*(volatile u32 *)0x10002010)

#define SDF_FROM_IPU_CHCR (*(volatile u32 *)0x1000B000)

#define SDF_FROM_IPU_MADR (*(volatile u32 *)0x1000B010)

#define SDF_FROM_IPU_QWC (*(volatile u32 *)0x1000B020)

#define SDF_TO_IPU_CHCR (*(volatile u32 *)0x1000B400)

#define SDF_TO_IPU_MADR (*(volatile u32 *)0x1000B410)

#define SDF_TO_IPU_QWC (*(volatile u32 *)0x1000B420)

#define SDF_TO_IPU_TADR (*(volatile u32 *)0x1000B430)

#define SDF_DMAC_ENABLE (*(volatile u32 *)0x1000F520)

typedef struct sdfHostPrefix_t {
    s8 b[7];
} sdfHostPrefix_t;

typedef u8 sdfDevOperationCode_t;

typedef s8 sdfDevStateCode_t;

/* Complete 0x40-byte request owner. sdfFileCmd_t describes only its opaque
 * 0x30-byte prefix for consumers outside the device queue implementation. */
typedef struct sdfFileCmdHandle_t {
    struct sdfFileCmdHandle_t *next;
    struct sdfFileCmdHandle_t *previous;
    struct sdfFileCmdHandle_t *workerNext;
    struct sdfFileCmdHandle_t *workerPrev;
    char *resolvedPath;
    u8 workerIndex;
    sdfDevOperationCode_t operation;
    sdfDevStateCode_t state;
    u8 pad17;
    s32 operationArg;
    s32 requestExtra;
    void *requestData;
    s32 options;
    s32 resourceId;
    s32 result;
    s32 transferred;
    u8 pad34[4];
    void (*callback)(struct sdfFileCmdHandle_t *, s32, s32, s32, s32);
    s32 callbackContext;
} sdfFileCmdHandle_t;

enum SdfFileIOCmdStat {
    SDF_FILE_CMD_STAT_INIT = 0,
    SDF_FILE_CMD_STAT_DORMANT = 1,
    SDF_FILE_CMD_STAT_OPEN = 2,
    SDF_FILE_CMD_STAT_SEEK = 3,
    SDF_FILE_CMD_STAT_READ = 4,
    SDF_FILE_CMD_STAT_WRITE = 5,
    SDF_FILE_CMD_STAT_CLOSE = 6,
    SDF_FILE_CMD_STAT_SUSPEND = 7,
    SDF_FILE_CMD_STAT_DONE = 8,
    SDF_FILE_CMD_STAT_ERROR = 9,
    SDF_FILE_CMD_STAT_CANCEL = 10
};

enum SdfFileIOCmd {
    SDF_FILE_CMD_NONE = 0,
    SDF_FILE_CMD_READOPEN = 1,
    SDF_FILE_CMD_WRITEOPEN = 2,
    SDF_FILE_CMD_SEEK = 3,
    SDF_FILE_CMD_FILESIZE = 4,
    SDF_FILE_CMD_READ = 5,
    SDF_FILE_CMD_WRITE = 6,
    SDF_FILE_CMD_CLOSE = 7,
    SDF_FILE_CMD_READFILE = 8,
    SDF_FILE_CMD_WRITEFILE = 9
};

typedef struct sdfClock_t {
    u8 pad00;
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} sdfClock_t;

enum sdf_tex_resource {
    SDF_TEX_RESOURCE_FREE = 0,
    SDF_TEX_RESOURCE_FRAME = 1,
    SDF_TEX_RESOURCE_TEXTURE = 2,
    SDF_TEX_RESOURCE_CLUT = 3,
    SDF_TEX_RESOURCE_KNOWN_MODE_COUNT = 4
};

struct SdfStreamInputDmaTag {
    u64 control;
    u64 reserved;
};

enum sdf_stream_output {
    SDF_STREAM_OUTPUT_RGBA32 = 0,
    SDF_STREAM_OUTPUT_RGBA16 = 1,
    SDF_STREAM_OUTPUT_RGBA16_DITHERED = 2
};

struct SoundFormat {
    u8 output16Bit;
    u8 dither;
    u8 loopMode;
    u8 playbackCadenceStep;
};

typedef struct sdfStreamFrameHeader_t {
    u8 reserved[8];
    u16 width;
    u16 height;
    u32 cycleLength;
} sdfStreamFrameHeader_t;

enum sdf_stream_playback {
    SDF_STREAM_PLAYBACK_INITIAL = 0,
    SDF_STREAM_PLAYBACK_FIRST_COMPLETION = 1,
    SDF_STREAM_PLAYBACK_CADENCED = 2
};

extern s32 DIntr();

extern void StartThread();

extern void *memcpy(void *, const void *, unsigned int);

extern s8 sdfPrintIopFlag;

extern char D_00551908[];

extern char D_00551928[];

extern sdfHostPrefix_t D_00551930[];

extern u8 D_00551938[];

extern char D_00551940[];

extern void EIntr(void);

extern sdfFileCmdHandle_t *D_00551948;

extern sdfFileCmdHandle_t *sdfFileCmdFreeList;

extern s16 sdfFileCmdFreeCount;

extern s16 sdfFileCmdCount;

extern sdfFileCmdHandle_t *D_00551954;

extern sdfFileCmdHandle_t *D_00551958;

extern char D_00551968[];

extern char D_00551970[];

extern char D_00551978[];

extern u16 sdfFileCmdDefaultCreateMode;

extern s32 sdfFileCmdThreadPrio;

extern u8 sdfDev9Ready;

extern u8 sdfCdClock;

extern sdfClock_t D_005519B8;

extern u32 sdfSndSendCmd_cmd_id;

extern void FlushCache(s32);

extern s32 p2sdSmgHandle;

extern char D_00551A20[];

extern char D_00551A28[];

extern char D_00551A30[];

extern char D_00551A38[];

extern char D_00551A40[];

extern char sdfDumpLineBuf[];

extern char D_00551A50[];

extern char D_00551A58[];

extern char D_00551A60[];

extern char D_00551B10[];

extern char D_00551B18[];

extern char D_0054A2D8[];

extern char D_00551B40[];

extern sdfIpuDecodeCb_t *sdfIpuQueueHead;

extern sdfIpuDecodeCb_t *sdfIpuQueueTail;

void *sdfAllocAuto(s32 size);

extern void * memset(void *, s32, u32);

extern sdfGsMemSlice_t *sdfGsMemAllocRect(s32, s32, s32, s32, s32);

void sdfFreeAuto(void *data);

extern s32 sceIpuSync(s32, s32);

extern sdfIpuDecodeCb_t *sdfCurrIpuDecodeCb;

extern void sdfImageSetChainReleaseMem(s32);

extern void sdfGsMemFreeDelayed(sdfGsMemSlice_t *texture);

extern sdfIpuDecodeCb_t *D_00552158;

extern s32 sdfIpuDecode(sdfIpuDecodeCb_t *);

extern void sceIpuStopDMA(void *);

extern s32 sdfIpuDecodeThId;

extern s32 iWakeupThread(s32 threadId);

extern s32 D_00552144;

u32 sdfTexGetPixelAddr(sdfTexHandle_t *texture);

extern s32 sdfFromIpuDmacHandlerHandle;

extern s32 sdfToIpuDmacHandlerHandle;

extern void sceIpuInit(void);

extern s32 DmacAddHandler(s32, s32, void *, s32, s32);

extern void EnableDmac(s32);

extern s32 CreateThreadSimple(void *entryAddress, void *workspace, s32 stackBytes, s32 priority);

extern u8 sdfIpuDecodeThStack[];

extern void sdfIpuRemoveHandler(void *);

extern s32 DisableDmac(s32);

extern s32 (*p2sdSmgBuf)(void);

void sdfIpuExecQueue(s32 skipInterruptGuard);

void sdfIpuMakeBltPacket(sdfIpuDecodeCb_t *node);

/* Remove a queued node without clearing its old links; the caller may own the interrupt guard. */
void sdfIpuDetachQueue(sdfIpuDecodeCb_t *node, s32 skipInterruptGuard) {
    s32 interruptsEnabled = 0;
    sdfIpuDecodeCb_t *previousNode;
    sdfIpuDecodeCb_t *nextNode;
    if (skipInterruptGuard == 0) {
        interruptsEnabled = DIntr();
    }
    if (node->queued != 0) {
        previousNode = node->streamPrev;
        nextNode = node->streamNext;
        if (previousNode == 0) {
            sdfIpuQueueHead = nextNode;
        } else {
            previousNode->streamNext = nextNode;
        }
        if (nextNode == 0) {
            sdfIpuQueueTail = previousNode;
        } else {
            nextNode->streamPrev = previousNode;
        }
        node->queued = 0;
    }
    if (skipInterruptGuard == 0 && interruptsEnabled != 0) {
        EIntr();
    }
}

/* Move a node to the queue tail, using one interrupt guard for unlink plus append. */
void sdfIpuAttachQueue(sdfIpuDecodeCb_t *node, s32 skipInterruptGuard) {
    s32 interruptsEnabled = 0;
    if (skipInterruptGuard == 0) {
        interruptsEnabled = DIntr();
    }
    if (node->queued != 0) {
        sdfIpuDetachQueue(node, 1);
    }
    node->queued = 1;
    if (sdfIpuQueueTail == 0) {
        sdfIpuQueueHead = node;
    } else {
        sdfIpuQueueTail->streamNext = node;
    }
    node->streamPrev = sdfIpuQueueTail;
    node->streamNext = 0;
    sdfIpuQueueTail = node;
    if (skipInterruptGuard == 0 && interruptsEnabled != 0) {
        EIntr();
    }
}

void sdfIpuAttachTask(sdfIpuDecodeCb_t *aNode)
{
    sdfIpuDecodeCb_t **link = &sdfIpuTaskLink;
    sdfIpuDecodeCb_t *cur;

    for (cur = *link; cur != NULL; cur = *link) {
        link = &cur->next;
    }
    *link = aNode;
    aNode->next = NULL;
}

void sdfIpuDetachTask(sdfIpuDecodeCb_t *aNode)
{
    sdfIpuDecodeCb_t **link = &sdfIpuTaskLink;
    sdfIpuDecodeCb_t *cur;

    for (cur = *link; cur != NULL; cur = *link) {
        if (cur == aNode) {
            *link = cur->next;
            return;
        }
        link = &cur->next;
    }
}

/* Allocate two width-by-height frames, using four bytes per pixel for mode zero, two otherwise. */
void sdfIpuAllocBuff(sdfIpuDecodeCb_t *node) {
    s32 bytesPerPixel = 4;
    s32 size;
    if (node->audioMode != SDF_STREAM_OUTPUT_RGBA32) {
        bytesPerPixel = 2;
    }
    size = node->width * node->height;
    size *= bytesPerPixel;
    node->bufferSize = size;
    node->frameBuffers[0] = sdfAllocAuto(size);
    node->frameBuffers[1] = sdfAllocAuto(size);
}

/* Each 16-byte DMA tag holds control/address bits and a zero reserved half. */
void sdfIpuMakeToIpuDmaTag(sdfIpuDecodeCb_t *node, u8 *source, s32 bytes) {
    s32 remaining = (bytes + 15) & ~15;
    struct SdfStreamInputDmaTag *tag;
    u32 address;
    tag = sdfAllocAuto(((remaining + 0xFFEFF) / 0xFFF00) * 16);
    node->inputDmaChain = tag;
    address = (u32)source & 0x0FFFFFFF;
    do {
        s32 chunk = remaining > 0xFFF00 ? 0xFFF00 : remaining;
        u32 id;
        remaining -= chunk;
        id = remaining != 0 ? 3 : 0;
        tag->control = ((u16)(chunk >> 4) | (id << 28)) | ((u64)address << 32);
        tag->reserved = 0;
        address += chunk;
        tag++;
    } while (remaining > 0);
    FlushCache(0);
}

INCLUDE_ASM(const s32, "sdf/sdfIpu", sdfIpuMakeBltPacket);

/* Clear the decoder, choose 32-bit/16-bit/dithered output and copy playback options. */
void sdfIpuInitCb(sdfIpuDecodeCb_t *node, struct SoundFormat *format) {
    memset(node, 0, 0x8C);
    if (format->output16Bit == 0) {
        node->audioMode = SDF_STREAM_OUTPUT_RGBA32;
    } else if (format->dither == 0) {
        node->audioMode = SDF_STREAM_OUTPUT_RGBA16;
    } else {
        node->audioMode = SDF_STREAM_OUTPUT_RGBA16_DITHERED;
    }
    node->loopMode = format->loopMode;
    node->playbackCadenceStep = format->playbackCadenceStep;
}

/* Decode the frame header, allocate its buffers, then queue the bytes after that header. */
void sdfIpuSetDecodeParams(sdfIpuDecodeCb_t *node, struct SoundFormat *format, SdfStreamRead sourceAddress, struct movieStream_t *sourceContext) {
    s32 sourceSize = (s32)sourceContext;
    s32 interruptsEnabled;
    u8 *frameBytes = (u8 *)sourceAddress;
    sdfIpuInitCb(node, format);
    node->width = ((sdfStreamFrameHeader_t *)frameBytes)->width;
    node->cycleLength = ((sdfStreamFrameHeader_t *)frameBytes)->cycleLength;
    node->height = ((sdfStreamFrameHeader_t *)frameBytes)->height;
    sdfIpuAllocBuff(node);
    sdfIpuMakeToIpuDmaTag(node, frameBytes + 0x10, sourceSize - 0x10);
    interruptsEnabled = DIntr();
    sdfIpuAttachQueue(node, 0);
    if (interruptsEnabled != 0) {
        EIntr();
    }
    sdfIpuExecQueue(0);
}

/* Store the callback and movie context, then allocate the feed ring and leading mirror bytes. */
void sdfIpuStrmDecodeInit(sdfIpuDecodeCb_t *node, struct SoundFormat *format, SdfStreamRead readSource, struct movieStream_t *readContext) {
    sdfIpuInitCb(node, format);
    node->read = readSource;
    node->readContext = readContext;
    node->active = 1;
    node->scratchBuffer = (u8 *)sdfAllocAuto(0x10100) + 0x100;
}

/* Query for a complete header before reading it. Texture dimensions are rounded
 * only when larger than one GS page; smaller dimensions remain unchanged.
 */
void sdfIpuAllocGsRect(sdfIpuDecodeCb_t *node) {
    sdfStreamFrameHeader_t header;
    s32 readStatus;
    s32 interruptsEnabled;
    if (node->headerReady != 0) {
        return;
    }
    if (node->read(node, node->readContext, SDF_STREAM_READ_QUERY, &readStatus, 0) < sizeof(header)) {
        return;
    }
    node->headerReady = 1;
    node->read(node, node->readContext, SDF_STREAM_READ_COPY, &header, sizeof(header));
    node->width = header.width;
    node->height = header.height;
    node->cycleLength = header.cycleLength;
    if (node->resourceWord == 0) {
        s32 textureWidth = node->width;
        s32 textureHeight = node->height;
        s32 pixelFormat;
        sdfGsMemSlice_t *texture;
        if (textureWidth > 64) {
            textureWidth = (textureWidth + 63) & ~63;
        }
        if (node->audioMode == SDF_STREAM_OUTPUT_RGBA32) {
            pixelFormat = 0;
            if (textureHeight > 32) {
                textureHeight = (textureHeight + 31) & ~31;
            }
        } else {
            pixelFormat = 2;
            if (textureHeight > 64) {
                textureHeight = (textureHeight + 63) & ~63;
            }
        }
        texture = sdfGsMemAllocRect(textureWidth, textureHeight, pixelFormat, SDF_TEX_RESOURCE_TEXTURE, 0);
        node->textureHead = texture;
        node->resourceWord = texture->addr;
    }
    sdfIpuAllocBuff(node);
    interruptsEnabled = DIntr();
    sdfIpuAttachQueue(node, 0);
    if (interruptsEnabled != 0) {
        EIntr();
    }
}

void sdfIpuFree(sdfIpuDecodeCb_t *node) {
    s32 interruptsEnabled;
    s32 wasActive;
    u32 dmaEnable;
    interruptsEnabled = DIntr();
    wasActive = 0;
    node->drained = 1;
    sdfIpuDetachTask(node);
    sdfIpuDetachQueue(node, 1);
    if (sdfCurrIpuDecodeCb == node) {
        SDF_IPU_CTRL = 0x40000000;
        sceIpuSync(0, 0);
        dmaEnable = SDF_DMAC_ENABLE;
        SDF_DMAC_ENABLE = 0x10000;
        SDF_TO_IPU_CHCR = 1;
        SDF_FROM_IPU_CHCR = 0;
        SDF_DMAC_ENABLE = dmaEnable;
        sdfCurrIpuDecodeCb = NULL;
        wasActive = 1;
    }
    if (interruptsEnabled != 0) {
        EIntr();
    }
    sdfFreeAuto(node->inputDmaChain);
    sdfFreeAuto(node->frameBuffers[0]);
    sdfFreeAuto(node->frameBuffers[1]);
    sdfImageSetChainReleaseMem((s32)node->transferPacketBuffers[0]);
    sdfImageSetChainReleaseMem((s32)node->transferPacketBuffers[1]);
    if (node->scratchBuffer != 0) {
        sdfFreeAuto(node->scratchBuffer - 0x100);
    }
    if (node->textureHead != 0) {
        sdfGsMemFreeDelayed(node->textureHead);
    }
    if (wasActive != 0) {
        sdfIpuExecQueue(0);
    }
}

/* Fill at most seven of eight 8-KiB slots. A short non-EOF query waits for more
 * data; EOF permits a partial final read. Copy the final slot's trailing bytes
 * into the prefix mirror, retaining the original copy even for a partial read.
 */
void sdfIpuHandleDmaEnd(sdfIpuDecodeCb_t *feed) {
    s32 writeSlot;
    s32 filledSlots;
    s32 availableBytes;
    u32 destinationAddress;
    s32 endOfStream;
    filledSlots = feed->filledSlots;
    if (feed->done == 0 && filledSlots < 7) {
        do {
            writeSlot = feed->firstSlot;
            endOfStream = 0;
            destinationAddress = (u32)feed->scratchBuffer;
            writeSlot += filledSlots;
            if (writeSlot >= 8) {
                writeSlot -= 8;
            }
            destinationAddress += writeSlot << 13;
            destinationAddress = (destinationAddress & 0x0FFFFFFF) | 0x20000000;
            availableBytes = feed->read(feed, feed->readContext, SDF_STREAM_READ_QUERY, &endOfStream, 0);
            if (availableBytes < 0x2000) {
                if (endOfStream == 0) {
                    return;
                }
                if (availableBytes > 0) {
                    feed->read(feed, feed->readContext, SDF_STREAM_READ_COPY, (void *)destinationAddress, availableBytes);
                    filledSlots++;
                }
                feed->done = 1;
            } else {
                if (availableBytes == 0x2000) {
                    if (endOfStream != 0) {
                        feed->done = 1;
                    }
                }
                feed->read(feed, feed->readContext, SDF_STREAM_READ_COPY, (void *)destinationAddress, 0x2000);
                filledSlots++;
            }
            if (writeSlot == 7) {
                memcpy((void *)(((u32)(feed->scratchBuffer - 0x100) & 0x0FFFFFFF) | 0x20000000), (void *)(destinationAddress + 0x1F00), 0x100);
            }
            feed->filledSlots = filledSlots;
            if (feed->done != 0) {
                break;
            }
        } while (filledSlots < 7);
    }
}

/* Start IPU input from the initial chain or a feed slot; defer an empty ring. */
void sdfIpuSetToIpuDma(sdfIpuDecodeCb_t *stream) {
    if (stream->active == 0) {
        D_00552158 = stream;
        SDF_TO_IPU_TADR = (u32)stream->inputDmaChain & 0x0FFFFFFF;
        SDF_TO_IPU_QWC = 0;
        SDF_TO_IPU_CHCR = 0x105;
    } else {
        if (stream->filledSlots == 0) {
            stream->inputDmaStartPending = 1;
            return;
        }
        D_00552158 = stream;
        stream->inputFeedDmaInFlight = 1;
        SDF_TO_IPU_MADR = ((u32)stream->scratchBuffer + (stream->firstSlot << 13)) & 0x0FFFFFFF;
        SDF_TO_IPU_QWC = 0x2000 / 16;
        SDF_TO_IPU_CHCR = 0x101;
    }
    stream->inputDmaStartPending = 0;
}

/* Start DMA from the IPU into the selected buffer, then toggle the byte index. */
void sdfIpuSetFromIpuDma(sdfIpuDecodeCb_t *stream) {
    u8 bufferIndex = stream->bufferIndex;
    SDF_FROM_IPU_MADR = (u32)stream->frameBuffers[bufferIndex] & 0x0FFFFFFF;
    SDF_FROM_IPU_QWC = stream->bufferSize / 16;
    SDF_FROM_IPU_CHCR = 0x100;
    stream->bufferIndex = bufferIndex ^ 1;
}

/* Read the FDEC result, then issue FDEC with an eight-bit advance; return the earlier result. */
s32 IpuWriteCommand(void) {
    s32 decodedBits;
    SDF_IPU_CMD = 0x40000000;
    sceIpuSync(0, 0);
    decodedBits = SDF_IPU_CMD;
    sceIpuSync(0, 0);
    SDF_IPU_CMD = 0x40000008;
    sceIpuSync(0, 0);
    return decodedBits;
}

INCLUDE_ASM(const s32, "sdf/sdfIpu", sdfIpuDecode);

/* Remove and advance the first list item matching the IPU cleanup test. */
void sdfIpuExecQueue(s32 skipInterruptGuard) {
    sdfIpuDecodeCb_t *node;
    s32 interruptsEnabled = 0;
    if (sdfCurrIpuDecodeCb != NULL) {
        return;
    }
    if (skipInterruptGuard == 0) {
        interruptsEnabled = DIntr();
    }
    node = sdfIpuQueueHead;
    while (node != 0) {
        if (node->completedBufferCount + node->pendingPlaybackSubmissions < 2 &&
            (node->active != 1 || node->filledSlots != 0)) {
            sdfIpuDetachQueue(node, 1);
            sdfIpuDecode(node);
            break;
        }
        node = node->streamNext;
    }
    if (skipInterruptGuard == 0 && interruptsEnabled != 0) {
        EIntr();
    }
}

/* Sleep for IPU completion, advance cycle state and requeue non-drained nodes;
 * the DMA worker runs indefinitely and retains the native completion-byte gates. */
void sdfIpuDecodeThread(void) {
    sdfIpuDecodeCb_t *work;
    s32 interruptsEnabled;
    for (;;) {
        SleepThread();
        work = sdfCurrIpuDecodeCb;
        if (work == NULL) {
            continue;
        }
        sceIpuStopDMA(&work->dma);
        if (work->dma.inputQwords == 0) {
            work->inputFeedEnabled = 0;
        }
        if (work->playbackPhase == SDF_STREAM_PLAYBACK_INITIAL) {
            work->playbackPhase = SDF_STREAM_PLAYBACK_FIRST_COMPLETION;
        }
        work->completedBufferCount++;
        sdfCurrIpuDecodeCb = NULL;
        work->tickCount++;
        if (work->tickCount == work->cycleLength) {
            if (work->loopMode != 0) {
                work->ipuCycleInitialized = 0;
                work->tickCount = 0;
                work->unk65 = 0;
            } else {
                work->drained = 1;
            }
        }
        if (work->drained == 0) {
            interruptsEnabled = DIntr();
            sdfIpuAttachQueue(work, 0);
            if (interruptsEnabled != 0) {
                EIntr();
            }
        }
        sdfIpuExecQueue(0);
    }
}

s32 sdfIpuFromDmacHandler(void) {
    iWakeupThread(sdfIpuDecodeThId);
    EE_SYNC_EI();
    return 0;
}

s32 sdfIpuToDmacHandler(void) {
    sdfIpuDecodeCb_t *stream = D_00552158;
    if (stream != NULL) {
        D_00552144 = stream->playbackFrameIndex;
        D_00552158 = NULL;
        if (stream->inputFeedEnabled != 0 && stream->active == 1) {
            if (stream->inputFeedDmaInFlight != 0) {
                stream->inputFeedDmaInFlight = 0;
                stream->firstSlot++;
                if (stream->firstSlot == 8) {
                    stream->firstSlot = 0;
                }
                stream->filledSlots--;
            }
            sdfIpuHandleDmaEnd(stream);
            sdfIpuSetToIpuDma(stream);
        }
    }
    EE_SYNC_EI();
    return 0;
}

void *sdfIpuGetActivePacket(sdfIpuDecodeCb_t *aNode)
{
    void *packet = NULL;

    if (aNode->completedBufferCount != 0) {
        packet = aNode->frameBuffers[aNode->transferPacketIndex];
    }
    return packet;
}

void *sdfIpuGetActiveFrame(sdfIpuDecodeCb_t *aNode)
{
    void *frame = NULL;

    if (aNode->completedBufferCount != 0) {
        frame = aNode->transferPacketBuffers[aNode->transferPacketIndex];
    }
    return frame;
}

/* Consume a pending buffer when present, but always advance processed; loop only on exact equality. */
void sdfIpuFinishActiveFrame(sdfIpuDecodeCb_t *node) {
    s32 interruptsEnabled = DIntr();
    s32 pendingBuffers = node->completedBufferCount;
    s32 remainingBuffers = pendingBuffers - 1;
    if (pendingBuffers > 0) {
        node->completedBufferCount = remainingBuffers;
        node->transferPacketIndex ^= 1;
        node->pendingPlaybackSubmissions++;
    }
    node->playbackFrameIndex++;
    if (node->playbackFrameIndex == node->cycleLength && node->loopMode != 0) {
        node->playbackFrameIndex = 0;
    }
    if (interruptsEnabled != 0) {
        EIntr();
    }
    sdfIpuExecQueue(0);
}

INCLUDE_ASM(const s32, "sdf/sdfIpu", sdfIpuSetChainTag);

INCLUDE_ASM(const s32, "sdf/sdfIpu", sdfIpuRunDecode);

void sdfIpuStartDecodeTask(sdfIpuDecodeCb_t *aNode, struct SoundFormat *aFormat, SdfStreamRead aRead, struct movieStream_t *aContext, s32 aResourceWord)
{
    sdfIpuSetDecodeParams(aNode, aFormat, aRead, aContext);
    aNode->resourceWord = aResourceWord;
    sdfIpuAttachTask(aNode);
}

void sdfIpuSetOutputTex(sdfIpuDecodeCb_t *node, struct SoundFormat *params, u8 *sourceData, s32 sourceSize, sdfTexHandle_t *source) {
    struct SoundFormat streamParams = *params;
    switch (source->pixelFormat) {
    case 0:
        streamParams.output16Bit = 0;
        break;
    case 2:
        streamParams.output16Bit = 1;
        break;
    }
    sdfIpuStartDecodeTask(node, &streamParams, (SdfStreamRead)sourceData, (struct movieStream_t *)sourceSize, sdfTexGetPixelAddr(source));
}

void movieAttachDecodeTask(sdfIpuDecodeCb_t *aNode, struct SoundFormat *aFormat, SdfStreamRead aRead, struct movieStream_t *aContext, s32 aResourceWord)
{
    sdfIpuStrmDecodeInit(aNode, aFormat, aRead, aContext);
    aNode->resourceWord = aResourceWord;
    sdfIpuAttachTask(aNode);
}

void sdfIpuInit(void) {
    s32 thread;
    sceIpuInit();
    SDF_IPU_CMD = 0x90000000;
    sdfIpuTaskLink = 0;
    sdfCurrIpuDecodeCb = NULL;
    sdfFromIpuDmacHandlerHandle = DmacAddHandler(1, 3, sdfIpuFromDmacHandler, -1, 0);
    EnableDmac(3);
    sdfToIpuDmacHandlerHandle = DmacAddHandler(1, 4, sdfIpuToDmacHandler, -1, 0);
    EnableDmac(4);
    thread = CreateThreadSimple((void *)sdfIpuDecodeThread, sdfIpuDecodeThStack, 0x800, 0x46);
    sdfIpuDecodeThId = thread;
    StartThread(thread, 0);
}

s32 sdfIpuClose(void) {
    if (sdfFromIpuDmacHandlerHandle != 0) {
        sdfIpuRemoveHandler((void *)sdfFromIpuDmacHandlerHandle);
        sdfFromIpuDmacHandlerHandle = 0;
    }
    DisableDmac(3);
    if (sdfToIpuDmacHandlerHandle != 0) {
        sdfIpuRemoveHandler((void *)sdfToIpuDmacHandlerHandle);
        sdfToIpuDmacHandlerHandle = 0;
    }
    return DisableDmac(4);
}

