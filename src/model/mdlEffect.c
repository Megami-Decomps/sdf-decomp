#include "common.h"
#include "libsdf.h"
#include "model.h"
#include "kernel.h"
#include "newdata.h"
#include "ps2asm.h"
#include "motion.h"

EQUATE_FPTODP();

typedef struct mdlBillboard_t mdlBillboard_t;

typedef struct mdlEffect_t mdlEffect_t;

typedef struct mdlTrackWork_t mdlTrackWork_t;

typedef struct mdlEffectResrc_IPU_t mdlEffectResrc_IPU_t;

typedef struct mdlEffectResrcItem_t mdlEffectResrcItem_t;

typedef struct mdlMarkPos_t mdlMarkPos_t;

typedef struct pibHeaderChunk_t {
    s32 group;       /* 0x00: 0xFFFF terminates the record chain */
    s32 nextOffset; /* 0x04: relative byte offset to next record */
    union {
        u32 word;               /* 0x08 */
        struct {
            u16 selectorA;
            u16 selectorB;
        } stream;
    } payload;
    union {
        u32 word;               /* 0x0C */
        struct {
            u16 count;
            u16 partIndex;
        } part;
    } parameter;
    union {
        u32 word10; /* 0x10: group-four stream parameter */
        struct {
            u16 slotCount; /* Kind 1/2 group records: following entity-slot packets. */
            u16 unk12;
        } half;
    } tail;
} pibHeaderChunk_t;

typedef struct mdlEffectResrcItem_t {
    u32 type;       /* 0x00 */
    s32 count;      /* 0x04 */
    void *handle;   /* 0x08 */
    u8 opaque0C[4]; /* 0x0C */
} mdlEffectResrcItem_t;

typedef struct pibChunkItem_P2A_D3P_t {
    u8 type[4];
    u32 size;          /* 0x04: checked against the optional anchor-scale extent */
    s32 markId;       /* 0x08 */
    u16 count;         /* 0x0C */
    u16 effectIndex;     /* 0x0E */
    f32 zOffset;   /* 0x10: read when size >= 0x11 */
} pibChunkItem_P2A_D3P_t;

typedef struct mdlPibIpu_t {
    u8 opaque00[8];
    s32 dataId;        /* 0x08: resource-key lookup */
    s32 minimumTime;   /* 0x0C: deferred initialization threshold */
    u8 playbackCadenceStep; /* 0x10: mdlIpuParams_t byte 3 */
    u8 loopMode;            /* 0x11: mdlIpuParams_t byte 2 */
    u16 index;         /* 0x12: part-slot index */
} mdlPibIpu_t;

typedef struct pibChunkItem_TRACK_t {
    u8 pad00[8];
    s32 mark;   /* +0x08: first sampled endpoint lookup */
    s32 endPositionId;     /* +0x0C: second sampled endpoint lookup */
    u16 startFrame;        /* +0x10: inclusive update start */
    u16 endFrame;          /* +0x12: inclusive update end */
    u16 interval;    /* +0x14: update-count modulus */
    u8 face;      /* +0x16: history allocation input */
    u8 blendAlgo;           /* +0x17: selected by the track's draw data */
    u32 colors[4]; /* +0x18: copied to mdlTrackCreateParams_t */
} pibChunkItem_TRACK_t;

typedef struct mdlTrackParams_t {
    u8 opaque00[8];
    s32 mark0;
    s32 mark1;
    u16 start;
    u16 end;
    u16 interval;
    u8 face;
    u8 blend;
    u32 colors[4];
} mdlTrackParams_t;

typedef struct mdlMarkPos_t {
    u32 nodeId;      /* 0x00: draw-node lookup key */
    s32 id;          /* 0x04: map-position record lookup key */
    u8 reserved08[8];
    u128 position;   /* 0x10 */
    u128 up;         /* 0x20 */
    u128 direction;  /* 0x30 */
} mdlMarkPos_t;

typedef struct mdlDebugLight_t {
    f32 (**lights)[4];
    f32 *ambient;
    struct mdlDebugPad_t *pad;
    u8 editing;
    u8 blinkTick;
    s16 selection;
    struct sdfDrawTag_t *packetList;
} mdlDebugLight_t;

typedef struct mdlDebugLabel_t {
    u16 x;              /* 0x00 */
    u16 y;              /* 0x02 */
    s32 style;          /* 0x04 */
    const char *format; /* 0x08 */
} mdlDebugLabel_t;

typedef struct mdlDebugPad_t {
    u8 pad00;
    s8 confirm;
    u8 pad02;
    s8 cancel;
    s8 stepDownA;  /* 0x04 */
    s8 stepUpA;    /* 0x05 */
    s8 stepDownB;  /* 0x06 */
    s8 stepUpB;    /* 0x07 */
} mdlDebugPad_t;

typedef struct mdlDebugScale_t {
    mdlDebugPad_t *pad;
    u8 editing;
    u8 blinkTick;
    s16 selection;
    sdfDrawTag_t *packetList;
} mdlDebugScale_t;

typedef struct mdlDebugEntry_t {
    s32 y;
    f32 *target;   /* 0x04 */
    s32 min;       /* 0x08 */
    s32 max;       /* 0x0C */
    const char *label;  /* 0x10 */
    const char *format; /* 0x14 */
} mdlDebugEntry_t;

typedef union mdlDebugOptionCtrl_t {
    u32 packed;
    struct {
        u8 active;
        u8 blink;
        s16 cursor;
    } fields;
} mdlDebugOptionCtrl_t;

typedef struct mdlDebugOption_t {
    s8 *pad;
    mdlDebugOptionCtrl_t control;
    u8 pad08[0x20];
    f32 value;
    f32 adjustment;
    sdfDrawTag_t *packetList;
    u8 pad34[4];
} mdlDebugOption_t;

typedef struct { u8 hasAudio, stereo, loopMode, playbackCadenceStep; } mdlIpuParams_t;

struct mdlEffectResrc_IPU_t
{
    u32 data; /* EE byte address handed to the IPU stream initializer. */
    sdfMemHandle_t *dataMemHandle;
    u8 inUse, playing;
    u8 unk0A[6];
    s32 dataSize;
    u8 unk14[0xC];
    u8 ipuState[0x8C];
};

typedef struct
{
    mdlObj_t *model;
    s32 idA, idB;
    f32 startFrame, endFrame;
    u32 sampleInterval;
    s32 historyLength;
    u32 unk1C;
    u32 kind;
    u32 gradientColors[4];
} mdlTrackCreateParams_t;

typedef struct
{
    f32 (*lights[4])[4];
    f32 ambient[4];
    u8 unk20[0x10];
    u32 fogColor;
} mdlLightParams_t;

typedef void (*mdlOtRegFunc_t)(sdfGrOt_t *, sdfDrawTag_t *);

extern sdfGrOt_t sdfGrOtTable[];

extern s8 dds3PadButtonState[];

extern s8 D_005298D8[16];

extern f32 D_002FA240[16];

extern u8 aViewEye[];

extern mdlLightParams_t dds3LightParam;

extern mdlDebugLight_t D_0032C530;

extern mdlDebugScale_t mdlDebugScaleWork;

extern mdlDebugEntry_t mdlDebugEntryTable[];

extern mdlDebugOption_t softenBiasWork;

