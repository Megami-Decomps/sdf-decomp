#include "common.h"
#include "sdf_types.h"
#include "sdf_resource.h"

/* Register selectors carried by GIF A+D writes in the two GS contexts. */
enum {
    SDF_GS_PRIM = 0x00,
    SDF_GS_RGBAQ = 0x01,
    SDF_GS_XYZ2 = 0x05,
    SDF_GS_TEX0_1 = 0x06,
    SDF_GS_TEX0_2 = 0x07,
    SDF_GS_CLAMP_1 = 0x08,
    SDF_GS_CLAMP_2 = 0x09,
    SDF_GIF_REGISTER_AD = 0x0E,
    SDF_GS_TEX1_1 = 0x14,
    SDF_GS_TEX1_2 = 0x15,
    SDF_GS_XYOFFSET_1 = 0x18,
    SDF_GS_XYOFFSET_2 = 0x19,
    SDF_GS_PRMODECONT = 0x1A,
    SDF_GS_TEXA = 0x3B,
    SDF_GS_TEXFLUSH = 0x3F,
    SDF_GS_SCISSOR_1 = 0x40,
    SDF_GS_SCISSOR_2 = 0x41,
    SDF_GS_ALPHA_1 = 0x42,
    SDF_GS_ALPHA_2 = 0x43,
    SDF_GS_DTHE = 0x45,
    SDF_GS_COLCLAMP = 0x46,
    SDF_GS_TEST_1 = 0x47,
    SDF_GS_TEST_2 = 0x48,
    SDF_GS_FRAME_1 = 0x4C,
    SDF_GS_FRAME_2 = 0x4D,
    SDF_GS_ZBUF_1 = 0x4E,
    SDF_GS_ZBUF_2 = 0x4F,
    SDF_GS_BITBLTBUF = 0x50,
    SDF_GS_TRXPOS = 0x51,
    SDF_GS_TRXREG = 0x52,
    SDF_GS_TRXDIR = 0x53
};
/* Values written to the PRIM and TEST registers by the centered-bounds packet. */
enum {
    SDF_GS_PRIMITIVE_SPRITE = 0x06,
    SDF_GS_CENTERED_VIEW_BOUNDS_TEST = 0x30003
};

extern u8 sdfFrameId;

#define SDF_DMA_TAG_REF_BYTE 0x30

#define SDF_DMA_CNT_ONE_WORD 0x10000001

#define SDF_VIF_DIRECT_ONE_FLUSHE 0x5000000110000000ULL

#define SDF_GIF_PACKED_AD_BITS 0x1000000000008000ULL

#define SDF_VIF_DIRECT_ONE_NOP 0x5000000100000000ULL

#define SDF_GS_DEFAULT_TEXA 0x4000000080ULL

#define SDF_GS_SCENE_TEST 0x517FB

#define SDF_TEXTURE_SCENE_PAYLOAD_QWORDS 0x15

#define SDF_GRAPH_DEPTH_BUFFER_INDEX 2

extern u8 D_0055188A;

extern u16 sdfReqDrawDelaySleepCount;

extern u32 sdfDrawFrameCount;

extern u8 sdfDrawImageTrans;

extern volatile u8 sdfDrawSyncFlag;

extern u32 sdfGetRunTimeCountBase(void);

extern u32 sdfGetRunTimeCount(u32 previousTick);

extern void sdfWaitVblank(s32 count);

struct sdfPrimTransElem_t;

struct sdfGrRefPacketHeader_t;

struct SdfPacketPatchLink;

struct sdfGrDrawFramePacket_t;

struct sdfGrClearPacket_t;

struct sdfGrRefGifPacketHeader_t;

struct SdfDmaReferenceChainPacket;

struct sdfGrDrawFrame_t;

struct SdfGsSceneBlendRegisters;

struct SdfSceneDrawPacket;

struct sdfGsMemSlice_t;

struct sdfGrFrameSet_t;

struct sdfDrawTag_t;

struct sdfGrOt_t;

struct sdfGrFramePacket_t;

struct SdfDmaTag;

struct SdfPacketSlot;

struct sdfPreDrawTag_t;

struct SdfPacketChain;

/* The frame worker passes the node and queued buffer index. The registered
 * handlers project the node into their distinct complete packet owners. */
typedef void (*SdfPacketPatchCallback)();

#define SDF_DMA_TAG_NEXT_BYTE 0x20

#define SDF_DMA_TAG_CALL_BYTE 0x50

#define SDF_DMA_TAG_END_BYTE 0x70

#define SDF_DMA_QWC_MASK 0xFFFF

#define SDF_DMA_ADDRESS_MASK 0x0FFFFFFF

#define SDF_QWORD_BYTES 0x10

#define SDF_DMA_EXTENDED_TAIL_OFFSET 0x30

#define SDF_DMA_REFERENCE_NODE_BYTES 0x20

#define SDF_DMA_TAG_REF_WORD 0x30000000

#define SDF_DMA_TAG_NEXT_WORD 0x20000000

#define SDF_VIF_DIRECT_WORD 0x50000000

#define SDF_VIF_FLUSHE_WORD 0x10000000

#define SDF_GIF_ONE_REGISTER_WORD 0x10000000

#define SDF_GIF_EOP_BIT 0x8000

