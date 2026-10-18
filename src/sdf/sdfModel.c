#include "common.h"
#include "libsdf.h"
#include "model.h"
#include "draw.h"
#include "ps2asm.h"

/* Audited SDK COP2/MMI operations: ee-gcc cannot express packed
 * vector transfers or the implicit VU matrix-register convention in C. */


/* The serialized node-def block has a 16-byte count/padding prefix. */
typedef struct
{
    s32 count;
    u32 pad[3];
} sdfModelNodeDefList_t;

extern void FromEuler(f32 x, f32 y, f32 z);
extern void ToMatrix(void);
extern void MultMatrixRevNoArg(void);
extern void MultMatrixRev(void *matrix);
extern void sdf3DPrimSetTmPacket(void *packet);
extern void sdfModelSetOtCallbacks(sdfGrOt_t *ot);


extern void *sdfChipAllocDirect(s32 aSize);
extern void *sdfChipAllocZeroDirect(s32 aSize);
extern void sdfChipFreeDirect(void *aPtr);
extern void sdfMemFreeDirect(sdfMemHandle_t *aHandle);

extern sdfItemArray_t *sdfItemArrayNew(s32 aCount, s32 aElemSize, s32 aGrowStep);
extern void sdfItemArrayFree(void *aArray);
extern void sdfItemArrayGrow(void *aArray);
extern void func_002BFF98(void *aArray, s32 aCount);

extern void sdfDelayedResrcInitCB(sdfDelayedResrcCB_t *aCB, sdfDelayedResrcFreeFunc aFunc);
extern void sdfDelayedResrcEntry(sdfDelayedResrcCB_t *aCB, void *aChunk);
extern sdfDelayedResrcCB_t sdfModelDelayedResrcCB;

extern sdfMatList_t *sdfMatListMake(void *aTexList, u32 *aData);
extern void sdfMatListFreeResrc(void *aList, s32 aDelayed);
extern void sdfMatListCopyStat(void *aDst, void *aSrc);
extern sdfDmaTag_t *sdfMatMakePacket(sdfMat_t *aMat, sdfDmaTag_t *aTag, s32 aIdx);

extern volatile u8 sdfFrameId;
extern f32 sdfFaceDivSoften;
extern f32 sdfFaceDivBias;
extern const char D_00529428[];

extern void *func_002B2628(void *aData, u32 aTag);
extern void sdfModelEvalTm2(sdfModelRootNode_t *aRoot, s32 aIdx);
extern void sdfModelLinkDrawPacket(sdfModelNode_t *aNode, void *aShape, s32 aOtIndex, s32 aMode,
                                   s32 aIdx);
extern void sdfModelNodeBaseSet(sdfModelNode_t *aNode, sdfModelNodeDef_t *aDef);
extern sdfModelNode_t *sdfModelNodeNew(void);
extern void sdfModelMakeLocalTm(sdfModelNode_t *aNode);
extern sdfModelRootNode_t *sdfModelRootNew(void);
extern void sdfModelAllocNodeList(sdfModelRootNode_t *aRoot, s32 aCount);

extern void sdfDrawTagInit(sdfDrawTag_t *aTag);
extern void sdfDrawTagLink(sdfDrawTag_t *aTag, u32 aPacket);
extern void sdfPrimSetOpaquePacket(sdfPrimTransPacket_t *aPacket);
extern void sdfPrimSetSemiTransPacket(sdfPrimTransPacket_t *aPacket);
extern void sdfPrimSetAddTransPacket(sdfPrimTransPacket_t *aPacket);
extern void sdfPrimSetSubTransPacket(sdfPrimTransPacket_t *aPacket);

/* An extra-data chunk: its four-character tag and its total size in bytes. */
typedef struct
{
    u32 tag;
    u32 size;
} sdfModelChunkHeader_t;

/* One MPOS record: 64 bytes, keyed by id, naming the node it hangs off. */
typedef struct QWORD_ALIGNED
{
    s32 nodeId;
    s32 id;
    u32 pad[2];
    sdfVector4_t pos;
    sdfVector4_t dir;
    sdfVector4_t up;
} sdfModelMarkPos_t;