extern mdlDebugLabel_t D_0032C4E0[4], D_0032C548, D_0032C5B8[4];
extern u16 D_0032C510[], D_0032C5E8[4] __attribute__((section(".data")));
extern u8 corePktSoftenBias __attribute__((section(".sdata")));

extern f32 sdfFaceDivSoften, sdfFaceDivBias;

extern char *D_0032C4D0[];

extern u8 D_0032C4C8[];

extern u16 D_0032C498[];

extern void *sdfChipAllocZeroDirect(u32 aSize);

extern void sdfChipFreeDirect(void *aPtr);

extern void sdfMemFreeDirect(sdfMemHandle_t *aHandle);

extern void *dds3BillboardCreate(s32, u32);

extern void dds3BillboardDelete(mdlBillboard_t *);

extern mdlBillboard_t *dds3BillboardCopy(mdlBillboard_t *);

extern void *dds3EffectLoad(void *pD3P);

extern void dds3EffectDelete(mdlEffect_t *);

extern mdlEffect_t *dds3EffectCopy(mdlEffect_t *);

extern void sdfIpuFree(void *cb);

extern void sdfIpuSetOutputTex(void *, mdlIpuParams_t *, u8 *, s32, sdfTexHandle_t *);

extern sdfItemArray_t *sdfItemArrayNew(u32, s32, s32);

extern void sdfItemArrayFree(sdfItemArray_t *);

extern mdlMarkPos_t *sdfModelSearchMarkPosData(sdfModelRootNode_t *, s32);

extern mdlTrackWork_t *Eff_ModelTrackPolyCreate(mdlTrackCreateParams_t *);

extern void Eff_ModelTrackPolyDelete(mdlTrackWork_t *);

extern void Eff_ModelTrackPolyDraw(mdlTrackWork_t *);

extern void mdlSetBlur(mdlObj_t *, s32, s32, u32, u32);

extern sdfTexHandle_t *sdfTexSearch(s32 id);

extern sdfModelNode_t *sdfModelSearchNode(sdfModelRootNode_t *, u32);

extern void dds3BillboardSetPosition(void *, const void *);

extern void dds3BillboardDraw(mdlBillboard_t *);

extern void dds3EffectSetPosition(mdlEffect_t *, const void *);

extern void dds3EffectDraw(mdlEffect_t *);

extern void dds3BillboardSetRGBA(mdlBillboard_t *, s32);

extern void dds3EffectSetRGBA(mdlEffect_t *, s32);

extern void dds3BillboardSetScale(mdlBillboard_t *, f32, f32);

extern void dds3EffectSetScale(mdlEffect_t *, f32);

extern void *sdfDevPrintf(s32, s32, s32, s32, const char *, ...);

extern void sdfDrawTagLink(sdfDrawTag_t *, u32);

extern void *dds3PrimFrameRect(s32, s32, s32, s32, s32, u32, u32);

extern void dds3DebugIntEditEx(void *, s32, s64, s64, s64, s64, s8 *);

extern void *dds3FontReqAllocDrawTag(void);

extern void FromRotation(f32);

extern void ToMatrix(void);

extern s32 dds3DebugFloatEditEx(f32 *, u8 *, f32, f32, f32, f32);

extern s32 dds3DebugColorEditEx(u32 *, s32, s8 *);

extern void dds3PrimDebugTextBox(sdfDrawTag_t *, s32, s32, s32, f32 *);

extern void dds3PrimDebugTextBoxRGB(void *, s32, s32, s32, u32, s32);

extern void *dds3PrimLine3D(s32, s32, s32, s32);

/* dds3ModelEffectCtrlr_t.type: which effect a controller drives. */
typedef enum dds3ModelEffectType {
    dds3ModelEffectType_P2A = 0,
    dds3ModelEffectType_D3P = 1,
    dds3ModelEffectType_TRACK = 2,
    dds3ModelEffectType_IPU = 3
} dds3ModelEffectType;

/* pibHeaderChunk_t.kind, as mdlCreateEffect dispatches on it. */
typedef enum PIB_CHUNK_TYPE {
    PIB_CHUNK_TYPE_P2A = 1,
    PIB_CHUNK_TYPE_D3P = 2,
    PIB_CHUNK_TYPE_TRACK = 3,
    PIB_CHUNK_TYPE_BLUR = 4,
    IPU = 5,
    PIB_CHUNK_TYPE_END = 0xFFFF
} PIB_CHUNK_TYPE;

extern char D_0054F648[];

extern char D_0054F650[];

extern char D_0054F678[];

extern char D_0054F680[];

extern char D_0054F6A0[];

extern char D_0054F6A8[];

extern char D_0054F6E0[];

extern char D_0054F700[];

extern char D_0054F708[];

extern char D_0054F710[];

extern char D_0054F718[];

extern char D_0054F720[];

#define MDL_RECORD_LIST_HEADER_BYTES 8

#define MDL_RECORD_END_KIND 0xFFFF

#define MDL_MARK_COLOR_COUNT 4

#define MDL_COLOR_CHANNEL_COUNT 4

#define MDL_COLOR_BYTE_MASK 0xFF

#define MDL_COLOR_BYTE_BITS 8

#define MDL_MARK_LAST_FIELD 0x16

#define MDL_MARK_COLOR_FIRST_FIELD 7

#define MDL_PAD_REPEAT_FLAG 2

#define MDL_PART_SLOT_BYTES 0x10

#define MDL_PART_VALUE_SIZE_THRESHOLD 0x11

#define MDL_PART_BILLBOARD 0

#define MDL_PART_EFFECT 1

#define MDL_PART_OBJECT 3

/* Read the PIB file version without following its relative link. */
u32 mdlPibGetVersion(pibHeaderChunk_t *record) {
    return record->payload.word;
}

/* The PIB chunk count is the low half of the packed parameter word. */
u16 mdlPibGetChunkCount(pibHeaderChunk_t *record) {
    return record->parameter.part.count;
}

/* Follow the relative links in a resource's record table to find an ID. */
pibHeaderChunk_t *mdlPibGetChunk(mdlObj_t *handle, s32 group) {
    pibHeaderChunk_t *recordTable = handle->sub->effectData;
    pibHeaderChunk_t *recordCursor;
    s32 remainingRecords;

    if (recordTable == NULL) {
        return NULL;
    }
    recordCursor = recordTable;
    remainingRecords = mdlPibGetChunkCount(recordCursor);
    do {
        remainingRecords--;
        recordCursor = (pibHeaderChunk_t *)((s32)recordCursor + recordCursor->nextOffset);
        if (remainingRecords == -1) {
            return NULL;
        }
    } while (recordCursor->group != group);
    return recordCursor;
}

/* The first eight bytes belong to the enclosing list, not its first record. */
pibHeaderChunk_t *mdlPibGetFirstChunkItem(pibHeaderChunk_t *data) {
    pibHeaderChunk_t *firstRecord = (pibHeaderChunk_t *)((u8 *)data + MDL_RECORD_LIST_HEADER_BYTES);

    if (firstRecord->group == MDL_RECORD_END_KIND) {
        firstRecord = NULL;
    }
    return firstRecord;
}