#define SDF_GS_CENTER_BIAS 0x1000

#define SDF_GS_FIELD_OFFSET_STEP 8

#define SDF_GS_DEFAULT_ALPHA 0x44

#define SDF_SCENE_DRAW_PAYLOAD_QWORDS 5

extern volatile s8 sdfReqDrawDelayCount;

/* TEST and ALPHA form the two A+D writes shared by the blend builders. */
typedef struct sdfPrimTransElem_t {
    sdfGrTagStdReg_t test;
    sdfGrTagStdReg_t alpha;
} sdfPrimTransElem_t;

/* REF tag and VIF commands followed by the trailing chain tag (0x20).
 * The complete trailing quadword is cleared before its NEXT header is patched. */
typedef struct sdfGrRefPacketHeader_t {
    u64 dmaTag;
    u64 vifCommands;
    s128 nextTag;
} sdfGrRefPacketHeader_t;

typedef struct SdfPacketPatchLink {
    struct SdfPacketPatchLink *next;
    SdfPacketPatchCallback patch;
} SdfPacketPatchLink;

/* Four A+D writes define one GS drawing context, without a DMA/GIF header. */
typedef struct sdfGrDrawFramePacket_t {
    sdfGrTagStdReg_t frame;
    sdfGrTagStdReg_t zbuf;
    sdfGrTagStdReg_t xyoffset;
    sdfGrTagStdReg_t scissor;
} sdfGrDrawFramePacket_t;

/* A+D writes that draw the centered viewport bounds as a sprite. */
typedef struct sdfGrClearPacket_t {
    sdfGrTagStdReg_t test;
    sdfGrTagStdReg_t primitive;
    sdfGrTagStdReg_t rgbaq;
    sdfGrTagStdReg_t xyz2[2];
} sdfGrClearPacket_t;

/* Shared A+D state for GS primitive modes, color clamping, dithering and TEXA. */
typedef struct sdfGrRefGifPacketHeader_t {
    sdfGrTagStdReg_t prmodecont;
    sdfGrTagStdReg_t colclamp;
    sdfGrTagStdReg_t dthe;
    sdfGrTagStdReg_t texa;
} sdfGrRefGifPacketHeader_t;

/* GIF tag followed by a REF transfer and the patchable NEXT link. */
typedef struct SdfDmaReferenceChainPacket {
    sdfGrPacketHeader_t header;
    u64 referenceTag;
    u64 referenceVifCommands;
    u64 nextTag;
    u64 zeroTailWord;
} SdfDmaReferenceChainPacket;

/* Accessed output span of the common scene-context builder. */
typedef struct sdfGrDrawFrame_t {
    sdfGrPacketHeader_t header;
    sdfGrDrawFramePacket_t registers;
} sdfGrDrawFrame_t;

/* TEST/ALPHA A+D state for the two independent GS drawing contexts. */
typedef struct SdfGsSceneBlendRegisters {
    sdfPrimTransElem_t contextOne;
    sdfPrimTransElem_t contextTwo;
} SdfGsSceneBlendRegisters;

/* Complete DMA/GIF owner used by scene builders and held-texture draw banks. */
typedef struct SdfSceneDrawPacket {
    sdfGrPacketHeader_t header;
    sdfGrRefGifPacketHeader_t drawDefaults;
    sdfGrDrawFramePacket_t contextOne;
    sdfGrDrawFramePacket_t contextTwo;
    sdfGrClearPacket_t centeredBounds;
    SdfGsSceneBlendRegisters blendState;
} SdfSceneDrawPacket;


/* Graph target: two color buffers followed by the auxiliary/depth buffer (0x14). */
typedef struct sdfGrFrameSet_t {
    s16 width;
    s16 h;
    s16 height;
    u8 bufferFormat;
    u8 auxiliaryFormat;
    sdfGsMemSlice_t *buffers[3];
} sdfGrFrameSet_t;


/* Draw-surface pool entry (0x20); the SDK initializes and flushes this owner. */
typedef struct sdfGrOt_t {
    struct sdfGrOt_t *next; /* 0x00 */
    sdfDrawTag_t *tagHead;     /* 0x04 */
    sdfDrawTag_t *tagTail;     /* 0x08 */
    u32 noSortTag;
    void (*regFunc)(struct sdfGrOt_t *, sdfDrawTag_t *);          /* 0x10 */
    s32 (*callbackFunc)(struct sdfGrOt_t *, s32, sdfDrawTag_t *); /* 0x14 */
    u32 data;
    u32 pad;
} sdfGrOt_t;

/* Frame/depth builder state: one DMA/GIF header and two A+D register writes. */
typedef struct sdfGrFramePacket_t {
    u8 pad00[4];
    void (*prepare)(struct sdfGrFramePacket_t *, s32 bufferIndex);
    u8 pad08[8];
    sdfGrPacketHeader_t packetHeader;
    sdfGrTagStdReg_t frame;
    sdfGrTagStdReg_t zbuf;
    sdfGrFrameSet_t *source;
    u32 frameMask;
    s32 region;
    s32 mode;
} sdfGrFramePacket_t;

