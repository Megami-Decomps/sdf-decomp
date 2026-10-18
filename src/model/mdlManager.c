#include "common.h"
#include "model.h"
#include "motion.h"

#define MDL_GROUP_COUNT 8
#define MDL_SLOT_COUNT 8

typedef struct
{
    s32 flags;
    s16 motionSlotIndex;
    s16 motionIndex;
    sdfMotionPlayBook_t *motionTable;
    sdfMemHandle_t *resourceHandle;
} mdlResrcSlot_t;

typedef struct mdlLoadingEntry_t
{
    struct mdlLoadingEntry_t *next;
    s32 id;
} mdlLoadingEntry_t;

typedef struct mdlResrc_t mdlResrc_t;
typedef struct mdlObj_t mdlObj_t;

typedef struct mdlEffectResrc_t
{
    struct mdlEffectResrc_t *next;
    u16 type;
    s16 motionSlotIndex;
    u8 payload[0x18];
} mdlEffectResrc_t;

struct mdlResrc_t
{
    mdlResrc_t *next;
    mdlResrc_t *prev;
    u16 group;
    u16 type;
    u8 ownsResources;
    u8 pad0D[3];
    mdlObj_t *modelContext;
    sdfModelTexList_t *resourceList;
    sdfModelDef_t *itemList;
    sdfMemHandle_t *requestAllocation;
    mdlResrcSlot_t slots[MDL_SLOT_COUNT];
    sdfMemHandle_t *resourceHandle;
    void *partInfo;
    sdfModelTexList_t *partList;
    f32 unk_AC;
    f32 objectRadius;
};

typedef struct
{
    sdfModelTexList_t *resourceList;
    sdfModelDef_t *itemList;
    sdfMemHandle_t *requestAllocation;
    sdfMotionPlayBook_t *motionTable;
    sdfMemHandle_t *motionResource;
    void *partInfo;
    sdfMemHandle_t *resourceHandle;
    sdfModelTexList_t *partList;
} mdlLoadPayload_t;

typedef struct
{
    u16 group;
    u16 id;
    u8 resourceListRequested;
    u8 itemsRequested;
    u8 deferred;
    u8 pad7;
    u32 options;
    mdlLoadPayload_t payload;
} mdlLoadRequest_t;

typedef struct mdlDevSlot_t
{
    struct mdlDevSlot_t *next;
    sdfModelRootNode_t *model;
    s32 remaining;
} mdlDevSlot_t;

typedef struct
{
    mdlDevSlot_t *first;
    s16 usedCount;
    u16 capacity;
    s16 stride;
    s16 growStep;
    s32 byteCount;
    f32 color[4];
    f32 colorStep[4];
} mdlDevList_t;

struct mdlObj_t
{
    u32 flags;
    mdlObj_t *next;
    mdlObj_t *prev;
    mdlResrc_t *sub;
    union
    {
        u32 word;
        struct
        {
            s16 id;
            s16 motionIndex;
        } h;
    } current;
    mdlEffectResrc_t *resourceItems;
    sdfModelRootNode_t *inner;
    sdfMotionCtrlr_t *first;
    sdfMotionCtrlr_t *slots[4];
    mdlDevList_t *devList;
    f32 objectRadius;
};

typedef struct
{
    s32 count;
    void *entries;
} mdlFileTableEntry_t;

extern s32 mdlManagerWork;
extern mdlResrc_t *mdlResrcMajorList[MDL_GROUP_COUNT];
extern mdlLoadingEntry_t *mdlLoadingMajorList[MDL_GROUP_COUNT];
extern mdlFileTableEntry_t modelFileKindTable[];

extern s32 sdfCreateSema(s32 initCount, s32 maxCount, s32 attr);
extern s32 WaitSema(s32 sema);
extern s32 SignalSema(s32 sema);
extern void *sdfChipAllocDirect(u32 size);
extern void *sdfChipAllocZeroDirect(u32 size);
extern void sdfChipFreeDirect(void *block);
extern void sdfMemFreeDirect(sdfMemHandle_t *handle);
extern void sdfMemFreeDelayedDirect(sdfMemHandle_t *handle);
extern void sdfTexListFree(sdfModelTexList_t *list, s32 freeData);
extern sdfModelTexList_t *sdfTexListDup(sdfModelTexList_t *list);
extern sdfModelRootNode_t *sdfModelMake(sdfModelTexList_t *texList, sdfModelDef_t *def);
extern void sdfModelRootFree(sdfModelRootNode_t *root, s32 a, s32 b);
extern sdfMotionCtrlr_t *sdfMotionCtrlrNew(sdfModelRootNode_t *root,
                                           sdfMotionPlayBook_t *playbook);
