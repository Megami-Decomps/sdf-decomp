#include "common.h"
#include "motion.h"
#include "model.h"
#ifdef SDF_SMT3
#include "event.h"
#include "nb.h"
#include "effBattle.h"
#else
#include "effPCPData.h"
#endif
#include "ps2asm.h"

/* DDS's names for the same objects. */
#if defined(SDF_DDS1) || defined(SDF_DDS2)
#define effPartPCPDataTable effParameterPresetBanks
#define effPartModelPosInit effModelDefaultPosition
#define effPartModelRotInit effModelDefaultQuaternion
#define effPartModelScaleInit effModelDefaultScale
#endif
#if defined(SDF_DDS1)
#define effPartModelSerial D_003BB044
#elif defined(SDF_DDS2)
#define effPartModelSerial D_00436434
#endif

extern sdfVector4_t effPartModelPosInit;

extern sdfVector4_t effPartModelRotInit;

extern sdfVector4_t effPartModelScaleInit;

typedef struct
{
    u16 type;
    void *pHandle;
} effPartHandle_t;

typedef struct
{
    u32 kateg;
    u32 param;
    void *pHandle;
} dds3EffectPCP_t;



typedef struct
{
    void *(*create)(void *);
    void (*draw)(void *);
    void (*destroy)(void *);
    void *(*copy)(void *);
    void (*setPosition)(void *, void *);
    void (*setScale)(void *, f32);
    void (*setRotateMatrix)(void *, void *);
    void (*setColor)(void *, u32);
    void (*setDirection)(void *, void *);
    void (*setArea)(void *, f32);
} dds3EffectFunc_t;

void mdlSetPosition(mdlObj_t *aHandle);

void mdlSetRotate(mdlObj_t *aHandle);

void mdlSetScale(mdlObj_t *aHandle);

void mdlSetColor(mdlObj_t *aHandle, u32 aColor);

void mdlLoopMotion(mdlObj_t *aHandle, s32 aGroup, s32 aNumber);

extern dds3EffectFunc_t dds3EffectFuncTable[7];

#ifdef SDF_SMT3
extern void *D_002F69B8[4];
#else
extern void *kwlnSceneSurfaceSets[13][4];
#define D_002F69B8 kwlnSceneSurfaceSets[10]
#endif

#ifdef SDF_SMT3
extern s32 effPartModelSerial;
#else
extern u16 effPartModelSerial;
#endif
#ifdef SDF_DDS2
extern dds3ModelResrc_t *mdlSearchResrc(s32 aMajor, s32 aMinor);
#endif

extern s32 effBTLCutCount;

/* Header of a packed model an effect part carries, followed by the pack data. */
typedef struct
{
    u32 size;
    u8 reserve[0xC];
    u8 data[1];
} dds3EffectPartModelData_t;

void *dds3BillboardCreate(s32 kind, addr_t data);

void mdlPackDecode(s32 major, s32 minor, s32 regFlags, const u8 *buff, s32 leng);

void mdlLink(mdlObj_t *aHandle, void **aModelOt);

void mdlFree(mdlObj_t *aHandle);

extern dds3EffectFunc_t EffectPCPFuncTable[];

extern void **effPartPCPDataTable[];

void *sdfChipAllocDirect(s32 aSize);

void sdfChipFreeDirect(void *aPtr);

void dds3BillboardSetScale(void *hBillboard, f32 x, f32 y);

s32 mdlGetMajorNum(mdlObj_t *aHandle);

s32 mdlGetMinorNum(mdlObj_t *aHandle);

mdlObj_t *mdlLoad(u32 aMajor, u32 aMinor);

void dds3EffectPartModelInit(mdlObj_t *hModel);

effPartHandle_t *dds3EffectPartCreate(u16 type, void *pData);





extern u32 effBTLPointLightProcess;

extern s8 effBTLFieldColorOn;

extern s8 D_0054F0F1;

extern const char *D_0054F0F4;

extern u8 effBTLDebugCameraAllEdit;

extern u8 effBTLDebugCameraAllTable;

extern u8 D_0054F1C3;

extern s32 effBTLPartyIndex;

extern s8 effBTLUnitLightOn;

extern char effBedDirName[];

void *dds3EffectPartGetHandle(effPartHandle_t *hEffPart);

u16 dds3EffectPartGetType(effPartHandle_t *hEffPart);


void dds3EffectPartDelete(effPartHandle_t *hEffPart);

effPartHandle_t *dds3EffectPartCopy(effPartHandle_t *hEffPartSrc);

void dds3EffectPartDraw(effPartHandle_t *hEffPart);

void dds3EffectPartSetPosition(effPartHandle_t *hEffPart, sdfVector4_t *pPos);

void dds3EffectPartSetScale(effPartHandle_t *hEffPart, f32 scale);

void dds3EffectPartSetColor(effPartHandle_t *hEffPart, u32 abgr);

void *dds3EffectPartGetHandle(effPartHandle_t *hEffPart)
{
    return hEffPart->pHandle;
}

u16 dds3EffectPartGetType(effPartHandle_t *hEffPart)
{
    return hEffPart->type;
}

effPartHandle_t *dds3EffectPartCreate(u16 type, void *pData)
{
    effPartHandle_t *hEffPart;

    hEffPart = sdfChipAllocDirect(sizeof(effPartHandle_t));
    hEffPart->type = type;
    hEffPart->pHandle = dds3EffectFuncTable[type].create(pData);
    return hEffPart;
}

void dds3EffectPartDelete(effPartHandle_t *hEffPart)
{
    dds3EffectFuncTable[hEffPart->type].destroy(hEffPart->pHandle);
    sdfChipFreeDirect(hEffPart);
}

void dds3EffectPartDraw(effPartHandle_t *hEffPart)
{
    dds3EffectFuncTable[hEffPart->type].draw(hEffPart->pHandle);
}

effPartHandle_t *dds3EffectPartCopy(effPartHandle_t *hEffPartSrc)
{
    effPartHandle_t *hEffPart;

    hEffPart = sdfChipAllocDirect(sizeof(effPartHandle_t));
    hEffPart->type = hEffPartSrc->type;
    hEffPart->pHandle = dds3EffectFuncTable[hEffPartSrc->type].copy(hEffPartSrc->pHandle);
    return hEffPart;
}

void dds3EffectPartSetPosition(effPartHandle_t *hEffPart, sdfVector4_t *pPos)
{
    if (dds3EffectFuncTable[hEffPart->type].setPosition != NULL) {
        dds3EffectFuncTable[hEffPart->type].setPosition(hEffPart->pHandle, pPos);
    }
}

void dds3EffectPartSetScale(effPartHandle_t *hEffPart, f32 scale)
{
    if (dds3EffectFuncTable[hEffPart->type].setScale != NULL) {
        dds3EffectFuncTable[hEffPart->type].setScale(hEffPart->pHandle, scale);
    }
}

void dds3EffectPartSetRotateMatrix(effPartHandle_t *hEffPart, void *pMatrix)
{
    if (dds3EffectFuncTable[hEffPart->type].setRotateMatrix != NULL) {
        dds3EffectFuncTable[hEffPart->type].setRotateMatrix(hEffPart->pHandle, pMatrix);
    }
}

void dds3EffectPartSetColor(effPartHandle_t *hEffPart, u32 abgr)
{
    if (dds3EffectFuncTable[hEffPart->type].setColor != NULL) {
        dds3EffectFuncTable[hEffPart->type].setColor(hEffPart->pHandle, abgr);
    }
}

void dds3EffectPartSetDirection(effPartHandle_t *hEffPart, void *pMatrix)
{
    if (dds3EffectFuncTable[hEffPart->type].setDirection != NULL) {
        dds3EffectFuncTable[hEffPart->type].setDirection(hEffPart->pHandle, pMatrix);
    }
}

void dds3EffectPartSetArea(effPartHandle_t *hEffPart, f32 area)
{
    if (dds3EffectFuncTable[hEffPart->type].setArea != NULL) {
        dds3EffectFuncTable[hEffPart->type].setArea(hEffPart->pHandle, area);
    }
}

void *dds3EffectPartTMXBillboardCreate(u32 data)
{
    return dds3BillboardCreate(0, data);
}

void *dds3EffectPartP2ABillboardCreate(u32 data)
{
    return dds3BillboardCreate(1, data);
}

void dds3EffectPartBillboardScale(void *hBillboard, f32 scale)
{
    dds3BillboardSetScale(hBillboard, scale, scale);
}

void dds3EffectPartModelInit(mdlObj_t *hModel)
{
    SDF_VU0_LOADVECTOR(&effPartModelPosInit);
    mdlSetPosition(hModel);
    SDF_VU0_LOADVECTOR(&effPartModelRotInit);
    mdlSetRotate(hModel);
    SDF_VU0_LOADVECTOR(&effPartModelScaleInit);
    mdlSetScale(hModel);
    mdlSetColor(hModel, 0x80808080);
    if (hModel->first != NULL) {
        mdlLoopMotion(hModel, 0, 0);
        hModel->first->speed = 1.0f;
    }
    hModel->flags &= ~1;
}

mdlObj_t *dds3EffectPartModelCreate(dds3EffectPartModelData_t *pData)
{
    mdlObj_t *hModel;

#ifdef SDF_DDS2
    while (mdlSearchResrc(7, effPartModelSerial) != 0) {
        effPartModelSerial++;
    }
#endif
    mdlPackDecode(7, effPartModelSerial, 0x101, pData->data, pData->size);
    hModel = mdlLoad(7, effPartModelSerial);
    dds3EffectPartModelInit(hModel);
    effPartModelSerial++;
    return hModel;
}

void dds3EffectPartModelDraw(mdlObj_t *hData)
{
    mdlLink(hData, D_002F69B8);
}

void dds3EffectPartModelDelete(mdlObj_t *hData)
{
    mdlFree(hData);
}

mdlObj_t *dds3EffectPartModelCopy(void *hData)
{
    s32 major;
    s32 minor;
    mdlObj_t *hModel;

    major = mdlGetMajorNum(hData);
    minor = mdlGetMinorNum(hData);
    hModel = mdlLoad(major, minor);
    dds3EffectPartModelInit(hModel);
    return hModel;
}

void dds3EffectPartModelSetPosition(mdlObj_t *hData, sdfVector4_t *pPos)
{
    SDF_VU0_LOADVECTOR(pPos);
    mdlSetPosition(hData);
}

void dds3EffectPartModelSetScale(void *hData, f32 scale)
{
    sdfVector4_t v;

    v.x = v.y = v.z = scale;
#ifdef NON_MATCHING
    v.w = 1.0f;
#endif
    SDF_VU0_LOADVECTOR_MEMOPS(v);
    mdlSetScale(hData);
}

void dds3EffectPartModelSetRotateMatrix(mdlObj_t *hData, sdfMatrix4_t *pMatrix)
{
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_STORE_ROW0(&hData->inner->tm.m[0]);
    SDF_VU0_STORE_ROW1(&hData->inner->tm.m[4]);
    SDF_VU0_STORE_ROW2(&hData->inner->tm.m[8]);
}