/* A terminal kind returns NULL; the link is a byte offset, not a pointer. */
pibHeaderChunk_t *mdlPibGetNextChunkItem(pibHeaderChunk_t *data) {
    pibHeaderChunk_t *nextRecord = (pibHeaderChunk_t *)((s32)data + data->nextOffset);

    if (nextRecord->group == MDL_RECORD_END_KIND) {
        nextRecord = NULL;
    }
    return nextRecord;
}

/* Count relative-offset records until the 0xffff sentinel. */
s32 mdlPibGetChunkItemCount(pibHeaderChunk_t *data) {
    s32 recordCount;
    pibHeaderChunk_t *recordCursor;

    if (data == NULL) {
        return 0;
    }
    recordCursor = mdlPibGetFirstChunkItem(data);
    recordCount = 0;
    while (recordCursor != NULL) {
        recordCount++;
        recordCursor = mdlPibGetNextChunkItem(recordCursor);
    }
    return recordCount;
}

s32 mdlPibCheckItemType(pibHeaderChunk_t *data, s32 type) {
    return data->group == type;
}

u16 mdlPibGetEffectCount(pibHeaderChunk_t *record) {
    return record->parameter.part.partIndex;
}

/* Read the PIB motion count from its packed halfword. */
u16 mdlPibGetMotionCount(const pibHeaderChunk_t *record) {
    return record->tail.half.slotCount;
}

void mdlPibDrawEffectInfo(sdfDrawTag_t *tag, s32 x, s32 y, s32 z, s32 attr, pibHeaderChunk_t *data) {
    if (data->group >= PIB_CHUNK_TYPE_P2A &&
        data->group <= IPU) {
        switch (data->group) {
        case PIB_CHUNK_TYPE_P2A:
        case PIB_CHUNK_TYPE_D3P: {
            char *label = data->group == PIB_CHUNK_TYPE_P2A ? D_0054F648 : D_0054F650;

            if (data->parameter.part.count == 1) {
                sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                                    "%s %d,%d", label, data->payload.word,
                                                                    data->parameter.part.partIndex)));
                return;
            }
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                                "%s %d~%d,%d", label, data->payload.word,
                                                                data->payload.word + data->parameter.part.count - 1,
                                                                data->parameter.part.partIndex)));
            return;
        }
        case PIB_CHUNK_TYPE_TRACK:
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                                "TRACK %d~%d", data->payload.word,
                                                                data->parameter.word)));
            return;
        case PIB_CHUNK_TYPE_BLUR:
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                                "BLUR %d,%d", data->payload.stream.selectorA,
                                                                data->payload.stream.selectorB)));
            return;
        case IPU:
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                                "IPU %08x", data->payload.word)));
            return;
        }
    } else {
        sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y, z, attr,
                                                            "UNKNOWN[%3d]", data->group)));
    }
}

/* Draw the scalar marker fields, then four colors with four byte channels each; negative selection hides the cursor. */
void mdlPibDrawTrackInfo(sdfDrawTag_t *tag, s32 x, s32 y, s32 z, mdlTrackParams_t *data, s32 cursor) {
    s32 colorY = y + 0x300;
    s32 labelX = x + 0xC0;
    u32 packedColor;
    s32 colorIndex;
    s32 channelIndex;

    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y, z, 0, "MARK0:%d", data->mark0)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0x60, z, 0, "MARK1:%d", data->mark1)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0xC0, z, 0, "START:%d", data->start)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0x120, z, 0, "END  :%d", data->end)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0x180, z, 0, "ITRVL:%d", data->interval)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0x1E0, z, 0, "FACE :%d", data->face)));
    sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, y + 0x240, z, 0, "BLEND:%d", data->blend)));
    for (colorIndex = 0; colorIndex != MDL_MARK_COLOR_COUNT; colorIndex++) {
        packedColor = data->colors[colorIndex];
        sdfDrawTagLink(tag, (u32)(dds3PrimFrameRect(x + 0x480, colorY, z, 0x300, 0x180, (packedColor & 0xFFFFFF) | 0x80000000, 0x60404040)));
        for (channelIndex = 0; channelIndex != MDL_COLOR_CHANNEL_COUNT; channelIndex++) {
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(labelX, colorY, z, D_0032C4C8[channelIndex], D_0032C4D0[channelIndex])));
            sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x + 0x240, colorY, z, 0, D_0054F678, packedColor & MDL_COLOR_BYTE_MASK)));
            packedColor >>= MDL_COLOR_BYTE_BITS;
            colorY += 0x60;
        }
        colorY += 0x60;
    }
    if (cursor >= 0) {
        sdfDrawTagLink(tag, (u32)(sdfDevPrintf(x, y + D_0032C498[cursor], z, 0, D_0054F680)));
    }
}

/* Edit the marker parameters with the pad: rows 0-6 are the scalar fields, 7-22 the color bytes. */
void mdlPibEditTrackInfo(mdlTrackParams_t *data, s16 *cursor_p) {
    s32 selectedField = *cursor_p;
    u8 *colorChannel;

    if (dds3PadButtonState[0x27] < 0) {
        selectedField = selectedField != MDL_MARK_LAST_FIELD ? selectedField + 1 : 0;
    } else if (((u8)dds3PadButtonState[0x27] & MDL_PAD_REPEAT_FLAG) != 0 && selectedField < MDL_MARK_LAST_FIELD) {
        selectedField++;
    } else if (dds3PadButtonState[0x26] < 0) {
        selectedField = selectedField != 0 ? selectedField - 1 : MDL_MARK_LAST_FIELD;
    } else if (((u8)dds3PadButtonState[0x26] & MDL_PAD_REPEAT_FLAG) != 0 && selectedField > 0) {
        selectedField--;
    } else {
        switch (selectedField) {
        case 0:
            dds3DebugIntEditEx(&data->mark0, 4, 0, 99999, 0, 1, dds3PadButtonState);
            break;
        case 1:
            dds3DebugIntEditEx(&data->mark1, 4, 0, 99999, 0, 1, dds3PadButtonState);
            break;
        case 2:
            dds3DebugIntEditEx(&data->start, 2, 0, 9999, 0, 1, dds3PadButtonState);
            break;
        case 3:
            dds3DebugIntEditEx(&data->end, 2, 0, 9999, 0, 1, dds3PadButtonState);
            break;
        case 4:
            dds3DebugIntEditEx(&data->interval, 2, 1, 10, 0, 1, dds3PadButtonState);
            break;
        case 5:
            dds3DebugIntEditEx(&data->face, 1, 1, 10, 0, 1, dds3PadButtonState);
            break;
        case 6:
            dds3DebugIntEditEx(&data->blend, 1, 0, 4, 0, 1, dds3PadButtonState);
            break;
        default:
            colorChannel = (u8 *)data->colors + selectedField - MDL_MARK_COLOR_FIRST_FIELD;
            if (dds3PadButtonState[0x25] < 0) {
                *colorChannel += 1;
            } else if (((u8)dds3PadButtonState[0x25] & MDL_PAD_REPEAT_FLAG) != 0 && *colorChannel < MDL_COLOR_BYTE_MASK) {
                *colorChannel += 1;
            } else if (dds3PadButtonState[0x24] < 0) {
                *colorChannel -= 1;
            } else if (((u8)dds3PadButtonState[0x24] & MDL_PAD_REPEAT_FLAG) != 0 && *colorChannel != 0) {
                *colorChannel -= 1;
            }
            break;
        }
    }
    *cursor_p = selectedField;
}

