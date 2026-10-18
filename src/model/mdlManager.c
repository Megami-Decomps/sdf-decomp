#include "common.h"
#include "model.h"
#include "motion.h"
#include "draw3d.h"
#include "ps2asm.h"

#define MDL_GROUP_COUNT 8

typedef struct mdlLoadingEntry_t
{
    struct mdlLoadingEntry_t *next;
    s32 id;
} mdlLoadingEntry_t;


typedef struct
{
    sdfModelTexList_t *resourceList;
    sdfModelDef_t *itemList;
    sdfMemHandle_t *requestAllocation;
    sdfMotionPlayBook_t *motionTable;
    sdfMemHandle_t *motionResource;
    void *partInfo;
    sdfMemHandle_t *resourceHandle;
    sdfItemArray_t *partList;
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


extern s32 mdlManagerWork;
extern mdlResrc_t *mdlResrcMajorList[MDL_GROUP_COUNT];
extern mdlLoadingEntry_t *mdlLoadingMajorList[MDL_GROUP_COUNT];

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


typedef struct fileRequest_t fileRequest_t;

extern void *fileGetLoadPtr(fileRequest_t *request);
extern sdfMemHandle_t *fileGetMemHandle(fileRequest_t *request);
extern void fileHandleFree(fileRequest_t *request);
extern sdfModelTexList_t *sdfTexSetTB(void *data);
extern sdfModelDef_t *sdfFixupModelBinary(void *data);
extern sdfMotionPlayBook_t *sdfFixupMotionBinary(void *data);

void TbMbAbLoadEnd(mdlLoadRequest_t *request);
void mdlFreeBlur(mdlObj_t *ctx);


extern mdlResrc_t *mdlLoadPrepare(u32 group, u32 id);
extern void mdlGenEffect(mdlObj_t *ctx, s32 a, s32 b);
extern void mdlFreeEffect(mdlEffectResrc_t *item);


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
    struct sdfPacWork_t *queueHead;
} filePacRequest_t;

extern char *strcpy(char *dst, const char *src);
extern void sdfTexListSetClutColor(sdfModelTexList_t *list, s32 a, s32 b, s32 c);
extern void pacFileHandleFree_(void *request);

s32 mdlPrepare(s32 group, s32 id, s32 blocking);
void mdlFreeMotionCtrlr(mdlObj_t *ctx, sdfMotionCtrlr_t *node);


extern void sdfPacInit(sdfPacState_t *state, sdfPacCallbackFunc callback);
extern void sdfPacSetStay(sdfPacState_t *state);
extern s32 sdfPacDecode(sdfPacState_t *state, const u8 *data, s32 size);
extern void sdfPacFree(sdfPacState_t *state);

sdfPacWork_t *mdlPackSet(sdfPacWork_t *work, s32 group, s32 id, s32 mode);

extern filePacRequest_t *pacLoadEx(const char *path, s32 a, s32 b, void *callback, void *arg);
extern void pacSync(filePacRequest_t *request);

void mdlFileCbFree(mdlDoneJob_t *job);

extern void sdfMotionNext(sdfMotionCtrlr_t *ctrlr);
extern void sdfModelEvalTm(sdfModelRootNode_t *root);
extern void sdfModelLinkOt(void *surfaces, sdfModelRootNode_t *root);
extern void mdlLinkEffect(mdlObj_t *ctx, mdlEffectResrc_t *item);
extern void mdlLinkBlur(mdlObj_t *ctx, sdfGrOt_t **surfaces);
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
extern mdlFileKindSlot_t modelFileKindTable[];
extern mdlFileKind_t *D_00551F04;
extern mdlHddFile_t *mdlHddFileList;
extern sdfMemHandle_t *sdfMemAllocDirect(u32 size);
extern void *sdfMemLock(sdfMemHandle_t *handle);
extern s32 strlen(const char *s);