/* Packet-list code views the same 16-byte row as QWC/ID/ADDR/VIF commands. */
typedef struct SdfDmaTag {
    u16 quadwordCount;
    u8 pad02;
    u8 kind;
    u32 address;
    u64 vifCommands;
} QWORD_ALIGNED SdfDmaTag;

void sdfDrawTagLink2(struct sdfDrawTag_t *list, u32 packetAddress,
                          u32 rangeTailAddress);

void sdfDrawTagLinkRef(struct sdfDrawTag_t *list, u32 packetAddress);

/* One 0x10-byte entry in the three-slot frame packet submission ring. */
typedef struct SdfPacketSlot {
    struct sdfDrawTag_t *packetList;
    SdfPacketPatchLink *callbackHead;
    u8 slotUpdatePhase;
    u8 queuedBufferIndex;
    u8 bufferIndex;
    u8 bufferSlotIndex;
    u8 pad0C[4];
} SdfPacketSlot;

/* Complete linked-packet list, distinct from the 0x20-byte DMA list head. */
typedef struct sdfPreDrawTag_t {
    u32 next;
    SdfPacketPatchLink *head;
    SdfPacketPatchLink *tail;
    u32 pad;
} sdfPreDrawTag_t;

/* Frame submission joins these 0x10-byte linked owners through head/tail. */
typedef struct SdfPacketChain {
    sdfPreDrawTag_t *head;
    sdfPreDrawTag_t *tail;
} SdfPacketChain;

/* Linked draw environment, including its two frame-mask writes and textures. */
typedef struct sdfSceneNode_t {
    SdfPacketPatchLink link;
    sdfGrFrameSet_t *view;
    u8 unk0C[4];
    sdfGrPacketHeader_t header;
    sdfGrRefGifPacketHeader_t drawDefaults;
    sdfGrDrawFramePacket_t contextOne;
    sdfGrDrawFramePacket_t contextTwo;
    sdfGrClearPacket_t centeredBounds;
    SdfGsSceneBlendRegisters blendState;
    sdfGrTagStdReg_t frameContextOne;
    sdfGrTagStdReg_t frameContextTwo;
    u8 texturePackets[0x80];
} sdfSceneNode_t;

void sdfGrStdDrawEnvPredraw(sdfSceneNode_t *node, s32 bufferIndex);

void sdfGrSetRefGifPacket(SdfDmaReferenceChainPacket *packet, u32 sourceAddress, s32 qwordCount);

void sdfGrSetDrawDefParmPacket(sdfGrRefGifPacketHeader_t *packet);

extern s32 EIntr(void);

extern s32 DIntr(void);

extern s32 sdfDrawReqQueueIndex;

extern SdfPacketSlot sdfGrDrawState[];

typedef struct EffPacketParams {
    s16 parameterCount;
    s16 vertexCount;
    u16 primitive;
    u16 mask;
    u32 color;
    const u32 *parameters;
    u128 *positions;
    u128 *normals;
    f32 *texcoords;
    u32 *extraTexcoords;
    u32 *colors;
    void *(*allocate)(s32);
    f32 depth;
} EffPacketParams;

s32 sdfGrAllocPacketBuff(s32 size);

void sdfDrawTagInit(sdfDrawTag_t *list);

void sdfDrawTagLink(sdfDrawTag_t *list, u32 packet);

void sdfDrawTagLinkRefGif(sdfDrawTag_t *list, u32 packetAddress);

void sdfDrawTagLinkCall(sdfDrawTag_t *list, u32 packetAddress);

void sdfGrOtFuncLinkToHead(sdfGrOt_t *destinationList, sdfDrawTag_t *incomingList);

void sdfGrOtNormalReg(sdfGrOt_t *list, sdfDrawTag_t *item);

s32 sdfGrOtNormalCallback(sdfGrOt_t *list, s32 mode, sdfDrawTag_t *packet);

void sdfGrOtInit(sdfGrOt_t *nodes, s32 count);

sdfGrRefPacketHeader_t *sdfGrNewExPacket(SdfDmaTag *sourceTag);

SdfDmaTag *sdfGrAddExPacket(SdfDmaTag *previousTag, SdfDmaTag *sourceTag);

void sdfGrJoinDrawTag(sdfDrawTag_t *previousList, sdfDrawTag_t *incomingList);

void sdfGrInitExPacket(sdfDrawTag_t *list);

sdfDrawTag_t *sdfGrOtClose(sdfGrOt_t *node);

void sdfPreDrawTagLink(sdfPreDrawTag_t *list, SdfPacketPatchLink *node);

void sdfPreDrawChainLink(SdfPacketChain *chain, sdfPreDrawTag_t *node);

void sdfGrSetRegsPacketHeader(sdfGrPacketHeader_t *header, s32 payloadQwords);

void sdfGrSetDrawFramePacket(sdfGrDrawFramePacket_t *packet, s32 frameAddress, s32 width, s32 height,
                  s32 frameFormat, s32 depthAddress, s32 depthFormat,
                  s32 fieldOffset, s32 gsContext);

void sdfGrSetClearPacket(sdfGrClearPacket_t *packet, s32 width, s32 height, s32 unused0, s32 unused1);

void sdfGrSetDrawFrame(sdfGrDrawFrame_t *packet, s32 frameAddress, s32 width, s32 height,
                           s32 frameFormat, s32 depthAddress, s32 depthFormat, s32 gsContext);