void dds3EffectPartModelSetColor(mdlObj_t *hData, u32 abgr)
{
    mdlSetColor(hData, abgr);
}

dds3EffectPCP_t *dds3EffectPCPCreate(dds3EffectPCPData_t *hPCPData)
{
    dds3EffectPCP_t *hPCP;

    hPCP = sdfChipAllocDirect(sizeof(dds3EffectPCP_t));
    hPCP->kateg = dds3EffectPCPDataGetKateg(hPCPData);
    hPCP->param = dds3EffectPCPDataGetParam(hPCPData);
    if (EffectPCPFuncTable[hPCP->kateg].copy == NULL) {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(effPartPCPDataTable[hPCP->kateg][hPCP->param]);
    } else {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(hPCPData);
    }
    return hPCP;
}

void dds3EffectPCPDelete(dds3EffectPCP_t *hData)
{
    EffectPCPFuncTable[hData->kateg].destroy(hData->pHandle);
    sdfChipFreeDirect(hData);
}

void dds3EffectPCPDraw(dds3EffectPCP_t *hData)
{
    EffectPCPFuncTable[hData->kateg].draw(hData->pHandle);
}

dds3EffectPCP_t *dds3EffectPCPCopy(dds3EffectPCP_t *hPCPSrc)
{
    dds3EffectPCP_t *hPCP;

    hPCP = sdfChipAllocDirect(sizeof(dds3EffectPCP_t));
    hPCP->kateg = hPCPSrc->kateg;
    hPCP->param = hPCPSrc->param;
    if (EffectPCPFuncTable[hPCP->kateg].copy == NULL) {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(effPartPCPDataTable[hPCP->kateg][hPCP->param]);
    } else {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].copy(hPCPSrc->pHandle);
    }
    return hPCP;
}

void dds3EffectPCPSetPosition(dds3EffectPCP_t *hData, void *pPos)
{
    if (EffectPCPFuncTable[hData->kateg].setPosition != NULL) {
        EffectPCPFuncTable[hData->kateg].setPosition(hData->pHandle, pPos);
    }
}

void dds3EffectPCPSetScale(dds3EffectPCP_t *hData, f32 scale)
{
    if (EffectPCPFuncTable[hData->kateg].setScale != NULL) {
        EffectPCPFuncTable[hData->kateg].setScale(hData->pHandle, scale);
    }
}

void dds3EffectPCPSetRotateMatrix(dds3EffectPCP_t *hData, void *pMatrix)
{
    if (EffectPCPFuncTable[hData->kateg].setRotateMatrix != NULL) {
        EffectPCPFuncTable[hData->kateg].setRotateMatrix(hData->pHandle, pMatrix);
    }
}

void dds3EffectPCPSetDirection(dds3EffectPCP_t *hData, void *pMatrix)
{
    if (EffectPCPFuncTable[hData->kateg].setDirection != NULL) {
        EffectPCPFuncTable[hData->kateg].setDirection(hData->pHandle, pMatrix);
    }
}

void dds3EffectPCPSetColor(dds3EffectPCP_t *hData, u32 abgr)
{
    if (EffectPCPFuncTable[hData->kateg].setColor != NULL) {
        EffectPCPFuncTable[hData->kateg].setColor(hData->pHandle, abgr);
    }
}

void dds3EffectPCPSetArea(dds3EffectPCP_t *hData, f32 area)
{
    if (EffectPCPFuncTable[hData->kateg].setArea != NULL) {
        EffectPCPFuncTable[hData->kateg].setArea(hData->pHandle, area);
    }
}

u32 dds3EffectPCPDataGetKateg(dds3EffectPCPData_t *hPCPData)
{
    return hPCPData->kateg;
}

u32 dds3EffectPCPDataGetParam(dds3EffectPCPData_t *hPCPData)
{
    return hPCPData->param;
}

void *dds3EffectPCPDataGetData(dds3EffectPCPData_t *hPCPData, u32 number)
{
    dds3EffectPCPData_Part_t *part = &hPCPData->pcpData[number];

    return (u8 *)hPCPData + part->offset;
}

u32 dds3EffectPCPDataGetSize(dds3EffectPCPData_t *hPCPData, u32 number)
{
    dds3EffectPCPData_Part_t *part = &hPCPData->pcpData[number];

    return part->size;
}

u32 dds3EffectPCPDataGetType(dds3EffectPCPData_t *hPCPData, u32 number)
{
    dds3EffectPCPData_Part_t *part = &hPCPData->pcpData[number];

    return part->type;
}

effPartHandle_t *dds3EffectPCPDataPartCreate(dds3EffectPCPData_t *hPCPData, u32 number)
{
    u16 type;
    void *pData;

    type = (u16)dds3EffectPCPDataGetType(hPCPData, number);
    pData = dds3EffectPCPDataGetData(hPCPData, number);
    return dds3EffectPartCreate(type, pData);
}

#if defined(SDF_DDS1)
INCLUDE_SDATA(const s32, "effect/effPart", D_003BB044);
#elif defined(SDF_DDS2)
INCLUDE_SDATA(const s32, "effect/effPart", D_00436434);
#endif

extern void *D_0031DD20[];
extern void *D_0031DD48[];
extern void *D_0031DDA0[];
extern void *D_0031DDA8[];
extern void *D_0031DDB0[];
extern void *D_0031DDB8[];
extern void *D_0031DDC0[];
extern void *D_0031DDC8[];