extern void sdfMotionCtrlrFree(sdfMotionCtrlr_t *ctrlr);
extern void sdfMotionPause(sdfMotionCtrlr_t *ctrlr);
extern void sdfMotionPauseOff(sdfMotionCtrlr_t *ctrlr);
extern void fileManager(void);
extern void mdlEffectResrcListFree(sdfModelTexList_t *list);
extern void mdlSetEffectColorSub(mdlObj_t *ctx, mdlEffectResrc_t *item, u32 value);
extern void mdlSetEffectScaleSub(mdlObj_t *ctx, mdlEffectResrc_t *item, f32 amount);
extern void ToMatrix(void);

void mdlUnRegist(s32 group, s32 type);
void mdlUnRegistResrc(mdlResrc_t *groupNode);
void mdlFree(mdlObj_t *ctx);
void mdlFreeMotion(mdlObj_t *ctx, s32 searchId);
void mdlUnRegistResrcMotion(mdlResrc_t *owner, s32 searchId);
void mdlRegistMotion(s32 group, s32 id, u32 mode, s32 motionIndex, s32 searchId, s32 slotIndex,
                     sdfMotionPlayBook_t *data, sdfMemHandle_t *resourceHandle);
mdlResrc_t *mdlSearchResrc(s32 group, s32 type);
void mdlMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex, s32 loop, f32 blendLeadFrames,
               f32 blendDurationFrames);
sdfMotionCtrlr_t *mdlGetMotionCtrlr(mdlObj_t *ctx, s32 searchId);
void mdlSetEffectColor(mdlObj_t *ctx, u32 value);

#define VU0_LOAD_VF10(addr) __asm__ volatile ( \
    ".set noreorder\n\tlqc2 $vf10, 0(%0)\n\t.set reorder" \
    : : "r" (addr) : "memory")
#define VU0_STORE_VF10(addr) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 $vf10, 0(%0)\n\t.set reorder" \
    : : "r" (addr) : "memory")
#define VU0_STORE_VF28(addr) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 $vf28, 0(%0)\n\t.set reorder" \
    : : "r" (addr) : "memory")
#define VU0_STORE_VF29(addr) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 $vf29, 0(%0)\n\t.set reorder" \
    : : "r" (addr) : "memory")
#define VU0_STORE_VF30(addr) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 $vf30, 0(%0)\n\t.set reorder" \
    : : "r" (addr) : "memory")
#define VU0_SET_VF10_W_ONE() __asm__ volatile ( \
    ".set noreorder\n\tvmove.w $vf10, $vf0\n\t.set reorder")

typedef struct fileRequest_t fileRequest_t;

extern void *fileGetLoadPtr(fileRequest_t *request);
extern sdfMemHandle_t *fileGetMemHandle(fileRequest_t *request);
extern void fileHandleFree(fileRequest_t *request);
extern sdfModelTexList_t *sdfTexSetTB(void *data);
extern sdfModelDef_t *sdfFixupModelBinary(void *data);
extern sdfMotionPlayBook_t *sdfFixupMotionBinary(void *data);

void TbMbAbLoadEnd(mdlLoadRequest_t *request);
void mdlFreeBlur(mdlObj_t *ctx);

#define MDL_COPY_VECTOR(dst, src) __asm__ volatile ( \
    ".set noreorder\n\tlq $2, 0(%1)\n\tsq $2, 0(%0)\n\t.set reorder" \
    : : "r" (dst), "r" (src) : "$2", "memory")

extern mdlResrc_t *mdlLoadPrepare(u32 group, u32 id);
extern void mdlGenEffect(mdlObj_t *ctx, s32 a, s32 b);
extern void mdlFreeEffect(mdlEffectResrc_t *item);

typedef struct
{
    u8 pad0[0x10];
    u8 command;
    u8 pad11[7];
    u32 tag;
} mdlPacHeader_t;

typedef struct
{
    u16 group;
    u16 id;
    s32 arg;
    void *owner;
    void (*done)(u32);
    u32 doneArg;
} mdlDoneJob_t;

typedef struct
{
    u8 pad0[0x60];
    struct pacWork_t *queueHead;
} filePacRequest_t;

extern char *strcpy(char *dst, const char *src);
extern void sdfTexListSetClutColor(sdfModelTexList_t *list, s32 a, s32 b, s32 c);
extern void pacFileHandleFree_(void *request);

s32 mdlPrepare(s32 group, s32 id, s32 blocking);
void mdlFreeMotionCtrlr(mdlObj_t *ctx, sdfMotionCtrlr_t *node);

typedef struct pacWork_t pacWork_t;

