#include "common.h"
#include "ps2asm.h"
#include "sdf_lighting.h"
#include "sdf_resource.h"

struct SdfDmaTagHeader;

struct SdfObjectRefCommand;

struct MotionEntry;

struct SdfChunkHeader;

struct sdfMemHandle_t;

struct DevRequest;

struct SdfCommandNode;

struct SdfDrawNode;

struct sdfModelRootNode_t;

struct SdfMotionCommand;

struct MotionTable;

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
typedef struct MotionEntry {
    union {
        struct {
            u16 frameCount;
            u16 unk02;
        };
        u32 packedHeader;
    };
    u32 bindingData[1];
} MotionEntry;

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
typedef struct SdfCommandNode {
    struct SdfCommandNode *next;
    u8 kind;
    s8 packetSelector;
    s16 quadwordCount;
    void *payload; /* Packet bytes for kind 1; serialized commands for kinds 2/3. */
    struct sdfMemHandle_t *allocation;
} SdfCommandNode;

/* Allocated as 0x100-byte nodes; each owns a circular list of child draw nodes. */
typedef struct SdfDrawNode {
    struct SdfDrawNode *previous; /* 0x00 */
    struct SdfDrawNode *next;     /* 0x04 */
    struct SdfDrawNode *parent;   /* 0x08 */
    struct SdfDrawNode *children; /* 0x0C */
    struct sdfModelRootNode_t *root;        /* 0x10: backlink installed by sdfModelNodeAdd */
    u16 flags;                   /* 0x14 */
    s16 unk16;                   /* 0x16: constructor sentinel is -1 */
    s32 nodeId;                   /* 0x18: lookup key when model flags bit 0 is set */
    u32 color;                   /* 0x1C */
    u8 pad20[8];
    SdfCommandNode *lists[2];     /* 0x28: command lists for both buffered frames */
    u8 *workspace; /* Two 0x80-byte frame packets, allocated on first use. */                  /* 0x30 */
    const f32 *boundsCorners;     /* 0x34: optional two xyz corners used by clipping */
    void *sourceItem;             /* 0x38: item this node was built from */
    u8 pad3C[0x14];
    f32 quaternion[4];           /* 0x50 */
    f32 translation[4];          /* 0x60 */
    f32 scale[4];                /* 0x70 */
    f32 localMatrix[4][4];       /* 0x80: all four local basis/translation rows */
    f32 worldMatrix[4][4];       /* 0xC0: local transform composed with its parent */
} SdfDrawNode;

/* The buffered-transform constructor allocates and clears all 0x9C bytes. */
typedef struct sdfModelRootNode_t {
    DevRequest *list;
    SdfDrawNode *rootNode;
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

typedef struct SdfMotionCommand {
    union {
        u32 packedCommand;
        struct {
            /* Runtime dispatch shifts the command word right by 16. */
            u16 selector;
            u16 family;
        } binding;
    };
    u32 targetIndex;
} SdfMotionCommand;

/* Resource header followed by the binding commands used to construct a motion. */
typedef struct MotionTable {
    u16 entryCount; /* Motion-index upper bound returned by mdlGetNumOfCut. */
    u16 commandCount;
    MotionEntry **entries;
    SdfMotionCommand commands[1];
} MotionTable;

/* SDK motions are allocated as 0x34-byte records; model slots retain these nodes. */
typedef struct sdfMotionCtrlr_t {
    struct sdfMotionCtrlr_t *next;
    sdfModelRootNode_t *owner;
    MotionTable *motionTable;
    s32 unkC;
    DevRequest *request;
    f32 blendDurationFrames;
    f32 blendStartFrame;
    f32 currentFrame;
    f32 frameStep;
    s32 unk24;
    s16 searchId;
    s16 slotIndex;
    u16 motionIndex;
    u16 frameCount;
    u8 state;
    u8 previousState;
    u8 loopEnabled;
    u8 pad33;
} sdfMotionCtrlr_t;

u32 sdfMemLock(struct sdfMemHandle_t *allocation);

void sdfMemFreeHandleDirect(s32 *slot);

/* The draw node owns the lazily allocated workspace; callers receive a borrowed
 * pointer to it, and must not release it independently. */
u8 *sdfModelAllocTmPacket(SdfDrawNode *root);

extern s32 corePktPointBuff;

extern s32 sdfCoreDrawBuffHandle;

extern void *sceDmaGetChan(s32);

extern void sceDmaSendN(void *, void *, s32);

void sdfCoreDrawScanOneSkinNodes(sdfModelRootNode_t *model, SdfObjectRefCommand *source);

void sdfCoreDrawSetFog(u8 *sourceObject);

void sdfCoreDrawInit(s32 workspaceBytes);

INCLUDE_ASM(const s32, "sdf/sdfCoreDraw", sdfCorePacketGen);

void sdfCoreDrawScanOneSkinNodes(sdfModelRootNode_t *model, SdfObjectRefCommand *source) {
    SdfDrawNode **objects = model->list->buffer;
    u16 *indices = &source->count;
    if ((source->flags & 0x800) != 0) {
        s32 count = *indices;
        if (count != 0) {
            indices++;
            do {
                sdfModelAllocTmPacket(objects[*indices++]);
            } while (--count != 0);
        }
    }
}

void sdfCoreDrawSetLight(u8 *sourceObject) {
    corePktCurrLight = (u32)sourceObject;
}

void sdfCoreDrawSetFog(u8 *sourceObject) {
    if (corePktCurrFog != (u32)sourceObject) {
        corePktCurrFog = (u32)sourceObject;
        SDF_VU0_COPYVECTOR(&corePktFogParm, sourceObject + 0x10);
    }
}

void sdfCoreDrawSetZBias(f32 bias) {
    corePktZBias = bias;
}

void sdfCoreDrawReset(void) {
    corePktCurrLight = 0;
    corePktCurrFog = 0;
}

void sdfCoreDrawInit(s32 workspaceBytes) {
    u32 *dmaChannel = sceDmaGetChan(0);
    HW_WRITE(dmaChannel, HW_READ(dmaChannel) & ~SDF_DMA_CHCR_TTE);
    sceDmaSendN(dmaChannel, vu0MicroPacket, (vu1MicroPacket - vu0MicroPacket) >> SDF_DMA_QWORD_SHIFT);
    sceDmaSync(dmaChannel, 0, 0);
    sdfMemFreeHandleDirect(&sdfCoreDrawBuffHandle);
    sdfCoreDrawBuffHandle = (u32)sdfMemAllocDirect(workspaceBytes);
    corePktPointBuff = sdfMemLock((struct sdfMemHandle_t *)(sdfCoreDrawBuffHandle));
}