void *dds3EffectLoad(void *);
void dds3EffectDraw(void *);
void dds3EffectDelete(void *);
void *dds3EffectCopy(void *);
void dds3EffectSetPosition(void *, void *);
void dds3EffectSetScale(void *, f32);
void dds3EffectSetRotateMatrix(void *, void *);
void dds3EffectSetRGBA(void *, u32);
void dds3BillboardDraw(void *);
void dds3BillboardDelete(void *);
void *dds3BillboardCopy(void *);
void dds3BillboardSetPosition(void *, void *);
void dds3BillboardSetRGBA(void *, u32);
void *dds3P2APolygonCreate(void *);
void dds3P2APolygonDraw(void *);
void dds3P2APolygonDelete(void *);
void *dds3P2APolygonCopy(void *);
void dds3P2APolygonSetPosition(void *, void *);
void dds3P2APolygonSetScale(void *, f32);
void dds3P2APolygonSetRotateMatrix(void *, void *);
void dds3P2APolygonSetRGBA(void *, u32);
void dds3P2APolygonSetDirection(void *, void *);
void *effPCPThunderSpherePolyCreate(void *);
void effPCPThunderSpherePolyDraw(void *);
void effPCPThunderSpherePolyDelete(void *);
void effPCPThunderSpherePolySetPosition(void *, void *);
void effPCPThunderSpherePolySetScale(void *, f32);
void effPCPThunderSpherePolySetColor(void *, u32);
void *effPCP_ThunderPolyCreate(void *);
void effPCP_ThunderPolyDraw(void *);
void effPCP_ThunderPolyDelete(void *);
void effPCP_ThunderPolySetPosition(void *, void *);
void effPCP_ThunderPolySetColor(void *, u32);
void *effPCP_ThunderRadiatePolyCreate(void *);
void effPCP_ThunderRadiatePolyDraw(void *);
void effPCP_ThunderRadiatePolyDelete(void *);
void effPCP_ThunderRadiatePolySetPosition(void *, void *);
void effPCP_ThunderRadiatePolySetColor(void *, u32);
void *effPCPHama01Create(void *);
void effPCPHama01Draw(void *);
void effPCPHama01Delete(void *);
void *effPCPHama01Copy(void *);
void effPCPHama01SetPosition(void *, void *);
void effPCPHama01SetScale(void *, f32);
void effPCPHama01SetColor(void *, u32);
void *effPCPHama02Create(void *);
void effPCPHama02Draw(void *);
void effPCPHama02Delete(void *);
void *effPCPHama02Copy(void *);
void effPCPHama02SetPosition(void *, void *);
void effPCPHama02SetScale(void *, f32);
void effPCPHama02SetColor(void *, u32);
void *effPCPHamaonCreate(void *);
void effPCPHamaonDraw(void *);
void effPCPHamaonDelete(void *);
void *effPCPHamaonCopy(void *);
void effPCPHamaonSetPosition(void *, void *);
void effPCPHamaonSetScale(void *, f32);
void effPCPHamaonSetColor(void *, u32);
void *effPCPMudoCreate(void *);
void effPCPMudoDraw(void *);
void effPCPMudoDelete(void *);
void *effPCPMudoCopy(void *);
void effPCPMudoSetPosition(void *, void *);
void effPCPMudoSetScale(void *, f32);
void effPCPMudoSetColor(void *, u32);
void *effPCPHwave01Create(void *);
void effPCPHwave01Draw(void *);
void effPCPHwave01Delete(void *);
void *effPCPHwave01Copy(void *);
void effPCPHwave01SetPosition(void *, void *);
void effPCPHwave01SetColor(void *, u32);
void *effPCP_ThunderPolyLowCreate(void *);
void effPCP_ThunderPolyLowDraw(void *);
void effPCP_ThunderPolyLowDelete(void *);
void effPCP_ThunderPolyLowSetPosition(void *, void *);
void effPCP_ThunderPolyLowSetColor(void *, u32);
void *effPCPZiodin01Create(void *);
void effPCPZiodin01Draw(void *);
void effPCPZiodin01Delete(void *);
void *effPCPZiodin01Copy(void *);
void effPCPZiodin01SetPosition(void *, void *);
void effPCPZiodin01SetColor(void *, u32);
void *effPCPMultiBlurExpCreate(void *);
void effPCPMultiBlurExpDraw(void *);
void effPCPMultiBlurExpDelete(void *);
void effPCPMultiBlurExpSetPosition(void *, void *);
void effPCPMultiBlurExpSetColor(void *, u32);
void effPCPMultiBlurExpSetArea(void *, f32);
void *effPCPMHanma01Create(void *);
void effPCPMHanma01Draw(void *);
void effPCPMHanma01Delete(void *);
void *effPCPMHanma01Copy(void *);
void effPCPMHanma01SetPosition(void *, void *);
void effPCPMHanma01SetColor(void *, u32);
void *effPCPMZiodin01Create(void *);
void effPCPMZiodin01Draw(void *);
void effPCPMZiodin01Delete(void *);
void *effPCPMZiodin01Copy(void *);
void effPCPMZiodin01SetPosition(void *, void *);
void effPCPMZiodin01SetColor(void *, u32);
void *effPCPSparkleSphereCreate(void *);
void effPCPSparkleSphereDraw(void *);
void effPCPSparkleSphereDelete(void *);
void effPCPSparkleSphereSetPosition(void *, void *);
void effPCPSparkleSphereSetColor(void *, u32);
void effPCPSparkleSphereSetArea(void *, f32);
void *effPCPFlashPolySphereCreate(void *);
void effPCPFlashPolySphereDraw(void *);
void effPCPFlashPolySphereDelete(void *);
void effPCPFlashPolySphereSetPosition(void *, void *);
void effPCPFlashPolySphereSetScale(void *, f32);
void effPCPFlashPolySphereSetColor(void *, u32);
void *effPCPFlashPolySphereCreatePCF(void *);
void *effPCPFlashPolySphereCopy(void *);
void *effPCPFlashPolyRingCreatePCF(void *);
void effPCPFlashPolyRingDraw(void *);
void effPCPFlashPolyRingDelete(void *);
void *effPCPFlashPolyRingCopy(void *);
void effPCPFlashPolyRingSetPosition(void *, void *);
void effPCPFlashPolyRingSetScale(void *, f32);
void effPCPFlashPolyRingSetColor(void *, u32);
void *effPCPFlashPolyPipeCreatePCF(void *);
void effPCPFlashPolyPipeDraw(void *);
void effPCPFlashPolyPipeDelete(void *);
void *effPCPFlashPolyPipeCopy(void *);
void effPCPFlashPolyPipeSetPosition(void *, void *);
void effPCPFlashPolyPipeSetScale(void *, f32);
void effPCPFlashPolyPipeSetColor(void *, u32);
void *effPCPFlashPolySphere2CreatePCF(void *);
void effPCPFlashPolySphere2Draw(void *);
void effPCPFlashPolySphere2Delete(void *);
void *effPCPFlashPolySphere2Copy(void *);
void effPCPFlashPolySphere2SetPosition(void *, void *);
void effPCPFlashPolySphere2SetScale(void *, f32);
void effPCPFlashPolySphere2SetColor(void *, u32);
void *effPCPFlashPolyRingExpCreatePCF(void *);
void effPCPFlashPolyRingExpDraw(void *);
void effPCPFlashPolyRingExpDelete(void *);
void *effPCPFlashPolyRingExpCopy(void *);
void effPCPFlashPolyRingExpSetPosition(void *, void *);
void effPCPFlashPolyRingExpSetScale(void *, f32);
void effPCPFlashPolyRingExpSetColor(void *, u32);
void *effPCPMZiodin02Create(void *);
void effPCPMZiodin02Draw(void *);
void effPCPMZiodin02Delete(void *);
void *effPCPMZiodin02Copy(void *);
void effPCPMZiodin02SetPosition(void *, void *);
void effPCPMZiodin02SetColor(void *, u32);
void *effPCPZiodin02Create(void *);
void effPCPZiodin02Draw(void *);
void effPCPZiodin02Delete(void *);
void *effPCPZiodin02Copy(void *);
void effPCPZiodin02SetPosition(void *, void *);
void effPCPZiodin02SetColor(void *, u32);
void *effPCPZiodin03Create(void *);
void effPCPZiodin03Draw(void *);
void effPCPZiodin03Delete(void *);
void *effPCPZiodin03Copy(void *);
void effPCPZiodin03SetPosition(void *, void *);
void effPCPZiodin03SetColor(void *, u32);
void *effPCPZiodin04Create(void *);
void effPCPZiodin04Draw(void *);
void effPCPZiodin04Delete(void *);
void *effPCPZiodin04Copy(void *);
void effPCPZiodin04SetPosition(void *, void *);
void effPCPZiodin04SetColor(void *, u32);
void *effPCPShockWave01Create(void *);
void effPCPShockWave01Draw(void *);
void effPCPShockWave01Delete(void *);
void *effPCPShockWave01Copy(void *);
void effPCPShockWave01SetPosition(void *, void *);
void effPCPShockWave01SetColor(void *, u32);
void *effPCPHouden01Create(void *);
void effPCPHouden01Draw(void *);
void effPCPHouden01Delete(void *);
void *effPCPHouden01Copy(void *);
void effPCPHouden01SetPosition(void *, void *);
void effPCPHouden01SetColor(void *, u32);
void *effPCPMultiFilterExpCreate(void *);
void effPCPMultiFilterExpDraw(void *);
void effPCPMultiFilterExpDelete(void *);
void effPCPMultiFilterExpSetPosition(void *, void *);
void effPCPMultiFilterExpSetColor(void *, u32);
void effPCPMultiFilterExpSetArea(void *, f32);
void *effPCPBlinkMultiBlurCreate(void *);
void effPCPBlinkMultiBlurDraw(void *);
void effPCPBlinkMultiBlurDelete(void *);
void effPCPBlinkMultiBlurSetPosition(void *, void *);
void effPCPBlinkMultiBlurSetColor(void *, u32);
void effPCPBlinkMultiBlurSetArea(void *, f32);
void *effPCPMultiFilterExpCreatePCF(void *);
void *effPCPMultiFilterExpCopy(void *);
void *effPCPShockWave02Create(void *);
void effPCPShockWave02Draw(void *);
void effPCPShockWave02Delete(void *);
void *effPCPShockWave02Copy(void *);
void effPCPShockWave02SetPosition(void *, void *);
void effPCPShockWave02SetColor(void *, u32);
void *effPCPFilterExpCreatePCF(void *);
void effPCPFilterExpDraw(void *);
void effPCPFilterExpDelete(void *);
void *effPCPFilterExpCopy(void *);
void effPCPFilterExpSetColor(void *, u32);
void *effPCPBlurExpCreatePCF(void *);
void effPCPBlurExpDraw(void *);
void effPCPBlurExpDelete(void *);
void *effPCPBlurExpCopy(void *);
void effPCPBlurExpSetColor(void *, u32);
void *effPCPDistortionBlurExpCreatePCF(void *);
void effPCPDistortionBlurExpDraw(void *);
void effPCPDistortionBlurExpDelete(void *);
void *effPCPDistortionBlurExpCopy(void *);
void effPCPDistortionBlurExpSetPosition(void *, void *);
void effPCPDistortionBlurExpSetColor(void *, u32);
void *effPCPMultiBlurExp2CreatePCF(void *);
void effPCPMultiBlurExp2Draw(void *);
void effPCPMultiBlurExp2Delete(void *);
void *effPCPMultiBlurExp2Copy(void *);
void effPCPMultiBlurExp2SetPosition(void *, void *);
void effPCPMultiBlurExp2SetColor(void *, u32);
void *effPCPMegidora01Create(void *);
void effPCPMegidora01Draw(void *);
void effPCPMegidora01Delete(void *);
void *effPCPMegidora01Copy(void *);
void effPCPMegidora01SetPosition(void *, void *);
void effPCPMegidora01SetColor(void *, u32);
void *effPCPMegidoraon01Create(void *);
void effPCPMegidoraon01Draw(void *);
void effPCPMegidoraon01Delete(void *);
void *effPCPMegidoraon01Copy(void *);
void effPCPMegidoraon01SetPosition(void *, void *);
void effPCPMegidoraon01SetColor(void *, u32);
void *effPCPBreathCreatePCF(void *);
void effPCPBreathDraw(void *);
void effPCPBreathDelete(void *);
void *effPCPBreathCopy(void *);
void effPCPBreathSetPosition(void *, void *);
void effPCPBreathSetRotateMatrix(void *, void *);
void effPCPBreathSetColor(void *, u32);
void *effPCPHamaCardSpiralCreatePCF(void *);
void effPCPHamaCardSpiralDraw(void *);
void effPCPHamaCardSpiralDelete(void *);
void *effPCPHamaCardSpiralCopy(void *);
void effPCPHamaCardSpiralSetPosition(void *, void *);
void effPCPHamaCardSpiralSetScale(void *, f32);
void effPCPHamaCardSpiralSetColor(void *, u32);
void *effPCPHamaCardSphereCreatePCF(void *);
void effPCPHamaCardSphereDraw(void *);
void effPCPHamaCardSphereDelete(void *);
void *effPCPHamaCardSphereCopy(void *);
void effPCPHamaCardSphereSetPosition(void *, void *);
void effPCPHamaCardSphereSetScale(void *, f32);
void effPCPHamaCardSphereSetColor(void *, u32);
void *effPCPAgiraoFireCreatePCF(void *);
void effPCPAgiraoFireDraw(void *);
void effPCPAgiraoFireDelete(void *);
void *effPCPAgiraoFireCopy(void *);
void effPCPAgiraoFireSetPosition(void *, void *);
void effPCPAgiraoFireSetColor(void *, u32);
void *effPCPBufuIceCreate(void *);
void effPCPBufuIceDraw(void *);
void effPCPBufuIceDelete(void *);
void *effPCPBufuIceCopy(void *);
void effPCPBufuIceSetPosition(void *, void *);
void effPCPBufuIceSetScale(void *, f32);
void effPCPBufuIceSetRotateMatrix(void *, void *);
void effPCPBufuIceSetColor(void *, u32);
void *effPCPBufuraIceCreate(void *);
void effPCPBufuraIceDraw(void *);
void effPCPBufuraIceDelete(void *);
void *effPCPBufuraIceCopy(void *);
void effPCPBufuraIceSetPosition(void *, void *);
void effPCPBufuraIceSetScale(void *, f32);
void effPCPBufuraIceSetRotateMatrix(void *, void *);
void effPCPBufuraIceSetColor(void *, u32);
void *effPCPBufudnIceCreate(void *);
void effPCPBufudnIceDraw(void *);
void effPCPBufudnIceDelete(void *);
void *effPCPBufudnIceCopy(void *);
void effPCPBufudnIceSetPosition(void *, void *);
void effPCPBufudnIceSetScale(void *, f32);
void effPCPBufudnIceSetRotateMatrix(void *, void *);
void effPCPBufudnIceSetColor(void *, u32);
void *effPCPBufuLightCreate(void *);
void effPCPBufuLightDraw(void *);
void effPCPBufuLightDelete(void *);
void *effPCPBufuLightCopy(void *);
void *effPCPAgiFireballCreatePCF(void *);
void effPCPAgiFireballDraw(void *);
void effPCPAgiFireballDelete(void *);
void *effPCPAgiFireballCopy(void *);
void effPCPAgiFireballSetPosition(void *, void *);
void effPCPAgiFireballSetRotateMatrix(void *, void *);
void effPCPAgiFireballSetColor(void *, u32);
void *effPCPNeedleSpiralCreatePCF(void *);
void effPCPNeedleSpiralDraw(void *);
void effPCPNeedleSpiralDelete(void *);
void *effPCPNeedleSpiralCopy(void *);
void effPCPNeedleSpiralSetPosition(void *, void *);
void effPCPNeedleSpiralSetColor(void *, u32);
void *effPCPEyeThunderCreate00_0(void *);
void effPCPEyeThunderDraw(void *);
void effPCPEyeThunderDelete(void *);
void *effPCPEyeThunderCopy00_0(void *);
void effPCPEyeThunderSetPosition(void *, void *);
void effPCPEyeThunderSetColor(void *, u32);
void *effPCPEyeThunderCreate00_1(void *);
void *effPCPEyeThunderCopy00_1(void *);
void *effPCPEyeThunderCreate01_0(void *);
void *effPCPEyeThunderCopy01_0(void *);
void *effPCPEyeThunderCreate01_1(void *);
void *effPCPEyeThunderCopy01_1(void *);
void *effPCPEyeThunderCreate02_0(void *);
void *effPCPEyeThunderCopy02_0(void *);
void *effPCPEyeThunderCreate02_1(void *);
void *effPCPEyeThunderCopy02_1(void *);
void *effPCPEyeThunderCreate03_0(void *);
void *effPCPEyeThunderCopy03_0(void *);
void *effPCPEyeThunderCreate03_1(void *);
void *effPCPEyeThunderCopy03_1(void *);
void *effPCPEyeThunderCreate04_0(void *);
void *effPCPEyeThunderCopy04_0(void *);
void *effPCPEyeThunderCreate04_1(void *);
void *effPCPEyeThunderCopy04_1(void *);
void *effPCPRippleBlurExpCreatePCF(void *);
void effPCPRippleBlurExpDraw(void *);
void effPCPRippleBlurExpDelete(void *);
void *effPCPRippleBlurExpCopy(void *);
void effPCPRippleBlurExpSetPosition(void *, void *);
void effPCPRippleBlurExpSetColor(void *, u32);
void *effPCPThunderSphereExpCreatePCF(void *);
void effPCPThunderSphereExpDraw(void *);
void effPCPThunderSphereExpDelete(void *);
void *effPCPThunderSphereExpCopy(void *);
void effPCPThunderSphereExpSetPosition(void *, void *);
void effPCPThunderSphereExpSetScale(void *, f32);
void effPCPThunderSphereExpSetColor(void *, u32);
void *effPCPRingPolyExpCreatePCF(void *);
void effPCPRingPolyExpDraw(void *);
void effPCPRingPolyExpDelete(void *);
void *effPCPRingPolyExpCopy(void *);
void effPCPRingPolyExpSetPosition(void *, void *);
void effPCPRingPolyExpSetScale(void *, f32);
void effPCPRingPolyExpSetRotateMatrix(void *, void *);
void effPCPRingPolyExpSetColor(void *, u32);
void effPCPRingPolyExpSetDirection(void *, void *);
void *effPCPWhirlwindPolyCreatePCF(void *);
void effPCPWhirlwindPolyDraw(void *);
void effPCPWhirlwindPolyDelete(void *);
void *effPCPWhirlwindPolyCopy(void *);
void effPCPWhirlwindPolySetPosition(void *, void *);
void effPCPWhirlwindPolySetScale(void *, f32);
void effPCPWhirlwindPolySetRotateMatrix(void *, void *);
void effPCPWhirlwindPolySetColor(void *, u32);
void *effPCPEyeThunderCreate05_0(void *);
void *effPCPEyeThunderCopy05_0(void *);
void *effPCPEyeThunderCreate05_1(void *);
void *effPCPEyeThunderCopy05_1(void *);
void *effPCPRadiateThunderCreatePCF(void *);
void effPCPRadiateThunderDraw(void *);
void effPCPRadiateThunderDelete(void *);
void *effPCPRadiateThunderCopy(void *);
void effPCPRadiateThunderSetPosition(void *, void *);
void effPCPRadiateThunderSetScale(void *, f32);
void effPCPRadiateThunderSetColor(void *, u32);
void *effPCPWwSpherePolyCreatePCF(void *);
void effPCPWwSpherePolyDraw(void *);
void effPCPWwSpherePolyDelete(void *);
void *effPCPWwSpherePolyCopy(void *);
void effPCPWwSpherePolySetPosition(void *, void *);
void effPCPWwSpherePolySetScale(void *, f32);
void effPCPWwSpherePolySetRotateMatrix(void *, void *);
void effPCPWwSpherePolySetColor(void *, u32);
void *effPCPFlashPolySphereExpCreatePCF(void *);
void effPCPFlashPolySphereExpDraw(void *);
void effPCPFlashPolySphereExpDelete(void *);
void *effPCPFlashPolySphereExpCopy(void *);
void effPCPFlashPolySphereExpSetPosition(void *, void *);
void effPCPFlashPolySphereExpSetScale(void *, f32);
void effPCPFlashPolySphereExpSetColor(void *, u32);
void *effPCPFlashPolySphere2ExpCreatePCF(void *);
void effPCPFlashPolySphere2ExpDraw(void *);
void effPCPFlashPolySphere2ExpDelete(void *);
void *effPCPFlashPolySphere2ExpCopy(void *);
void effPCPFlashPolySphere2ExpSetPosition(void *, void *);
void effPCPFlashPolySphere2ExpSetScale(void *, f32);
void effPCPFlashPolySphere2ExpSetColor(void *, u32);
void *effPCPFlashPolySphere3CreatePCF(void *);
void effPCPFlashPolySphere3Draw(void *);
void effPCPFlashPolySphere3Delete(void *);
void *effPCPFlashPolySphere3Copy(void *);
void effPCPFlashPolySphere3SetPosition(void *, void *);
void effPCPFlashPolySphere3SetScale(void *, f32);
void effPCPFlashPolySphere3SetColor(void *, u32);
void *effPCPFlashPolyRingExp2CreatePCF(void *);
void effPCPFlashPolyRingExp2Draw(void *);
void effPCPFlashPolyRingExp2Delete(void *);
void *effPCPFlashPolyRingExp2Copy(void *);
void effPCPFlashPolyRingExp2SetPosition(void *, void *);
void effPCPFlashPolyRingExp2SetScale(void *, f32);
void effPCPFlashPolyRingExp2SetColor(void *, u32);
void *effPCPWhirlwind2PolyCreatePCF(void *);
void effPCPWhirlwind2PolyDraw(void *);
void effPCPWhirlwind2PolyDelete(void *);
void *effPCPWhirlwind2PolyCopy(void *);
void effPCPWhirlwind2PolySetPosition(void *, void *);
void effPCPWhirlwind2PolySetScale(void *, f32);
void effPCPWhirlwind2PolySetRotateMatrix(void *, void *);
void effPCPWhirlwind2PolySetColor(void *, u32);
void *effPCPMakkaCreat(void *);
void effPCPMakkaDraw(void *);
void effPCPMakkaDelete(void *);
void *effPCPMakkaCopy(void *);
void effPCPMakkaSetPosition(void *, void *);
void effPCPMakkaSetScale(void *, f32);
void effPCPMakkaSetColor(void *, u32);
void *effPCPRingPolyExp2CreatePCF(void *);
void effPCPRingPolyExp2Draw(void *);
void effPCPRingPolyExp2Delete(void *);
void *effPCPRingPolyExp2Copy(void *);
void effPCPRingPolyExp2SetPosition(void *, void *);
void effPCPRingPolyExp2SetScale(void *, f32);
void effPCPRingPolyExp2SetColor(void *, u32);
void *effPCPAgiFireball2CreatePCF(void *);
void effPCPAgiFireball2Draw(void *);
void effPCPAgiFireball2Delete(void *);
void *effPCPAgiFireball2Copy(void *);
void effPCPAgiFireball2SetPosition(void *, void *);
void effPCPAgiFireball2SetRotateMatrix(void *, void *);
void effPCPAgiFireball2SetColor(void *, u32);
void *effPCPRadiateBEDCreatePCF(void *);
void effPCPRadiateBEDDraw(void *);
void effPCPRadiateBEDDelete(void *);
void *effPCPRadiateBEDCopy(void *);
void effPCPRadiateBEDSetPosition(void *, void *);
void effPCPRadiateBEDSetScale(void *, f32);
void effPCPRadiateBEDSetColor(void *, u32);
void *effAPPoint4CreatePCF(void *);
void effAPPoint4Draw(void *);
void effAPPoint4Delete(void *);
void *effAPPoint4Copy(void *);
void *effPCPRadiateTBCreatePCF(void *);
void effPCPRadiateTBDraw(void *);
void effPCPRadiateTBDelete(void *);
void *effPCPRadiateTBCopy(void *);
void effPCPRadiateTBSetPosition(void *, void *);
void effPCPRadiateTBSetColor(void *, u32);
void *effPCPWhirlwind3PolyCreatePCF(void *);
void effPCPWhirlwind3PolyDraw(void *);
void effPCPWhirlwind3PolyDelete(void *);
void *effPCPWhirlwind3PolyCopy(void *);
void effPCPWhirlwind3PolySetPosition(void *, void *);
void effPCPWhirlwind3PolySetScale(void *, f32);
void effPCPWhirlwind3PolySetRotateMatrix(void *, void *);
void effPCPWhirlwind3PolySetColor(void *, u32);
void *effPCPThunderSpherePolyCreatePCF(void *);
void *effPCPThunderSpherePolyCopy(void *);
void *effPCPThunderSpherePolyLowCreate(void *);
void effPCPThunderSpherePolyLowDraw(void *);
void effPCPThunderSpherePolyLowDelete(void *);
void *effPCPThunderSpherePolyLowCopy(void *);
void effPCPThunderSpherePolyLowSetPosition(void *, void *);
void effPCPThunderSpherePolyLowSetScale(void *, f32);
void effPCPThunderSpherePolyLowSetColor(void *, u32);
void *effPCPSpiralBEDCreatePCF(void *);
void effPCPSpiralBEDDraw(void *);
void effPCPSpiralBEDDelete(void *);
void *effPCPSpiralBEDCopy(void *);
void effPCPSpiralBEDSetPosition(void *, void *);
void effPCPSpiralBEDSetScale(void *, f32);
void effPCPSpiralBEDSetColor(void *, u32);
void *effPCPAgiFireballSpCreatePCF(void *);
void *effPCPAgiFireballSpCopy(void *);
void *effPCPAgiFireballSp2CreatePCF(void *);
void *effPCPAgiFireballSp2Copy(void *);
void *effPCPBezierBillCreatePCF(void *);
void effPCPBezierBillDraw(void *);
void effPCPBezierBillDelete(void *);
void *effPCPBezierBillCopy(void *);
void effPCPBezierBillSetPosition(void *, void *);
void effPCPBezierBillSetColor(void *, u32);
void *effPCPCrackCreatePCF(void *);
void effPCPCrackDraw(void *);
void effPCPCrackDelete(void *);
void *effPCPCrackCopy(void *);
void effPCPCrackSetPosition(void *, void *);
void effPCPCrackSetColor(void *, u32);
void *effPCPHelperBEDCreatePCF(void *);
void effPCPHelperBEDDraw(void *);
void effPCPHelperBEDDelete(void *);
void *effPCPHelperBEDCopy(void *);
void effPCPHelperBEDSetPosition(void *, void *);
void effPCPHelperBEDSetScale(void *, f32);
void effPCPHelperBEDSetColor(void *, u32);
void *effPCPFlashPolyCircleCreatePCF(void *);
void effPCPFlashPolyCircleDraw(void *);
void effPCPFlashPolyCircleDelete(void *);
void *effPCPFlashPolyCircleCopy(void *);
void effPCPFlashPolyCircleSetPosition(void *, void *);
void effPCPFlashPolyCircleSetScale(void *, f32);
void effPCPFlashPolyCircleSetColor(void *, u32);
void *effPCPFlashPolyRingExpHCreatePCF(void *);
void effPCPFlashPolyRingExpHDraw(void *);
void effPCPFlashPolyRingExpHDelete(void *);
void *effPCPFlashPolyRingExpHCopy(void *);
void effPCPFlashPolyRingExpHSetPosition(void *, void *);
void effPCPFlashPolyRingExpHSetScale(void *, f32);
void effPCPFlashPolyRingExpHSetRotateMatrix(void *, void *);
void effPCPFlashPolyRingExpHSetColor(void *, u32);
void *effPCPBossBaelCreatePCF(void *);
void effPCPBossBaelDraw(void *);
void effPCPBossBaelDelete(void *);
void *effPCPBossBaelCopy(void *);
void effPCPBossBaelSetPosition(void *, void *);
void effPCPBossBaelSetColor(void *, u32);
void *effPCPBossDengekiCreatePCF(void *);
void effPCPBossDengekiDraw(void *);
void effPCPBossDengekiDelete(void *);
void *effPCPBossDengekiCopy(void *);
void effPCPBossDengekiSetColor(void *, u32);
void *effPCPAnalyzeBEDCreatePCF(void *);
void effPCPAnalyzeBEDDraw(void *);
void effPCPAnalyzeBEDDelete(void *);
void *effPCPAnalyzeBEDCopy(void *);
void effPCPAnalyzeBEDSetScale(void *, f32);
void effPCPAnalyzeBEDSetColor(void *, u32);
void *effPCPBossThunderboltCreatePCF(void *);
void effPCPBossThunderboltDraw(void *);
void effPCPBossThunderboltDelete(void *);
void *effPCPBossThunderboltCopy(void *);
void effPCPBossThunderboltSetPosition(void *, void *);
void effPCPBossThunderboltSetColor(void *, u32);
void *effPCPBossKAGUInvokeCreatePCF(void *);
void effPCPBossKAGUInvokeDraw(void *);
void effPCPBossKAGUInvokeDelete(void *);
void *effPCPBossKAGUInvokeCopy(void *);
void effPCPBossKAGUInvokeSetColor(void *, u32);
void *effPCPBossKAGUAttackCreatePCF(void *);
void effPCPBossKAGUAttackDraw(void *);
void effPCPBossKAGUAttackDelete(void *);
void *effPCPBossKAGUAttackCopy(void *);
void effPCPBossKAGUAttackSetColor(void *, u32);
void *effPCPBossKAGUThunderboltCreatePCF(void *);
void effPCPBossKAGUThunderboltDraw(void *);
void effPCPBossKAGUThunderboltDelete(void *);
void *effPCPBossKAGUThunderboltCopy(void *);
void effPCPBossKAGUThunderboltSetPosition(void *, void *);
void effPCPBossKAGUThunderboltSetColor(void *, u32);
void *effPCPBossKAGU2AttackCreatePCF(void *);
void effPCPBossKAGU2AttackDraw(void *);
void effPCPBossKAGU2AttackDelete(void *);
void *effPCPBossKAGU2AttackCopy(void *);
void effPCPBossKAGU2AttackSetColor(void *, u32);
void effPCPBossKAGU2ThunderboltDraw(void *);
void *effPCPFLEACHBlurCreatePCF(void *);
void effPCPFLEACHBlurDraw(void *);
void effPCPFLEACHBlurDelete(void *);
void *effPCPFLEACHBlurCopy(void *);
void effPCPFLEACHBlurSetColor(void *, u32);
void *effPCPCrack2CreatePCF(void *);
void effPCPCrack2Draw(void *);
void effPCPCrack2Delete(void *);
void *effPCPCrack2Copy(void *);
void effPCPCrack2SetPosition(void *, void *);
void effPCPCrack2SetColor(void *, u32);
void *effPCPBossMETATOBeamCreatePCF(void *);
void effPCPBossMETATOBeamDraw(void *);
void effPCPBossMETATOBeamDelete(void *);
void *effPCPBossMETATOBeamCopy(void *);
void effPCPBossMETATOBeamSetPosition(void *, void *);
void effPCPBossMETATOBeamSetColor(void *, u32);
void *effPCPScreenCrackModelCreatePCF(void *);
void effPCPScreenCrackModelDraw(void *);
void effPCPScreenCrackModelDelete(void *);
void *effPCPScreenCrackModelCopy(void *);
void effPCPScreenCrackModelSetColor(void *, u32);
void *effPCPMuzzleFlashCreatePCF(void *);
void effPCPMuzzleFlashDraw(void *);
void effPCPMuzzleFlashDelete(void *);
void *effPCPMuzzleFlashCopy(void *);
void effPCPMuzzleFlashSetPosition(void *, void *);
void effPCPMuzzleFlashSetScale(void *, f32);
void effPCPMuzzleFlashSetRotateMatrix(void *, void *);
void effPCPMuzzleFlashSetColor(void *, u32);
void *effPCPDANTE_Thunder_CreatePCF(void *);
void effPCPDANTE_Thunder_DrawSub(void *);
void effPCPDANTE_Thunder_Delete(void *);
void *effPCPDANTE_Thunder_Copy(void *);
void effPCPDANTE_Thunder_SetPosition(void *, void *);
void effPCPDANTE_Thunder_SetRotateMatrix(void *, void *);
void effPCPDANTE_Thunder_SetColor(void *, u32);
void *effPCPDANTE_Damage_CreatePCF(void *);
void effPCPDANTE_Damage_Run(void *);
void effPCPDANTE_Damage_Delete(void *);
void *effPCPDANTE_Damage_Copy(void *);
void effPCPDANTE_Damage_SetColor(void *, u32);
void *effPCP_PBCamera_CreatePCF(void *);
void effPCP_PBCamera_Run(void *);
void effPCP_PBCamera_Delete(void *);
void *effPCP_PBCamera_Copy(void *);
void effPCP_PBCamera_SetPosition(void *, void *);
void effPCP_PBCamera_SetRotateMatrix(void *, void *);
void *effPCPDANTE_ShowTime_CreatePCF(void *);
void effPCPDANTE_ShowTime_Draw(void *);
void effPCPDANTE_ShowTime_Delete(void *);
void *effPCPDANTE_ShowTime_Copy(void *);
void effPCPDANTE_ShowTime_SetPosition(void *, void *);
void *effPCP_BED_CreatePCF(void *);
void effPCP_BED_Draw(void *);
void effPCP_BED_Delete(void *);
void *effPCP_BED_Copy(void *);
void effPCP_BED_SetPosition(void *, void *);
void effPCP_BED_SetRotateMatrix(void *, void *);
void effPCP_BED_SetColor(void *, u32);
void *effPCP_HBED2_CreatePCF(void *);
void effPCP_HBED2_Draw(void *);
void effPCP_HBED2_Delete(void *);
void *effPCP_HBED2_Copy(void *);
void effPCP_HBED2_SetPosition(void *, void *);
void effPCP_HBED2_SetScale(void *, f32);
void effPCP_HBED2_SetRotateMatrix(void *, void *);
void effPCP_HBED2_SetColor(void *, u32);