/* Append a newly created billboard to the next part-list slot. */
void mdlCreateEffectResrc_P2A(sdfItemArray_t *partList, void *descriptor) {
    mdlEffectResrcItem_t *partEntry = &((mdlEffectResrcItem_t *)partList->items)[partList->count];

    partEntry->count = 0;
    partEntry->type = MDL_PART_BILLBOARD;
    partEntry->handle = dds3BillboardCreate(1, (u32)descriptor);
    partList->count += 1;
}

/* Append a newly created effect to the next part-list slot. */
void mdlCreateEffectResrc_D3P(sdfItemArray_t *partList, void *descriptor) {
    mdlEffectResrcItem_t *partEntry = &((mdlEffectResrcItem_t *)partList->items)[partList->count];

    partEntry->type = MDL_PART_EFFECT;
    partEntry->count = 0;
    partEntry->handle = dds3EffectLoad(descriptor);
    partList->count += 1;
}

void mdlCreateEffectResrc_IPU(sdfItemArray_t *list, u8 *sourceAddress, sdfMemHandle_t *backingAllocation, s32 sourceSize) {
    mdlEffectResrc_IPU_t *node = sdfChipAllocZeroDirect(sizeof(*node));
    mdlEffectResrcItem_t *entry = &((mdlEffectResrcItem_t *)list->items)[list->count];

    node->dataSize = sourceSize;
    node->data = (u32)sourceAddress;
    node->dataMemHandle = backingAllocation;
    entry->type = MDL_PART_OBJECT;
    entry->count = 0;
    entry->handle = node;
    list->count += 1;
}

void mdlDeleteEffectResrc_IPU(mdlEffectResrc_IPU_t *ipu) {
    if (ipu->playing != 0) {
        sdfIpuFree(&ipu->ipuState);
    }
    sdfMemFreeDirect(ipu->dataMemHandle);
    sdfChipFreeDirect(ipu);
}

void mdlStartEffectResrc_IPU(mdlEffectResrc_IPU_t *ipu, sdfTexHandle_t *tex, mdlIpuParams_t *parm) {
    if (ipu->playing == 0) {
        ipu->playing = 1;
        sdfIpuSetOutputTex(&ipu->ipuState, parm, (u8 *)ipu->data, ipu->dataSize, tex);
    }
}

sdfItemArray_t *mdlEffectResrcListNew(u32 initCapa) {
    return sdfItemArrayNew(initCapa, 0x10, 4);
}

/* Destroy each part according to its stored kind, then release the owning request. */
void mdlEffectResrcListFree(sdfItemArray_t *efcResList) {
    s32 partIndex;

    if (efcResList != NULL) {
        for (partIndex = 0; partIndex < efcResList->count; partIndex++) {
            mdlEffectResrcItem_t *partEntry = &((mdlEffectResrcItem_t *)efcResList->items)[partIndex];

            switch (partEntry->type) {
            case MDL_PART_BILLBOARD:
                dds3BillboardDelete(partEntry->handle);
                break;
            case MDL_PART_EFFECT:
                dds3EffectDelete(partEntry->handle);
                break;
            case MDL_PART_OBJECT:
                mdlDeleteEffectResrc_IPU(partEntry->handle);
                break;
            }
        }
        sdfItemArrayFree(efcResList);
    }
}

/* Prepend a cleared, typed resource item to the owner's list. */
dds3ModelEffectCtrlr_t *mdlCreateEffectCtrlr(mdlObj_t *aHandle, s32 aType, s32 aPlayGroup) {
    dds3ModelEffectCtrlr_t *item = sdfChipAllocZeroDirect(sizeof(dds3ModelEffectCtrlr_t));
    dds3ModelEffectCtrlr_t *previousHead = aHandle->resourceItems;
    item->type = aType;
    item->next = previousHead;
    item->playGroup = aPlayGroup;
    aHandle->resourceItems = item;
    return item;
}

void *mdlEffectDup_P2A(mdlEffectResrcItem_t *efcResPtr) {
    struct mdlBillboard_t *clone = dds3BillboardCopy(efcResPtr->handle);
    efcResPtr->count = efcResPtr->count + 1;
    return clone;
}

void *mdlEffectDup_D3P(mdlEffectResrcItem_t *efcResPtr) {
    mdlEffect_t *clone = dds3EffectCopy(efcResPtr->handle);
    efcResPtr->count = efcResPtr->count + 1;
    return clone;
}

/* Resolve a 16-byte part slot; retail checks only the upper index bound. */
mdlEffectResrcItem_t *mdlGetEffectResrc(mdlObj_t *handle, s32 effectIndex) {
    sdfItemArray_t *slotTable;

    slotTable = handle->sub->effectResrcList;
    if (slotTable == NULL) {
        return NULL;
    }
    if (effectIndex >= slotTable->count) {
        return NULL;
    }
    return (mdlEffectResrcItem_t *)((u8 *)slotTable->items + effectIndex * MDL_PART_SLOT_BYTES);
}

/* Bind each consecutive record ID to a newly created part when the chunk contains it. */
void mdlCreateEffect_P2A_D3P(mdlObj_t *owner, pibChunkItem_P2A_D3P_t *partRecord, s32 motionSlotIndex, s32 resourceType, void *(*createPart)(mdlEffectResrcItem_t *)) {
    mdlEffectResrcItem_t *partSlot = mdlGetEffectResrc(owner, partRecord->effectIndex);

    if (partSlot != NULL) {
        sdfModelRootNode_t *model = owner->inner;
        s32 recordId = partRecord->markId;
        s32 remainingRecords = partRecord->count;
        f32 optionalAnchorScale = 0.0f;

        /* Retain the native 17-byte size threshold for the optional anchor scale. */
        if (partRecord->size >= MDL_PART_VALUE_SIZE_THRESHOLD) {
            optionalAnchorScale = partRecord->zOffset;
        }
        do {
            mdlMarkPos_t *chunkRecord = sdfModelSearchMarkPosData(model, recordId++);

            if (chunkRecord != NULL) {
                dds3ModelEffectCtrlr_t *resourceItem = mdlCreateEffectCtrlr(owner, resourceType, motionSlotIndex);

                resourceItem->payload.part.instance = createPart(partSlot);
                resourceItem->payload.part.slot = partSlot;
                resourceItem->payload.part.mapPositionRecord = chunkRecord;
                resourceItem->payload.part.anchorScale = optionalAnchorScale;
            }
        } while (--remainingRecords != 0);
    }
}

/* Convert the effect record to native track parameters and retain its work pointer. */
void mdlCreateEffect_TRACK(mdlObj_t *owner, pibChunkItem_TRACK_t *record, s32 motionSlotIndex) {
    mdlTrackCreateParams_t effectParams;
    dds3ModelEffectCtrlr_t *resourceItem;

    effectParams.model = owner;
    effectParams.idA = record->mark;
    effectParams.idB = record->endPositionId;
    effectParams.startFrame = record->startFrame;
    effectParams.endFrame = record->endFrame;
    effectParams.sampleInterval = record->interval;
    effectParams.historyLength = record->face;
    effectParams.unk1C = 3;
    effectParams.kind = record->blendAlgo;
    effectParams.gradientColors[0] = record->colors[0];
    effectParams.gradientColors[1] = record->colors[1];
    effectParams.gradientColors[2] = record->colors[2];
    effectParams.gradientColors[3] = record->colors[3];
    resourceItem = mdlCreateEffectCtrlr(owner, dds3ModelEffectType_TRACK, motionSlotIndex);
    resourceItem->payload.part.track = Eff_ModelTrackPolyCreate(&effectParams);
}

