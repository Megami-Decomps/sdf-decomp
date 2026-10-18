#include "common.h"
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
extern u32 sdfMotInterpolateKeyColor(sdfMotGetKeyIf_t *aItf);
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

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionChangeBlend);

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

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGetKey);

f32 sdfMotInterpolateKeyFloat(sdfMotGetKeyIf_t *aItf)
{
    f32 *key0;
    f32 *key1;

    key0 = aItf->key0;
    key1 = aItf->key1;
    return *key0 + *key1 * aItf->rate - *key0 * aItf->rate;
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyXYZ);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyColor);

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

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelVisible);

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

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelVisibleInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelPosInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelRotInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelScaleInitBlend);

sdfMotionNode_t *sdfMotGenMatNode(sdfMotionCtrlr_t *aCtrlr, u32 aTypeId, u32 aNodeId)
{
    return sdfMotGenMatTable[aTypeId & 0xFFFF](aCtrlr, aTypeId, aNodeId);
}

void sdfMotGenMatInit(sdfMotNodeMatBase_t *aBase, sdfMotionCtrlr_t *aCtrlr, sdfMotionNodeVTBL_t *aVTBL, u32 aNodeId)
{
    sdfMotionNodeBaseInit(&aBase->nodeBase, aCtrlr, aVTBL);
    aBase->mat = aCtrlr->rootNode->matList->items[aNodeId];
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatDiffuse);

void sdfMotCLBK_matDiffuseSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetDiffuse(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matDiffuseBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matDiffuseInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatAmbient);

void sdfMotCLBK_matAmbientSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetAmbient(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matAmbientBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matAmbientInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatSpecular);

void sdfMotCLBK_matSpecularSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetSpecular(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matSpecularBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matSpecularInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatRefMap);

void sdfMotCLBK_matRefMapSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetRefMap(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matRefMapBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matRefMapInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatConstant);

void sdfMotCLBK_matConstantSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetConstant(aThis->mat, sdfMotInterpolateKeyFloat(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matConstantBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matConstantInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatUvw);

void sdfMotCLBK_matUvwSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;
    sdfMatUvwParm_t uvw;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &uvw);
    sdfMatSetTexUvwByPtr(aThis->mat, &uvw);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvwBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvwInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiUvw);

void sdfMotCLBK_matMultiUvwSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;
    sdfMatUvwParm_t uvw;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMotInterpolateKeyUvw(&itf, &uvw);
    sdfMatSetMultiTexUvwByPtr(aThis->mat, &uvw);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvwBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvwInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiDiffuse);

void sdfMotCLBK_matMultiDiffuseSeek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetMultiDiffuse(aThis->mat, sdfMotInterpolateKeyColor(&itf));
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiDiffuseBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiDiffuseInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatUvw2);

void sdfMotCLBK_matUvw2Seek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetTexUvwByPtr(aThis->mat, itf.key0);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvw2BlendSeek);

void sdfMotCLBK_matUvw2InitBlend(void) {
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiUvw2);

void sdfMotCLBK_matMultiUvw2Seek(sdfMotNodeMatBase_t *aThis, f32 aFrame)
{
    sdfMotGetKeyIf_t itf;

    sdfMotGetKey(&aThis->nodeBase, aFrame, &itf);
    sdfMatSetMultiTexUvwByPtr(aThis->mat, itf.key0);
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvw2BlendSeek);

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