void sdfGrLinkStdDrawEnv(sdfDrawTag_t *dmaList, sdfPreDrawTag_t *linkedList, SdfPacketPatchLink *linkedNode);

void sdfGrLinkDispFrameTex(sdfDrawTag_t *list, u32 source, sdfGrRefPacketHeader_t *node);

void sdfGrLinkDrawFrameTex(sdfDrawTag_t *list, u32 source, sdfGrRefPacketHeader_t *node);

void sdfGrFramePacketPredraw(sdfGrFramePacket_t *packet, s32 bufferIndex);

void sdfGrSetFramePacket(sdfGrFramePacket_t *packet, sdfGrFrameSet_t *source, u32 frameMask, s32 region, s32 mode);

void sdfGrReqDrawDelay(s32 value);

void sdfGrInitDraw(void);

void sdfDrawTagInit(sdfDrawTag_t *list) {
    list->z = 0xFFFF;
    list->next = NULL;
    list->head = 0;
    list->tail = 0;
    list->lightPacket = 0;
    list->fogPacket = 0;
    list->texPacket[0] = 0;
    list->texPacket[1] = 0;
}

void sdfDrawTagLink(sdfDrawTag_t *list, u32 packet) {
    s32 last;

    last = list->tail;
    if (last == 0) {
        list->head = packet;
    }
    else {
        ((SdfDmaTag *)last)->kind = SDF_DMA_TAG_NEXT_BYTE;
        ((SdfDmaTag *)last)->address = packet & SDF_DMA_ADDRESS_MASK;
    }
    list->tail = packet;
}

void sdfDrawTagLink2(sdfDrawTag_t *list, u32 packetAddress, u32 rangeTailAddress) {
    s32 previousTailAddress;

    previousTailAddress = list->tail;
    if (previousTailAddress == 0) {
        list->head = packetAddress;
    }
    else {
        ((SdfDmaTag *)previousTailAddress)->kind = SDF_DMA_TAG_NEXT_BYTE;
        ((SdfDmaTag *)previousTailAddress)->address = packetAddress & SDF_DMA_ADDRESS_MASK;
    }
    list->tail = rangeTailAddress;
}

void sdfDrawTagLinkRef(sdfDrawTag_t *list, u32 packetAddress) {
    s32 previousTailAddress;

    ((SdfDmaTag *)packetAddress)->kind = SDF_DMA_TAG_REF_BYTE;
    previousTailAddress = list->tail;
    if (previousTailAddress == 0) {
        list->head = packetAddress;
    }
    else {
        ((SdfDmaTag *)previousTailAddress)->kind = SDF_DMA_TAG_NEXT_BYTE;
        ((SdfDmaTag *)previousTailAddress)->address = packetAddress & SDF_DMA_ADDRESS_MASK;
    }
    list->tail = packetAddress + SDF_QWORD_BYTES;
}

void sdfDrawTagLinkRefGif(sdfDrawTag_t *list, u32 packetAddress) {
    s32 previousTailAddress;

    previousTailAddress = list->tail;
    if (previousTailAddress == 0) {
        list->head = packetAddress;
    }
    else {
        ((SdfDmaTag *)previousTailAddress)->kind = SDF_DMA_TAG_NEXT_BYTE;
        ((SdfDmaTag *)previousTailAddress)->address = packetAddress & SDF_DMA_ADDRESS_MASK;
    }
    list->tail = packetAddress + SDF_DMA_EXTENDED_TAIL_OFFSET;
}

void sdfDrawTagLinkCall(sdfDrawTag_t *list, u32 packetAddress) {
    s32 previousTailAddress;

    ((SdfDmaTag *)packetAddress)->kind = SDF_DMA_TAG_CALL_BYTE;
    previousTailAddress = list->tail;
    if (previousTailAddress == 0) {
        list->head = packetAddress;
    }
    else {
        ((SdfDmaTag *)previousTailAddress)->kind = SDF_DMA_TAG_NEXT_BYTE;
        ((SdfDmaTag *)previousTailAddress)->address = packetAddress & SDF_DMA_ADDRESS_MASK;
    }
    list->tail = packetAddress + SDF_QWORD_BYTES;
}

void sdfGrOtFuncLinkToHead(sdfGrOt_t *destinationList, sdfDrawTag_t *incomingList) {
    sdfDrawTag_t *firstList;

    if (incomingList->tail == 0) {
        return;
    }
    firstList = destinationList->tagHead;
    if (firstList == NULL) {
        destinationList->tagTail = incomingList;
    } else {
        sdfGrJoinDrawTag(incomingList, firstList);
    }
    incomingList->next = firstList;
    destinationList->tagHead = incomingList;
}

void sdfGrOtNormalReg(sdfGrOt_t *list, sdfDrawTag_t *item) {
    sdfDrawTag_t *last;

    if (item->head != 0) {
        last = list->tagTail;
        if (last == NULL) {
            list->tagHead = item;
        }
        else {
            last->next = item;
            sdfGrJoinDrawTag(last, item);
        }
        list->tagTail = item;
    }
}

s32 sdfGrOtNormalCallback(sdfGrOt_t *list, s32 mode, sdfDrawTag_t *packet) {
    if (mode == 1) {
        sdfGrOtFuncLinkToHead(list, packet);
    }
}