void sdfModelRootFreeResrc(sdfModelRootNode_t *aRoot);
void sdfModelPacketFree(sdfModelNode_t *aNode);
void sdfModelNodeFree(sdfModelNode_t *aNode);
u8 *sdfModelAllocTmPacket(sdfModelNode_t *aNode);
void sdfModelNodeUnlink(sdfModelNode_t *aNode);
void sdfModelNodeSetParent(sdfModelNode_t *aNode, sdfModelNode_t *aParent);
void sdfModelNodeAdd(sdfModelRootNode_t *aRoot, sdfModelNode_t *aParent, sdfModelNode_t *aNode);
sdfModelNode_t *sdfModelSearchNode(sdfModelRootNode_t *aRoot, s32 aId);
sdfDmaTag_t *sdfModelSetMatRefPacket(sdfModelRootNode_t *aRoot, s32 aIndex, sdfDmaTag_t *aTag,
                                     s32 aIdx);
void sdfModelMakeMorphList(sdfModelRootNode_t *aRoot, s32 aCount);
sdfModelRootNode_t *sdfModelMakeRoot(void *aTexList, sdfModelDef_t *aDef);
void sdfModelNodeSet(sdfModelNode_t *aNode, sdfModelNodeDef_t *aDef);
void sdfModelMorphListCopyStat(sdfModelMorphList_t *aDst, sdfModelMorphList_t *aSrc);
void *sdfModelGetExtraChunk(sdfModelRootNode_t *aRoot, u32 aTag);
sdfModelMarkPos_t *sdfModelSearchMarkPosData(sdfModelRootNode_t *aRoot, s32 aId);
void sdfModelMakeMarkPosMatrix(sdfModelRootNode_t *aRoot, sdfModelMarkPos_t *aData);
void sdfModelMakeMarkPos(sdfModelRootNode_t *aRoot, sdfModelMarkPos_t *aData);