typedef struct
{
    u8 pad0[0x30];
    pacWork_t *queueHead;
    u8 pad34[0xC];
} pacState_t;

extern void sdfPacInit(pacState_t *state, s32 mode);
extern void sdfPacSetStay(pacState_t *state);
extern void sdfPacDecode(pacState_t *state, const u8 *data, s32 size);
extern void sdfPacFree(pacState_t *state);

pacWork_t *mdlPackSet(pacWork_t *work, s32 group, s32 id, s32 mode);

extern filePacRequest_t *pacLoadEx(const char *path, s32 a, s32 b, void *callback, void *arg);
extern void pacSync(filePacRequest_t *request);

void mdlFileCbFree(mdlDoneJob_t *job);

extern void sdfMotionNext(sdfMotionCtrlr_t *ctrlr);
extern void sdfModelEvalTm(sdfModelRootNode_t *root);
extern void sdfModelLinkOt(void *surfaces, sdfModelRootNode_t *root);
extern void mdlLinkEffect(mdlObj_t *ctx, mdlEffectResrc_t *item);
extern void mdlLinkBlur(mdlObj_t *ctx, void *surfaces);
void mdlLoopMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex);

typedef struct
{
    char *tbName;
    char *mbName;
    char *abName;
} mdlHddFile_t;

typedef struct
{
    mdlHddFile_t *entries;
    s32 count;
} mdlFileSlot_t;

typedef struct
{
    u16 kind;
    u16 index;
    u16 unk4;
    u16 unk6;
} mdlFileKind_t;

typedef struct
{
    mdlFileKind_t *entries;
    s32 count;
} mdlFileKindSlot_t;

extern mdlFileSlot_t modelFileTable[];
extern mdlFileKindSlot_t D_0032C420[];
extern mdlFileKind_t *D_00551F04;
extern mdlHddFile_t *mdlHddFileList;
extern sdfMemHandle_t *sdfMemAllocDirect(u32 size);
extern void *sdfMemLock(sdfMemHandle_t *handle);
extern s32 strlen(const char *s);

void mdlReleaseHddFileList(void);
extern sdfMemHandle_t *D_0054F634;
extern sdfMemHandle_t *D_0054F638;
extern void sdfMemFreeHandleDirect(sdfMemHandle_t **handle);

void mdlInitManager(void)
{
    s32 groupIndex;

    mdlManagerWork = sdfCreateSema(1, 0x7F, 0);
    for (groupIndex = 0; groupIndex != MDL_GROUP_COUNT; groupIndex++)
    {
        mdlResrcMajorList[groupIndex] = NULL;
        mdlLoadingMajorList[groupIndex] = NULL;
    }
}

mdlResrc_t *mdlSearchResrc(s32 group, s32 type)
{
    mdlResrc_t *entry;

    entry = mdlResrcMajorList[group];
    while (entry != NULL)
    {
        if (entry->type == type)
        {
            break;
        }
        entry = entry->next;
    }
    return entry;
}

s32 mdlCheckLoading(s32 groupIndex, s32 wantedId)
{
    mdlLoadingEntry_t *cursor;

    cursor = mdlLoadingMajorList[groupIndex];
    while (cursor != NULL)
    {
        if (cursor->id == wantedId)
        {
            return 1;
        }
        cursor = cursor->next;
    }
    return 0;
}

void mdlRegLoading(s32 groupIndex, s32 newId)
{
    mdlLoadingEntry_t *entry;
    mdlLoadingEntry_t **head;

    entry = sdfChipAllocDirect(sizeof(mdlLoadingEntry_t));
    head = &mdlLoadingMajorList[groupIndex];
    entry->id = newId;
    entry->next = *head;
    *head = entry;
}

void mdlUnRegLoading(s32 groupIndex, s32 wantedId)
{
    mdlLoadingEntry_t **link;
    mdlLoadingEntry_t *cursor;

    link = &mdlLoadingMajorList[groupIndex];
    cursor = *link;
    if (cursor == NULL)
    {
        return;
    }
    do
    {
        if (cursor->id == wantedId)
        {
            *link = cursor->next;
            sdfChipFreeDirect(cursor);
            break;
        }
        else
        {
            link = &cursor->next;
            cursor = cursor->next;
        }
    } while (cursor != NULL);
}