void sdfGrOtInit(sdfGrOt_t *nodes, s32 count) {
    sdfGrOt_t *node = nodes;
    sdfGrOt_t *next;
    s32 remaining = count - 1;

    node->regFunc = sdfGrOtNormalReg;
    node->callbackFunc = sdfGrOtNormalCallback;
    node->tagHead = 0;
    node->tagTail = 0;
    node->noSortTag = 0;
    node->data = 0;
    if (remaining == 0) {
        node->next = NULL;
        return;
    }
link:
    next = node + 1;
    remaining--;
    node->next = next;
    node = next;
    next->regFunc = sdfGrOtNormalReg;
    next->callbackFunc = sdfGrOtNormalCallback;
    next->tagHead = 0;
    next->tagTail = 0;
    next->noSortTag = 0;
    next->data = 0;
    if (remaining != 0) {
        goto link;
    }
    node->next = NULL;
}

sdfGrRefPacketHeader_t *sdfGrNewExPacket(SdfDmaTag *sourceTag) {
    sdfGrRefPacketHeader_t *referenceNode = (sdfGrRefPacketHeader_t *)sdfGrAllocPacketBuff(SDF_DMA_REFERENCE_NODE_BYTES);
    u64 dmaHeader = sourceTag->quadwordCount;
    u32 payloadAddress = ((u32)sourceTag + SDF_QWORD_BYTES) & SDF_DMA_ADDRESS_MASK;
    s64 packedAddress = (s64)payloadAddress << 32;

    dmaHeader |= SDF_DMA_TAG_REF_WORD;
    dmaHeader |= packedAddress;
    referenceNode->dmaTag = dmaHeader;
    QUAD_ZERO(referenceNode->nextTag);
    referenceNode->vifCommands = sourceTag->vifCommands;
    return referenceNode;
}

SdfDmaTag *sdfGrAddExPacket(SdfDmaTag *previousTag, SdfDmaTag *sourceTag) {
    sdfGrRefPacketHeader_t *referenceNode;

    referenceNode = sdfGrNewExPacket(sourceTag);
    previousTag->kind = SDF_DMA_TAG_NEXT_BYTE;
    previousTag->address = (u32)referenceNode & SDF_DMA_ADDRESS_MASK;
    return (SdfDmaTag *)((u32)referenceNode + SDF_QWORD_BYTES);
}

void sdfGrJoinDrawTag(sdfDrawTag_t *previousList, sdfDrawTag_t *incomingList) {
    SdfDmaTag *tailTag = (SdfDmaTag *)previousList->tail;
    u32 previousSource;
    u32 incomingSource;

    previousSource = previousList->lightPacket;
    incomingSource = incomingList->lightPacket;
    if (previousSource != incomingSource) {
        if (incomingSource != 0) {
            tailTag = sdfGrAddExPacket(tailTag, (SdfDmaTag *)incomingSource);
        } else {
            incomingList->lightPacket = previousSource;
        }
    }
    previousSource = previousList->fogPacket;
    incomingSource = incomingList->fogPacket;
    if (previousSource != incomingSource) {
        if (incomingSource != 0) {
            tailTag = sdfGrAddExPacket(tailTag, (SdfDmaTag *)incomingSource);
        } else {
            incomingList->fogPacket = previousSource;
        }
    }
    tailTag->kind = SDF_DMA_TAG_NEXT_BYTE;
    tailTag->address = incomingList->head & SDF_DMA_ADDRESS_MASK;
}

void sdfGrInitExPacket(sdfDrawTag_t *list) {
    sdfGrRefPacketHeader_t *referenceNode;
    u32 priorHeadAddress;
    u32 sourceTagAddress;

    sourceTagAddress = list->fogPacket;
    if (sourceTagAddress != 0) {
        referenceNode = sdfGrNewExPacket((SdfDmaTag *)sourceTagAddress);
        priorHeadAddress = list->head & SDF_DMA_ADDRESS_MASK;
        list->head = (u32)referenceNode;
        *(u64 *)&referenceNode->nextTag = ((s64)priorHeadAddress << 32) | SDF_DMA_TAG_NEXT_WORD;
    }
    sourceTagAddress = list->lightPacket;
    if (sourceTagAddress != 0) {
        referenceNode = sdfGrNewExPacket((SdfDmaTag *)sourceTagAddress);
        priorHeadAddress = list->head & SDF_DMA_ADDRESS_MASK;
        list->head = (u32)referenceNode;
        *(u64 *)&referenceNode->nextTag = ((s64)priorHeadAddress << 32) | SDF_DMA_TAG_NEXT_WORD;
    }
}

sdfDrawTag_t *sdfGrOtClose(sdfGrOt_t *node) {
    sdfDrawTag_t *tail = NULL;
    sdfDrawTag_t *head = NULL;

    for (; node != NULL; node = node->next) {
        node->callbackFunc(node, 0, NULL);
        if (node->tagHead != 0) {
            if (head != 0) {
                sdfGrJoinDrawTag(tail, node->tagHead);
            } else {
                head = node->tagHead;
                sdfGrInitExPacket(head);
            }
            tail = node->tagTail;
        }
    }
    if (tail != NULL) {
        ((SdfDmaTag *)tail->tail)->kind = SDF_DMA_TAG_END_BYTE;
        ((SdfDmaTag *)tail->tail)->address = 0;
    }
    return head;
}

