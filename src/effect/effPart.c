#include "common.h"
#include "motion.h"
#include "model.h"
#include "event.h"
#include "nb.h"
#include "effBattle.h"
#include "ps2asm.h"

extern u128 D_0031DF60;

extern u128 D_0031DF70;

extern u128 D_0031DF80;

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
    u32 size;
    u32 offset;
    u32 type;
    u32 reserve;
} dds3EffectPCPData_Part_t;

typedef struct
{
    u32 kateg;
    u32 param;
    u32 part;
    u32 reserve;
    dds3EffectPCPData_Part_t pcpData[1];
} dds3EffectPCPData_t;

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

extern void *D_002F69B8[4];

extern s32 D_0054F1E4;

extern s32 effBTLCutCount;

/* Header of a packed model an effect part carries, followed by the pack data. */
typedef struct
{
    u32 size;
    u8 reserve[0xC];
    u8 data[1];
} dds3EffectPartModelData_t;

void *dds3BillboardCreate(s32 kind, u32 data);

void mdlPackDecode(s32 group, s32 id, s32 mode, const u8 *data, s32 size);

void mdlLink(mdlObj_t *hModel, void **surfaces);

void mdlFree(mdlObj_t *hModel);

extern dds3EffectFunc_t EffectPCPFuncTable[];

extern void **D_0031DDD0[];

void *sdfChipAllocDirect(s32 aSize);

void sdfChipFreeDirect(void *aPtr);

void dds3BillboardSetScale(void *hBillboard, f32 x, f32 y);

s32 mdlGetMajorNum(mdlObj_t *aHandle);

s32 mdlGetMinorNum(mdlObj_t *aHandle);

mdlObj_t *mdlLoad(u32 aMajor, u32 aMinor);

void dds3EffectPartModelInit(mdlObj_t *hModel);

effPartHandle_t *dds3EffectPartCreate(u16 type, void *pData);

u32 dds3EffectPCPDataGetKateg(dds3EffectPCPData_t *hPCPData);

u32 dds3EffectPCPDataGetParam(dds3EffectPCPData_t *hPCPData);

u32 dds3EffectPCPDataGetType(dds3EffectPCPData_t *hPCPData, u32 number);

void *dds3EffectPCPDataGetData(dds3EffectPCPData_t *hPCPData, u32 number);

extern u32 effBTLPointLightProcess;

extern s8 effBTLFieldColorOn;

extern s8 D_0054F0F1;

extern s32 D_0054F0F4;

extern u8 effBTLDebugCameraAllEdit;

extern u8 effBTLDebugCameraAllTable;

extern u8 D_0054F1C3;

extern s32 D_0054F130;

extern s8 D_0054F134;

extern char D_0054F100[];

void *dds3EffectPartGetHandle(effPartHandle_t *hEffPart);

u16 dds3EffectPartGetType(effPartHandle_t *hEffPart);

u32 dds3EffectPCPDataGetSize(dds3EffectPCPData_t *hPCPData, u32 number);

void dds3EffectPartDelete(effPartHandle_t *hEffPart);

effPartHandle_t *dds3EffectPartCopy(effPartHandle_t *hEffPart);

void dds3EffectPartDraw(effPartHandle_t *hEffPart);

void dds3EffectPartSetPosition(effPartHandle_t *hEffPart, sdfVector4_t *pPosition);

void dds3EffectPartSetScale(effPartHandle_t *hEffPart, f32 scale);

void dds3EffectPartSetColor(effPartHandle_t *hEffPart, u32 color);

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

