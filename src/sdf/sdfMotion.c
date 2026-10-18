#include "common.h"
#include "ps2asm.h"
#include "libsdf.h"
#include "model.h"
#include "motion.h"

typedef sdfMotionNode_t *(*sdfMotGenNodeFunc)(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId);

extern void *sdfChipAllocDirect(s32 aSize);
extern void sdfChipFreeDirect(void *aPtr);
extern sdfModelNode_t *sdfModelSearchNode(sdfModelRootNode_t *aRoot, u32 aId);

extern void sdfMotionChangeBlend(sdfMotionCtrlr_t *aCtrlr, s32 aCutNum, s32 aType, f32 aPreFrame,
                                 f32 aBlendLeng);

/* The type id's high half picks one of these group tables, its low half the
 * generator inside the group. */
extern void *sdfChipAllocZeroDirect(s32 aSize);
extern void *sdfItemArrayNew(s32 aInitCapa, s32 aElemSize, s32 aGrowStep);
extern void sdfItemArrayFree(void *aArray);
extern void sdfMatSetDiffuse(sdfMat_t *aMat, u32 aColor);
extern void sdfMatSetAmbient(sdfMat_t *aMat, u32 aColor);
extern void sdfMatSetSpecular(sdfMat_t *aMat, u32 aColor);
extern void sdfMatSetRefMap(sdfMat_t *aMat, u32 aColor);
extern void sdfMatSetMultiDiffuse(sdfMat_t *aMat, u32 aColor);
extern void sdfMatSetConstant(sdfMat_t *aMat, f32 aRatio);
extern void sdfMatSetTexUvwByPtr(sdfMat_t *aMat, void *aParm);
extern void sdfMatSetMultiTexUvwByPtr(sdfMat_t *aMat, void *aParm);

extern void sdfMotGetKey(sdfMotionNode_t *aThis, f32 aFrame, sdfMotGetKeyIf_t *aItf);
extern /* vu0 routine: blend the two colour keys */
u32 sdfMotInterpolateKeyColor(sdfMotGetKeyIf_t *aItf);
extern f32 sdfMotInterpolateKeyFloat(sdfMotGetKeyIf_t *aItf);
extern void sdfMotInterpolateKeyUvw(sdfMotGetKeyIf_t *aItf, void *aDst);
extern void sdfMotInterpolateKeyUvwSub(f32 *aDst, f32 *aSrc0, f32 *aSrc1, f32 aRate);

extern sdfMotGenNodeFunc D_00529448[];
extern sdfMotGenNodeFunc sdfMotGenNodeDef[];
extern sdfMotGenNodeFunc sdfMotGenMatTable[];
extern sdfMotGenNodeFunc D_00529600[];

extern sdfMotionNodeVTBL_t D_00529470; /* model liner pos */
extern sdfMotionNodeVTBL_t sdfMotModelLinerRotXYZTable;
extern sdfMotionNodeVTBL_t sdfMotModelLinerScaleTable;
extern sdfMotionNodeVTBL_t sdfMotModelLerpRotQuatTable;

typedef struct
{
    sdfMotNodeModelBase_t modelBase;
    s8 blendFrom;
} sdfMotNodeModelFlag_t;

typedef struct
{
    sdfMotNodeMatBase_t matBase;
    u32 blendFrom;
} sdfMotNodeMatColor_t;

typedef struct
{
    sdfMotNodeMatBase_t matBase;
    f32 blendFrom;
} sdfMotNodeMatFloat_t;

/* Five weights, not the six-word sdfMatUvwParm_t: only offset, tile and angle
 * are keyed. */
typedef struct
{
    f32 parm[5];
} sdfMotUvwKey_t;

typedef struct
{
    sdfMotNodeMatBase_t matBase;
    sdfMotUvwKey_t blendFrom;
} sdfMotNodeMatUvw_t;

/* out = a + (b - a) * t per RGBA byte lane, rounded through vf3x. `half` is
 * 0.5f in a general register. */