void mdlRegist(s32 groupIndex, s32 entityType, s32 ownershipFlags,
               sdfModelTexList_t *resourceList, sdfModelDef_t *itemList,
               sdfMemHandle_t *requestAllocation)
{
    mdlResrc_t *groupNode;
    mdlResrc_t *previousHead;
    s32 slotIndex;

    mdlUnRegist(groupIndex, entityType);
    groupNode = sdfChipAllocDirect(sizeof(mdlResrc_t));
    previousHead = mdlResrcMajorList[groupIndex];
    if (previousHead != NULL)
    {
        previousHead->prev = groupNode;
    }
    mdlResrcMajorList[groupIndex] = groupNode;
    groupNode->next = previousHead;
    groupNode->group = groupIndex;
    groupNode->type = entityType;
    groupNode->resourceList = resourceList;
    groupNode->itemList = itemList;
    groupNode->requestAllocation = requestAllocation;
    groupNode->prev = NULL;
    groupNode->modelContext = NULL;
    for (slotIndex = 0; slotIndex != MDL_SLOT_COUNT; slotIndex++)
    {
        groupNode->slots[slotIndex].flags = 0;
        groupNode->slots[slotIndex].motionTable = NULL;
        groupNode->slots[slotIndex].resourceHandle = NULL;
    }
    groupNode->ownsResources = ownershipFlags & 1;
    groupNode->resourceHandle = NULL;
    groupNode->partInfo = NULL;
    groupNode->partList = NULL;
    groupNode->unk_AC = 1.0f;
    groupNode->objectRadius = 100.0f;
}

void mdlUnRegistResrc(mdlResrc_t *groupNode)
{
    mdlResrc_t *previousNode;
    mdlResrc_t *nextNode;
    u8 ownsResources;
    s32 slotIndex;

    if (groupNode == NULL)
    {
        return;
    }
    previousNode = groupNode->prev;
    nextNode = groupNode->next;
    if (previousNode == NULL)
    {
        mdlResrcMajorList[groupNode->group] = nextNode;
    }
    else
    {
        previousNode->next = nextNode;
    }
    if (nextNode != NULL)
    {
        nextNode->prev = previousNode;
    }
    ownsResources = groupNode->ownsResources;
    groupNode->ownsResources = 0;
    if (groupNode->modelContext != NULL)
    {
        do
        {
            mdlFree(groupNode->modelContext);
        } while (groupNode->modelContext != NULL);
    }
    if (ownsResources != 0)
    {
        sdfTexListFree(groupNode->resourceList, 1);
        sdfMemFreeDelayedDirect(groupNode->requestAllocation);
        for (slotIndex = 0; slotIndex != MDL_SLOT_COUNT; slotIndex++)
        {
            if (groupNode->slots[slotIndex].resourceHandle != NULL)
            {
                sdfMemFreeDirect(groupNode->slots[slotIndex].resourceHandle);
            }
        }
    }
    mdlEffectResrcListFree(groupNode->partList);
    sdfMemFreeDirect(groupNode->resourceHandle);
    sdfChipFreeDirect(groupNode);
}

void mdlUnRegist(s32 group, s32 type)
{
    mdlUnRegistResrc(mdlSearchResrc(group, type));
}

void mdlUnRegistAll(void)
{
    u32 groupIndex;
    mdlResrc_t **groupHead;
    mdlResrc_t *groupNode;
    mdlResrc_t *nextNode;

    groupIndex = 0;
    groupHead = mdlResrcMajorList;
    do
    {
        groupNode = *groupHead;
        while (groupNode != NULL)
        {
            nextNode = groupNode->next;
            mdlUnRegistResrc(groupNode);
            groupNode = nextNode;
        }
        groupIndex++;
        groupHead++;
    } while (groupIndex < MDL_GROUP_COUNT);
}

sdfMotionCtrlr_t *mdlCreateMotion(mdlObj_t *owner, s32 searchId)
{
    sdfMotionPlayBook_t *motionTable;
    s16 motionSlotIndex;
    sdfMotionCtrlr_t *object;

    motionTable = owner->sub->slots[searchId].motionTable;
    motionSlotIndex = owner->sub->slots[searchId].motionSlotIndex;
    object = sdfMotionCtrlrNew(owner->inner, motionTable);
    object->userId = searchId;
    owner->slots[motionSlotIndex] = object;
    object->userId2 = motionSlotIndex;
    if (motionSlotIndex == 0)
    {
        owner->first = object;
    }
    return object;
}

void mdlFreeMotionCtrlr(mdlObj_t *ctx, sdfMotionCtrlr_t *node)
{
    if (ctx->slots[node->userId2] == node)
    {
        ctx->slots[node->userId2] = NULL;
    }
    sdfMotionCtrlrFree(node);
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlFreeMotion);