void mdlCreateEffect_BLUR(mdlObj_t *owner, pibHeaderChunk_t *record) {
    mdlSetBlur(owner, record->payload.stream.selectorA,
                  record->payload.stream.selectorB,
                  record->parameter.word, record->tail.word10);
}

/* Claim an unused object only after its data resource resolves, retaining record flags and deferred-init parameters. */
void mdlCreateEffect_IPU(mdlObj_t *aHandle, mdlPibIpu_t *aData, s32 aPlayGroup) {
    mdlEffectResrcItem_t *partSlot = mdlGetEffectResrc(aHandle, aData->index);

    if (partSlot != 0) {
        mdlEffectResrc_IPU_t *object = partSlot->handle;
        if (object->inUse == 0) {
            sdfTexHandle_t *texture = sdfTexSearch(aData->dataId);
            if (texture != 0) {
                dds3ModelEffectCtrlr_t *resourceItem;
                u8 *attributes;
                object->inUse = 1;
                resourceItem = mdlCreateEffectCtrlr(aHandle, dds3ModelEffectType_IPU, aPlayGroup);
                /* Fill the object and resource data before attaching the owner. */
                resourceItem->payload.object.object = object;
                attributes = resourceItem->payload.object.attributes;
                resourceItem->payload.object.texture = texture;
                resourceItem->payload.object.owner = aHandle;
                resourceItem->payload.object.minimumTime = aData->minimumTime;
                attributes[1] = 1;
                attributes[2] = aData->loopMode;
                attributes[3] = aData->playbackCadenceStep;
            }
        }
    }
}

/* Start the IPU once its owner reaches the stored motion-frame threshold. */
void mdlDrawEffect_IPU(dds3ModelEffectCtrlr_t *effect) {
    mdlEffectResrc_IPU_t *object = effect->payload.object.object;
    if (object->playing == 0) {
        s32 minimumTime = effect->payload.object.minimumTime;
        f32 motionTime = effect->payload.object.owner->first->frame;
        if ((u32)(s32)motionTime < (u32)minimumTime) {
            return;
        }
        mdlStartEffectResrc_IPU(object, effect->payload.object.texture,
                   (mdlIpuParams_t *)effect->payload.object.attributes);
    }
}

/* Dispatch the five record kinds, retaining the game's native return behavior. */
void mdlCreateEffect(mdlObj_t *owner, pibHeaderChunk_t *record, s32 motionSlotIndex) {
    switch (record->group) {
    case PIB_CHUNK_TYPE_P2A:
        mdlCreateEffect_P2A_D3P(owner, (pibChunkItem_P2A_D3P_t *)record, motionSlotIndex, dds3ModelEffectType_P2A, mdlEffectDup_P2A);
        return;
    case PIB_CHUNK_TYPE_D3P:
        mdlCreateEffect_P2A_D3P(owner, (pibChunkItem_P2A_D3P_t *)record, motionSlotIndex, dds3ModelEffectType_D3P, mdlEffectDup_D3P);
        return;
    case PIB_CHUNK_TYPE_TRACK:
        mdlCreateEffect_TRACK(owner, (pibChunkItem_TRACK_t *)record, motionSlotIndex);
        return;
    case PIB_CHUNK_TYPE_BLUR:
        mdlCreateEffect_BLUR(owner, record);
        return;
    case IPU:
        mdlCreateEffect_IPU(owner, (mdlPibIpu_t *)record, motionSlotIndex);
        break;
    }
}

/* Find the requested record list and associate each entry with this motion slot. */
void mdlGenEffect(mdlObj_t *aHandle, s32 aGroup, s32 aPlayGroup) {
    pibHeaderChunk_t *recordList = mdlPibGetChunk(aHandle, aGroup);
    if (recordList != NULL) {
        pibHeaderChunk_t *recordCursor = mdlPibGetFirstChunkItem(recordList);
        while (recordCursor != NULL) {
            mdlCreateEffect(aHandle, recordCursor, aPlayGroup);
            recordCursor = mdlPibGetNextChunkItem(recordCursor);
        }
    }
}

/* Release billboard, effect, or tracked-poly handles when applicable, then always free the list item. */
void mdlFreeEffect(dds3ModelEffectCtrlr_t *aCtrlr) {
    switch (aCtrlr->type) {
    case dds3ModelEffectType_P2A:
        dds3BillboardDelete((mdlBillboard_t *)aCtrlr->payload.part.instance);
        break;
    case dds3ModelEffectType_D3P:
        dds3EffectDelete((mdlEffect_t *)aCtrlr->payload.part.instance);
        break;
    case dds3ModelEffectType_TRACK:
        Eff_ModelTrackPolyDelete(aCtrlr->payload.part.track);
        break;
    }
    sdfChipFreeDirect((void *)aCtrlr);
}

/* Release all items for this slot without losing the link across consecutive matches. */
void mdlStopEffect(mdlObj_t *aHandle, s32 aPlayGroup) {
    dds3ModelEffectCtrlr_t **itemLink = &aHandle->resourceItems;
    dds3ModelEffectCtrlr_t *item = *itemLink;
    while (item != 0) {
        if (item->playGroup == aPlayGroup) {
            dds3ModelEffectCtrlr_t *nextItem = item->next;
            mdlFreeEffect(item);
            *itemLink = nextItem;
            item = nextItem;
        } else {
            itemLink = &item->next;
            item = item->next;
        }
    }
}

/* vu0 routine: transform an anchor, then offset it away from the eye. */
void mdlEffect_P2A_D3P_GetPos(sdfModelRootNode_t *model, dds3ModelEffectCtrlr_t *effect, f32 *vect) {
    mdlMarkPos_t *mapPosition = effect->payload.part.mapPositionRecord;
    sdfModelNode_t *drawNode = sdfModelSearchNode(model, mapPosition->nodeId);
    f32 scale = effect->payload.part.anchorScale;

    SDF_VU0_LOADMATRIX(&drawNode->tm);
        SDF_VU0_LOADVECTOR( &mapPosition->position);
    SDF_VU0_APPLYMATRIX_XYZ1();
    SDF_VU0_COPYVECTOR_A_TO_B();
        SDF_VU0_LOADVECTOR_C( aViewEye);
    SDF_VU0_SUBVECTOR_A_C();
    SDF_VU0_NORMALIZE();
    SDF_VU0_SCALEVECTOR_BITS(scale);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR_NOMEM( vect);
}