#define SDF_MOT_RGBA_LERP(out, a, b, t, half) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "qmtc2.ni %4, vf3\n\t" \
    "mfc1 $8, %3\n\t" \
    "pextlb $4, $0, %1\n\t" \
    "pextlb $5, $0, %2\n\t" \
    "vsub.xyzw vf2, vf0, vf0\n\t" \
    "qmtc2.ni $8, vf4\n\t" \
    "pextlh $4, $0, $4\n\t" \
    "pextlh $5, $0, $5\n\t" \
    "vaddax.xyzw ACC, vf2, vf3x\n\t" \
    "vsubx.w vf5, vf0, vf4x\n\t" \
    "qmtc2.ni $4, vf2\n\t" \
    "qmtc2.ni $5, vf3\n\t" \
    "vsubx.w vf5, vf0, vf4x\n\t" \
    "vitof0.xyzw vf2, vf2\n\t" \
    "vitof0.xyzw vf3, vf3\n\t" \
    "vmaddaw.xyzw ACC, vf2, vf5w\n\t" \
    "vmaddx.xyzw vf5, vf3, vf4x\n\t" \
    "vftoi0.xyzw vf4, vf5\n\t" \
    "qmfc2.ni $4, vf4\n\t" \
    "ppach $5, $0, $4\n\t" \
    "ppacb %0, $0, $5\n\t" \
    ".set reorder" \
    : "=r" (out) : "r" (a), "r" (b), "f" (t), "r" (half) : "$4", "$5", "$6", "$7", "$8")

extern sdfMatUvwParm_t *sdfMatGetTexUvwParm(sdfMat_t *aMat);
extern sdfMatUvwParm_t *sdfMatGetMultiTexUvwParm(sdfMat_t *aMat);

extern sdfMotionNodeVTBL_t sdfMotModelVisibleTable;
extern sdfMotionNodeVTBL_t sdfMotMatDiffuseTable;
extern sdfMotionNodeVTBL_t sdfMotMatAmbientTable;
extern sdfMotionNodeVTBL_t sdfMotMatSpecularTable;
extern sdfMotionNodeVTBL_t sdfMotMatRefMapTable;
extern sdfMotionNodeVTBL_t sdfMotMatConstantTable;
extern sdfMotionNodeVTBL_t sdfMotMatUvwTable;
extern sdfMotionNodeVTBL_t sdfMotMatMultiUvwTable;
extern sdfMotionNodeVTBL_t sdfMotMatMultiDiffuseTable;
extern sdfMotionNodeVTBL_t sdfMotMatUvw2Table;
extern sdfMotionNodeVTBL_t sdfMotMatMultiUvw2Table;

sdfMotionNode_t *sdfMotionNodeNew(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    return D_00529448[aTypeId >> 16](aCtrlr, aTypeId, aNodeId);
}

void sdfMotionNodeFree(sdfMotionNode_t *aNode)
{
    ((void (*)(sdfMotionNode_t *))aNode->vtbl->destroy)(aNode);
}

void sdfMotionNodeBaseInit(sdfMotionNode_t *aNode, sdfMotionCtrlr_t *aCtrlr, sdfMotionNodeVTBL_t *aVTBL)
{
    aNode->vtbl = aVTBL;
    aNode->owner = aCtrlr;
}

sdfMotionCtrlr_t *sdfMotionCtrlrNew(sdfModelRootNode_t *aModel, sdfMotionPlayBook_t *aBook)
{
    sdfMotionCtrlr_t *ctrlr;
    sdfMotionNodeDef_t *def;
    s32 numOfNode;
    s32 i;

    ctrlr = sdfChipAllocZeroDirect(sizeof(sdfMotionCtrlr_t));
    numOfNode = aBook->numOfNode;
    ctrlr->next = aModel->motion;
    aModel->motion = ctrlr;
    ctrlr->rootNode = aModel;
    ctrlr->playbook = aBook;
    ctrlr->nodeList = sdfItemArrayNew(numOfNode, 4, 8);
    ctrlr->nodeList->count = numOfNode;
    def = ctrlr->playbook->nodeDefList;
    for (i = 0; i < numOfNode; i++) {
        ctrlr->nodeList->items[i] = sdfMotionNodeNew(ctrlr, def->typeId, def->nodeId);
        def++;
    }
    ctrlr->stat = 0;
    ctrlr->speed = 1.0f;
    return ctrlr;
}

void sdfMotionCtrlrFree(sdfMotionCtrlr_t *aCtrlr)
{
    sdfMotionCtrlr_t **link;
    sdfMotionCtrlr_t *cur;
    sdfMotionNode_t **items;
    s32 count;
    s32 i;

    if (aCtrlr == NULL) {
        return;
    }
    if (aCtrlr->rootNode != NULL) {
        link = &aCtrlr->rootNode->motion;
        for (;;) {
            cur = *link;
            if (cur == NULL) {
                break;
            }
            if (cur == aCtrlr) {
                *link = aCtrlr->next;
                break;
            }
            link = &cur->next;
        }
    }
    count = aCtrlr->nodeList->count;
    items = aCtrlr->nodeList->items;
    for (i = 0; i < count; i++) {
        sdfMotionNodeFree(items[i]);
    }
    sdfItemArrayFree(aCtrlr->nodeList);
    sdfChipFreeDirect(aCtrlr);
}