void mdlReleaseHddFileList(void);
extern sdfMemHandle_t *D_0054F634;
extern sdfMemHandle_t *D_0054F638;
extern void sdfMemFreeHandleDirect(sdfMemHandle_t **handle);

typedef struct
{
    char bytes[8];
} mdlPathPrefix_t;

typedef struct
{
    char text[5];
} mdlWorkPrefix_t;

typedef struct
{
    u32 mode;
    u8 stat[0x3C];
    char name[0x100];
    u32 privateWords[2];
} mdlDirectoryEntry_t;

typedef struct mdlHddFileEntry_t
{
    struct mdlHddFileEntry_t *next;
    char name[47];
    u8 available[3];
    s8 extension[3][2];
} mdlHddFileEntry_t;

typedef struct mdlPibRecord_t {
    s32 kind;       /* 0x00: 0xFFFF terminates the record chain */
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
        u32 word10; /* 0x10: kind-four stream parameter */
        struct {
            u16 slotCount; /* Kind 1/2 group records: following entity-slot packets. */
            u16 unk12;
        } half;
    } tail;
} mdlPibRecord_t;

typedef struct mdlPartTagRecord_t {
    u32 tag;         /* 0x00 */
    u8 state;        /* 0x04 */
    u8 opaque05[3];  /* 0x05 */
} mdlPartTagRecord_t;


struct mdlEffectDescriptor_t;
extern const mdlPathPrefix_t D_0054F628[]; /* unit-owned eight-byte "/model/" copy */
extern const char D_0054F630[]; /* padded unit-owned ".PB" suffix */
extern const mdlWorkPrefix_t D_0054F640[]; /* padded unit-owned "work/" prefix */
extern mdlHddFileEntry_t *mdlHddFileCount; /* retail name: linked directory entries */
extern u8 sdfHddValid;
extern const char D_0054B550[]; /* newlib character classes: EOF slot, then 256 bytes */
extern s32 sceDopen(const char *name);
extern s32 sceDread(s32 handle, mdlDirectoryEntry_t *entry);
extern s32 sceDclose(s32 handle);
extern s32 memcmp(const void *first, const void *second, u32 size);
extern void *memcpy(void *dst, const void *src, u32 size);
extern void *memset(void *dst, s32 value, u32 size);
extern char *strcat(char *dst, const char *src);
extern char *strrchr(const char *text, s32 character);
extern s32 strcasecmp(const char *first, const char *second);
extern void sdfDummyFunc(void);
extern fileRequest_t *fileLoadEx(const char *name, s32 mode, u32 callback, u32 context);
extern void sdfModelCopyStat(sdfModelRootNode_t *dst, sdfModelRootNode_t *src);
extern void sdfMotionChangeBlend(sdfMotionCtrlr_t *ctrlr, s32 motion, s32 loop, f32 lead, f32 duration);
extern void mdlStopEffect(mdlObj_t *ctx, s32 slot);
extern s32 mdlPibGetEffectCount(mdlPibRecord_t *record);
extern s32 mdlPibGetMotionCount(mdlPibRecord_t *record);
extern s32 mdlPibGetVersion(mdlPibRecord_t *record);
extern sdfPacWork_t *sdfPacFreeResrc(sdfPacWork_t *work);
extern sdfMatrix4_t D_002FA240;
extern sdf3DScreenParam_t dds3ScreenParam;
extern void MultMatrixRev(sdfMatrix4_t *matrix);
extern void MultMatrixRevNoArg(void);
s32 mdlFileLoad(s32 group, s32 id, s32 arg, const char *path, void (*onComplete)(u32), u32 callbackArg);

#define MDL_CHAR_IS_LOWER(c) ((D_0054B550 + 1)[(unsigned)(c)] & 2)

static inline s32 mdlFileUpper(s32 c)
{
    return MDL_CHAR_IS_LOWER(c) ? c - 'a' + 'A' : c;
}


static inline void eeUnpackRGBAWordFloat(u32 color, f32 unit) {
    u32 rgba[4];
    rgba[0] = color;
    EE_MMI_RGBA_UNPACK(rgba, unit);
}