void **effPartPCPDataTable[28] = {
    (void **)&D_0031DD20, (void **)&D_0031DD48, (void **)&D_0031DDA0, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, (void **)&D_0031DDA8, NULL,
    NULL, (void **)&D_0031DDB0, (void **)&D_0031DDB8, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, (void **)&D_0031DDC0, (void **)&D_0031DDC8,
};
dds3EffectFunc_t dds3EffectFuncTable[7] = {
    { dds3EffectLoad, dds3EffectDraw, dds3EffectDelete, dds3EffectCopy, dds3EffectSetPosition, dds3EffectSetScale, dds3EffectSetRotateMatrix, dds3EffectSetRGBA, NULL, NULL },
    { (void * (*)(void *))dds3EffectPartP2ABillboardCreate, dds3BillboardDraw, dds3BillboardDelete, dds3BillboardCopy, dds3BillboardSetPosition, dds3EffectPartBillboardScale, NULL, dds3BillboardSetRGBA, NULL, NULL },
    { (void * (*)(void *))dds3EffectPartTMXBillboardCreate, dds3BillboardDraw, dds3BillboardDelete, dds3BillboardCopy, dds3BillboardSetPosition, dds3EffectPartBillboardScale, NULL, dds3BillboardSetRGBA, NULL, NULL },
    { (void * (*)(void *))dds3EffectPartModelCreate, (void (*)(void *))dds3EffectPartModelDraw, (void (*)(void *))dds3EffectPartModelDelete, (void * (*)(void *))dds3EffectPartModelCopy, (void (*)(void *, void *))dds3EffectPartModelSetPosition, dds3EffectPartModelSetScale, (void (*)(void *, void *))dds3EffectPartModelSetRotateMatrix, (void (*)(void *, u32))dds3EffectPartModelSetColor, NULL, NULL },
    { (void * (*)(void *))dds3EffectPCPCreate, (void (*)(void *))dds3EffectPCPDraw, (void (*)(void *))dds3EffectPCPDelete, (void * (*)(void *))dds3EffectPCPCopy, (void (*)(void *, void *))dds3EffectPCPSetPosition, (void (*)(void *, f32))dds3EffectPCPSetScale, (void (*)(void *, void *))dds3EffectPCPSetRotateMatrix, (void (*)(void *, u32))dds3EffectPCPSetColor, NULL, (void (*)(void *, f32))dds3EffectPCPSetArea },
    { dds3P2APolygonCreate, dds3P2APolygonDraw, dds3P2APolygonDelete, dds3P2APolygonCopy, dds3P2APolygonSetPosition, dds3P2APolygonSetScale, dds3P2APolygonSetRotateMatrix, dds3P2APolygonSetRGBA, dds3P2APolygonSetDirection, NULL },
    { (void * (*)(void *))dds3EffectPCPCreate, (void (*)(void *))dds3EffectPCPDraw, (void (*)(void *))dds3EffectPCPDelete, (void * (*)(void *))dds3EffectPCPCopy, (void (*)(void *, void *))dds3EffectPCPSetPosition, (void (*)(void *, f32))dds3EffectPCPSetScale, (void (*)(void *, void *))dds3EffectPCPSetRotateMatrix, (void (*)(void *, u32))dds3EffectPCPSetColor, (void (*)(void *, void *))dds3EffectPCPSetDirection, (void (*)(void *, f32))dds3EffectPCPSetArea },
};
sdfVector4_t effPartModelPosInit __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
sdfVector4_t effPartModelRotInit __attribute__((aligned(16))) = { 0.0f, 0.0f, 0.0f, 0.0f };
sdfVector4_t effPartModelScaleInit __attribute__((aligned(16))) = { 1.0f, 1.0f, 1.0f, 0.0f };
/* Indexed by PCP category. The IDB names entries from 1 on, as effPCP*Table. */
dds3EffectFunc_t EffectPCPFuncTable[106] = {
    { effPCPThunderSpherePolyCreate, effPCPThunderSpherePolyDraw, effPCPThunderSpherePolyDelete, NULL, effPCPThunderSpherePolySetPosition, effPCPThunderSpherePolySetScale, NULL, effPCPThunderSpherePolySetColor, NULL, NULL },
    /* effPCPThunderPolyTable */
    { effPCP_ThunderPolyCreate, effPCP_ThunderPolyDraw, effPCP_ThunderPolyDelete, NULL, effPCP_ThunderPolySetPosition, NULL, NULL, effPCP_ThunderPolySetColor, NULL, NULL },
    /* effPCPThunderRadiatePolyTable */
    { effPCP_ThunderRadiatePolyCreate, effPCP_ThunderRadiatePolyDraw, effPCP_ThunderRadiatePolyDelete, NULL, effPCP_ThunderRadiatePolySetPosition, NULL, NULL, effPCP_ThunderRadiatePolySetColor, NULL, NULL },
    /* effPCPHama01Table */
    { effPCPHama01Create, effPCPHama01Draw, effPCPHama01Delete, effPCPHama01Copy, effPCPHama01SetPosition, effPCPHama01SetScale, NULL, effPCPHama01SetColor, NULL, NULL },
    /* effPCPHama02Table */
    { effPCPHama02Create, effPCPHama02Draw, effPCPHama02Delete, effPCPHama02Copy, effPCPHama02SetPosition, effPCPHama02SetScale, NULL, effPCPHama02SetColor, NULL, NULL },
    /* effPCPHamaonTable */
    { effPCPHamaonCreate, effPCPHamaonDraw, effPCPHamaonDelete, effPCPHamaonCopy, effPCPHamaonSetPosition, effPCPHamaonSetScale, NULL, effPCPHamaonSetColor, NULL, NULL },
    /* effPCPMudoTable */
    { effPCPMudoCreate, effPCPMudoDraw, effPCPMudoDelete, effPCPMudoCopy, effPCPMudoSetPosition, effPCPMudoSetScale, NULL, effPCPMudoSetColor, NULL, NULL },
    /* effPCPHwave01Table */
    { effPCPHwave01Create, effPCPHwave01Draw, effPCPHwave01Delete, effPCPHwave01Copy, effPCPHwave01SetPosition, NULL, NULL, effPCPHwave01SetColor, NULL, NULL },
    /* effPCPThunderPolyLowTable */
    { effPCP_ThunderPolyLowCreate, effPCP_ThunderPolyLowDraw, effPCP_ThunderPolyLowDelete, NULL, effPCP_ThunderPolyLowSetPosition, NULL, NULL, effPCP_ThunderPolyLowSetColor, NULL, NULL },
    /* effPCPZiodin01Table */
    { effPCPZiodin01Create, effPCPZiodin01Draw, effPCPZiodin01Delete, effPCPZiodin01Copy, effPCPZiodin01SetPosition, NULL, NULL, effPCPZiodin01SetColor, NULL, NULL },
    /* effPCPMultiBlurExpTable */
    { effPCPMultiBlurExpCreate, effPCPMultiBlurExpDraw, effPCPMultiBlurExpDelete, NULL, effPCPMultiBlurExpSetPosition, NULL, NULL, effPCPMultiBlurExpSetColor, NULL, effPCPMultiBlurExpSetArea },
    /* effPCPMHanma01Table */
    { effPCPMHanma01Create, effPCPMHanma01Draw, effPCPMHanma01Delete, effPCPMHanma01Copy, effPCPMHanma01SetPosition, NULL, NULL, effPCPMHanma01SetColor, NULL, NULL },
    /* effPCPMZiodin01Table */
    { effPCPMZiodin01Create, effPCPMZiodin01Draw, effPCPMZiodin01Delete, effPCPMZiodin01Copy, effPCPMZiodin01SetPosition, NULL, NULL, effPCPMZiodin01SetColor, NULL, NULL },
    /* effPCPSparkleSphereTable */
    { effPCPSparkleSphereCreate, effPCPSparkleSphereDraw, effPCPSparkleSphereDelete, NULL, effPCPSparkleSphereSetPosition, NULL, NULL, effPCPSparkleSphereSetColor, NULL, effPCPSparkleSphereSetArea },
    { effPCPFlashPolySphereCreate, effPCPFlashPolySphereDraw, effPCPFlashPolySphereDelete, NULL, effPCPFlashPolySphereSetPosition, effPCPFlashPolySphereSetScale, NULL, effPCPFlashPolySphereSetColor, NULL, NULL },
    /* effPCPFlashPolySphereTable */
    { effPCPFlashPolySphereCreatePCF, effPCPFlashPolySphereDraw, effPCPFlashPolySphereDelete, effPCPFlashPolySphereCopy, effPCPFlashPolySphereSetPosition, effPCPFlashPolySphereSetScale, NULL, effPCPFlashPolySphereSetColor, NULL, NULL },
    /* effPCPFlashPolyRingTable */
    { effPCPFlashPolyRingCreatePCF, effPCPFlashPolyRingDraw, effPCPFlashPolyRingDelete, effPCPFlashPolyRingCopy, effPCPFlashPolyRingSetPosition, effPCPFlashPolyRingSetScale, NULL, effPCPFlashPolyRingSetColor, NULL, NULL },
    /* effPCPFlashPolyPipeTable */
    { effPCPFlashPolyPipeCreatePCF, effPCPFlashPolyPipeDraw, effPCPFlashPolyPipeDelete, effPCPFlashPolyPipeCopy, effPCPFlashPolyPipeSetPosition, effPCPFlashPolyPipeSetScale, NULL, effPCPFlashPolyPipeSetColor, NULL, NULL },
    /* effPCPFlashPolySphere2Table */
    { effPCPFlashPolySphere2CreatePCF, effPCPFlashPolySphere2Draw, effPCPFlashPolySphere2Delete, effPCPFlashPolySphere2Copy, effPCPFlashPolySphere2SetPosition, effPCPFlashPolySphere2SetScale, NULL, effPCPFlashPolySphere2SetColor, NULL, NULL },
    /* effPCPFlashPolyRingExpTable */
    { effPCPFlashPolyRingExpCreatePCF, effPCPFlashPolyRingExpDraw, effPCPFlashPolyRingExpDelete, effPCPFlashPolyRingExpCopy, effPCPFlashPolyRingExpSetPosition, effPCPFlashPolyRingExpSetScale, NULL, effPCPFlashPolyRingExpSetColor, NULL, NULL },
    /* effPCPMZiodin02Table */
    { effPCPMZiodin02Create, effPCPMZiodin02Draw, effPCPMZiodin02Delete, effPCPMZiodin02Copy, effPCPMZiodin02SetPosition, NULL, NULL, effPCPMZiodin02SetColor, NULL, NULL },
    /* effPCPZiodin02Table */
    { effPCPZiodin02Create, effPCPZiodin02Draw, effPCPZiodin02Delete, effPCPZiodin02Copy, effPCPZiodin02SetPosition, NULL, NULL, effPCPZiodin02SetColor, NULL, NULL },
    /* effPCPZiodin03Table */
    { effPCPZiodin03Create, effPCPZiodin03Draw, effPCPZiodin03Delete, effPCPZiodin03Copy, effPCPZiodin03SetPosition, NULL, NULL, effPCPZiodin03SetColor, NULL, NULL },
    /* effPCPZiodin04Table */
    { effPCPZiodin04Create, effPCPZiodin04Draw, effPCPZiodin04Delete, effPCPZiodin04Copy, effPCPZiodin04SetPosition, NULL, NULL, effPCPZiodin04SetColor, NULL, NULL },
    /* effPCPShockWave01Table */
    { effPCPShockWave01Create, effPCPShockWave01Draw, effPCPShockWave01Delete, effPCPShockWave01Copy, effPCPShockWave01SetPosition, NULL, NULL, effPCPShockWave01SetColor, NULL, NULL },
    /* effPCPHouden01Table */
    { effPCPHouden01Create, effPCPHouden01Draw, effPCPHouden01Delete, effPCPHouden01Copy, effPCPHouden01SetPosition, NULL, NULL, effPCPHouden01SetColor, NULL, NULL },
    /* effPCPMultiFilterExpTable */
    { effPCPMultiFilterExpCreate, effPCPMultiFilterExpDraw, effPCPMultiFilterExpDelete, NULL, effPCPMultiFilterExpSetPosition, NULL, NULL, effPCPMultiFilterExpSetColor, NULL, effPCPMultiFilterExpSetArea },
    /* effPCPBlinkMultiBlurTable */
    { effPCPBlinkMultiBlurCreate, effPCPBlinkMultiBlurDraw, effPCPBlinkMultiBlurDelete, NULL, effPCPBlinkMultiBlurSetPosition, NULL, NULL, effPCPBlinkMultiBlurSetColor, NULL, effPCPBlinkMultiBlurSetArea },
    { effPCPMultiFilterExpCreatePCF, effPCPMultiFilterExpDraw, effPCPMultiFilterExpDelete, effPCPMultiFilterExpCopy, effPCPMultiFilterExpSetPosition, NULL, NULL, effPCPMultiFilterExpSetColor, NULL, effPCPMultiFilterExpSetArea },
    /* effPCPShockWave02Table */
    { effPCPShockWave02Create, effPCPShockWave02Draw, effPCPShockWave02Delete, effPCPShockWave02Copy, effPCPShockWave02SetPosition, NULL, NULL, effPCPShockWave02SetColor, NULL, NULL },
    /* effPCPFilterExpTable */
    { effPCPFilterExpCreatePCF, effPCPFilterExpDraw, effPCPFilterExpDelete, effPCPFilterExpCopy, NULL, NULL, NULL, effPCPFilterExpSetColor, NULL, NULL },
    /* effPCPBlurExpTable */
    { effPCPBlurExpCreatePCF, effPCPBlurExpDraw, effPCPBlurExpDelete, effPCPBlurExpCopy, NULL, NULL, NULL, effPCPBlurExpSetColor, NULL, NULL },
    /* effPCPDistortionBlurExpTable */
    { effPCPDistortionBlurExpCreatePCF, effPCPDistortionBlurExpDraw, effPCPDistortionBlurExpDelete, effPCPDistortionBlurExpCopy, effPCPDistortionBlurExpSetPosition, NULL, NULL, effPCPDistortionBlurExpSetColor, NULL, NULL },
    /* effPCPMultiBlurExp2Table */
    { effPCPMultiBlurExp2CreatePCF, effPCPMultiBlurExp2Draw, effPCPMultiBlurExp2Delete, effPCPMultiBlurExp2Copy, effPCPMultiBlurExp2SetPosition, NULL, NULL, effPCPMultiBlurExp2SetColor, NULL, NULL },
    /* effPCPMegidora01Table */
    { effPCPMegidora01Create, effPCPMegidora01Draw, effPCPMegidora01Delete, effPCPMegidora01Copy, effPCPMegidora01SetPosition, NULL, NULL, effPCPMegidora01SetColor, NULL, NULL },
    /* effPCPMegidoraon01Table */
    { effPCPMegidoraon01Create, effPCPMegidoraon01Draw, effPCPMegidoraon01Delete, effPCPMegidoraon01Copy, effPCPMegidoraon01SetPosition, NULL, NULL, effPCPMegidoraon01SetColor, NULL, NULL },
    /* effPCPBreathTable */
    { effPCPBreathCreatePCF, effPCPBreathDraw, effPCPBreathDelete, effPCPBreathCopy, effPCPBreathSetPosition, NULL, effPCPBreathSetRotateMatrix, effPCPBreathSetColor, NULL, NULL },
    /* effPCPHamaCardSpiralTable */
    { effPCPHamaCardSpiralCreatePCF, effPCPHamaCardSpiralDraw, effPCPHamaCardSpiralDelete, effPCPHamaCardSpiralCopy, effPCPHamaCardSpiralSetPosition, effPCPHamaCardSpiralSetScale, NULL, effPCPHamaCardSpiralSetColor, NULL, NULL },
    /* effPCPHamaCardSphereTable */
    { effPCPHamaCardSphereCreatePCF, effPCPHamaCardSphereDraw, effPCPHamaCardSphereDelete, effPCPHamaCardSphereCopy, effPCPHamaCardSphereSetPosition, effPCPHamaCardSphereSetScale, NULL, effPCPHamaCardSphereSetColor, NULL, NULL },
    /* effPCPAgiraoFireTable */
    { effPCPAgiraoFireCreatePCF, effPCPAgiraoFireDraw, effPCPAgiraoFireDelete, effPCPAgiraoFireCopy, effPCPAgiraoFireSetPosition, NULL, NULL, effPCPAgiraoFireSetColor, NULL, NULL },
    /* effPCPBufuIceTable */
    { effPCPBufuIceCreate, effPCPBufuIceDraw, effPCPBufuIceDelete, effPCPBufuIceCopy, effPCPBufuIceSetPosition, effPCPBufuIceSetScale, effPCPBufuIceSetRotateMatrix, effPCPBufuIceSetColor, NULL, NULL },
    /* effPCPBufuraIceTable */
    { effPCPBufuraIceCreate, effPCPBufuraIceDraw, effPCPBufuraIceDelete, effPCPBufuraIceCopy, effPCPBufuraIceSetPosition, effPCPBufuraIceSetScale, effPCPBufuraIceSetRotateMatrix, effPCPBufuraIceSetColor, NULL, NULL },
    /* effPCPBufudnIceTable */
    { effPCPBufudnIceCreate, effPCPBufudnIceDraw, effPCPBufudnIceDelete, effPCPBufudnIceCopy, effPCPBufudnIceSetPosition, effPCPBufudnIceSetScale, effPCPBufudnIceSetRotateMatrix, effPCPBufudnIceSetColor, NULL, NULL },
    /* effPCPBufuLightTable */
    { effPCPBufuLightCreate, effPCPBufuLightDraw, effPCPBufuLightDelete, effPCPBufuLightCopy, NULL, NULL, NULL, NULL, NULL, NULL },
    { effPCPAgiFireballCreatePCF, effPCPAgiFireballDraw, effPCPAgiFireballDelete, effPCPAgiFireballCopy, effPCPAgiFireballSetPosition, NULL, effPCPAgiFireballSetRotateMatrix, effPCPAgiFireballSetColor, NULL, NULL },
    /* effPCPNeedleSpiralTable */
    { effPCPNeedleSpiralCreatePCF, effPCPNeedleSpiralDraw, effPCPNeedleSpiralDelete, effPCPNeedleSpiralCopy, effPCPNeedleSpiralSetPosition, NULL, NULL, effPCPNeedleSpiralSetColor, NULL, NULL },
    { effPCPEyeThunderCreate00_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy00_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate00_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy00_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate01_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy01_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate01_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy01_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate02_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy02_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate02_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy02_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate03_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy03_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate03_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy03_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate04_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy04_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate04_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy04_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    /* effPCPRippleBlurExpTable */
    { effPCPRippleBlurExpCreatePCF, effPCPRippleBlurExpDraw, effPCPRippleBlurExpDelete, effPCPRippleBlurExpCopy, effPCPRippleBlurExpSetPosition, NULL, NULL, effPCPRippleBlurExpSetColor, NULL, NULL },
    /* effPCPThunderSphereExpTable */
    { effPCPThunderSphereExpCreatePCF, effPCPThunderSphereExpDraw, effPCPThunderSphereExpDelete, effPCPThunderSphereExpCopy, effPCPThunderSphereExpSetPosition, effPCPThunderSphereExpSetScale, NULL, effPCPThunderSphereExpSetColor, NULL, NULL },
    /* effPCPRingPolyExpTable */
    { effPCPRingPolyExpCreatePCF, effPCPRingPolyExpDraw, effPCPRingPolyExpDelete, effPCPRingPolyExpCopy, effPCPRingPolyExpSetPosition, effPCPRingPolyExpSetScale, effPCPRingPolyExpSetRotateMatrix, effPCPRingPolyExpSetColor, effPCPRingPolyExpSetDirection, NULL },
    /* effPCPWhirlwindPolyTable */
    { effPCPWhirlwindPolyCreatePCF, effPCPWhirlwindPolyDraw, effPCPWhirlwindPolyDelete, effPCPWhirlwindPolyCopy, effPCPWhirlwindPolySetPosition, effPCPWhirlwindPolySetScale, effPCPWhirlwindPolySetRotateMatrix, effPCPWhirlwindPolySetColor, NULL, NULL },
    { effPCPEyeThunderCreate05_0, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy05_0, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    { effPCPEyeThunderCreate05_1, effPCPEyeThunderDraw, effPCPEyeThunderDelete, effPCPEyeThunderCopy05_1, effPCPEyeThunderSetPosition, NULL, NULL, effPCPEyeThunderSetColor, NULL, NULL },
    /* effPCPRadiateThunderTable */
    { effPCPRadiateThunderCreatePCF, effPCPRadiateThunderDraw, effPCPRadiateThunderDelete, effPCPRadiateThunderCopy, effPCPRadiateThunderSetPosition, effPCPRadiateThunderSetScale, NULL, effPCPRadiateThunderSetColor, NULL, NULL },
    /* effPCPWwSpherePolyTable */
    { effPCPWwSpherePolyCreatePCF, effPCPWwSpherePolyDraw, effPCPWwSpherePolyDelete, effPCPWwSpherePolyCopy, effPCPWwSpherePolySetPosition, effPCPWwSpherePolySetScale, effPCPWwSpherePolySetRotateMatrix, effPCPWwSpherePolySetColor, NULL, NULL },
    /* effPCPFlashPolySphereExpTable */
    { effPCPFlashPolySphereExpCreatePCF, effPCPFlashPolySphereExpDraw, effPCPFlashPolySphereExpDelete, effPCPFlashPolySphereExpCopy, effPCPFlashPolySphereExpSetPosition, effPCPFlashPolySphereExpSetScale, NULL, effPCPFlashPolySphereExpSetColor, NULL, NULL },
    /* effPCPFlashPolySphere2ExpTable */
    { effPCPFlashPolySphere2ExpCreatePCF, effPCPFlashPolySphere2ExpDraw, effPCPFlashPolySphere2ExpDelete, effPCPFlashPolySphere2ExpCopy, effPCPFlashPolySphere2ExpSetPosition, effPCPFlashPolySphere2ExpSetScale, NULL, effPCPFlashPolySphere2ExpSetColor, NULL, NULL },
    /* effPCPFlashPolySphere3Table */
    { effPCPFlashPolySphere3CreatePCF, effPCPFlashPolySphere3Draw, effPCPFlashPolySphere3Delete, effPCPFlashPolySphere3Copy, effPCPFlashPolySphere3SetPosition, effPCPFlashPolySphere3SetScale, NULL, effPCPFlashPolySphere3SetColor, NULL, NULL },
    /* effPCPFlashPolyRingExp2Table */
    { effPCPFlashPolyRingExp2CreatePCF, effPCPFlashPolyRingExp2Draw, effPCPFlashPolyRingExp2Delete, effPCPFlashPolyRingExp2Copy, effPCPFlashPolyRingExp2SetPosition, effPCPFlashPolyRingExp2SetScale, NULL, effPCPFlashPolyRingExp2SetColor, NULL, NULL },
    /* effPCPWhirlwind2PolyTable */
    { effPCPWhirlwind2PolyCreatePCF, effPCPWhirlwind2PolyDraw, effPCPWhirlwind2PolyDelete, effPCPWhirlwind2PolyCopy, effPCPWhirlwind2PolySetPosition, effPCPWhirlwind2PolySetScale, effPCPWhirlwind2PolySetRotateMatrix, effPCPWhirlwind2PolySetColor, NULL, NULL },
    /* effPCPMakkaTable */
    { effPCPMakkaCreat, effPCPMakkaDraw, effPCPMakkaDelete, effPCPMakkaCopy, effPCPMakkaSetPosition, effPCPMakkaSetScale, NULL, effPCPMakkaSetColor, NULL, NULL },
    /* effPCPRingPolyExp2Table */
    { effPCPRingPolyExp2CreatePCF, effPCPRingPolyExp2Draw, effPCPRingPolyExp2Delete, effPCPRingPolyExp2Copy, effPCPRingPolyExp2SetPosition, effPCPRingPolyExp2SetScale, NULL, effPCPRingPolyExp2SetColor, NULL, NULL },
    /* effPCPAgiFireball2Table */
    { effPCPAgiFireball2CreatePCF, effPCPAgiFireball2Draw, effPCPAgiFireball2Delete, effPCPAgiFireball2Copy, effPCPAgiFireball2SetPosition, NULL, effPCPAgiFireball2SetRotateMatrix, effPCPAgiFireball2SetColor, NULL, NULL },
    /* effPCPRadiateBEDTable */
    { effPCPRadiateBEDCreatePCF, effPCPRadiateBEDDraw, effPCPRadiateBEDDelete, effPCPRadiateBEDCopy, effPCPRadiateBEDSetPosition, effPCPRadiateBEDSetScale, NULL, effPCPRadiateBEDSetColor, NULL, NULL },
    /* effAPPoint4Table */
    { effAPPoint4CreatePCF, effAPPoint4Draw, effAPPoint4Delete, effAPPoint4Copy, NULL, NULL, NULL, NULL, NULL, NULL },
    /* effPCPRadiateTBTable */
    { effPCPRadiateTBCreatePCF, effPCPRadiateTBDraw, effPCPRadiateTBDelete, effPCPRadiateTBCopy, effPCPRadiateTBSetPosition, NULL, NULL, effPCPRadiateTBSetColor, NULL, NULL },
    /* effPCPWhirlwind3PolyTable */
    { effPCPWhirlwind3PolyCreatePCF, effPCPWhirlwind3PolyDraw, effPCPWhirlwind3PolyDelete, effPCPWhirlwind3PolyCopy, effPCPWhirlwind3PolySetPosition, effPCPWhirlwind3PolySetScale, effPCPWhirlwind3PolySetRotateMatrix, effPCPWhirlwind3PolySetColor, NULL, NULL },
    /* effPCPThunderSpherePolyTable */
    { effPCPThunderSpherePolyCreatePCF, effPCPThunderSpherePolyDraw, effPCPThunderSpherePolyDelete, effPCPThunderSpherePolyCopy, effPCPThunderSpherePolySetPosition, effPCPThunderSpherePolySetScale, NULL, effPCPThunderSpherePolySetColor, NULL, NULL },
    /* effPCPThunderSpherePolyLowTable */
    { effPCPThunderSpherePolyLowCreate, effPCPThunderSpherePolyLowDraw, effPCPThunderSpherePolyLowDelete, effPCPThunderSpherePolyLowCopy, effPCPThunderSpherePolyLowSetPosition, effPCPThunderSpherePolyLowSetScale, NULL, effPCPThunderSpherePolyLowSetColor, NULL, NULL },
    /* effPCPSpiralBEDTable */
    { effPCPSpiralBEDCreatePCF, effPCPSpiralBEDDraw, effPCPSpiralBEDDelete, effPCPSpiralBEDCopy, effPCPSpiralBEDSetPosition, effPCPSpiralBEDSetScale, NULL, effPCPSpiralBEDSetColor, NULL, NULL },
    { effPCPAgiFireballSpCreatePCF, effPCPAgiFireballDraw, effPCPAgiFireballDelete, effPCPAgiFireballSpCopy, effPCPAgiFireballSetPosition, NULL, effPCPAgiFireballSetRotateMatrix, effPCPAgiFireballSetColor, NULL, NULL },
    { effPCPAgiFireballSp2CreatePCF, effPCPAgiFireballDraw, effPCPAgiFireballDelete, effPCPAgiFireballSp2Copy, effPCPAgiFireballSetPosition, NULL, effPCPAgiFireballSetRotateMatrix, effPCPAgiFireballSetColor, NULL, NULL },
    /* effPCPBezierBillTable */
    { effPCPBezierBillCreatePCF, effPCPBezierBillDraw, effPCPBezierBillDelete, effPCPBezierBillCopy, effPCPBezierBillSetPosition, NULL, NULL, effPCPBezierBillSetColor, NULL, NULL },
    /* effPCPCrackTable */
    { effPCPCrackCreatePCF, effPCPCrackDraw, effPCPCrackDelete, effPCPCrackCopy, effPCPCrackSetPosition, NULL, NULL, effPCPCrackSetColor, NULL, NULL },
    /* effPCPHelperBEDTable */
    { effPCPHelperBEDCreatePCF, effPCPHelperBEDDraw, effPCPHelperBEDDelete, effPCPHelperBEDCopy, effPCPHelperBEDSetPosition, effPCPHelperBEDSetScale, NULL, effPCPHelperBEDSetColor, NULL, NULL },
    /* effPCPFlashPolyCircleTable */
    { effPCPFlashPolyCircleCreatePCF, effPCPFlashPolyCircleDraw, effPCPFlashPolyCircleDelete, effPCPFlashPolyCircleCopy, effPCPFlashPolyCircleSetPosition, effPCPFlashPolyCircleSetScale, NULL, effPCPFlashPolyCircleSetColor, NULL, NULL },
    /* effPCPFlashPolyRingExpHTable */
    { effPCPFlashPolyRingExpHCreatePCF, effPCPFlashPolyRingExpHDraw, effPCPFlashPolyRingExpHDelete, effPCPFlashPolyRingExpHCopy, effPCPFlashPolyRingExpHSetPosition, effPCPFlashPolyRingExpHSetScale, effPCPFlashPolyRingExpHSetRotateMatrix, effPCPFlashPolyRingExpHSetColor, NULL, NULL },
    /* effPCPBossBaelTable */
    { effPCPBossBaelCreatePCF, effPCPBossBaelDraw, effPCPBossBaelDelete, effPCPBossBaelCopy, effPCPBossBaelSetPosition, NULL, NULL, effPCPBossBaelSetColor, NULL, NULL },
    /* effPCPBossDengekiTable */
    { effPCPBossDengekiCreatePCF, effPCPBossDengekiDraw, effPCPBossDengekiDelete, effPCPBossDengekiCopy, NULL, NULL, NULL, effPCPBossDengekiSetColor, NULL, NULL },
    /* effPCPAnalyzeBEDTable */
    { effPCPAnalyzeBEDCreatePCF, effPCPAnalyzeBEDDraw, effPCPAnalyzeBEDDelete, effPCPAnalyzeBEDCopy, NULL, effPCPAnalyzeBEDSetScale, NULL, effPCPAnalyzeBEDSetColor, NULL, NULL },
    /* effPCPBossThunderboltTable */
    { effPCPBossThunderboltCreatePCF, effPCPBossThunderboltDraw, effPCPBossThunderboltDelete, effPCPBossThunderboltCopy, effPCPBossThunderboltSetPosition, NULL, NULL, effPCPBossThunderboltSetColor, NULL, NULL },
    /* effPCPBossKAGUInvokeTable */
    { effPCPBossKAGUInvokeCreatePCF, effPCPBossKAGUInvokeDraw, effPCPBossKAGUInvokeDelete, effPCPBossKAGUInvokeCopy, NULL, NULL, NULL, effPCPBossKAGUInvokeSetColor, NULL, NULL },
    /* effPCPBossKAGUAttackTable */
    { effPCPBossKAGUAttackCreatePCF, effPCPBossKAGUAttackDraw, effPCPBossKAGUAttackDelete, effPCPBossKAGUAttackCopy, NULL, NULL, NULL, effPCPBossKAGUAttackSetColor, NULL, NULL },
    /* effPCPBossKAGUThunderboltTable */
    { effPCPBossKAGUThunderboltCreatePCF, effPCPBossKAGUThunderboltDraw, effPCPBossKAGUThunderboltDelete, effPCPBossKAGUThunderboltCopy, effPCPBossKAGUThunderboltSetPosition, NULL, NULL, effPCPBossKAGUThunderboltSetColor, NULL, NULL },
    /* effPCPBossKAGU2AttackTable */
    { effPCPBossKAGU2AttackCreatePCF, effPCPBossKAGU2AttackDraw, effPCPBossKAGU2AttackDelete, effPCPBossKAGU2AttackCopy, NULL, NULL, NULL, effPCPBossKAGU2AttackSetColor, NULL, NULL },
    /* effPCPBossKAGTable */
    { effPCPBossKAGUThunderboltCreatePCF, effPCPBossKAGU2ThunderboltDraw, effPCPBossKAGUThunderboltDelete, effPCPBossKAGUThunderboltCopy, effPCPBossKAGUThunderboltSetPosition, NULL, NULL, effPCPBossKAGUThunderboltSetColor, NULL, NULL },
    /* effPCPFLEACHBlurTable */
    { effPCPFLEACHBlurCreatePCF, effPCPFLEACHBlurDraw, effPCPFLEACHBlurDelete, effPCPFLEACHBlurCopy, NULL, NULL, NULL, effPCPFLEACHBlurSetColor, NULL, NULL },
    /* effPCPCrack2Table */
    { effPCPCrack2CreatePCF, effPCPCrack2Draw, effPCPCrack2Delete, effPCPCrack2Copy, effPCPCrack2SetPosition, NULL, NULL, effPCPCrack2SetColor, NULL, NULL },
    /* effPCPBossMETATOBeamTable */
    { effPCPBossMETATOBeamCreatePCF, effPCPBossMETATOBeamDraw, effPCPBossMETATOBeamDelete, effPCPBossMETATOBeamCopy, effPCPBossMETATOBeamSetPosition, NULL, NULL, effPCPBossMETATOBeamSetColor, NULL, NULL },
    /* effPCPScreenCrackModelTable */
    { effPCPScreenCrackModelCreatePCF, effPCPScreenCrackModelDraw, effPCPScreenCrackModelDelete, effPCPScreenCrackModelCopy, NULL, NULL, NULL, effPCPScreenCrackModelSetColor, NULL, NULL },
    /* effPCPMuzzleFlashTable */
    { effPCPMuzzleFlashCreatePCF, effPCPMuzzleFlashDraw, effPCPMuzzleFlashDelete, effPCPMuzzleFlashCopy, effPCPMuzzleFlashSetPosition, effPCPMuzzleFlashSetScale, effPCPMuzzleFlashSetRotateMatrix, effPCPMuzzleFlashSetColor, NULL, NULL },
    /* effPCPDANTEThunderTable */
    { effPCPDANTE_Thunder_CreatePCF, effPCPDANTE_Thunder_DrawSub, effPCPDANTE_Thunder_Delete, effPCPDANTE_Thunder_Copy, effPCPDANTE_Thunder_SetPosition, NULL, effPCPDANTE_Thunder_SetRotateMatrix, effPCPDANTE_Thunder_SetColor, NULL, NULL },
    /* effPCPDANTEDamageTable */
    { effPCPDANTE_Damage_CreatePCF, effPCPDANTE_Damage_Run, effPCPDANTE_Damage_Delete, effPCPDANTE_Damage_Copy, NULL, NULL, NULL, effPCPDANTE_Damage_SetColor, NULL, NULL },
    /* effPCPPBCameraTable */
    { effPCP_PBCamera_CreatePCF, effPCP_PBCamera_Run, effPCP_PBCamera_Delete, effPCP_PBCamera_Copy, effPCP_PBCamera_SetPosition, NULL, effPCP_PBCamera_SetRotateMatrix, NULL, NULL, NULL },
    /* effPCPDANTEShowTimeTable */
    { effPCPDANTE_ShowTime_CreatePCF, effPCPDANTE_ShowTime_Draw, effPCPDANTE_ShowTime_Delete, effPCPDANTE_ShowTime_Copy, effPCPDANTE_ShowTime_SetPosition, NULL, NULL, NULL, NULL, NULL },
    /* effPCPBEDTable */
    { effPCP_BED_CreatePCF, effPCP_BED_Draw, effPCP_BED_Delete, effPCP_BED_Copy, effPCP_BED_SetPosition, NULL, effPCP_BED_SetRotateMatrix, effPCP_BED_SetColor, NULL, NULL },
    /* effPCPHBED2Table */
    { effPCP_HBED2_CreatePCF, effPCP_HBED2_Draw, effPCP_HBED2_Delete, effPCP_HBED2_Copy, effPCP_HBED2_SetPosition, effPCP_HBED2_SetScale, effPCP_HBED2_SetRotateMatrix, effPCP_HBED2_SetColor, NULL, NULL },
};
