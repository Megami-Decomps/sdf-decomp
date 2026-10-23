#include "common.h"
#include "ps2asm.h"
#include "sdf_lighting.h"
#include "sdf_resource.h"

struct SdfDmaTagHeader;

struct SdfObjectRefCommand;

struct sdfMotionCutDef_t;

struct SdfChunkHeader;

struct sdfMemHandle_t;

struct DevRequest;

struct sdfModelPacketHeader_tag;

struct sdfModelNode_t;

struct sdfModelRootNode_t;

struct sdfMotionNodeDef_t;

struct sdfMotionPlayBook_t;

struct sdfMotionCtrlr_t;

/* DMA tag prefix followed by the two VIF words used by indexed-model writers.
 * The control byte stays whole, and address is a hardware word rather than a
 * CPU pointer. */
typedef struct SdfDmaTagHeader {
    u16 quadwordCount;
    u8 reservedByte;
    u8 control;
    u32 address;
    u32 firstVifCode;
    u32 secondVifCode;
} QWORD_ALIGNED SdfDmaTagHeader;

/* Serialized kinds 5/7 name draw nodes by index in the model's buffered list. */
typedef struct SdfObjectRefCommand {
    u8 kind;
    u8 pad01[0x15];
    u16 flags;
    u16 count;
    u16 objectIndices[1];
} SdfObjectRefCommand;

/* Serialized clip duration/reserved remain halfwords (tools/fld.py).
 * The viewer MAXFRAME diagnostic reads their packed little-endian word. */
typedef struct sdfMotionCutDef_t {
    union {
        struct {
            u16 frameLength;
            u16 pad;
        };
        u32 packedHeader;
    };
    u32 chunk[1];
} sdfMotionCutDef_t;

#define HW_READ(reg) (*(reg))

#define HW_WRITE(reg, value) (*(reg) = (value))

extern u32 corePktCurrLight;

extern f32 corePktZBias;

#define SDF_DMA_QWORD_SHIFT 4

#define SDF_DMA_CHCR_TTE 0x40

extern u32 corePktCurrFog;

extern u128 corePktFogParm;

extern u8 vu0MicroPacket[];

extern u8 vu1MicroPacket[];

extern s32 sceDmaSync(void *, s32, s32);

/* Variable-size serialized chunk streams share only this eight-byte header.
 * The word at +8 belongs to each chunk's payload. */
typedef struct SdfChunkHeader {
    u32 id;   /* 0x00: zero terminates the chunk stream */
    u32 size; /* 0x04: byte extent to the next chunk */
} SdfChunkHeader;


typedef struct DevRequest {
    struct sdfMemHandle_t *backingAllocation;
    s16 usedCount;
    u16 capacity;
    s16 stride;
    s16 growStep;
    void *buffer;
} DevRequest;

/* Command-list node: allocation owns the node and trailing command data.
 * A null descriptor identifies a separately chip-allocated node. */
typedef struct sdfModelPacketHeader_tag {
    struct sdfModelPacketHeader_tag *next;
    u8 type;
    s8 otIndex;
    s16 size;
    void *dataPtr; /* Packet bytes for type 1; serialized commands for kinds 2/3. */
    struct sdfMemHandle_t *memHandle;
} sdfModelPacketHeader_tag;

/* Allocated as 0x100-byte nodes; each owns a circular list of child draw nodes. */
typedef struct sdfModelNode_t {
    struct sdfModelNode_t *prev; /* 0x00 */
    struct sdfModelNode_t *next;     /* 0x04 */
    struct sdfModelNode_t *parent;   /* 0x08 */
    struct sdfModelNode_t *children; /* 0x0C */
    struct sdfModelRootNode_t *root;        /* 0x10: backlink installed by sdfModelNodeAdd */
    u16 flags;                   /* 0x14 */
    s16 drawAttr;                   /* 0x16: constructor sentinel is -1 */
    s32 id;                   /* 0x18: lookup key when model flags bit 0 is set */
    u32 color;                   /* 0x1C */
    u8 drawPacketMemHandle[8];
    sdfModelPacketHeader_tag *drawPacket[2];     /* 0x28: command drawPacket for both buffered frames */
    u8 *tmPacket; /* Two 0x80-byte frame packets, allocated on first use. */                  /* 0x30 */
    const f32 *bounds;     /* 0x34: optional two xyz corners used by clipping */
    void *shape;             /* 0x38: item this node was built from */
    u32 pad0;
    void *userData;
    u32 pad1[3];
    f32 rotate[4];           /* 0x50 */
    f32 pos[4];          /* 0x60 */
    f32 scale[4];                /* 0x70 */
    f32 localNoScaledTm[4][4];       /* 0x80: all four local basis/pos rows */
    f32 tm[4][4];       /* 0xC0: local transform composed with its parent */
} sdfModelNode_t;