void sdfModelInitSystem(void)
{
    sdfDelayedResrcInitCB(&sdfModelDelayedResrcCB, (sdfDelayedResrcFreeFunc)sdfModelRootFreeResrc);
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
sdfModelNode_t *sdfModelNodeNew(void) {
    sdfModelNode_t *node = sdfChipAllocZeroDirect(sizeof(sdfModelNode_t));

    node->color = 0x80808080;
    node->drawAttr = -1;
    node->prev = node;
    node->next = node;
    VU0_STORE_VF0(&node->rotate);
    VU0_STORE_VF0(&node->pos);
    VU0_UNIT_SCALE();
    SDF_VU0_STOREVECTOR(&node->scale);
    SDF_VU0_STOREIDENTITY(&node->localNoScaledTm);
    return node;
}

u8 *sdfModelAllocTmPacket(sdfModelNode_t *aNode)
{
    u8 *packet = aNode->tmPacket;

    if (packet == NULL) {
        packet = sdfChipAllocDirect(0x100);
        aNode->tmPacket = packet;
    }
    return packet;
}

void sdfModelPacketFree(sdfModelNode_t *aNode)
{
    sdfModelPacketHeader_t **chain = aNode->drawPacket;
    s32 i = 0;
    s32 count = 2;

    do {
        sdfModelPacketHeader_t *packet = *chain;

        while (packet != NULL) {
            sdfModelPacketHeader_t *next = packet->next;

            if (packet->memHandle != NULL) {
                sdfMemFreeDirect(packet->memHandle);
            } else {
                sdfChipFreeDirect(packet);
            }
            packet = next;
        }
        i++;
        chain++;
    } while (i != count);
}

void sdfModelNodeFree(sdfModelNode_t *aNode)
{
    sdfModelPacketFree(aNode);
    sdfChipFreeDirect(aNode->tmPacket);
    aNode->tmPacket = NULL;
    sdfChipFreeDirect(aNode);
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
sdfModelRootNode_t *sdfModelRootNew(void) {
    sdfModelRootNode_t *slot;

    slot = sdfChipAllocZeroDirect(sizeof(sdfModelRootNode_t));
    slot->nodeList = (sdfModelNodeList_t *)sdfItemArrayNew(0, 4, 0x20);
    SDF_VU0_STOREIDENTITY(&slot->tm);
    VU0_STORE_VF0(&slot->rotate);
    VU0_UNIT_SCALE();
    SDF_VU0_STOREVECTOR(&slot->scale);
    slot->drawAttr = -1;
    slot->color = 0x80808080;
    return slot;
}

void sdfModelRootFreeResrc(sdfModelRootNode_t *aRoot)
{
    s32 i;

    for (i = 0; i < aRoot->nodeList->count; i++) {
        sdfModelNodeFree(aRoot->nodeList->items[i]);
    }
    sdfItemArrayFree(aRoot->nodeList);
    sdfChipFreeDirect(aRoot);
}

void sdfModelRootFree(sdfModelRootNode_t *aRoot, s32 aDelayed, s32 aFreeMat)
{
    if (aRoot == NULL) {
        return;
    }
    if (aFreeMat != 0) {
        sdfMatListFreeResrc(aRoot->matList, 1);
    }
    if (aRoot->morphList != NULL) {
        sdfItemArrayFree(aRoot->morphList);
    }
    if (aDelayed != 0) {
        sdfDelayedResrcEntry(&sdfModelDelayedResrcCB, aRoot);
    } else {
        sdfItemArrayFree(aRoot->nodeList);
        sdfChipFreeDirect(aRoot);
    }
}

void sdfModelAllocNodeList(sdfModelRootNode_t *aRoot, s32 aCount)
{
    func_002BFF98(aRoot->nodeList, aCount);
}

void sdfModelMakeNodeList(sdfModelRootNode_t *hierarchy) {
    sdfModelNode_t **nodeArray = hierarchy->nodeList->items;
    sdfModelNode_t *currentNode = hierarchy->headNode;
    sdfModelNode_t **output;

    if (currentNode != NULL) {
        output = nodeArray;
        do {
            *output++ = currentNode;
            if (currentNode->children != NULL) {
                currentNode = currentNode->children;
            } else {
                do {
                    sdfModelNode_t *nextSibling = currentNode->next;
                    if (nextSibling != NULL) {
                        currentNode = nextSibling;
                        break;
                    }
                    currentNode = currentNode->parent;
                } while (currentNode != NULL);
            }
        } while (currentNode != NULL);
    }
}

void sdfModelNodeAdd(sdfModelRootNode_t *aRoot, sdfModelNode_t *aParent, sdfModelNode_t *aNode)
{
    s16 count;
    sdfModelNode_t **items;
    sdfModelNodeList_t *list;
    s32 newCount;

    list = aRoot->nodeList;
    count = list->count;
    newCount = count + 1;
    if ((s16)list->capacity < newCount) {
        sdfItemArrayGrow(list);
        list = aRoot->nodeList;
    }
    items = list->items;
    aNode->root = aRoot;
    list->count = newCount;
    items[count] = aNode;
    sdfModelNodeSetParent(aNode, aParent);
}

void sdfModelNodeUnlink(sdfModelNode_t *aNode)
{
    sdfModelNode_t *parent = aNode->parent;

    if (parent == NULL) {
        sdfModelRootNode_t *root = aNode->root;

        if (root->headNode == aNode) {
            root->headNode = NULL;
        }
        return;
    }
    {
        sdfModelNode_t *next = aNode->next;
        sdfModelNode_t *prev = aNode->prev;

        if (next != aNode) {
            next->prev = prev;
            prev->next = next;
            if (parent->children == aNode) {
                parent->children = next;
            }
            aNode->prev = aNode;
            aNode->next = aNode;
            return;
        }
        if (parent->children == aNode) {
            parent->children = NULL;
        }
    }
}

void sdfModelNodeSetParent(sdfModelNode_t *aNode, sdfModelNode_t *aParent)
{
    if (aParent == NULL) {
        sdfModelRootNode_t *root = aNode->root;
        sdfModelNode_t *head = root->headNode;

        if (head != aNode) {
            root->headNode = aNode;
            if (head != NULL) {
                aNode->children = head;
                head->parent = aNode;
            }
            aNode->parent = NULL;
        }
    } else if (aNode->parent != aParent) {
        sdfModelNode_t *head;

        sdfModelNodeUnlink(aNode);
        head = aParent->children;
        if (head == NULL) {
            aParent->children = aNode;
        } else {
            sdfModelNode_t *prev = head->prev;

            aNode->prev = prev;
            prev->next = aNode;
            head->prev = aNode;
            aNode->next = head;
        }
        aNode->parent = aParent;
    }
}

sdfModelNode_t *sdfModelSearchNode(sdfModelRootNode_t *aRoot, s32 aId)
{
    sdfModelNodeList_t *list = aRoot->nodeList;
    s16 count;
    sdfModelNode_t **items;
    s32 i;

    count = list->count;
    items = list->items;
    if (aRoot->flags & SDF_MODEL_ROOT_FLAG_COMPLEXNODEID) {
        for (i = 0; i < count; i++) {
            sdfModelNode_t *node = items[i];

            if (node->id == aId) {
                return node;
            }
        }
        return 0;
    }
    if ((u32)aId >= (u32)count) {
        return 0;
    }
    return items[aId];
}

sdfDmaTag_t *sdfModelSetTmRefPacket(sdfModelNode_t *aNode, sdfDmaTag_t *aTag, s32 aIdx)
{
    u32 addr = (u32)((u8 *)aNode->tmPacket + (aIdx << 7)) & 0x0FFFFFFF;

    *(u64 *)aTag = ((u64)addr << 32) | 0x30000008;
    aTag->dat1 = 0x6C07C000;
    aTag->dat0 = 0;
    return aTag + 1;
}

sdfDmaTag_t *sdfModelSetTermPacket(sdfDmaTag_t *aTag)
{
    *(u64 *)&aTag->dat0 = 0;
    *(u64 *)aTag = 0x60000000;
    return aTag + 1;
}

sdfDmaTag_t *sdfModelSetMatRefPacket(sdfModelRootNode_t *aRoot, s32 aIndex, sdfDmaTag_t *aTag,
                                     s32 aIdx)
{
    return sdfMatMakePacket(aRoot->matList->items[aIndex], aTag, aIdx);
}

s32 sdfModelGetDrawPacketSize(const u32 *aShape, s32 *aNumOfSkin, s32 *aTempQwc)
{
    s32 count;
    const sdfShapePacket_t *packet;
    s32 bytes;
    s32 skinCount;
    s32 tempQwc;

    if (aShape == 0) {
        *aNumOfSkin = 0;
        *aTempQwc = 0;
        return 0;
    }
    count = aShape[0];
    aShape++;
    bytes = 0;
    skinCount = 0;
    count &= 0xFFFF;
    tempQwc = 0;
    if (count != 0) {
        do {
            packet = (const sdfShapePacket_t *)*aShape;
            aShape++;
            switch (packet->packetType) {
            case 1:
                bytes += 0x20;
                break;
            case 2:
                bytes += (*(const u16 *)&((const sdfShapeOneSkinPacket_t *)packet)->boneCount
                          << 4) +
                         0x40;
                break;
            case 3:
                skinCount++;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                tempQwc += 0x10;
                break;
            }
            count--;
        } while (count != 0);
    }
    *aNumOfSkin += skinCount;
    *aTempQwc = tempQwc;
    return bytes != 0 ? bytes + 0x20 : 0;
}

sdfDmaTag_t *sdfModelSetNormalDrawPacket(sdfModelNode_t *aNode, sdfShapePacket_t *aPacket,
                                         sdfDmaTag_t *aTag, s32 aIdx)
{
    u32 packed = *(u32 *)&aPacket->packetSize;
    u16 matIndex = packed >> 16;
    u16 qwc = packed;
    sdfDmaTag_t *body;

    body = sdfModelSetMatRefPacket(aNode->root, matIndex, aTag, aIdx);
    body->id = 0x30;
    body->qwc = qwc;
    body->addr = (void *)((u32)aPacket->dataBody & 0x0FFFFFFF);
    body->dat0 = 0;
    body->dat1 = 0;
    return body + 1;
}

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelSetOneSkinDrawPacket);

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelLinkShapedDrawPacket);

sdfModelPacketHeader_t *sdfModelNewPacketHeader(sdfModelNode_t *aNode, s32 aOtIndex, s32 aIdx)
{
    sdfModelPacketHeader_t *packet = sdfChipAllocDirect(sizeof(*packet));
    sdfModelPacketHeader_t **head =
        (sdfModelPacketHeader_t **)((u32)aIdx * sizeof(*aNode->drawPacket) + (u32)aNode +
                                    (u32)&((sdfModelNode_t *)0)->drawPacket);
    sdfModelPacketHeader_t *next = *head;

    packet->otIndex = aOtIndex;
    *head = packet;
    packet->size = 1;
    packet->memHandle = NULL;
    packet->next = next;
    return packet;
}

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelLinkDrawPacket);

/* vu0 routine: native SDK vector/matrix operations around model state. */
void sdfModelMakeLocalTm(sdfModelNode_t *node) {
    SDF_VU0_LOADVECTOR(&node->rotate);
    ToMatrix();
    SDF_VU0_LOAD_ROW3(&node->pos);
    SDF_VU0_STOREMATRIX(&node->localNoScaledTm);
}

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelNodeBaseSet);

void sdfModelNodeSet(sdfModelNode_t *aNode, sdfModelNodeDef_t *aDef)
{
    s32 pass;
    s32 slot;
    struct sdfShapeHeader_t **cursor;
    struct sdfShapeHeader_t *shape;

    sdfModelNodeBaseSet(aNode, aDef);
    switch (aDef->format) {
    case 0:
        for (pass = 0; pass != 2; pass++) {
            for (slot = 0; slot != 3; slot++) {
                sdfModelLinkDrawPacket(aNode, aDef->shape[slot], slot, 0, pass);
            }
        }
        break;
    case 1:
        if (aDef->shape[0] != NULL) {
            for (pass = 0; pass != 2; pass++) {
                cursor = (struct sdfShapeHeader_t **)aDef->shape[0];
                while ((shape = *cursor++) != NULL) {
                    sdfModelLinkDrawPacket(aNode, shape, 0, 1, pass);
                }
            }
        }
        break;
    }
}