static inline u32 mdlPackDevSlotColor(s32 age, f32 unit) {
    u32 rgba[4];

    VU0_SEQ_MDL_PACK_DEV_SLOT_COLOR(age, unit);
    return rgba[0];
}

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
    groupNode->defaultScale = 1.0f;
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

void mdlFreeMotion(mdlObj_t *ctx, s32 searchId) {
    sdfMotionCtrlr_t *node = ctx->inner->motion;

    while (node != NULL) {
        if (node->userId == searchId) {
            mdlFreeMotionCtrlr(ctx, node);
            break;
        }
        node = node->next;
    }
}

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

char *mdlSetFileName(char *dst, const char *src) {
    *(mdlPathPrefix_t *)dst = *(mdlPathPrefix_t *)D_0054F628;
    return strcat(dst, src);
}

s32 mdlPrepare(s32 group, s32 id, s32 blocking) {
    char requestPath[0x80];
    mdlResrc_t *entity;
    mdlFileKind_t *selection;
    mdlHddFile_t *paths;
    mdlLoadRequest_t *request;
    char *extension;
    s32 packedBundle;

    WaitSema(mdlManagerWork);
    entity = mdlSearchResrc(group, id);
    if (entity != NULL) {
        SignalSema(mdlManagerWork);
        return (s32)entity;
    }
    if (mdlCheckLoading(group, id)) {
        SignalSema(mdlManagerWork);
        if (blocking != 0) {
            return (s32)mdlWaitLoadEnd(group, id);
        }
        return -1;
    }
    selection = (mdlFileKind_t *)modelFileKindTable[group].entries;
    selection += id;
    paths = &((mdlHddFile_t *)modelFileTable[selection->kind].entries)[selection->index];
    packedBundle = 0;
    extension = strrchr(paths->mbName, '.');
    if (extension != NULL) {
        packedBundle = strcasecmp(extension, D_0054F630) == 0;
    }
    mdlRegLoading(group, id);
    SignalSema(mdlManagerWork);
    if (packedBundle != 0) {
        mdlSetFileName(requestPath, paths->mbName);
        mdlFileLoad(group, id, 0x101, requestPath,
                                  (void (*)(u32))sdfDummyFunc, 0);
    } else {
        request = sdfChipAllocZeroDirect(sizeof(*request));
        request->options = 0x101;
        request->group = group;
        request->id = id;
        if (paths->abName != NULL) {
            request->deferred = 1;
        }
        if (paths->tbName != NULL) {
            request->resourceListRequested = 1;
            mdlSetFileName(requestPath, paths->tbName);
            fileLoadEx(requestPath, 0, (u32)TbLoadCallback, (u32)request);
        }
        request->itemsRequested = 1;
        mdlSetFileName(requestPath, paths->mbName);
        fileLoadEx(requestPath, 0, (u32)MbLoadCallback, (u32)request);
        if (paths->abName != NULL) {
            mdlSetFileName(requestPath, paths->abName);
            fileLoadEx(requestPath, 0, (u32)AbLoadCallback, (u32)request);
        }
    }
    if (blocking != 0) {
        return (s32)mdlWaitLoadEnd(group, id);
    }
    return 0;
}

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