void sdfPreDrawTagInit(sdfPreDrawTag_t *list) {
    list->next = 0;
    list->head = 0;
    list->tail = 0;
    list->pad = 0;
}

void sdfPreDrawTagLink(sdfPreDrawTag_t *list, SdfPacketPatchLink *node) {
    if (list->tail == 0) {
        list->head = node;
    }
    else {
        list->tail->next = node;
    }
    list->tail = node;
    node->next = NULL;
}

void sdfPreDrawChainInit(SdfPacketChain *chain) {
    chain->head = NULL;
    chain->tail = NULL;
}

void sdfPreDrawChainLink(SdfPacketChain *chain, sdfPreDrawTag_t *node) {
    if (chain->tail == NULL) {
        chain->head = node;
    }
    else {
        chain->tail->tail->next = node->tail;
    }
    chain->tail = node;
}

void sdfGrSetRegsPacketHeader(sdfGrPacketHeader_t *header, s32 payloadQwords) {
    s64 dmaQwords;

    dmaQwords = payloadQwords + 1;
    header->dmaWords.vif = (((dmaQwords | SDF_VIF_DIRECT_WORD) << 32) | SDF_VIF_FLUSHE_WORD);
    header->giftag.parm = (payloadQwords | (((s64)SDF_GIF_ONE_REGISTER_WORD << 32) | SDF_GIF_EOP_BIT));
    header->dmaWords.tag = dmaQwords;
    header->giftag.regs = SDF_GIF_REGISTER_AD;
}

void sdfGrSetRefGifPacket(SdfDmaReferenceChainPacket *packet, u32 sourceAddress, s32 qwordCount) {
    packet->header.dmaWords.tag = SDF_DMA_CNT_ONE_WORD;
    packet->header.dmaWords.vif = SDF_VIF_DIRECT_ONE_FLUSHE;
    packet->header.giftag.parm = (u64)qwordCount | SDF_GIF_PACKED_AD_BITS;
    packet->header.giftag.regs = SDF_GIF_REGISTER_AD;
    packet->referenceTag = (u32)((qwordCount & SDF_DMA_QWC_MASK) | SDF_DMA_TAG_REF_WORD) | ((u64)(sourceAddress & SDF_DMA_ADDRESS_MASK) << 32);
    packet->referenceVifCommands = SDF_VIF_DIRECT_ONE_NOP;
    packet->nextTag = SDF_DMA_TAG_NEXT_WORD;
    packet->zeroTailWord = 0;
}

void sdfGrSetDrawFramePacket(sdfGrDrawFramePacket_t *packet, s32 frameAddress, s32 width, s32 height,
                  s32 frameFormat, s32 depthAddress, s32 depthFormat,
                  s32 fieldOffset, s32 gsContext) {
    s64 frameRegisterId;
    s64 depthRegisterId;
    s64 offsetRegisterId;
    s64 scissorRegisterId;
    s32 xOffset;
    s32 yOffset;

    if (gsContext == 0) {
        frameRegisterId = SDF_GS_FRAME_1;
        depthRegisterId = SDF_GS_ZBUF_1;
        offsetRegisterId = SDF_GS_XYOFFSET_1;
        scissorRegisterId = SDF_GS_SCISSOR_1;
    } else {
        frameRegisterId = SDF_GS_FRAME_2;
        depthRegisterId = SDF_GS_ZBUF_2;
        offsetRegisterId = SDF_GS_XYOFFSET_2;
        scissorRegisterId = SDF_GS_SCISSOR_2;
    }
    packet->scissor.reg = scissorRegisterId;
    yOffset = (SDF_GS_CENTER_BIAS - height) << 3;
    xOffset = (SDF_GS_CENTER_BIAS - width) << 3;
    packet->frame.data = (s64)(frameFormat << 24) | (s64)(((width + 63) >> 6) << 16) | (s64)(frameAddress >> 11);
    if (fieldOffset != 0) {
        yOffset += SDF_GS_FIELD_OFFSET_STEP;
    }
    packet->frame.reg = frameRegisterId;
    packet->zbuf.data = (s64)(depthFormat << 24) | (s64)(depthAddress >> 11);
    packet->zbuf.reg = depthRegisterId;
    packet->xyoffset.reg = offsetRegisterId;
    packet->scissor.data = ((s64)(height - 1) << 48) | ((s64)(width - 1) << 16);
    packet->xyoffset.data = xOffset | ((s64)yOffset << 32);
}