/* Update anchored billboard/effect positions, tracked polygons, or deferred object initialization by type. */
void mdlLinkEffect(mdlObj_t *owner, dds3ModelEffectCtrlr_t *anchorRecord) {
    sdfModelRootNode_t *model = owner->inner;
    f32 position[4];
    void *resourceInstance;

    switch (anchorRecord->type) {
    case dds3ModelEffectType_P2A:
        mdlEffect_P2A_D3P_GetPos(model, anchorRecord, position);
        resourceInstance = anchorRecord->payload.part.instance;
        dds3BillboardSetPosition(resourceInstance, position);
        dds3BillboardDraw((mdlBillboard_t *)resourceInstance);
        break;
    case dds3ModelEffectType_D3P:
        mdlEffect_P2A_D3P_GetPos(model, anchorRecord, position);
        resourceInstance = anchorRecord->payload.part.instance;
        dds3EffectSetPosition((mdlEffect_t *)resourceInstance, position);
        dds3EffectDraw((mdlEffect_t *)resourceInstance);
        break;
    case dds3ModelEffectType_TRACK:
        Eff_ModelTrackPolyDraw(anchorRecord->payload.part.track);
        break;
    case dds3ModelEffectType_IPU:
        mdlDrawEffect_IPU(anchorRecord);
        break;
    }
}

/* Set billboard/effect color; other controller kinds have no RGBA dispatch. */
void mdlSetEffectColorSub(mdlObj_t *aHandle, dds3ModelEffectCtrlr_t *aEffect, u32 aColor) {
    switch (aEffect->type) {
    case dds3ModelEffectType_P2A:
        dds3BillboardSetRGBA((mdlBillboard_t *)aEffect->payload.part.instance, aColor);
        return;
    case dds3ModelEffectType_D3P:
        dds3EffectSetRGBA((mdlEffect_t *)aEffect->payload.part.instance, aColor);
        break;
    }
}

/* Set billboard/effect scale; other controller kinds have no scale dispatch. */
void mdlSetEffectScaleSub(mdlObj_t *aHandle, dds3ModelEffectCtrlr_t *aEffect, float aScale) {
    switch (aEffect->type) {
    case dds3ModelEffectType_P2A:
        dds3BillboardSetScale((struct mdlBillboard_t *)aEffect->payload.part.instance, aScale, aScale);
        return;
    case dds3ModelEffectType_D3P:
        dds3EffectSetScale((mdlEffect_t *)aEffect->payload.part.instance, aScale);
        break;
    }
}

/* vu0 routine: rotate the edited light direction about the view's X/Y axes. */
void mdlSetDebugMatrix(mdlDebugLight_t *context, f32 *position)
{
    if (context->pad->stepUpA != 0) {
        SDF_VU0_LOADVECTOR_MTXCOL(D_002FA240 + 1);
        FromRotation(2.0f * 3.14159265f / 180.0f);
        ToMatrix();
        SDF_VU0_LOADVECTOR( position);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_VECTOR_W_SET_ONE();
        SDF_VU0_STOREVECTOR( position);
    } else if (context->pad->stepDownA != 0) {
        SDF_VU0_LOADVECTOR_MTXCOL(D_002FA240 + 1);
        FromRotation(-2.0f * 3.14159265f / 180.0f);
        ToMatrix();
        SDF_VU0_LOADVECTOR( position);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_VECTOR_W_SET_ONE();
        SDF_VU0_STOREVECTOR( position);
    }
    if (context->pad->stepUpB != 0) {
        SDF_VU0_LOADVECTOR_MTXCOL(D_002FA240);
        FromRotation(-2.0f * 3.14159265f / 180.0f);
        ToMatrix();
        SDF_VU0_LOADVECTOR( position);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_VECTOR_W_SET_ONE();
        SDF_VU0_STOREVECTOR( position);
    } else if (context->pad->stepDownB != 0) {
        SDF_VU0_LOADVECTOR_MTXCOL(D_002FA240);
        FromRotation(2.0f * 3.14159265f / 180.0f);
        ToMatrix();
        SDF_VU0_LOADVECTOR( position);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_VECTOR_W_SET_ONE();
        SDF_VU0_STOREVECTOR( position);
    }
}

/* Update/draw the task-selected light and ambient editor; return zero. */
s32 mdlDebugPrintEffectScale(dds3ProcessID_t *task) {
    mdlDebugLight_t *state = (mdlDebugLight_t *)dds3GetProcessData(task);
    f32 (*light)[4];
    mdlDebugLabel_t *label;
    s32 changed;
    s32 labelIndex;
    s32 lightIndex;
    s32 j;
    s32 selected;
    s32 y;
    s32 directionStyle;
    s32 directionY;
    f32 *color;

    if ((u32)state == 0) {
        state = &D_0032C530;
        state->pad = (mdlDebugPad_t *)(dds3PadButtonState + 0x10);
    } else if ((u32)state == 1) {
        state = &D_0032C530;
        state->pad = (mdlDebugPad_t *)dds3PadButtonState;
    }
    if (state->pad == NULL) {
        state->pad = (mdlDebugPad_t *)(dds3PadButtonState + 0x10);
    }
    if (state->lights == NULL) {
        state->lights = dds3LightParam.lights;
    }
    if (state->ambient == NULL) {
        state->ambient = dds3LightParam.ambient;
    }
    state->packetList = dds3FontReqAllocDrawTag();
    if (state->editing == 0) {
        changed = 1;
        if (state->pad->stepUpB & 0x80) {
            if (state->selection >= 14) {
                state->selection = 0;
            } else {
                state->selection++;
            }
        } else if (state->pad->stepUpB & 2) {
            if (state->selection < 14) {
                state->selection++;
            }
        } else if (state->pad->stepDownB & 0x80) {
            if (state->selection > 0) {
                state->selection--;
            } else {
                state->selection = 14;
            }
        } else if (state->pad->stepDownB & 2) {
            if (state->selection > 0) {
                state->selection--;
            }
        } else if (state->pad->confirm & 0x80) {
            state->editing = 1;
        } else {
            changed = 0;
        }
        if (changed != 0) {
            state->blinkTick = 0;
        }
    } else {
        if (state->pad->confirm & 0x80) {
            state->editing = 0;
        } else if (state->pad->cancel & 0x80) {
            state->editing = 0;
        } else if (state->selection < 12) {
            light = state->lights[state->selection >> 2];
            if (light != NULL) {
                if ((state->selection & 3) == 3) {
                    mdlSetDebugMatrix(state, light[1]);
                } else {
                    dds3DebugFloatEditEx(&light[0][state->selection & 3], (u8 *)state->pad,
                                      0.0f, 2.0f, 0.1f, 0.01f);
                }
            }
        } else if (state->selection < 15) {
            dds3DebugFloatEditEx(&state->ambient[state->selection - 12], (u8 *)state->pad,
                              0.0f, 2.0f, 0.1f, 0.01f);
        }
    }
    state->blinkTick++;
    if (state->blinkTick == 30) {
        state->blinkTick = 0;
    }
    sdfDrawTagLink(state->packetList,
                    (u32)dds3PrimFrameRect(0x8650, 0x79A8, 0xFF007F, 0x8A0, 0xAB0,
                                  0x60000000, 0x40806020));
    label = D_0032C4E0;
    for (labelIndex = 0; labelIndex != 4; labelIndex++, label++) {
        sdfDrawTagLink(state->packetList,
                        (u32)sdfDevPrintf(label->x, label->y,
                                                         0xFF0080, label->style,
                                                         label->format));
    }
    if (state->editing == 0 && state->blinkTick < 20) {
        sdfDrawTagLink(state->packetList,
                        (u32)sdfDevPrintf(0x8680, D_0032C510[state->selection],
                                                         0xFF0080, 0, D_0054F6A0));
    }
    selected = state->selection;
    y = 0x7A20;
    for (lightIndex = 0; lightIndex != 3; lightIndex++, selected -= 4, y += 0x300) {
        light = state->lights[lightIndex];
        if (light != NULL) {
            SDF_VU0_LOADVECTOR( light[1]);
            directionY = y + 0x1B0;
            directionStyle = 0;
            if (state->editing == 1) {
                directionStyle = selected == 3;
            }
            sdfDrawTagLink(state->packetList,
                            (u32)dds3PrimLine3D(0x88C0, directionY, 0xFF0080, directionStyle));
            for (j = 0; j != 3; j++) {
                sdfDrawTagLink(state->packetList,
                                (u32)sdfDevPrintf(0x8A40, y + 0x120 + j * 0x60, 0xFF0080,
                                                                 0, D_0054F6A8, light[1][j]));
            }
            color = light[0];
        } else {
            color = NULL;
        }
        dds3PrimDebugTextBox(state->packetList, 0x8740, y,
                          state->editing == 1 ? selected : -1, color);
    }
    dds3PrimDebugTextBox(state->packetList, 0x8740, 0x8320,
                      state->editing == 1 ? state->selection - 12 : -1, state->ambient);
    { sdfGrOt_t *ot = &sdfGrOtTable[81]; ((mdlOtRegFunc_t)ot->regFunc)(ot, state->packetList); }
    return 0;
}