void mdlUnRegistResrcMotion(mdlResrc_t *owner, s32 searchId)
{
    mdlObj_t *ctx;

    if (owner != NULL)
    {
        if (owner->slots[searchId].motionTable == NULL)
        {
            return;
        }
        for (ctx = owner->modelContext; ctx != NULL; ctx = ctx->next)
        {
            mdlFreeMotion(ctx, searchId);
        }
        if (owner->ownsResources != 0)
        {
            if (owner->slots[searchId].resourceHandle != NULL)
            {
                sdfMemFreeDirect(owner->slots[searchId].resourceHandle);
            }
        }
        owner->slots[searchId].motionTable = NULL;
        owner->slots[searchId].resourceHandle = NULL;
    }
}

void mdlUnRegistMotion(s32 group, s32 id, s32 searchId)
{
    mdlResrc_t *entity;

    entity = mdlSearchResrc(group, id);
    mdlUnRegistResrcMotion(entity, searchId);
}

void mdlRegistMotion(s32 group, s32 id, u32 mode, s32 motionIndex, s32 searchId, s32 slotIndex,
                     sdfMotionPlayBook_t *data, sdfMemHandle_t *resourceHandle)
{
    mdlResrc_t *owner;
    mdlResrcSlot_t *slot;

    owner = mdlSearchResrc(group, id);
    mdlUnRegistResrcMotion(owner, searchId);
    slot = &owner->slots[searchId];
    slot->motionSlotIndex = slotIndex;
    slot->motionIndex = motionIndex;
    slot->motionTable = data;
    slot->resourceHandle = resourceHandle;
    slot->flags = 0;
    if (mode & 0x100)
    {
        slot->flags = 1;
    }
    if (mode & 0x200)
    {
        slot->flags |= 2;
    }
}

void mdlRegistModelSet(s32 group, s32 id, s32 mode, mdlLoadPayload_t *setup)
{
    mdlResrc_t *entity;

    mdlRegist(group, id, mode, setup->resourceList, setup->itemList, setup->requestAllocation);
    if (setup->motionTable != NULL)
    {
        mdlRegistMotion(group, id, mode, 0, 0, 0, setup->motionTable, setup->motionResource);
    }
    if (setup->partInfo != NULL)
    {
        entity = mdlSearchResrc(group, id);
        entity->partInfo = setup->partInfo;
        entity->resourceHandle = setup->resourceHandle;
        entity->partList = setup->partList;
    }
}

mdlResrc_t *mdlWaitLoadEnd(s32 group, s32 id)
{
    while (mdlCheckLoading(group, id))
    {
        fileManager();
    }
    return mdlSearchResrc(group, id);
}

void TbMbAbLoadEnd(mdlLoadRequest_t *request)
{
    mdlRegistModelSet(request->group, request->id, request->options, &request->payload);
    WaitSema(mdlManagerWork);
    mdlUnRegLoading(request->group, request->id);
    SignalSema(mdlManagerWork);
    sdfChipFreeDirect(request);
}

void TbLoadCallback(fileRequest_t *request, mdlLoadRequest_t *destination)
{
    destination->payload.resourceList = sdfTexSetTB(fileGetLoadPtr(request));
    sdfMemFreeDirect(fileGetMemHandle(request));
    fileHandleFree(request);
}

void MbLoadCallback(fileRequest_t *request, mdlLoadRequest_t *destination)
{
    destination->payload.requestAllocation = fileGetMemHandle(request);
    destination->payload.itemList = sdfFixupModelBinary(fileGetLoadPtr(request));
    fileHandleFree(request);
    if (destination->deferred == 0)
    {
        TbMbAbLoadEnd(destination);
    }
}

void AbLoadCallback(fileRequest_t *request, mdlLoadRequest_t *destination)
{
    destination->payload.motionResource = fileGetMemHandle(request);
    destination->payload.motionTable = sdfFixupMotionBinary(fileGetLoadPtr(request));
    fileHandleFree(request);
    TbMbAbLoadEnd(destination);
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetFileName);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPrepare);

mdlResrc_t *mdlLoadPrepare(u32 group, u32 id)
{
    return (mdlResrc_t *)mdlPrepare(group, id, 1);
}

void mdlUnlinkResrc(mdlObj_t *ctx)
{
    mdlObj_t *nextContext;
    mdlObj_t *previousContext;
    mdlResrc_t *group;

    nextContext = ctx->next;
    previousContext = ctx->prev;
    if (nextContext != NULL)
    {
        nextContext->prev = previousContext;
    }
    if (previousContext != NULL)
    {
        previousContext->next = nextContext;
    }
    else
    {
        group = ctx->sub;
        if (nextContext != NULL)
        {
            group->modelContext = nextContext;
        }
        else
        {
            group->modelContext = NULL;
            if (group->ownsResources != 0)
            {
                mdlUnRegistResrc(group);
            }
        }
    }
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetBlur);