void sdfGrSetClearPacket(sdfGrClearPacket_t *packet, s32 width, s32 height, s32 unused0, s32 unused1) {
    u32 lowerBounds = ((SDF_GS_CENTER_BIAS - height) << 19) | ((SDF_GS_CENTER_BIAS - width) << 3);
    u32 upperBounds = ((height + SDF_GS_CENTER_BIAS) << 19) | ((width + SDF_GS_CENTER_BIAS) << 3);

    packet->primitive.reg = SDF_GS_PRIM;
    packet->xyz2[0].reg = SDF_GS_XYZ2;
    packet->test.data = SDF_GS_CENTERED_VIEW_BOUNDS_TEST;
    packet->test.reg = SDF_GS_TEST_1;
    packet->primitive.data = SDF_GS_PRIMITIVE_SPRITE;
    packet->rgbaq.data = (u64)0xFE00 << 46;
    packet->rgbaq.reg = SDF_GS_RGBAQ;
    packet->xyz2[0].data = lowerBounds;
    packet->xyz2[1].data = upperBounds;
    packet->xyz2[1].reg = SDF_GS_XYZ2;
}

void sdfGrSetDrawDefParmPacket(sdfGrRefGifPacketHeader_t *packet) {
    packet->prmodecont.data = 1;
    packet->prmodecont.reg = SDF_GS_PRMODECONT;
    packet->colclamp.data = 1;
    packet->colclamp.reg = SDF_GS_COLCLAMP;
    packet->dthe.data = 0;
    packet->dthe.reg = SDF_GS_DTHE;
    packet->texa.data = SDF_GS_DEFAULT_TEXA;
    packet->texa.reg = SDF_GS_TEXA;
}

void sdfGrSetDrawFrame(sdfGrDrawFrame_t *packet, s32 frameAddress, s32 width, s32 height,
                           s32 frameFormat, s32 depthAddress, s32 depthFormat, s32 gsContext) {
    sdfGrSetRegsPacketHeader(&packet->header, SDF_SCENE_DRAW_PAYLOAD_QWORDS);
    sdfGrSetDrawFramePacket(&packet->registers, frameAddress, width, height, frameFormat, depthAddress, depthFormat, 0, gsContext);
}

void sdfGrSetDrawEnv(SdfSceneDrawPacket *packet, sdfGrFrameSet_t *view, s32 bufferIndex) {
    s32 frameAddress;
    s32 depthAddress;
    s32 width;
    s32 height;
    s32 frameFormat;
    s32 depthFormat;

    sdfGrSetRegsPacketHeader(&packet->header, SDF_TEXTURE_SCENE_PAYLOAD_QWORDS);
    frameAddress = view->buffers[bufferIndex]->addr;
    depthAddress = view->buffers[SDF_GRAPH_DEPTH_BUFFER_INDEX]->addr;
    width = view->width;
    frameFormat = view->bufferFormat;
    height = view->height;
    depthFormat = view->auxiliaryFormat;
    sdfGrSetDrawFramePacket(&packet->contextOne, frameAddress, width, height, frameFormat, depthAddress, depthFormat, D_0055188A, 0);
    sdfGrSetDrawFramePacket(&packet->contextTwo, frameAddress, width, height, frameFormat, depthAddress, depthFormat, D_0055188A, 1);
    sdfGrSetClearPacket(&packet->centeredBounds, view->width, view->height, view->bufferFormat, view->auxiliaryFormat);
    packet->blendState.contextOne.test.data = SDF_GS_SCENE_TEST;
    packet->blendState.contextOne.test.reg = SDF_GS_TEST_1;
    packet->blendState.contextOne.alpha.data = SDF_GS_DEFAULT_ALPHA;
    packet->blendState.contextOne.alpha.reg = SDF_GS_ALPHA_1;
    packet->blendState.contextTwo.test.data = SDF_GS_SCENE_TEST;
    packet->blendState.contextTwo.test.reg = SDF_GS_TEST_2;
    packet->blendState.contextTwo.alpha.data = SDF_GS_DEFAULT_ALPHA;
    packet->blendState.contextTwo.alpha.reg = SDF_GS_ALPHA_2;
    sdfGrSetDrawDefParmPacket(&packet->drawDefaults);
}

INCLUDE_ASM(const s32, "sdf/sdfDraw", sdfGrStdDrawEnvPredraw);

void sdfGrSetStdDrawEnv(sdfSceneNode_t *node, sdfGrFrameSet_t *view) {
    sdfGrSetRegsPacketHeader(&node->header, SDF_TEXTURE_SCENE_PAYLOAD_QWORDS);
    node->view = view;
    node->link.patch = sdfGrStdDrawEnvPredraw;
    sdfGrSetClearPacket(&node->centeredBounds, view->width, view->height, view->bufferFormat, view->auxiliaryFormat);
    node->blendState.contextOne.test.data = SDF_GS_SCENE_TEST;
    node->blendState.contextOne.test.reg = SDF_GS_TEST_1;
    node->blendState.contextOne.alpha.data = SDF_GS_DEFAULT_ALPHA;
    node->blendState.contextOne.alpha.reg = SDF_GS_ALPHA_1;
    node->blendState.contextTwo.test.data = SDF_GS_SCENE_TEST;
    node->blendState.contextTwo.test.reg = SDF_GS_TEST_2;
    node->blendState.contextTwo.alpha.data = SDF_GS_DEFAULT_ALPHA;
    node->blendState.contextTwo.alpha.reg = SDF_GS_ALPHA_2;
    sdfGrSetDrawDefParmPacket(&node->drawDefaults);
}

void sdfGrLinkStdDrawEnv(sdfDrawTag_t *dmaList, sdfPreDrawTag_t *linkedList, SdfPacketPatchLink *linkedNode) {
    sdfPreDrawTagLink(linkedList, linkedNode);
    sdfDrawTagLink(dmaList, (s32)linkedNode + SDF_QWORD_BYTES);
}