void sdfMotionChangeBlend(sdfMotionCtrlr_t *aCtrlr, s32 aCutNum, s32 aType, f32 aPreFrame,
                          f32 aBlendLeng)
{
    s32 i;
    sdfMotionCutDef_t *cut;
    sdfMotionCutChunk_t *chunk;
    sdfMotionNode_t *node;

    aCtrlr->cutNumber = aCutNum;
    if (aBlendLeng > 0.0f) {
        aCtrlr->blendLength = aBlendLeng;
        aCtrlr->blendPreLength = -aPreFrame;
        for (i = 0; i < aCtrlr->nodeList->count; i++) {
            node = aCtrlr->nodeList->items[i];
            ((void (*)(sdfMotionNode_t *))node->vtbl->initBlend)(node);
        }
    } else {
        aCtrlr->blendLength = 0.0f;
        aCtrlr->blendPreLength = 0.0f;
    }
    aCtrlr->playType = aType;
    aCtrlr->stat = SDF_MOTION_STAT_SET;
    cut = aCtrlr->playbook->cutList[aCutNum];
    aCtrlr->frame = aCtrlr->blendPreLength;
    if (cut == NULL) {
        aCtrlr->frameLength = 0;
        aCtrlr->stat = SDF_MOTION_STAT_NOCUT;
        return;
    }
    aCtrlr->frameLength = cut->frameLength;
    chunk = cut->chunk;
    for (i = 0; i < aCtrlr->nodeList->count; i++) {
        node = aCtrlr->nodeList->items[i];
        ((void (*)(sdfMotionNode_t *, sdfMotionCutChunk_t *))node->vtbl->setCut)(node, chunk);
        chunk = (sdfMotionCutChunk_t *)((u8 *)chunk + chunk->size);
    }
}

void sdfMotionChange(sdfMotionCtrlr_t *aCtrlr, s32 aCutNum, s32 aType)
{
    sdfMotionChangeBlend(aCtrlr, aCutNum, aType, 0.0f, 0.0f);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionNext);

void sdfMotionPause(sdfMotionCtrlr_t *aCtrlr)
{
    u8 stat;

    stat = aCtrlr->stat;
    if (stat != SDF_MOTION_STAT_PAUSE) {
        aCtrlr->statBeforePause = stat;
        aCtrlr->stat = SDF_MOTION_STAT_PAUSE;
    }
}

void sdfMotionPauseOff(sdfMotionCtrlr_t *aCtrlr)
{
    if (aCtrlr->stat == SDF_MOTION_STAT_PAUSE) {
        aCtrlr->stat = aCtrlr->statBeforePause;
    }
}

void sdfMotCLBK_Destroy(sdfMotionNode_t *aNode)
{
    sdfChipFreeDirect(aNode);
}

void sdfMotCLBK_SetCut(sdfMotionNode_t *aNode, sdfMotionCutChunk_t *aTable)
{
    aNode->table = aTable;
}

void sdfMotGetKey(sdfMotionNode_t *aThis, f32 aFrame, sdfMotGetKeyIf_t *aItf)
{
    sdfMotionCutChunk_t *chunk;
    u16 *frameList;
    u8 *keyData;
    u8 *key0;
    s32 frame;
    s32 numOfKey;
    s32 elemSize;
    s32 lo;
    s32 hi;
    s32 index;
    s32 next;
    s32 frame0;
    s32 frame1;
    s32 span;

    frame = aFrame;
    chunk = aThis->table;
    numOfKey = chunk->numOfKey;
    frameList = chunk->keyFrameList;
    elemSize = chunk->keyElemSize;
    lo = 0;
    hi = numOfKey - 1;
    do {
        index = (lo + hi + 1) >> 1;
        frame0 = frameList[index];
        if (frame < frame0) {
            index--;
            hi = index;
        } else {
            lo = index;
        }
    } while (lo < hi);
    keyData = (u8 *)&frameList[(numOfKey + 1) & ~1];
    frame0 = frameList[index];
    key0 = keyData + index * elemSize;
    aItf->key0 = key0;
    next = index + 1;
    if (next == numOfKey) {
        if (aThis->owner->playType == 0) {
            aItf->key1 = key0;
            frame1 = frame0;
        } else {
            aItf->key1 = keyData;
            frame1 = aThis->owner->frameLength;
        }
    } else {
        aItf->key1 = key0 + elemSize;
        frame1 = frameList[next];
    }
    span = frame1 - frame0;
    if (span == 0) {
        aItf->key1 = key0;
        aItf->rate = 0.0f;
        return;
    }
    aItf->rate = (aFrame - frame0) / span;
}