void mdlFreeBlur(mdlObj_t *ctx)
{
    mdlDevList_t *list;
    mdlDevSlot_t *slot;
    mdlDevSlot_t *next;

    list = ctx->devList;
    if (list == NULL)
    {
        return;
    }
    next = list->first;
    while (next != NULL)
    {
        slot = next;
        next = next->next;
        sdfModelRootFree(slot->model, 1, 1);
        sdfChipFreeDirect(slot);
    }
    sdfChipFreeDirect(list);
    ctx->devList = NULL;
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlLinkBlur);

mdlObj_t *mdlLoad(u32 group, u32 id)
{
    mdlResrc_t *node;
    mdlObj_t *ctx;
    mdlObj_t *head;
    sdfModelRootNode_t *inner;
    f32 modelScale;
    f32 scale[4];
    s32 searchId;

    node = mdlLoadPrepare(group, id);
    ctx = sdfChipAllocZeroDirect(sizeof(mdlObj_t));
    ctx->sub = node;
    head = node->modelContext;
    ctx->next = head;
    if (head != NULL)
    {
        head->prev = ctx;
    }
    node->modelContext = ctx;
    inner = sdfModelMake(sdfTexListDup(node->resourceList), node->itemList);
    modelScale = node->unk_AC;
    ctx->inner = inner;
    scale[0] = modelScale;
    scale[1] = modelScale;
    scale[2] = modelScale;
    scale[3] = 1.0f;
    MDL_COPY_VECTOR(&inner->scale, scale);
    ctx->objectRadius = node->objectRadius;
    ctx->current.h.id = -1;
    ctx->current.h.motionIndex = -1;
    for (searchId = 0; searchId != MDL_SLOT_COUNT; searchId++)
    {
        if (node->slots[searchId].motionTable != NULL && (node->slots[searchId].flags & 1))
        {
            mdlCreateMotion(ctx, searchId);
            if (searchId > 0)
            {
                if (node->slots[searchId].motionIndex >= 0)
                {
                    mdlLoopMotion(ctx, searchId, node->slots[searchId].motionIndex);
                }
            }
        }
    }
    ctx->resourceItems = NULL;
    mdlGenEffect(ctx, -1, -1);
    return ctx;
}

void mdlFree(mdlObj_t *ctx)
{
    sdfModelRootNode_t *inner;
    mdlEffectResrc_t *item;
    mdlEffectResrc_t *next;

    inner = ctx->inner;
    while (inner->motion != NULL)
    {
        sdfMotionCtrlrFree(inner->motion);
    }
    sdfTexListFree(inner->texList, 1);
    for (item = ctx->resourceItems; item != NULL; item = next)
    {
        next = item->next;
        mdlFreeEffect(item);
    }
    mdlFreeBlur(ctx);
    sdfModelRootFree(inner, 1, 1);
    mdlUnlinkResrc(ctx);
    sdfChipFreeDirect(ctx);
}

void mdlLink(mdlObj_t *ctx, void *surfaces)
{
    sdfMotionCtrlr_t **slot;
    sdfModelRootNode_t *inner;
    mdlEffectResrc_t *item;
    s32 i;

    slot = ctx->slots;
    for (i = 0; i != 4; i++)
    {
        if (*slot != NULL)
        {
            if ((*slot)->stat != 0)
            {
                sdfMotionNext(*slot);
            }
        }
        slot++;
    }
    if (ctx->flags & 1)
    {
        return;
    }
    inner = ctx->inner;
    sdfModelEvalTm(inner);
    sdfModelLinkOt(surfaces, inner);
    if (ctx->flags & 2)
    {
        return;
    }
    if (ctx->flags & 4)
    {
        if ((inner->flags & 0x10) == 0)
        {
            return;
        }
    }
    for (item = ctx->resourceItems; item != NULL; item = item->next)
    {
        mdlLinkEffect(ctx, item);
    }
    if (ctx->devList == NULL)
    {
        return;
    }
    mdlLinkBlur(ctx, surfaces);
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlHideAllNode);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMotion);

void mdlLoopMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex)
{
    mdlMotion(ctx, searchId, motionIndex, 1, 0.0f, 0.0f);
}

void mdlSingleMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex)
{
    mdlMotion(ctx, searchId, motionIndex, 0, 0.0f, 0.0f);
}

void mdlSmoothLoopMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex, f32 blendLeadFrames,
                         f32 blendDurationFrames)
{
    mdlMotion(ctx, searchId, motionIndex, 1, blendLeadFrames, blendDurationFrames);
}

void mdlSmoothSingleMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex, f32 blendLeadFrames,
                           f32 blendDurationFrames)
{
    mdlMotion(ctx, searchId, motionIndex, 0, blendLeadFrames, blendDurationFrames);
}