void sdfModelNodeSetShape(sdfModelNode_t *aNode, void *aShape, s32 aOtIndex)
{
    s32 pass;

    aNode->shape = aShape;
    sdfModelPacketFree(aNode);
    sdfModelAllocTmPacket(aNode);
    sdfModelMakeLocalTm(aNode);
    for (pass = 0; pass != 2; pass++) {
        sdfModelLinkDrawPacket(aNode, aShape, aOtIndex, 0, pass);
    }
}

void sdfModelMakeMorphList(sdfModelRootNode_t *aRoot, s32 aCount)
{
    sdfModelMorphList_t *list;
    sdfModelMorph_t *items;
    s32 i;
    s32 j;

    if (aCount > 0) {
        list = (sdfModelMorphList_t *)sdfItemArrayNew(aCount, 0x10, 1);
        aRoot->morphList = list;
        items = list->items;
        for (i = 0; i != aCount; i++) {
            for (j = 0; j != 2; j++) {
                items[i].target[j].targetId = 0;
                items[i].target[j].blend = 0.0f;
            }
        }
        list->count = aCount;
    }
}

sdfModelRootNode_t *sdfModelMakeRoot(void *assetData, sdfModelDef_t *listRef) {
    s32 itemIndex = 0;
    sdfModelRootNode_t *model = sdfModelRootNew();
    sdfModelNodeDefList_t *list;
    s32 count;
    sdfModelNodeDef_t *item;
    sdfModelNode_t *node;
    sdfModelNode_t *parent;

    model->texList = assetData;
    model->matList = sdfMatListMake(assetData, listRef->matDef);
    model->extraData = listRef->extraData;
    sdfModelMakeMorphList(model, listRef->morphCount);
    list = listRef->modelDef;
    count = list->count;
    list++;
    sdfModelAllocNodeList(model, count);
    item = (sdfModelNodeDef_t *)list;
    if (count != itemIndex) {
        do {
            node = sdfModelNodeNew();
            parent = NULL;
            if (item->parentIndex >= 0) {
                parent = ((sdfModelNode_t **)model->nodeList->items)[item->parentIndex];
            }
            sdfModelNodeAdd(model, parent, node);
            itemIndex++;
            node->id = item->id;
            item++;
        } while (itemIndex != count);
    }
    return model;
}

sdfModelRootNode_t *sdfModelMake(void *aTexList, sdfModelDef_t *aDef)
{
    s32 i = 0;
    sdfModelRootNode_t *root = sdfModelMakeRoot(aTexList, aDef);
    u32 *def = aDef->modelDef;
    s32 count = def[0];
    sdfModelNodeDef_t *nodeDef = (sdfModelNodeDef_t *)(def + 4);

    if (count != i) {
        do {
            sdfModelNodeSet(root->nodeList->items[i], nodeDef);
            nodeDef++;
            i++;
        } while (i != count);
    }
    return root;
}