f32 sdfMotInterpolateKeyFloat(sdfMotGetKeyIf_t *aItf)
{
    f32 *key0;
    f32 *key1;

    key0 = aItf->key0;
    key1 = aItf->key1;
    return *key0 + *key1 * aItf->rate - *key0 * aItf->rate;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyXYZ);

/* vu0 routine: blend the two colour keys */
u32 sdfMotInterpolateKeyColor(sdfMotGetKeyIf_t *aItf)
{
    u32 *key0;
    u32 *key1;
    u32 color;

    key0 = aItf->key0;
    key1 = aItf->key1;
    SDF_MOT_RGBA_LERP(color, *key0, *key1, aItf->rate, 0.5f);
    return color;
}

void sdfMotInterpolateKeyUvwSub(f32 *aDst, f32 *aSrc0, f32 *aSrc1, f32 aRate)
{
    s32 i;

    for (i = 0; i != 5; i++) {
        aDst[i] = aSrc0[i] * (1.0f - aRate) + aSrc1[i] * aRate;
    }
}

void sdfMotInterpolateKeyUvw(sdfMotGetKeyIf_t *aItf, void *aDst)
{
    sdfMotInterpolateKeyUvwSub(aDst, aItf->key0, aItf->key1, aItf->rate);
}

sdfMotionNode_t *sdfMotGenNode(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    return sdfMotGenNodeDef[aTypeId & 0xFFFF](aCtrlr, aTypeId, aNodeId);
}

void sdfMotGenModelInit(sdfMotNodeModelBase_t *aBase, sdfMotionCtrlr_t *aCtrlr, sdfMotionNodeVTBL_t *aVTBL, u32 aNodeId)
{
    sdfMotionNodeBaseInit(&aBase->nodeBase, aCtrlr, aVTBL);
    aBase->modelNode = sdfModelSearchNode(aCtrlr->rootNode, aNodeId);
}

sdfMotionNode_t *sdfMotGenModelLinerPos(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeModelVec_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeModelVec_t));
    sdfMotGenModelInit(&node->modelBase, aCtrlr, &D_00529470, aNodeId);
    return &node->modelBase.nodeBase;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerPosSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerPosBlendSeek);

sdfMotionNode_t *sdfMotGenModelLinerRotXYZ(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeModelVec_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeModelVec_t));
    sdfMotGenModelInit(&node->modelBase, aCtrlr, &sdfMotModelLinerRotXYZTable, aNodeId);
    return &node->modelBase.nodeBase;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerRotXYZSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerRotXYZBlendSeek);

sdfMotionNode_t *sdfMotGenModelLinerScale(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeModelVec_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeModelVec_t));
    sdfMotGenModelInit(&node->modelBase, aCtrlr, &sdfMotModelLinerScaleTable, aNodeId);
    return &node->modelBase.nodeBase;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerScaleSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerScaleBlendSeek);

sdfMotionNode_t *sdfMotGenModelLerpRotQuat(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeModelVec_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeModelVec_t));
    sdfMotGenModelInit(&node->modelBase, aCtrlr, &sdfMotModelLerpRotQuatTable, aNodeId);
    return &node->modelBase.nodeBase;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLerpRotQuatSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLerpRotQuatBlendSeek);

sdfMotionNode_t *sdfMotGenModelVisible(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeModelFlag_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeModelFlag_t));
    sdfMotGenModelInit(&node->modelBase, aCtrlr, &sdfMotModelVisibleTable, aNodeId);
    return &node->modelBase.nodeBase;
}

void sdfMotCLBK_modelVisibleSeek(sdfMotNodeModelBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;
    sdfModelNode_t *node;
    u8 *key;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    key = itf.key0;
    node = aThis->modelNode;
    if (*key == 0) {
        node->flags |= SDF_MODEL_NODE_FLAG_HIDDEN2;
    } else {
        node->flags &= ~SDF_MODEL_NODE_FLAG_HIDDEN2;
    }
}