void mdlSetBlur(mdlObj_t *ctx, s32 stride, s32 capacity,
                         u32 endColor, u32 startColor) {
    mdlDevList_t *deviceList;
    f32 inverseSpan;
    /* Both retail versions home the packed colors 16 bytes apart. */
    u32 startCarrier[4];
    u32 endCarrier[4];

    mdlFreeBlur(ctx);
    if (capacity > 0) {
        deviceList = sdfChipAllocZeroDirect(sizeof(mdlDevList_t));
        deviceList->growStep = 1;
        ctx->devList = deviceList;
        deviceList->usedCount = capacity;
        deviceList->capacity = capacity;
        deviceList->stride = stride;
        /* Final argument use: see docs/idioms.md, "Final argument use: mdlSetBlur". */
        deviceList->byteCount = capacity * stride;
        inverseSpan = 1.0f / deviceList->byteCount;
        startCarrier[0] = startColor;
        EE_MMI_RGBA_UNPACK(startCarrier, 0.0078125f);
        SDF_VU0_STOREVECTOR(deviceList->color);
        SDF_VU0_COPYVECTOR_A_TO_B_EXTENDED();
        endCarrier[0] = endColor;
        EE_MMI_RGBA_UNPACK(endCarrier, 0.0078125f);
        SDF_VU0_SUBVECTOR_EXTENDED();
        SDF_VU0_SCALEVECTOR_V0(inverseSpan);
        SDF_VU0_STOREVECTOR(deviceList->colorStep);
    }
}

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

void mdlLinkBlur(mdlObj_t *ctx, struct sdfGrOt_t **surfaces) {
    mdlDevList_t *list = ctx->devList;
    sdfModelRootNode_t *original = ctx->inner;
    mdlDevSlot_t *slot;
    sdfModelRootNode_t *model;
    f32 tint[4];
    s32 remaining;
    u32 color;

    if (list->usedCount != 0 && --list->growStep == 0) {
        list->usedCount--;
        list->growStep = list->stride;
        slot = sdfChipAllocDirect(sizeof(mdlDevSlot_t));
        model = sdfModelMake(ctx->sub->resourceList, ctx->sub->itemList);
        /* Finish initialization before publishing the new list head. */
        slot->next = list->first;
        slot->model = model;
        slot->remaining = 1;
        model->zBias = 1000.0f;
        list->first = slot;
    }
    eeUnpackRGBAWordFloat(original->color, 1.0f / 128.0f);
    SDF_VU0_STOREVECTOR(tint);
    slot = list->first;
    while (slot != NULL) {
        remaining = slot->remaining;
        model = slot->model;
        remaining--;
        if (remaining == 0) {
            remaining = list->byteCount;
            sdfModelCopyStat(model, original);
        } else {
            sdfModelEvalTm(model);
        }
        /* Both callbacks complete before the countdown is published. */
        slot->remaining = remaining;
        SDF_VU0_LOADVECTOR_BARRIER(list->color);
        SDF_VU0_LOADVECTOR_B(list->colorStep);
        SDF_VU0_LOADVECTOR_C(tint);
        color = mdlPackDevSlotColor(remaining - 1, 128.0f);
        model->color = color;
        sdfModelLinkOt(surfaces, model);
        slot = slot->next;
    }
}

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
    modelScale = node->defaultScale;
    ctx->inner = inner;
    scale[0] = modelScale;
    scale[1] = modelScale;
    scale[2] = modelScale;
    scale[3] = 1.0f;
    SDF_VU0_COPYVECTOR(&inner->scale, scale);
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

void mdlMotion(mdlObj_t *ctx, s32 searchId, s32 motionIndex, s32 loopEnabled,
                                     f32 blendLeadFrames, f32 blendDurationFrames) {
    sdfMotionCtrlr_t *node;
    s16 slotIndex;

    for (node = ctx->inner->motion; node != NULL; node = node->next) {
        if (node->userId == searchId) {
            break;
        }
    }
    if (node == NULL) {
        node = mdlCreateMotion(ctx, searchId);
    }
    slotIndex = node->userId2;
    ctx->slots[slotIndex] = node;
    if (slotIndex == 0) {
        ctx->first = node;
    }
    sdfMotionChangeBlend(node, motionIndex, loopEnabled, blendLeadFrames, blendDurationFrames);
    ctx->current.h.id = searchId;
    ctx->current.h.motionIndex = motionIndex;
    mdlStopEffect(ctx, slotIndex);
    mdlGenEffect(ctx, motionIndex, slotIndex);
}

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
    SDF_VU0_LOADVECTOR_BARRIER(&ctx->inner->tm.m[12]);
}