void sdfGrSetFrameMaskRGB(const u8 *source, SdfDmaReferenceChainPacket *packet, s32 gsContext) {
    if (gsContext == 0) {
        sdfGrSetRefGifPacket(packet, (u32)(source + 0x180), 1);
        return;
    }
    sdfGrSetRefGifPacket(packet, (u32)(source + 400), 1);
}

void sdfGrSetFrameMaskReset(const u8 *source, SdfDmaReferenceChainPacket *packet, s32 gsContext) {
    if (gsContext == 0) {
        sdfGrSetRefGifPacket(packet, (u32)(source + 0x70), 1);
        return;
    }
    sdfGrSetRefGifPacket(packet, (u32)(source + 0xb0), 1);
}

void sdfGrLinkDispFrameTex(sdfDrawTag_t *list, u32 source, sdfGrRefPacketHeader_t *node) {
    node->vifCommands = (((u64)0x50000004 << 16) | 0x1000) << 16;
    node->dmaTag = ((u64)((source + 0x1a0) & 0xfffffff) << 32) | 0x30000004;
    QUAD_ZERO(node->nextTag);
    sdfDrawTagLinkRef(list, (u32)node);
}

void sdfGrLinkDrawFrameTex(sdfDrawTag_t *list, u32 source, sdfGrRefPacketHeader_t *node) {
    node->vifCommands = (((u64)0x50000004 << 16) | 0x1000) << 16;
    node->dmaTag = ((u64)((source + 0x1e0) & 0xfffffff) << 32) | 0x30000004;
    QUAD_ZERO(node->nextTag);
    sdfDrawTagLinkRef(list, (u32)node);
}

void sdfGrFramePacketPredraw(sdfGrFramePacket_t *packet, s32 bufferIndex) {
    sdfGrFrameSet_t *view = packet->source;
    u64 frameRegister;
    u64 depthRegister;
    s16 widthPages;

    if (packet->mode == 0) {
        frameRegister = 0x4C;
        depthRegister = 0x4E;
    } else {
        frameRegister = 0x4D;
        depthRegister = 0x4F;
    }
    widthPages = (view->width + 63) >> 6;
    packet->frame.data = ((u64)packet->frameMask << 32) |
        (u64)(view->buffers[bufferIndex]->addr >> 11) |
        ((s64)(view->bufferFormat << 24) | ((s64)widthPages << 16));
    packet->frame.reg = frameRegister;
    packet->zbuf.data = (s64)(view->auxiliaryFormat << 24) |
        (u64)(view->buffers[2]->addr >> 11);
    packet->zbuf.reg = depthRegister;
}

void sdfGrSetFramePacket(sdfGrFramePacket_t *packet, sdfGrFrameSet_t *source, u32 frameMask, s32 region, s32 mode) {
    sdfGrSetRegsPacketHeader(&packet->packetHeader, 2);
    packet->mode = mode;
    packet->source = source;
    packet->frameMask = frameMask;
    packet->region = region;
    packet->prepare = sdfGrFramePacketPredraw;
}

void sdfGrReqDraw(sdfDrawTag_t *list, SdfPacketChain *chain) {
    u32 start = sdfGetRunTimeCountBase();
    s32 interrupts;
    s32 index;
    SdfPacketSlot *slot;

    while (sdfReqDrawDelayCount != 0) {
    }
    sdfReqDrawDelaySleepCount = sdfGetRunTimeCount(start);
    interrupts = DIntr();
    index = sdfDrawReqQueueIndex + 1;
    if (index == 3) {
        index = 0;
    }
    slot = &sdfGrDrawState[index];
    slot->packetList = NULL;
    slot->callbackHead = NULL;
    slot->slotUpdatePhase = 0;
    if (list != NULL && list->tail != 0) {
        slot->packetList = list;
        if (chain != NULL) {
            slot->callbackHead = chain->head->head;
            if (slot->callbackHead != NULL) {
                chain->tail->tail->next = NULL;
            }
        }
        slot->queuedBufferIndex = sdfFrameId;
    }
    sdfDrawReqQueueIndex = index;
    sdfDrawImageTrans = 1;
    sdfDrawFrameCount++;
    if (interrupts != 0) {
        EIntr();
    }
}

void sdfGrReqDrawDelay(s32 value) {
    sdfReqDrawDelayCount = (value < 0) ? 0 : value;
}

void sdfGrInitDraw(void) {
    sdfReqDrawDelayCount = 0;
    sdfGrDrawState[0].packetList = NULL;
    sdfDrawReqQueueIndex = 0;
}

INCLUDE_ASM(const s32, "sdf/sdfDraw", sdfGrDrawFrame);

void sdfDrawSync(void) {
    SdfPacketSlot *table = sdfGrDrawState;

    for (;;) {
        while (sdfDrawSyncFlag != 0) {
        }
        if (table[sdfDrawReqQueueIndex].packetList == NULL) {
            break;
        }
        if (table[sdfDrawReqQueueIndex].slotUpdatePhase >= 2) {
            break;
        }
        sdfWaitVblank(0);
    }
}