void sdfMotCLBK_modelVisibleBlendSeek(sdfMotNodeModelBase_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    sdfModelNode_t *node;
    u8 *key;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    key = itf.key0;
    node = aThis->modelNode;
    if (*key == 0) {
        node->flags |= SDF_MODEL_NODE_FLAG_HIDDEN2;
    } else {
        node->flags &= ~SDF_MODEL_NODE_FLAG_HIDDEN2;
    }
}

void sdfMotCLBK_modelVisibleInitBlend(sdfMotNodeModelFlag_t *aThis)
{
    aThis->blendFrom = ((aThis->modelBase.modelNode->flags >> 4) ^ 1) & 1;
}

void sdfMotCLBK_modelPosInitBlend(sdfMotNodeModelVec_t *aThis)
{
    PS2_COPY_QUAD(&aThis->blendFrom, &aThis->modelBase.modelNode->pos);
}

void sdfMotCLBK_modelRotInitBlend(sdfMotNodeModelVec_t *aThis)
{
    PS2_COPY_QUAD(&aThis->blendFrom, &aThis->modelBase.modelNode->rotate);
}

void sdfMotCLBK_modelScaleInitBlend(sdfMotNodeModelVec_t *aThis)
{
    PS2_COPY_QUAD(&aThis->blendFrom, &aThis->modelBase.modelNode->scale);
}

sdfMotionNode_t *sdfMotGenMatNode(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    return sdfMotGenMatTable[aTypeId & 0xFFFF](aCtrlr, aTypeId, aNodeId);
}

void sdfMotGenMatInit(sdfMotNodeMatBase_t *aBase, sdfMotionCtrlr_t *aCtrlr, sdfMotionNodeVTBL_t *aVTBL, u32 aNodeId)
{
    sdfMotionNodeBaseInit(&aBase->nodeBase, aCtrlr, aVTBL);
    aBase->mat = aCtrlr->rootNode->matList->items[aNodeId];
}