void mdlSetPosition(mdlObj_t *ctx)
{
    SDF_VU0_VECTOR_W_SET_ONE();
    SDF_VU0_STOREVECTOR(&ctx->inner->tm.m[12]);
}

void mdlGetRotate(mdlObj_t *ctx)
{
    SDF_VU0_LOADVECTOR_BARRIER(&ctx->inner->rotate);
}

void mdlSetRotate(mdlObj_t *ctx)
{
    SDF_VU0_STOREVECTOR(&ctx->inner->rotate);
    ToMatrix();
    SDF_VU0_STORE_ROW0(&ctx->inner->tm.m[0]);
    SDF_VU0_STORE_ROW1(&ctx->inner->tm.m[4]);
    SDF_VU0_STORE_ROW2(&ctx->inner->tm.m[8]);
}

void mdlGetScale(mdlObj_t *ctx)
{
    SDF_VU0_LOADVECTOR_BARRIER(&ctx->inner->scale);
}

void mdlSetScale(mdlObj_t *ctx)
{
    SDF_VU0_STOREVECTOR(&ctx->inner->scale);
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

/* vu0 routine: project the point through the view and model matrices into vf10. */
void mdlGetScreenOffset(mdlObj_t *ctx, void *point)
{
    SDF_VU0_LOADMATRIX(&D_002FA240);
    MultMatrixRev(&dds3ScreenParam.TM);
    SDF_VU0_COPYMATRIX_A_TO_B();
    SDF_VU0_LOADMATRIX(&ctx->inner->tm);
    SDF_VU0_SCALE_XYZ();
    MultMatrixRevNoArg();
    SDF_VU0_LOADVECTOR_BARRIER(point);
    SDF_VU0_APPLYMATRIX_XYZ1();
    SDF_VU0_PERSVECTOR();
    SDF_VU0_LOADVECTOR_B(&dds3ScreenParam.scale);
    SDF_VU0_MULVECTOR();
    SDF_VU0_LOADVECTOR_B(&dds3ScreenParam.trans);
    SDF_VU0_ADDVECTOR();
}

/* vu0 routine: project an array of points and store their screen coordinates. */
void mdl_1DD9C0(mdlObj_t *ctx, f32 (*inputPoints)[4], f32 (*outputPoints)[4], s32 pointCount)
{
    s32 pointIndex;

    SDF_VU0_LOADMATRIX(&ctx->inner->tm);
    SDF_VU0_LOADVECTOR_BARRIER(&ctx->inner->scale);
    SDF_VU0_SCALE_XYZ();
    MultMatrixRev(&D_002FA240);
    MultMatrixRev(&dds3ScreenParam.TM);
    for (pointIndex = 0; pointIndex < pointCount; pointIndex++) {
        SDF_VU0_LOADVECTOR_BARRIER(inputPoints[pointIndex]);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_PERSVECTOR();
        SDF_VU0_LOADVECTOR_B(&dds3ScreenParam.scale);
        SDF_VU0_MULVECTOR();
        SDF_VU0_LOADVECTOR_B(&dds3ScreenParam.trans);
        SDF_VU0_ADDVECTOR();
        SDF_VU0_STOREVECTOR(outputPoints[pointIndex]);
    }
}

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

void mdlGetFilebase(s32 selectionListIndex, s32 selectionIndex, char *destination, s32 capacity) {
    mdlFileKind_t *selection;
    mdlHddFile_t *pathEntry;
    const char *path;
    const char *basename;
    s32 pathLength;
    s32 startIndex;
    s32 endIndex;
    s32 copyLength;

    selection = (mdlFileKind_t *)modelFileKindTable[selectionListIndex].entries;
    selection += selectionIndex;
    pathEntry = (mdlHddFile_t *)modelFileTable[selection->kind].entries;
    pathEntry += selection->index;
    path = pathEntry->mbName;
    startIndex = strlen(path);
    pathLength = startIndex;
    startIndex--;

    while (1) {
        basename = &path[startIndex];
        if (*basename == '/') {
            startIndex++;
            basename = &path[startIndex];
            break;
        }
        if (startIndex == 0) {
            break;
        }
        startIndex--;
    }

    endIndex = startIndex;
    do {
        endIndex++;
        if (endIndex >= pathLength) {
            break;
        }
    } while (path[endIndex] != '.');

    copyLength = endIndex - startIndex;
    capacity--;
    if (capacity < copyLength) {
        copyLength = capacity;
    }
    memcpy(destination, basename, copyLength);
    destination[copyLength] = '\0';
}

s32 mdlPackCheck(sdfPacWork_t *work)
{
    s32 result;

    result = 0;
    if (((sdfPacPacketHeader_t *)work->header)->command == 1)
    {
        result = ((sdfPacPacketHeader_t *)work->header)->tag == 0x30424950;
    }
    return result;
}

sdfPacWork_t *mdlPackSetOld(sdfPacWork_t *work, s32 group, s32 id, s32 mode) {
    mdlLoadPayload_t setup;
    sdfItemArray_t *parts;
    s32 count;

    memset(&setup, 0, sizeof(setup));
    parts = NULL;
    while (work != NULL) {
        switch (((sdfPacPacketHeader_t *)work->header)->command) {
        case 1:
            switch (((sdfPacPacketHeader_t *)work->header)->tag) {
            case 0x30424950:
                setup.partInfo = work->data;
                setup.resourceHandle = (sdfMemHandle_t *)work->resrc;
                count = mdlPibGetEffectCount((mdlPibRecord_t *)work->data);
                if (count > 0) {
                    parts = mdlEffectResrcListNew(count);
                    setup.partList = parts;
                }
                break;
            case 0x413250:
                mdlCreateEffectResrc_P2A(parts, work->data);
                sdfMemFreeDirect((sdfMemHandle_t *)work->resrc);
                break;
            case 0x503344:
                mdlCreateEffectResrc_D3P(parts, (struct mdlEffectDescriptor_t *)work->data);
                sdfMemFreeDirect((sdfMemHandle_t *)work->resrc);
                break;
            }
            break;
        case 9:
            setup.resourceList = (sdfModelTexList_t *)work->resrc;
            break;
        case 6:
            if (setup.requestAllocation == NULL) {
                setup.requestAllocation = (sdfMemHandle_t *)work->resrc;
                setup.itemList = (sdfModelDef_t *)work->data;
            }
            break;
        case 8:
            if (setup.motionResource == NULL) {
                setup.motionResource = (sdfMemHandle_t *)work->resrc;
                setup.motionTable = (sdfMotionPlayBook_t *)work->data;
            }
            break;
        }
        work = sdfPacFreeResrc(work);
    }
    mdlRegistModelSet(group, id, mode, &setup);
    return work;
}

sdfPacWork_t *mdlPackSet(sdfPacWork_t *packetWork, s32 group, s32 id, s32 mode) {
    mdlLoadPayload_t groupSetup;
    mdlPartTagRecord_t partTagRecord;
    sdfPacPacketHeader_t *packetHeader;
    mdlPibRecord_t *record;
    mdlPibRecord_t *slotRecord;
    s32 partCount;
    sdfItemArray_t *partList;
    s32 slotCount;
    s32 objectSize;
    s32 remainingParts;
    s32 payloadValue;

    if (mdlPackCheck(packetWork) == 0) {
        return mdlPackSetOld(packetWork, group, id, mode);
    }
    record = (mdlPibRecord_t *)packetWork->data;
    payloadValue = mdlPibGetVersion(record);
    if (payloadValue <= 0) {
        return mdlPackSetOld(packetWork, group, id, mode);
    }

    memset(&groupSetup, 0, sizeof(groupSetup));
    groupSetup.partInfo = record;
    groupSetup.resourceHandle = (sdfMemHandle_t *)packetWork->resrc;
    packetWork = sdfPacFreeResrc(packetWork);
    partCount = mdlPibGetEffectCount(record);
    remainingParts = partCount;
    if (partCount > 0) {
        partList = mdlEffectResrcListNew(partCount);
        groupSetup.partList = partList;
    }
    if (remainingParts > 0) {
        do {
            switch (((sdfPacPacketHeader_t *)packetWork->header)->tag) {
            case 0x413250:
                mdlCreateEffectResrc_P2A(partList, packetWork->data);
                sdfMemFreeDirect((sdfMemHandle_t *)packetWork->resrc);
                break;
            case 0x503344:
                mdlCreateEffectResrc_D3P(partList, (struct mdlEffectDescriptor_t *)packetWork->data);
                sdfMemFreeDirect((sdfMemHandle_t *)packetWork->resrc);
                break;
            case 0x555049:
                objectSize = ((sdfPacPacketHeader_t *)packetWork->header)->decodedSize;
                if (objectSize == 0) {
                    objectSize = ((sdfPacPacketHeader_t *)packetWork->header)->size
                               + (((sdfPacPacketHeader_t *)packetWork->header)->flags & 0xF0) - 0x10;
                }
                mdlCreateEffectResrc_IPU(partList, packetWork->data,
                                    (sdfMemHandle_t *)packetWork->resrc, objectSize);
                break;
            default:
                /* Retained metadata stores: SMT3 1DE078/1DE07C and
                 * DDS1 2188E0/2188E4 (same otherwise-unused local record). */
                packetHeader = (sdfPacPacketHeader_t *)packetWork->header;
                partTagRecord.tag = packetHeader->tag;
                partTagRecord.state = 0;
                break;
            }
            packetWork = sdfPacFreeResrc(packetWork);
            remainingParts--;
        } while (remainingParts != 0);
    }

    if (((sdfPacPacketHeader_t *)packetWork->header)->command == 9) {
        groupSetup.resourceList = (sdfModelTexList_t *)packetWork->resrc;
        packetWork = sdfPacFreeResrc(packetWork);
    }
    groupSetup.requestAllocation = (sdfMemHandle_t *)packetWork->resrc;
    groupSetup.itemList = (sdfModelDef_t *)packetWork->data;
    packetWork = sdfPacFreeResrc(packetWork);
    mdlRegistModelSet(group, id, mode, &groupSetup);

    slotCount = mdlPibGetMotionCount(record);
    while (slotCount > 0) {
        slotRecord = (mdlPibRecord_t *)(packetWork->data - (u32)&((mdlPibRecord_t *)0)->tail);
        mdlRegistMotion(group, id, mode, (s16)slotRecord->parameter.word,
                                      slotRecord->payload.stream.selectorA,
                                      slotRecord->payload.stream.selectorB,
                                      (sdfMotionPlayBook_t *)packetWork->data,
                                      (sdfMemHandle_t *)packetWork->resrc);
        packetWork = sdfPacFreeResrc(packetWork);
        slotCount--;
    }
    return packetWork;
}

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
    sdfPacState_t state;

    sdfPacInit(&state, 0);
    if (mode & 2)
    {
        sdfPacSetStay(&state);
    }
    sdfPacDecode(&state, data, size);
    mdlPackSet(state.head, group, id, mode);
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
    modelFileKindTable[5].entries = NULL;
    modelFileKindTable[5].count = 0;
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
    modelFileKindTable[5].entries = D_00551F04;
    modelFileKindTable[5].count = 1;
    modelFileTable[5].entries = mdlHddFileList;
    modelFileTable[5].count = 1;
}