sdfMotionCtrlr_t *mdlGetMotionCtrlr(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *node;

    for (node = ctx->inner->motion; node != NULL; node = node->next)
    {
        if (node->userId == searchId)
        {
            return node;
        }
    }
    return NULL;
}

s32 mdlGetMotionNum(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched == NULL)
    {
        return -1;
    }
    return matched->cutNumber;
}

u16 mdlGetMotionLengthEx(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched == NULL)
    {
        return 0;
    }
    return matched->frameLength;
}

s32 mdlGetMotionFrameEx(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched == NULL)
    {
        return 0;
    }
    return (s32)matched->frame;
}

s32 mdlChkMotionEndEx(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched == NULL)
    {
        return 2;
    }
    return matched->stat == 5;
}

f32 mdlGetMotionSpeedEx(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;
    f32 result;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    result = 0.0f;
    if (matched != NULL)
    {
        result = matched->speed;
    }
    return result;
}

void mdlSetMotionSpeedEx(mdlObj_t *ctx, s32 searchId, f32 frameStep)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched != NULL)
    {
        matched->speed = frameStep;
    }
}

void mdlGetPosition(mdlObj_t *ctx)
{
    VU0_LOAD_VF10(&ctx->inner->tm.m[12]);
}

void mdlSetPosition(mdlObj_t *ctx)
{
    VU0_SET_VF10_W_ONE();
    VU0_STORE_VF10(&ctx->inner->tm.m[12]);
}

void mdlGetRotate(mdlObj_t *ctx)
{
    VU0_LOAD_VF10(&ctx->inner->rotate);
}

void mdlSetRotate(mdlObj_t *ctx)
{
    VU0_STORE_VF10(&ctx->inner->rotate);
    ToMatrix();
    VU0_STORE_VF28(&ctx->inner->tm.m[0]);
    VU0_STORE_VF29(&ctx->inner->tm.m[4]);
    VU0_STORE_VF30(&ctx->inner->tm.m[8]);
}

void mdlGetScale(mdlObj_t *ctx)
{
    VU0_LOAD_VF10(&ctx->inner->scale);
}

void mdlSetScale(mdlObj_t *ctx)
{
    VU0_STORE_VF10(&ctx->inner->scale);
}

u32 mdlGetColor(mdlObj_t *ctx)
{
    return ctx->inner->color;
}

void mdlSetEffectColor(mdlObj_t *ctx, u32 value)
{
    mdlEffectResrc_t *item;

    for (item = ctx->resourceItems; item != NULL; item = item->next)
    {
        mdlSetEffectColorSub(ctx, item, value);
    }
}

void mdlSetColor(mdlObj_t *ctx, u32 value)
{
    ctx->inner->color = value;
    mdlSetEffectColor(ctx, (value & 0xFF000000) | 0x808080);
}

void mdlSetColorWithEffect(mdlObj_t *ctx, u32 value)
{
    ctx->inner->color = value;
    mdlSetEffectColor(ctx, value);
}

void mdlSetEffectScale(mdlObj_t *ctx, f32 amount)
{
    mdlEffectResrc_t *item;

    for (item = ctx->resourceItems; item != NULL; item = item->next)
    {
        mdlSetEffectScaleSub(ctx, item, amount);
    }
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetScreenOffset);

INCLUDE_ASM(const s32, "model/mdlManager", mdl_1DD9C0);

void mdlMotionPause(mdlObj_t *ctx)
{
    sdfMotionCtrlr_t *node;

    for (node = ctx->inner->motion; node != NULL; node = node->next)
    {
        sdfMotionPause(node);
    }
}

void mdlMotionPauseOff(mdlObj_t *ctx)
{
    sdfMotionCtrlr_t *node;

    for (node = ctx->inner->motion; node != NULL; node = node->next)
    {
        sdfMotionPauseOff(node);
    }
}

s32 mdlHasMotionEx(mdlObj_t *ctx, s32 searchId)
{
    return mdlGetMotionCtrlr(ctx, searchId) != NULL;
}

s32 mdlGetMajorNum(mdlObj_t *ctx)
{
    return ctx->sub->group;
}

s32 mdlGetMinorNum(mdlObj_t *ctx)
{
    return ctx->sub->type;
}

s32 mdlGetNumOfMajor(void)
{
    return 7;
}

s32 mdlGetNumOfMinor(s32 tableIndex)
{
    return modelFileKindTable[tableIndex].count;
}

s32 mdlGetNumOfCut(mdlObj_t *ctx, s32 searchId)
{
    sdfMotionCtrlr_t *matched;

    matched = mdlGetMotionCtrlr(ctx, searchId);
    if (matched == NULL)
    {
        return 0;
    }
    return matched->playbook->numOfCut;
}