sdfModelRootNode_t *sdfModelMakeEarly(void *aTexList, sdfModelDef_t *aDef)
{
    s32 i = 0;
    sdfModelRootNode_t *root = sdfModelMakeRoot(aTexList, aDef);
    u32 *def;
    s32 count;
    sdfModelNodeDef_t *nodeDef;

    root->flags |= SDF_MODEL_ROOT_FLAG_TEMPPACKET;
    def = aDef->modelDef;
    count = def[0];
    nodeDef = (sdfModelNodeDef_t *)(def + 4);
    if (count != i) {
        do {
            sdfModelNodeBaseSet(root->nodeList->items[i], nodeDef);
            nodeDef++;
            i++;
        } while (i != count);
    }
    return root;
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
void sdfModelEvalNode(sdfModelNode_t *drawNode, const f32 (*parentMatrix)[4], s32 frame) {
    f32 *xAxis = &drawNode->localNoScaledTm.m[0];
    f32 *yAxis;
    f32 *zAxis;
    f32 *scale;
    f32 *translation;
    f32 (*transformed)[4];
    u8 *workspace;
    sdfModelNode_t *child;

    SDF_VU0_LOAD_ROW0_BARRIER(xAxis);
    yAxis = &drawNode->localNoScaledTm.m[4];
    SDF_VU0_LOAD_ROW1_BARRIER(yAxis);
    zAxis = &drawNode->localNoScaledTm.m[8];
    SDF_VU0_LOAD_ROW2_BARRIER(zAxis);
    scale = (f32 *)&drawNode->scale;
    SDF_VU0_LOADVECTOR_BARRIER(scale);
    VU0_SCALE_MATRIX_ROWS_VF10();
    translation = (f32 *)&drawNode->pos;
    SDF_VU0_LOAD_ROW3_BARRIER(translation);
    SDF_VU0_LOADMATRIX_B(parentMatrix);
    MultMatrixRevNoArg();
    transformed = (f32 (*)[4])drawNode->tm.m;
    SDF_VU0_STOREMATRIX(transformed);
    workspace = drawNode->tmPacket;
    if (workspace != NULL) {
        sdf3DPrimSetTmPacket((void *)(workspace + (frame << 7)));
    }
    child = drawNode->children;
    if (child == 0) {
        return;
    }
    do {
        sdfModelEvalNode(child, (const f32 (*)[4])transformed, frame);
        child = child->next;
    } while (child != drawNode->children);
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
void sdfModelEvalTm2(sdfModelRootNode_t *model, s32 frame) {
    u128 rootMatrix[4];
    void *transform = &model->tm;
    f32 *scale;
    sdfModelNodeList_t *list;

    SDF_VU0_LOADMATRIX(transform);
    scale = (f32 *)&model->scale;
    SDF_VU0_LOADVECTOR_BARRIER(scale);
    VU0_MUL_MATRIX_ROWS_VF10();
    VU0_STORE_MATRIX_M(rootMatrix[0], rootMatrix[1], rootMatrix[2], rootMatrix[3]);
    list = model->nodeList;
    sdfModelEvalNode(((sdfModelNode_t **)list->items)[0], (const f32 (*)[4])rootMatrix, frame);
}

void sdfModelEvalTm(sdfModelRootNode_t *aRoot)
{
    sdfModelEvalTm2(aRoot, (s8)sdfFrameId);
}

void sdfModelOtInit(sdfModelOt_t *table, sdfGrOt_t *first, sdfGrOt_t *second,
                    sdfGrOt_t *third, sdfGrOt_t *fourth)
{
    table->ot[3] = fourth;
    table->ot[0] = first;
    table->ot[2] = third;
    table->ot[1] = second;
    sdfModelSetOtCallbacks(second);
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
s32 sdfModelCheckBounds(const f32 *bounds) {
    s32 flags;
    s32 result;

    VU0_CLIP_TEST_BOX_CORNERS(flags, bounds);
    result = flags & 0x3F;
    result &= (flags >> 12) & (flags >> 6);
    result &= flags >> 18;
    return result;
}

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelTempPacketLinkOt);

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelLinkOt);

void sdfModelMorphListCopyStat(sdfModelMorphList_t *aDst, sdfModelMorphList_t *aSrc)
{
    s16 count;

    if (aDst == NULL) {
        return;
    }
    count = aDst->count;
    if (count > 0) {
        memcpy(aDst->items, aSrc->items, count * 16);
    }
}

INCLUDE_ASM(const s32, "sdf/sdfModel", sdfModelCopyStat);

void sdfModelSetDrawEnv(sdfModelDrawEnv_t *aEnv)
{
    sdfModelDrawEnvSet_t *set = aEnv->set;
    s32 i;

    sdfPrimSetOpaquePacket(&aEnv->set[0].transPacket);
    sdfPrimSetSemiTransPacket(&aEnv->set[1].transPacket);
    sdfPrimSetAddTransPacket(&aEnv->set[2].transPacket);
    sdfPrimSetSubTransPacket(&aEnv->set[3].transPacket);
    for (i = 0; i != 4; i++) {
        set->transPacket.header.dmatag.dat0 = 0x11000000;
        sdfDrawTagInit(&set->drawTag);
        sdfDrawTagLink(&set->drawTag, (u32)&set->transPacket);
        set++;
    }
    sdfDrawTagInit(&aEnv->term.drawTag);
    *(u64 *)&aEnv->term.footer = 0;
    *(u64 *)&aEnv->term.footer.dat0 = 0x13000000;
    sdfDrawTagLink(&aEnv->term.drawTag, (u32)&aEnv->term.footer);
}

void sdfModelDrawEnvLinkOt(sdfGrOt_t **aOt, sdfModelDrawEnv_t *aEnv)
{
    s32 i;

    for (i = 0; i != 4; i++) {
        ((void (*)(sdfGrOt_t *, s32, sdfDrawTag_t *))aOt[i]->callbackFunc)(aOt[i], 1,
                                                                          &aEnv->set[i].drawTag);
    }
    ((void (*)(sdfGrOt_t *, sdfDrawTag_t *))aOt[3]->regFunc)(aOt[3], &aEnv->term.drawTag);
}

void *func_002B2628(void *data, u32 chunkId) {
    sdfModelChunkHeader_t *chunk = data;
    u32 currentId;

    if (chunk == NULL) {
        return NULL;
    }
    currentId = chunk->tag;
    while (currentId != 0) {
        if (currentId == chunkId) {
            return chunk;
        }
        chunk = (sdfModelChunkHeader_t *)((u8 *)chunk + chunk->size);
        currentId = chunk->tag;
    }
    return NULL;
}

void *sdfModelGetExtraChunk(sdfModelRootNode_t *aRoot, u32 aTag)
{
    return func_002B2628(aRoot->extraData, aTag);
}

s32 sdfModelGetNodeIndexByName(sdfModelRootNode_t *aRoot, const char *aName)
{
    sdfModelChunkHeader_t *chunk = sdfModelGetExtraChunk(aRoot, 0x4D4E444E);
    u8 *entry;
    u8 *end;
    u32 len;

    if (chunk == NULL) {
        return -1;
    }
    entry = (u8 *)chunk + 8;
    end = (u8 *)chunk + chunk->size;
    len = strlen(aName);
    do {
        u32 entryLen = strlen((char *)entry);
        u8 *id = (u8 *)(((u32)(entry + entryLen + 4)) & ~3U);

        if (entryLen == len && memcmp(entry, aName, len) == 0) {
            return *(s32 *)id;
        }
        entry = id + 4;
    } while (entry < end);
    return -1;
}

const char *sdfModelGetNodeName(sdfModelRootNode_t *model, s32 nameId) {
    sdfModelChunkHeader_t *chunk = sdfModelGetExtraChunk(model, 0x4D4E444E);
    const u8 *name;
    const u8 *end;
    if (chunk == NULL) {
        return D_00529428;
    }
    name = (const u8 *)(chunk + 1);
    end = (const u8 *)chunk + chunk->size;
    do {
        const u8 *cursor = name;
        const s32 *id;
        while (*cursor++ != 0) {
        }
        id = (const s32 *)(((u32)cursor + 3) & ~3U);
        if (*id == nameId) {
            return (const char *)name;
        }
        name = (const u8 *)(id + 1);
        if (name >= end) {
            return D_00529428;
        }
    } while (1);
}

u32 sdfModelGetMarkPosCount(sdfModelRootNode_t *aRoot)
{
    const sdfModelChunkHeader_t *chunk = sdfModelGetExtraChunk(aRoot, 0x534F504D);

    if (chunk != NULL) {
        return (chunk->size - 0x10) >> 6;
    }
    return 0;
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
void sdfModelMakeMarkPosMatrix(sdfModelRootNode_t *model, sdfModelMarkPos_t *record) {
    sdfModelNode_t *drawNode = sdfModelSearchNode(model, record->nodeId);
    u8 *vector;

    vector = (u8 *)&record->dir;
    SDF_VU0_LOADVECTOR(vector);
    SDF_VU0_SET_ROW2();
    SDF_VU0_COPYVECTOR_A_TO_B();
    vector = (u8 *)&record->up;
    SDF_VU0_LOADVECTOR(vector);
    SDF_VU0_NEGVECTOR_XYZ();
    SDF_VU0_SET_ROW1();
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_SET_ROW0();
    SDF_VU0_LOAD_ROW3(&record->pos);
    SDF_VU0_MATRIX_ROW3_W_SET_ONE();
    MultMatrixRev(&drawNode->tm);
}

/* vu0 routine: native SDK vector/matrix operations around model state. */
void sdfModelMakeMarkPos(sdfModelRootNode_t *model, sdfModelMarkPos_t *record) {
    sdfModelNode_t *drawNode = sdfModelSearchNode(model, record->nodeId);

    SDF_VU0_LOADMATRIX(&drawNode->tm);
    SDF_VU0_LOADVECTOR(&record->pos);
    SDF_VU0_APPLYMATRIX_XYZ1();
}

sdfModelMarkPos_t *sdfModelSearchMarkPosData(sdfModelRootNode_t *aRoot, s32 aId)
{
    sdfModelChunkHeader_t *chunk = sdfModelGetExtraChunk(aRoot, 0x534F504D);
    sdfModelMarkPos_t *data;
    u8 *end;

    if (chunk == NULL) {
        return NULL;
    }
    data = (sdfModelMarkPos_t *)((u8 *)chunk + 0x10);
    end = (u8 *)chunk + chunk->size;
    while ((u8 *)data < end) {
        if (data->id == aId) {
            return data;
        }
        data++;
    }
    return NULL;
}

s32 sdfModelGetMarkPosMatrix(sdfModelRootNode_t *aRoot, s32 aId)
{
    sdfModelMarkPos_t *data = sdfModelSearchMarkPosData(aRoot, aId);

    if (data != NULL) {
        sdfModelMakeMarkPosMatrix(aRoot, data);
        return 1;
    }
    return 0;
}

s32 sdfModelGetMarkPos(sdfModelRootNode_t *aRoot, s32 aId)
{
    sdfModelMarkPos_t *data = sdfModelSearchMarkPosData(aRoot, aId);

    if (data != NULL) {
        sdfModelMakeMarkPos(aRoot, data);
        return 1;
    }
    return 0;
}

void sdfModelSetFaceDivParm(sdfModelRootNode_t *aRoot, f32 aSoften, f32 aBias)
{
    aRoot->faceDivSoften = aSoften;
    aRoot->faceDivBias = aBias;
    aRoot->flags = aRoot->flags | SDF_MODEL_ROOT_FLAG_OWNFACEDIV;
}

void sdfModelResetFaceDivParm(sdfModelRootNode_t *aRoot)
{
    aRoot->flags = aRoot->flags & ~SDF_MODEL_ROOT_FLAG_OWNFACEDIV;
}

f32 sdfModelGetFaceDivSoften(sdfModelRootNode_t *aRoot)
{
    if (aRoot->flags & SDF_MODEL_ROOT_FLAG_OWNFACEDIV) {
        return aRoot->faceDivSoften;
    }
    return sdfFaceDivSoften;
}

f32 sdfModelGetFaceDivBias(sdfModelRootNode_t *aRoot)
{
    if (aRoot->flags & SDF_MODEL_ROOT_FLAG_OWNFACEDIV) {
        return aRoot->faceDivBias;
    }
    return sdfFaceDivBias;
}
INCLUDE_SDATA(const s32, "sdf/sdfModel", sdf3DLightDefault);

INCLUDE_SDATA(const s32, "sdf/sdfModel", sdf3DFogDefault);