void mdlMakeHddFileList(void) {
    mdlDirectoryEntry_t directory;
    s32 index;
    s32 count;
    s32 handle;
    s32 length;
    s32 end;
    mdlHddFileEntry_t *next;
    mdlHddFileEntry_t *file;
    mdlHddFileEntry_t **link;

    if (sdfHddValid && (handle = sceDopen("pfs0:/model/work")) >= 0) {
        mdlReleaseHddFileList();
        while ((length = sceDread(handle, &directory)) > 0) {
            if ((directory.mode & 0xF000) != 0x1000 && length >= 4 && length < 49) {
                s32 kind = -1;

                if (directory.name[length - 3] == '.' &&
                    (s8)mdlFileUpper(directory.name[length - 1]) == 'B') {
                    s32 ext = directory.name[length - 2];

                    if (MDL_CHAR_IS_LOWER(ext)) {
                        ext -= 'a' - 'A';
                    }
                    switch (ext) {
                    case 'A':
                        kind = 2;
                        break;
                    case 'T':
                        kind = 1;
                        break;
                    case 'M':
                    case 'P':
                        kind = 0;
                        break;
                    }
                    if (kind >= 0) {
                        file = mdlHddFileCount;
                        while (file != NULL && memcmp(file->name, directory.name, length - 2) != 0) {
                            file = file->next;
                        }
                        if (file == NULL) {
                            file = sdfChipAllocDirect(sizeof(*file));
                            memset(file, 0, sizeof(*file));
                            memcpy(file->name, directory.name, length - 2);
                            file->name[length - 2] = 0;
                            file->next = mdlHddFileCount;
                            mdlHddFileCount = file;
                        }
                        file->available[kind] = 1;
                        file->extension[kind][0] = directory.name[length - 2];
                        file->extension[kind][1] = directory.name[length - 1];
                    }
                }
            }
        }
        count = 0;
        sceDclose(handle);
        link = &mdlHddFileCount;
        next = mdlHddFileCount;
        while (next != NULL) {
            file = next;
            next = file->next;
            if (!file->available[0]) {
                *link = next;
                mdlFreeFileItem(file);
            } else {
                if (mdlFileUpper(file->extension[0][0]) == 'P') {
                    file->available[1] = 0;
                    file->available[2] = 0;
                }
                link = &file->next;
                count++;
            }
        }
        if (count != 0) {
            index = 0;
            D_0054F634 = sdfMemAllocDirect(count * sizeof(mdlFileKind_t));
            D_00551F04 = sdfMemLock(D_0054F634);
            D_0054F638 = sdfMemAllocDirect(count * sizeof(mdlHddFile_t));
            mdlHddFileList = sdfMemLock(D_0054F638);
            memset(mdlHddFileList, 0, count * sizeof(mdlHddFile_t));
            file = mdlHddFileCount;
            while (file != NULL) {
                const char *name = file->name;
                s32 kind;

                D_00551F04[index].kind = 5;
                D_00551F04[index].index = index;
                D_00551F04[index].unk4 = 0x1000;
                D_00551F04[index].unk6 = 1000;
                length = strlen(name);
                end = length + 5;
                for (kind = 0; kind != 3; kind++) {
                    if (file->available[kind]) {
                        char *path = sdfChipAllocDirect(length + 8);

                        *(mdlWorkPrefix_t *)path = D_0054F640[0];
                        memcpy(path + 5, name, length);
                        path[end] = file->extension[kind][0];
                        path[end + 1] = file->extension[kind][1];
                        path[end + 2] = 0;
                        switch (kind) {
                        case 0:
                            mdlHddFileList[index].mbName = path;
                            break;
                        case 1:
                            mdlHddFileList[index].tbName = path;
                            break;
                        case 2:
                            mdlHddFileList[index].abName = path;
                            break;
                        }
                    }
                }
                file = file->next;
                index++;
            }
            modelFileKindTable[5].entries = D_00551F04;
            modelFileKindTable[5].count = count;
            modelFileTable[5].entries = mdlHddFileList;
            modelFileTable[5].count = count;
            return;
        }
    }
    mdlMakeHddFileListDummy();
}
INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F628);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F630);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F634);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F638);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F640);

