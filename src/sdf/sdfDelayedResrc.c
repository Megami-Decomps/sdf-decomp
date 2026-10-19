#include "common.h"
#include "sdf_types.h"

#define SDF_PENDING_LAST_SLOT 1

struct sdfDelayedResrcCB_t;

struct SdfPendingBuffer;

typedef struct sdfDelayedResrcChunk_t sdfDelayedResrcChunk_t;

/* Deferred handlers share one address-word argument at the native dispatch
 * boundary, while their C consumers interpret that word as different owners. */
typedef void (*SdfPendingCallback)();

void sdfChipFreeDirect(void *memory);

#define SDF_PENDING_BUFFER_CAPACITY 0x3F

#define SDF_PENDING_SLOT_COUNT 2

extern s8 sdfDelayedResrcFlag;

/* Interrupt-synchronized deferred callback owner (0x08). */
typedef struct sdfDelayedResrcCB_t {
    SdfPendingCallback handler;
    sdfDelayedResrcChunk_t *pending;
} sdfDelayedResrcCB_t;

/* A queued batch links its next chunk before 63 deferred owner addresses. */
typedef struct SdfPendingBuffer {
    struct SdfPendingBuffer *next;
    void *entry[SDF_PENDING_BUFFER_CAPACITY];
} SdfPendingBuffer;

extern sdfDelayedResrcChunk_t *sdfDelayedResrcLink;

void *sdfChipAllocDirect(s32 size);

extern s32 EIntr(void);

extern s32 D_005520A8[2];

extern s32 DIntr(void);

extern s32 sdfDelayedResrcSemaId;

extern s32 sdfCreateSema(u32, u32, u32);

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

struct sdfDelayedResrcChunk_t {
    sdfDelayedResrcChunk_t *next;
    sdfDelayedResrcCB_t *owner;
    SdfPendingBuffer *buffer;
    s32 remaining;
};

void sdfDelayedResrcInitCB(sdfDelayedResrcCB_t *request, SdfPendingCallback callback);

void sdfDelayedResrcEntry(sdfDelayedResrcCB_t *owner, void *entry);

sdfDelayedResrcChunk_t *sdfDelayedResrcClose(void);

void sdfDelayedResrcRelease(sdfDelayedResrcChunk_t *pendingNode);

u64 sdfDelayedResrcCheckFunc(void);

u64 sdfDelayedResrcCheck(void);

void sdfDelayedResrcInitCB(sdfDelayedResrcCB_t *request, SdfPendingCallback callback) {
    if (sdfDelayedResrcSemaId < 0) {
        sdfDelayedResrcSemaId = sdfCreateSema(1, 0x7f, 0);
    }
    request->handler = callback;
    request->pending = NULL;
}

void sdfDelayedResrcEntry(sdfDelayedResrcCB_t *owner, void *entry) {
    sdfDelayedResrcChunk_t *pendingNode;
    SdfPendingBuffer *entryBuffer;
    s32 freeEntries;

    if (entry != NULL) {
        WaitSema(sdfDelayedResrcSemaId);
        pendingNode = owner->pending;
        if (pendingNode == NULL) {
            pendingNode = (sdfDelayedResrcChunk_t *)sdfChipAllocDirect(sizeof(sdfDelayedResrcChunk_t));
            pendingNode->remaining = 0;
            pendingNode->next = sdfDelayedResrcLink;
            pendingNode->buffer = NULL;
            pendingNode->owner = owner;
            owner->pending = pendingNode;
            sdfDelayedResrcLink = pendingNode;
        }
        freeEntries = pendingNode->remaining;
        entryBuffer = pendingNode->buffer;
        if (freeEntries == 0) {
            SdfPendingBuffer *newBuffer = (SdfPendingBuffer *)sdfChipAllocDirect(sizeof(SdfPendingBuffer));
            newBuffer->next = entryBuffer;
            pendingNode->buffer = newBuffer;
            entryBuffer = newBuffer;
            freeEntries = SDF_PENDING_BUFFER_CAPACITY;
        }
        entryBuffer->entry[SDF_PENDING_BUFFER_CAPACITY - freeEntries] = entry;
        pendingNode->remaining = freeEntries - 1;
        SignalSema(sdfDelayedResrcSemaId);
    }
}

sdfDelayedResrcChunk_t *sdfDelayedResrcClose(void) {
    sdfDelayedResrcChunk_t *head = sdfDelayedResrcLink;
    sdfDelayedResrcChunk_t *node;

    sdfDelayedResrcLink = NULL;
    for (node = head; node != NULL; node = node->next) {
        node->owner->pending = NULL;
    }
    return head;
}

void sdfDelayedResrcRelease(sdfDelayedResrcChunk_t *pendingNode) {
    sdfDelayedResrcChunk_t *nextPendingNode;
    SdfPendingBuffer *entryBuffer;
    SdfPendingBuffer *nextEntryBuffer;
    SdfPendingCallback entryHandler;
    s32 entryCount;
    s32 entryIndex;

    if (pendingNode != NULL) {
        do {
            entryCount = SDF_PENDING_BUFFER_CAPACITY - pendingNode->remaining;
            entryHandler = pendingNode->owner->handler;
            entryBuffer = pendingNode->buffer;
            if (entryBuffer != NULL) {
                do {
                    entryIndex = 0;
                    do {
                        entryHandler(entryBuffer->entry[entryIndex]);
                        entryIndex++;
                    } while (entryIndex < entryCount);
                    nextEntryBuffer = entryBuffer->next;
                    sdfChipFreeDirect(entryBuffer);
                    entryBuffer = nextEntryBuffer;
                    entryCount = SDF_PENDING_BUFFER_CAPACITY;
                } while (entryBuffer != NULL);
            }
            nextPendingNode = pendingNode->next;
            sdfChipFreeDirect(pendingNode);
            pendingNode = nextPendingNode;
        } while (pendingNode != NULL);
    }
}

void sdfDelayedResrcProc(void) {
    u32 slotIndex;

    sdfDelayedResrcFlag = 1;
    WaitSema(sdfDelayedResrcSemaId);
    sdfDelayedResrcRelease((sdfDelayedResrcChunk_t *)D_005520A8[0]);
    for (slotIndex = 0; slotIndex < SDF_PENDING_LAST_SLOT; slotIndex++) {
        D_005520A8[slotIndex] = D_005520A8[slotIndex + 1];
    }
    D_005520A8[SDF_PENDING_LAST_SLOT] = (s32)sdfDelayedResrcClose();
    SignalSema(sdfDelayedResrcSemaId);
    sdfDelayedResrcFlag = 0;
}

u64 sdfDelayedResrcCheckFunc(void) {
    u32 slotIndex;
    s32 *slotCursor;
    if (sdfDelayedResrcFlag != 0) {
        return 1;
    }
    if (sdfDelayedResrcLink != NULL) {
        return 1;
    }
    slotIndex = 0;
    slotCursor = D_005520A8;
    do {
        if (*slotCursor != 0) {
            return 1;
        }
        slotCursor++;
        slotIndex++;
    } while (slotIndex < SDF_PENDING_SLOT_COUNT);
    return 0;
}

u64 sdfDelayedResrcCheck(void) {
    s64 interruptState;
    u64 pendingWork;

    interruptState = DIntr();
    pendingWork = sdfDelayedResrcCheckFunc();
    if (interruptState != 0) {
        EIntr();
    }
    return pendingWork;
}