void mdlDebugTextBox(s32 first, s32 second, s32 color, s32 variant) {
    s32 packedColor = color & 0xffffff;

    dds3PrimDebugTextBoxRGB(mdlDebugScaleWork.packetList, first, second,
                  (mdlDebugScaleWork.editing == 0) ? -1 : variant, packedColor | 0x80000000, 1);
}

/* Step the number being edited in table slot `index`: coarse steps (A) and fine steps (B),
   held-button repeat, wrapping from one end of [min, max] to the other on a fresh press. */
void mdlGetDebugEntry(s32 valueEditIndex) {
    mdlDebugEntry_t *edit = &mdlDebugEntryTable[valueEditIndex];
    f32 *target = edit->target;
    s32 value = (s32)(*target + 0.5f);
    s32 stepA;
    s32 stepB;
    mdlDebugPad_t *pad;
    s32 min;
    s32 max;

    if (value < 1000) {
        stepA = 10;
        stepB = 1;
    } else if (value < 10000) {
        stepA = 100;
        stepB = 10;
    } else {
        stepA = 1000;
        stepB = 100;
    }
    min = edit->min;
    pad = mdlDebugScaleWork.pad;
    max = edit->max;
    if (pad->stepUpA & 0x80) {
        if (value < max) {
            value += stepA;
            if (value > max) {
                value = max;
            }
        } else {
            value = min;
        }
    } else if (pad->stepUpA & 2) {
        value += stepA;
        if (value > max) {
            value = max;
        }
    } else if (pad->stepDownA & 0x80) {
        if (min < value) {
            value -= stepA;
            if (value < min) {
                value = min;
            }
        } else {
            value = max;
        }
    } else if (pad->stepDownA & 2) {
        value -= stepA;
        if (value < min) {
            value = min;
        }
    } else if (pad->stepUpB & 0x80) {
        if (value < max) {
            value += stepB;
            if (value > max) {
                value = max;
            }
        } else {
            value = min;
        }
    } else if (pad->stepUpB & 2) {
        value += stepB;
        if (value > max) {
            value = max;
        }
    } else if (pad->stepDownB & 0x80) {
        if (min < value) {
            value -= stepB;
            if (value < min) {
                value = min;
            }
        } else {
            value = max;
        }
    } else if (pad->stepDownB & 2) {
        value -= stepB;
        if (value < min) {
            value = min;
        }
    } else {
        return;
    }
    *target = value;
}

/* Edit the draw-vector target: four numeric rows and three packed-colour channels. */
s32 mdlDebugPrintEffectScale2(dds3ProcessID_t *task) {
    mdlDebugScale_t *state = &mdlDebugScaleWork;
    s8 *pad;
    s32 changed;
    s32 value;
    s32 i;

    if (dds3GetProcessData(task) == 0) {
        pad = D_005298D8;
    } else {
        pad = dds3PadButtonState;
    }
    state->pad = (mdlDebugPad_t *)pad;
    mdlDebugScaleWork.packetList = dds3FontReqAllocDrawTag();
    if (++mdlDebugScaleWork.blinkTick == 30) {
        mdlDebugScaleWork.blinkTick = 0;
    }
    if (mdlDebugScaleWork.editing == 0) {
        changed = 1;
        if (mdlDebugScaleWork.pad->stepUpB & 0x80) {
            if (mdlDebugScaleWork.selection >= 6) {
                mdlDebugScaleWork.selection = 0;
            } else {
                mdlDebugScaleWork.selection++;
            }
        } else if (mdlDebugScaleWork.pad->stepUpB & 2) {
            if (mdlDebugScaleWork.selection < 6) {
                mdlDebugScaleWork.selection++;
            }
        } else if (mdlDebugScaleWork.pad->stepDownB & 0x80) {
            if (mdlDebugScaleWork.selection > 0) {
                mdlDebugScaleWork.selection--;
            } else {
                mdlDebugScaleWork.selection = 6;
            }
        } else if (mdlDebugScaleWork.pad->stepDownB & 2) {
            if (mdlDebugScaleWork.selection > 0) {
                mdlDebugScaleWork.selection--;
            }
        } else if (mdlDebugScaleWork.pad->confirm & 0x80) {
            mdlDebugScaleWork.editing = 1;
        } else {
            changed = 0;
        }
        if (changed != 0) {
            mdlDebugScaleWork.blinkTick = 0;
        }
    } else {
        if (mdlDebugScaleWork.pad->confirm & 0x80) {
            mdlDebugScaleWork.editing = 0;
        } else if (mdlDebugScaleWork.pad->cancel & 0x80) {
            mdlDebugScaleWork.editing = 0;
        } else if (mdlDebugScaleWork.selection < 4) {
            mdlGetDebugEntry(mdlDebugScaleWork.selection);
        } else {
            dds3DebugColorEditEx(&dds3LightParam.fogColor, mdlDebugScaleWork.selection - 4, (s8 *)mdlDebugScaleWork.pad);
        }
    }
    sdfDrawTagLink(mdlDebugScaleWork.packetList,
                    (u32)dds3PrimFrameRect(0x8590, 0x79A8, 0xFF007F, 0x960, 0x570, 0x60000000, 0x40806020));
    sdfDrawTagLink(mdlDebugScaleWork.packetList,
                    (u32)sdfDevPrintf(D_0032C548.x, D_0032C548.y, 0xFF0080, D_0032C548.style, D_0032C548.format));
    if (mdlDebugScaleWork.editing == 0 && mdlDebugScaleWork.blinkTick < 0x14) {
        s32 y;

        if (mdlDebugScaleWork.selection < 4) {
            y = mdlDebugEntryTable[mdlDebugScaleWork.selection].y;
        } else {
            y = mdlDebugScaleWork.selection * 0x60 + 0x7AE0;
        }
        sdfDrawTagLink(mdlDebugScaleWork.packetList, (u32)sdfDevPrintf(0x85C0, y, 0xFF0080, 0, D_0054F6E0));
    }
    for (i = 0; i != 4; i++) {
        s32 y = mdlDebugEntryTable[i].y;

        sdfDrawTagLink(mdlDebugScaleWork.packetList, (u32)sdfDevPrintf(0x8680, y, 0xFF0080, 0, mdlDebugEntryTable[i].label));
        value = *mdlDebugEntryTable[i].target + 0.5f;
        sdfDrawTagLink(mdlDebugScaleWork.packetList,
                        (u32)sdfDevPrintf(0x8A40, y, 0xFF0080,
                                                          (mdlDebugScaleWork.editing == 1 && mdlDebugScaleWork.selection == i) ? 6 : 0,
                                                          mdlDebugEntryTable[i].format, value));
    }
    mdlDebugTextBox(0x8680, 0x7C60, dds3LightParam.fogColor, mdlDebugScaleWork.selection - 4);
    { sdfGrOt_t *ot = &sdfGrOtTable[81]; ((mdlOtRegFunc_t)ot->regFunc)(ot, mdlDebugScaleWork.packetList); }
    return 0;
}