/* The buffered-transform constructor allocates and clears all 0x9C bytes. */
typedef struct sdfModelRootNode_t {
    DevRequest *list;
    sdfModelNode_t *rootNode;
    void *assetData;
    DevRequest *resources;
    DevRequest *slotPairs;
    struct sdfMotionCtrlr_t *motionList;
    u8 complexNodeId;
    u8 flags;
    u16 drawFlags;
    u32 color;
    f32 matrix[4][4];
    f32 rotationQuaternion[4];
    f32 scaleVector[4];
    struct SdfDmaTagHeader *lighting; /* Borrowed first DMA reference prefix. */
    struct SdfDmaTagHeader *secondaryLighting; /* Borrowed second DMA reference prefix. */
    f32 scalarOverrideFirst;
    f32 scalarOverrideSecond;
    struct SdfChunkHeader *chunkTable; /* Borrowed from the model resource data. */
    /* Model scale read by sdfVuSetGlobalScale and copied unchanged into the
     * three-scalar VIF payload (DDS1 0x2D9530). */
    f32 modelScale;
    u8 lodIndex; /* Native model-viewer and MODEL_LOD_CHG setters use byte 0x98. */
    u8 pad99[3];
} sdfModelRootNode_t;

typedef struct sdfMotionNodeDef_t {
    union {
        u32 typeId;
        struct {
            /* Runtime dispatch shifts the command word right by 16. */
            u16 selector;
            u16 family;
        } binding;
    };
    u32 nodeId;
} sdfMotionNodeDef_t;

/* Resource header followed by the binding commands used to construct a motion. */
typedef struct sdfMotionPlayBook_t {
    u16 numOfCut; /* Motion-index upper bound returned by mdlGetNumOfCut. */
    u16 numOfNode;
    sdfMotionCutDef_t **cutList;
    sdfMotionNodeDef_t nodeDefList[1];
} sdfMotionPlayBook_t;

/* SDK motions are allocated as 0x34-byte records; model slots retain these nodes. */
typedef struct sdfMotionCtrlr_t {
    struct sdfMotionCtrlr_t *next;
    sdfModelRootNode_t *owner;
    sdfMotionPlayBook_t *motionTable;
    s32 playbookMemHandle;
    DevRequest *request;
    f32 blendDurationFrames;
    f32 blendStartFrame;
    f32 currentFrame;
    f32 frameStep;
    s32 userData;
    s16 searchId;
    s16 slotIndex;
    u16 motionIndex;
    u16 frameCount;
    u8 state;
    u8 previousState;
    u8 loopEnabled;
    u8 pad33;
} sdfMotionCtrlr_t;

u32 sdfMemLock(struct sdfMemHandle_t *aHandle);

void sdfMemFreeHandleDirect(s32 *aHandlePtr);

/* The draw node owns the lazily allocated workspace; callers receive a borrowed
 * pointer to it, and must not release it independently. */
u8 *sdfModelAllocTmPacket(sdfModelNode_t *aNode);

extern s32 corePktPointBuff;

extern s32 sdfCoreDrawBuffHandle;

extern void *sceDmaGetChan(s32);

extern void sceDmaSendN(void *, void *, s32);

void sdfCoreDrawScanOneSkinNodes(sdfModelRootNode_t *root, SdfObjectRefCommand *p);

void sdfCoreDrawSetFog(u8 *fog);

void sdfCoreDrawInit(s32 buffSize);

INCLUDE_ASM(const s32, "sdf/sdfCoreDraw", sdfCorePacketGen);

void sdfCoreDrawScanOneSkinNodes(sdfModelRootNode_t *root, SdfObjectRefCommand *p) {
    sdfModelNode_t **objects = root->list->buffer;
    u16 *indices = &p->count;
    if ((p->flags & 0x800) != 0) {
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
    sdfCoreDrawBuffHandle = (u32)sdfMemAllocDirect(buffSize);
    corePktPointBuff = sdfMemLock((struct sdfMemHandle_t *)(sdfCoreDrawBuffHandle));
}