void mdlSetClutColor(mdlObj_t *ctx, s32 a, s32 b, s32 c)
{
    sdfTexListSetClutColor(ctx->inner->texList, a, b, c);
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetFilebase);

s32 mdlPackCheck(mdlPacHeader_t *header)
{
    s32 result;

    result = 0;
    if (header->command == 1)
    {
        result = header->tag == 0x30424950;
    }
    return result;
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackSetOld);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackSet);

void mdlFileCbFree(mdlDoneJob_t *job)
{
    pacFileHandleFree_(job->owner);
    sdfChipFreeDirect(job);
}

void mdlFileLoadCallback(filePacRequest_t *request, mdlDoneJob_t *job)
{
    job->owner = request;
    mdlPackSet(request->queueHead, job->group, job->id, job->arg);
    WaitSema(mdlManagerWork);
    mdlUnRegLoading(job->group, job->id);
    SignalSema(mdlManagerWork);
    if (job->done != NULL)
    {
        job->done(job->doneArg);
        mdlFileCbFree(job);
    }
}

s32 mdlFileLoad(s32 group, s32 id, s32 arg, const char *path, void (*onComplete)(u32),
                u32 callbackArg)
{
    mdlDoneJob_t *job;
    filePacRequest_t *request;

    job = sdfChipAllocZeroDirect(sizeof(mdlDoneJob_t));
    job->group = group;
    job->id = id;
    job->arg = arg;
    job->doneArg = callbackArg;
    job->done = onComplete;
    request = pacLoadEx(path, 0, 0, mdlFileLoadCallback, job);
    job->owner = request;
    if (onComplete == NULL)
    {
        pacSync(request);
        mdlFileCbFree(job);
    }
    return 0;
}

void mdlPackDecode(s32 group, s32 id, s32 mode, const u8 *data, s32 size)
{
    pacState_t state;

    sdfPacInit(&state, 0);
    if (mode & 2)
    {
        sdfPacSetStay(&state);
    }
    sdfPacDecode(&state, data, size);
    mdlPackSet(state.queueHead, group, id, mode);
    sdfPacFree(&state);
}

void mdlFreeFileItem(void *item)
{
    sdfChipFreeDirect(item);
}

void mdlReleaseHddFileList(void)
{
    mdlHddFile_t *entries;
    s32 count;
    s32 i;

    count = modelFileTable[5].count;
    if (count > 0)
    {
        entries = modelFileTable[5].entries;
        i = 0;
        do
        {
            sdfChipFreeDirect(entries[i].mbName);
            sdfChipFreeDirect(entries[i].tbName);
            sdfChipFreeDirect(entries[i].abName);
            i++;
        } while (i < count);
    }
    D_0032C420[5].entries = NULL;
    D_0032C420[5].count = 0;
    modelFileTable[5].entries = NULL;
    modelFileTable[5].count = 0;
    sdfMemFreeHandleDirect(&D_0054F634);
    sdfMemFreeHandleDirect(&D_0054F638);
}

void mdlMakeHddFileListDummy(void)
{
    char *name;
    char *src;
    s32 i;

    mdlReleaseHddFileList();
    D_0054F634 = sdfMemAllocDirect(sizeof(mdlFileKind_t));
    D_00551F04 = sdfMemLock(D_0054F634);
    D_00551F04->kind = 5;
    D_00551F04->index = 0;
    D_00551F04->unk4 = 0x1000;
    D_00551F04->unk6 = 1000;
    D_0054F638 = sdfMemAllocDirect(sizeof(mdlHddFile_t));
    mdlHddFileList = sdfMemLock(D_0054F638);
    for (i = 0; i != 3; i++)
    {
        switch (i)
        {
        case 0:
            src = modelFileTable[0].entries->mbName;
            break;
        case 1:
            src = modelFileTable[0].entries->tbName;
            break;
        default:
            src = modelFileTable[0].entries->abName;
            break;
        }
        if (src == NULL)
        {
            continue;
        }
        name = sdfChipAllocDirect(strlen(src) + 1);
        strcpy(name, src);
        switch (i)
        {
        case 0:
            mdlHddFileList->mbName = name;
            break;
        case 1:
            mdlHddFileList->tbName = name;
            break;
        case 2:
            mdlHddFileList->abName = name;
            break;
        }
    }
    D_0032C420[5].entries = D_00551F04;
    D_0032C420[5].count = 1;
    modelFileTable[5].entries = mdlHddFileList;
    modelFileTable[5].count = 1;
}

INCLUDE_ASM(const s32, "model/mdlManager", mdlMakeHddFileList);
INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F628);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F630);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F634);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F638);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F640);