sdfMotionNode_t *sdfMotGenMatDiffuse(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatColor_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatColor_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatDiffuseTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matDiffuseSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetDiffuse(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

void sdfMotCLBK_matDiffuseBlendSeek(sdfMotNodeMatColor_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    u32 to;
    u32 color;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyColor(&itf);
    SDF_MOT_RGBA_LERP(color, aThis->blendFrom, to, aBlendRate, 0.5f);
    sdfMatSetDiffuse(aThis->matBase.mat, color);
}

void sdfMotCLBK_matDiffuseInitBlend(sdfMotNodeMatColor_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->diffuse;
}

sdfMotionNode_t *sdfMotGenMatAmbient(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatColor_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatColor_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatAmbientTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matAmbientSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetAmbient(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

void sdfMotCLBK_matAmbientBlendSeek(sdfMotNodeMatColor_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    u32 to;
    u32 color;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyColor(&itf);
    SDF_MOT_RGBA_LERP(color, aThis->blendFrom, to, aBlendRate, 0.5f);
    sdfMatSetAmbient(aThis->matBase.mat, color);
}

void sdfMotCLBK_matAmbientInitBlend(sdfMotNodeMatColor_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->ambient;
}

sdfMotionNode_t *sdfMotGenMatSpecular(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatColor_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatColor_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatSpecularTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matSpecularSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetSpecular(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

void sdfMotCLBK_matSpecularBlendSeek(sdfMotNodeMatColor_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    u32 to;
    u32 color;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyColor(&itf);
    SDF_MOT_RGBA_LERP(color, aThis->blendFrom, to, aBlendRate, 0.5f);
    sdfMatSetSpecular(aThis->matBase.mat, color);
}

void sdfMotCLBK_matSpecularInitBlend(sdfMotNodeMatColor_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->specular;
}

sdfMotionNode_t *sdfMotGenMatRefMap(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatColor_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatColor_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatRefMapTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matRefMapSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetRefMap(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

void sdfMotCLBK_matRefMapBlendSeek(sdfMotNodeMatColor_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    u32 to;
    u32 color;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyColor(&itf);
    SDF_MOT_RGBA_LERP(color, aThis->blendFrom, to, aBlendRate, 0.5f);
    sdfMatSetRefMap(aThis->matBase.mat, color);
}

void sdfMotCLBK_matRefMapInitBlend(sdfMotNodeMatColor_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->refmap;
}

sdfMotionNode_t *sdfMotGenMatConstant(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatFloat_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatFloat_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatConstantTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matConstantSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetConstant(aThis->mat, sdfMotInterpolateKeyFloat(&itf));
}

void sdfMotCLBK_matConstantBlendSeek(sdfMotNodeMatFloat_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    f32 to;
    f32 from;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyFloat(&itf);
    from = aThis->blendFrom;
    sdfMatSetConstant(aThis->matBase.mat, from + to * aBlendRate - from * aBlendRate);
}

void sdfMotCLBK_matConstantInitBlend(sdfMotNodeMatFloat_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->constant;
}

sdfMotionNode_t *sdfMotGenMatUvw(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatUvw_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatUvw_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatUvwTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matUvwSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;
    sdfMatUvwParm_t uvw;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &uvw);
    sdfMatSetTexUvwByPtr(aThis->mat, &uvw);
}

void sdfMotCLBK_matUvwBlendSeek(sdfMotNodeMatUvw_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    sdfMotUvwKey_t dst;
    sdfMotUvwKey_t key;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &key);
    sdfMotInterpolateKeyUvwSub(dst.parm, aThis->blendFrom.parm, key.parm, aBlendRate);
    sdfMatSetTexUvwByPtr(aThis->matBase.mat, &dst);
}

void sdfMotCLBK_matUvwInitBlend(sdfMotNodeMatUvw_t *aThis)
{
    aThis->blendFrom = *(sdfMotUvwKey_t *)sdfMatGetTexUvwParm(aThis->matBase.mat);
}

sdfMotionNode_t *sdfMotGenMatMultiUvw(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatUvw_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatUvw_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatMultiUvwTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matMultiUvwSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;
    sdfMatUvwParm_t uvw;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &uvw);
    sdfMatSetMultiTexUvwByPtr(aThis->mat, &uvw);
}

void sdfMotCLBK_matMultiUvwBlendSeek(sdfMotNodeMatUvw_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    sdfMotUvwKey_t key;
    sdfMotUvwKey_t dst;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &key);
    sdfMotInterpolateKeyUvwSub(dst.parm, aThis->blendFrom.parm, key.parm, aBlendRate);
    sdfMatSetMultiTexUvwByPtr(aThis->matBase.mat, &dst);
}

void sdfMotCLBK_matMultiUvwInitBlend(sdfMotNodeMatUvw_t *aThis)
{
    aThis->blendFrom = *(sdfMotUvwKey_t *)sdfMatGetMultiTexUvwParm(aThis->matBase.mat);
}

sdfMotionNode_t *sdfMotGenMatMultiDiffuse(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatColor_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatColor_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatMultiDiffuseTable, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matMultiDiffuseSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetMultiDiffuse(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

void sdfMotCLBK_matMultiDiffuseBlendSeek(sdfMotNodeMatColor_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;
    u32 to;
    u32 color;

    sdfMotGetKey(&aThis->matBase.nodeBase, aFrame, &itf);
    to = sdfMotInterpolateKeyColor(&itf);
    SDF_MOT_RGBA_LERP(color, aThis->blendFrom, to, aBlendRate, 0.5f);
    sdfMatSetMultiDiffuse(aThis->matBase.mat, color);
}

void sdfMotCLBK_matMultiDiffuseInitBlend(sdfMotNodeMatColor_t *aThis)
{
    aThis->blendFrom = aThis->matBase.mat->multiTexDiffuse;
}

sdfMotionNode_t *sdfMotGenMatUvw2(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatUvw_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatUvw_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatUvw2Table, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matUvw2Seek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetTexUvwByPtr(aThis->mat, itf.key0);
}

void sdfMotCLBK_matUvw2BlendSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetTexUvwByPtr(aThis->mat, itf.key0);
}

void sdfMotCLBK_matUvw2InitBlend(void) {
}

sdfMotionNode_t *sdfMotGenMatMultiUvw2(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    sdfMotNodeMatUvw_t *node;

    node = sdfChipAllocDirect(sizeof(sdfMotNodeMatUvw_t));
    sdfMotGenMatInit(&node->matBase, aCtrlr, &sdfMotMatMultiUvw2Table, aNodeId);
    return &node->matBase.nodeBase;
}

void sdfMotCLBK_matMultiUvw2Seek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetMultiTexUvwByPtr(aThis->mat, itf.key0);
}

void sdfMotCLBK_matMultiUvw2BlendSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame, f32 aBlendRate)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetMultiTexUvwByPtr(aThis->mat, itf.key0);
}

void sdfMotCLBK_matMultiUvw2InitBlend(void) {
}

sdfMotionNode_t *sdfMotGenMorphNode(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    return D_00529600[aTypeId & 0xFFFF](aCtrlr, aTypeId, aNodeId);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMorphInit);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMorph);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphInitBlend);