void dds3EffectPartSetPosition(effPartHandle_t *hEffPart, sdfVector4_t *pPosition)
{
    if (dds3EffectFuncTable[hEffPart->type].setPosition != NULL) {
        dds3EffectFuncTable[hEffPart->type].setPosition(hEffPart->pHandle, pPosition);
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

void dds3EffectPartSetColor(effPartHandle_t *hEffPart, u32 color)
{
    if (dds3EffectFuncTable[hEffPart->type].setColor != NULL) {
        dds3EffectFuncTable[hEffPart->type].setColor(hEffPart->pHandle, color);
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
    SDF_VU0_LOADVECTOR(&D_0031DF60);
    mdlSetPosition(hModel);
    SDF_VU0_LOADVECTOR(&D_0031DF70);
    mdlSetRotate(hModel);
    SDF_VU0_LOADVECTOR(&D_0031DF80);
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

    mdlPackDecode(7, D_0054F1E4, 0x101, pData->data, pData->size);
    hModel = mdlLoad(7, D_0054F1E4);
    dds3EffectPartModelInit(hModel);
    D_0054F1E4++;
    return hModel;
}

void dds3EffectPartModelDraw(mdlObj_t *hModel)
{
    mdlLink(hModel, D_002F69B8);
}

void dds3EffectPartModelDelete(mdlObj_t *hModel)
{
    mdlFree(hModel);
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

void dds3EffectPartModelSetPosition(mdlObj_t *hModel, sdfVector4_t *pPosition)
{
    SDF_VU0_LOADVECTOR(pPosition);
    mdlSetPosition(hModel);
}

void dds3EffectPartModelSetScale(void *hData, f32 scale)
{
    f32 v[3];

    v[0] = v[1] = v[2] = scale;
    SDF_VU0_LOADVECTOR_MEMOPS(v[0]);
    mdlSetScale(hData);
}

void dds3EffectPartModelSetRotateMatrix(mdlObj_t *hModel, sdfMatrix4_t *pMatrix)
{
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_STORE_ROW0(&hModel->inner->tm.m[0]);
    SDF_VU0_STORE_ROW1(&hModel->inner->tm.m[4]);
    SDF_VU0_STORE_ROW2(&hModel->inner->tm.m[8]);
}

void dds3EffectPartModelSetColor(mdlObj_t *hModel, u32 color)
{
    mdlSetColor(hModel, color);
}

dds3EffectPCP_t *dds3EffectPCPCreate(dds3EffectPCPData_t *hPCPData)
{
    dds3EffectPCP_t *hPCP;

    hPCP = sdfChipAllocDirect(sizeof(dds3EffectPCP_t));
    hPCP->kateg = dds3EffectPCPDataGetKateg(hPCPData);
    hPCP->param = dds3EffectPCPDataGetParam(hPCPData);
    if (EffectPCPFuncTable[hPCP->kateg].copy == NULL) {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(D_0031DDD0[hPCP->kateg][hPCP->param]);
    } else {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(hPCPData);
    }
    return hPCP;
}

void dds3EffectPCPDelete(dds3EffectPCP_t *hPCP)
{
    EffectPCPFuncTable[hPCP->kateg].destroy(hPCP->pHandle);
    sdfChipFreeDirect(hPCP);
}

void dds3EffectPCPDraw(dds3EffectPCP_t *hPCP)
{
    EffectPCPFuncTable[hPCP->kateg].draw(hPCP->pHandle);
}

dds3EffectPCP_t *dds3EffectPCPCopy(dds3EffectPCP_t *hPCPSrc)
{
    dds3EffectPCP_t *hPCP;

    hPCP = sdfChipAllocDirect(sizeof(dds3EffectPCP_t));
    hPCP->kateg = hPCPSrc->kateg;
    hPCP->param = hPCPSrc->param;
    if (EffectPCPFuncTable[hPCP->kateg].copy == NULL) {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].create(D_0031DDD0[hPCP->kateg][hPCP->param]);
    } else {
        hPCP->pHandle = EffectPCPFuncTable[hPCP->kateg].copy(hPCPSrc->pHandle);
    }
    return hPCP;
}

void dds3EffectPCPSetPosition(dds3EffectPCP_t *hPCP, void *pPos)
{
    if (EffectPCPFuncTable[hPCP->kateg].setPosition != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setPosition(hPCP->pHandle, pPos);
    }
}

void dds3EffectPCPSetScale(dds3EffectPCP_t *hPCP, f32 scale)
{
    if (EffectPCPFuncTable[hPCP->kateg].setScale != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setScale(hPCP->pHandle, scale);
    }
}

void dds3EffectPCPSetRotateMatrix(dds3EffectPCP_t *hPCP, void *pMatrix)
{
    if (EffectPCPFuncTable[hPCP->kateg].setRotateMatrix != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setRotateMatrix(hPCP->pHandle, pMatrix);
    }
}

void dds3EffectPCPSetDirection(dds3EffectPCP_t *hPCP, void *pMatrix)
{
    if (EffectPCPFuncTable[hPCP->kateg].setDirection != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setDirection(hPCP->pHandle, pMatrix);
    }
}

void dds3EffectPCPSetColor(dds3EffectPCP_t *hPCP, u32 abgr)
{
    if (EffectPCPFuncTable[hPCP->kateg].setColor != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setColor(hPCP->pHandle, abgr);
    }
}

void dds3EffectPCPSetArea(dds3EffectPCP_t *hPCP, f32 area)
{
    if (EffectPCPFuncTable[hPCP->kateg].setArea != NULL) {
        EffectPCPFuncTable[hPCP->kateg].setArea(hPCP->pHandle, area);
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