/* Update/draw the FACE-DIV soften, bias and divide controls; return zero. */
s32 soften_bias_Process(dds3ProcessID_t *task) {
    mdlDebugOption_t *state = &softenBiasWork;
    mdlDebugLabel_t *label;
    s8 *pad;
    f32 value;
    f32 adjustment;
    s32 changed;
    s32 i;
    s32 valueStyle;
    s32 adjustmentStyle;
    const char *format;

    if (dds3GetProcessData(task) == 0) {
        pad = D_005298D8;
    } else {
        pad = dds3PadButtonState;
    }
    value = sdfFaceDivSoften;
    adjustment = sdfFaceDivBias;
    state->pad = pad;
    softenBiasWork.value = value;
    softenBiasWork.adjustment = adjustment;
    softenBiasWork.packetList = dds3FontReqAllocDrawTag();
    softenBiasWork.control.fields.blink++;
    if (softenBiasWork.control.fields.blink == 30) {
        softenBiasWork.control.fields.blink = 0;
    }

    if (softenBiasWork.control.fields.active == 0) {
        changed = 1;
        if (softenBiasWork.pad[7] & 0x80) {
            if (softenBiasWork.control.fields.cursor >= 2) {
                softenBiasWork.control.fields.cursor = 0;
            } else {
                softenBiasWork.control.fields.cursor++;
            }
        } else if (softenBiasWork.pad[7] & 2) {
            if (softenBiasWork.control.fields.cursor < 2) {
                softenBiasWork.control.fields.cursor++;
            }
        } else if (softenBiasWork.pad[6] & 0x80) {
            if (softenBiasWork.control.fields.cursor > 0) {
                softenBiasWork.control.fields.cursor--;
            } else {
                softenBiasWork.control.fields.cursor = 2;
            }
        } else if (softenBiasWork.pad[6] & 2) {
            if (softenBiasWork.control.fields.cursor > 0) {
                softenBiasWork.control.fields.cursor--;
            }
        } else if (softenBiasWork.pad[1] & 0x80) {
            if (softenBiasWork.control.fields.cursor == 2) {
                corePktSoftenBias ^= 1;
            } else {
                softenBiasWork.control.fields.active = 1;
            }
        } else {
            changed = 0;
        }
        if (changed != 0) {
            softenBiasWork.control.fields.blink = 0;
        }
    } else {
        if (softenBiasWork.pad[1] & 0x80) {
            softenBiasWork.control.fields.active = 0;
        } else if (softenBiasWork.pad[3] & 0x80) {
            softenBiasWork.control.fields.active = 0;
        } else {
            switch (softenBiasWork.control.fields.cursor) {
            case 0:
                dds3DebugFloatEditEx(&softenBiasWork.value, (u8 *)softenBiasWork.pad,
                                  1.0f, 50.0f, 1.0f, 0.1f);
                break;
            case 1:
                dds3DebugFloatEditEx(&softenBiasWork.adjustment, (u8 *)softenBiasWork.pad,
                                  -50.0f, 50.0f, 0.1f, 0.01f);
                break;
            }
        }
    }

    sdfDrawTagLink(softenBiasWork.packetList,
                    (u32)dds3PrimFrameRect(0x84D0, 0x79A8, 0xFF007F, 0xA20, 0x270,
                                  0x60000000, 0x40806020));
    label = D_0032C5B8;
    for (i = 0; i != 4; i++, label++) {
        sdfDrawTagLink(softenBiasWork.packetList,
                        (u32)sdfDevPrintf(label->x, label->y,
                                                         0xFF0080, label->style,
                                                         label->format));
    }
    if (softenBiasWork.control.fields.active == 0 && softenBiasWork.control.fields.blink < 20) {
        sdfDrawTagLink(softenBiasWork.packetList,
                        (u32)sdfDevPrintf(0x8500,
                                                         D_0032C5E8[softenBiasWork.control.fields.cursor],
                                                         0xFF0080, 0, D_0054F700));
    }

    /* Ignore the blink byte when testing the active mode and selected row. */
    valueStyle = (softenBiasWork.control.packed & 0xFFFF00FF) == 1 ? 6 : 0;
    sdfDrawTagLink(softenBiasWork.packetList,
                    (u32)sdfDevPrintf(0x8B00, 0x7A80, 0xFF0080,
                                                     valueStyle, D_0054F708, softenBiasWork.value));
    adjustmentStyle = (softenBiasWork.control.packed & 0xFFFF00FF) == 0x10001 ? 6 : 0;
    sdfDrawTagLink(softenBiasWork.packetList,
                    (u32)sdfDevPrintf(0x8B00, 0x7AE0, 0xFF0080,
                                                     adjustmentStyle, D_0054F710, softenBiasWork.adjustment));
    format = corePktSoftenBias == 0 ? D_0054F718 : D_0054F720;
    sdfDrawTagLink(softenBiasWork.packetList,
                    (u32)sdfDevPrintf(0x8B00, 0x7BA0, 0xFF0080,
                                                     0, format));
    { sdfGrOt_t *ot = &sdfGrOtTable[81]; ((mdlOtRegFunc_t)ot->regFunc)(ot, softenBiasWork.packetList); }
    sdfFaceDivSoften = softenBiasWork.value;
    sdfFaceDivBias = softenBiasWork.adjustment;
    return 0;
}

INCLUDE_RODATA(const s32, "model/mdlEffect", D_0053A788);

INCLUDE_RODATA(const s32, "model/mdlEffect", D_0053A798);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F648);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F650);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F658);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F660);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F668);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F670);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F678);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F680);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F688);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F690);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F698);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6A0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6A8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6B0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6B8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6C0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6C8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6D0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6D8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6E0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6E8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6F0);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F6F8);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F700);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F708);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F710);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F718);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F720);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F728);

INCLUDE_SDATA(const s32, "model/mdlEffect", dds3BitEditorTable);

INCLUDE_SDATA(const s32, "model/mdlEffect", dds3BitEditorTable2);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F734);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F738);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F740);

INCLUDE_SDATA(const s32, "model/mdlEffect", D_0054F748);

INCLUDE_SDATA(const s32, "model/mdlEffect", evtNextEventNo);

