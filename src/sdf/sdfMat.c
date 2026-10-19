#include "common.h"
#include "libsdf.h"

extern void *sdfChipAllocDirect(s32 aSize);
extern void *sdfChipAllocZeroDirect(s32 aSize);
extern void sdfChipFreeDirect(void *aPtr);

extern sdfItemArray_t *sdfItemArrayNew(s32 aCount, s32 aElemSize, s32 aGrowStep);
extern void sdfItemArrayFree(sdfItemArray_t *aArray);
extern void sdfItemArrayGrow(sdfItemArray_t *aArray);
extern void func_002BFF98(sdfItemArray_t *aArray, s32 aCapacity);

extern void sdfDelayedResrcInitCB(sdfDelayedResrcCB_t *aCB, sdfDelayedResrcFreeFunc aFunc);
extern void sdfDelayedResrcEntry(sdfDelayedResrcCB_t *aCB, void *aChunk);

extern sdfDelayedResrcCB_t sdfMatDelayedResrcCB;
extern sdfDelayedResrcCB_t sdfMatDelayedResrcCB2;
extern s32 sdfMatAllocCount;

/* The GS ALPHA_2 value per multi-texture method, indexed by sdfMat_t.multiTexMethod. */
extern u16 D_00529438[];

extern void sdfTexFreeDelayed(sdfTexHandle_t *aHandle);
extern sdfTexHandle_t *sdfTexDup(sdfTexHandle_t *aHandle);
extern void sdfTexSetClutColor(sdfTexHandle_t *aHandle, s32 aColor, f32 aRatio);
extern u64 sdfTexGetTex0Reg(sdfTexHandle_t *aHandle);
extern u64 sdfTexGetTex1Reg(sdfTexHandle_t *aHandle);
extern u64 sdfTexGetClampReg(sdfTexHandle_t *aHandle);

extern f32 sdfSin(f32 aAngle);
extern f32 sdfCos(f32 aAngle);

sdfItemArray_t *sdfTexListNew(s32 aCount);
sdfItemArray_t *sdfMatListNew(s32 aCount);
void sdfMatListGrow(sdfItemArray_t *aList);
void sdfMatListFree(sdfItemArray_t *aList);
void sdfMatFree(sdfMat_t *aMat);
sdfMatUvwParm_t *sdfMatAllocTexUvwParm(void);
sdfMatUvwParm_t *sdfMatGetTexUvwParm(sdfMat_t *aMat);
sdfMatUvwParm_t *sdfMatGetMultiTexUvwParm(sdfMat_t *aMat);
void sdfMatCopyStat(sdfMat_t *aDst, sdfMat_t *aSrc);
void sdfMatListCopyStat(sdfItemArray_t *aDst, sdfItemArray_t *aSrc);
void sdfMatSetUvwParm(sdfMatTexTm_t *aTm, const sdfMatUvwParm_t *aParm);
void sdfMatUpdatePacketBasic(sdfMat_t *aMat, sdfMatPacket_t *aPacket);
void sdfMatUpdatePacketUvwParm(sdfMat_t *aMat, sdfMatPacket_t *aPacket);
void sdfMatUpdatePacketMultiTex(sdfMat_t *aMat, sdfMatPacket_t *aPacket);
void sdfMatUpdatePacketFaceDiv(sdfMat_t *aMat, sdfMatPacket_t *aPacket);
void sdfMatSetAmbient(sdfMat_t *aMat, u32 aColor);
void sdfMatSetDiffuse(sdfMat_t *aMat, u32 aColor);
void sdfMatSetSpecular(sdfMat_t *aMat, u32 aColor);
void sdfMatSetRefMap(sdfMat_t *aMat, u32 aColor);
void sdfMatSetConstant(sdfMat_t *aMat, f32 aRatio);
void sdfMatSetTex(sdfMat_t *aMat, sdfTexHandle_t *aHandle);
void sdfMatSetTexUvw(sdfMat_t *aMat, f32 aOffsetU, f32 aOffsetV, f32 aTileU, f32 aTileV,
                     f32 aAngle);
void sdfMatSetMultiDiffuse(sdfMat_t *aMat, u32 aColor);
void sdfMatSetMultiMethod(sdfMat_t *aMat, u32 aMethod);
void sdfMatSetMultiTex(sdfMat_t *aMat, sdfTexHandle_t *aHandle);
void sdfMatSetMultiTexUvw(sdfMat_t *aMat, f32 aOffsetU, f32 aOffsetV, f32 aTileU, f32 aTileV,
                          f32 aAngle);
void sdfMatSetFaceDivParm(sdfMat_t *aMat, f32 aBias, f32 aSoften);
sdfMat_t *sdfMatNew(void);
u8 *sdfMatSet(sdfMat_t *aMat, sdfItemArray_t *aTexList, u8 *aData);
void sdfMatListAdd(sdfItemArray_t *aList, sdfMat_t *aMat);

sdfItemArray_t *sdfTexListNew(s32 aCount)
{
    return sdfItemArrayNew(aCount, 4, 4);
}

void sdfTexListFree(sdfItemArray_t *aList, s32 aFreeTex)
{
    s32 i;
    s32 count;

    if (aList == NULL) {
        return;
    }
    if (aFreeTex != 0) {
        count = aList->count;
        for (i = 0; i < count; i++) {
            sdfTexFreeDelayed((sdfTexHandle_t *)((u32 *)aList->items)[i]);
        }
    }
    sdfItemArrayFree(aList);
}

void sdfTexListAdd(sdfItemArray_t *aList, sdfTexHandle_t *aHandle)
{
    if (aList->count >= (s16)aList->capacity) {
        sdfItemArrayGrow(aList);
    }
    ((u32 *)aList->items)[aList->count] = (u32)aHandle;
    aList->count++;
}

void sdfTexListSetCapacity(sdfItemArray_t *aList, s32 aCount, s32 aFreeTex)
{
    s32 i;
    s32 count;

    count = aList->count;
    if (aCount < count && aFreeTex != 0) {
        /* Retail only counts down to the new capacity here; the loop body is
         * empty and ee-gcc pads it to the R5900 short-loop minimum. */
        i = aCount;
        do {
            i++;
        } while (i != count);
        aList->count = aCount;
    }
    func_002BFF98(aList, aCount);
}

sdfItemArray_t *sdfTexListDup(sdfItemArray_t *aList)
{
    s32 count;
    sdfItemArray_t *dup;
    s32 i;

    if (aList == NULL) {
        return NULL;
    }
    count = aList->count;
    dup = sdfTexListNew(count);
    for (i = 0; i < count; i++) {
        ((u32 *)dup->items)[i] = (u32)sdfTexDup((sdfTexHandle_t *)((u32 *)aList->items)[i]);
    }
    dup->count = count;
    return dup;
}

void sdfTexListSetClutColor(sdfItemArray_t *aList, s32 aColor, f32 aRatio)
{
    s32 i;
    s32 count;

    if (aList == NULL) {
        return;
    }
    count = aList->count;
    for (i = 0; i < count; i++) {
        sdfTexHandle_t *handle = ((sdfTexHandle_t **)aList->items)[i];

        if (handle->numOfCLUT != 0) {
            sdfTexSetClutColor(handle, aColor, aRatio);
        }
    }
}

void sdfMatInitSystem(void)
{
    sdfDelayedResrcInitCB(&sdfMatDelayedResrcCB, (sdfDelayedResrcFreeFunc)sdfMatListFree);
    sdfDelayedResrcInitCB(&sdfMatDelayedResrcCB2, (sdfDelayedResrcFreeFunc)sdfMatFree);
}

sdfItemArray_t *sdfMatListNew(s32 aCount)
{
    return sdfItemArrayNew(aCount, 4, 8);
}

void sdfMatListFree(sdfItemArray_t *aList)
{
    s32 i;

    for (i = 0; i < aList->count; i++) {
        sdfMatFree(((sdfMat_t **)aList->items)[i]);
    }
    sdfItemArrayFree(aList);
}

void sdfMatListFreeResrc(void *aList, s32 aDelayed)
{
    if (aList == NULL) {
        return;
    }
    if (aDelayed != 0) {
        sdfDelayedResrcEntry(&sdfMatDelayedResrcCB, aList);
    } else {
        sdfItemArrayFree(aList);
    }
}

void sdfMatListGrow(sdfItemArray_t *aList)
{
    sdfItemArrayGrow(aList);
}

void sdfMatListAdd(sdfItemArray_t *aList, sdfMat_t *aMat)
{
    if (aList->count >= (s16)aList->capacity) {
        sdfMatListGrow(aList);
    }
    ((sdfMat_t **)aList->items)[aList->count] = aMat;
    aList->count++;
}

void sdfMatSetAmbient(sdfMat_t *aMat, u32 aColor)
{
    aMat->ambient = aColor;
    aMat->duty |= 3;
}

void sdfMatSetDiffuse(sdfMat_t *aMat, u32 aColor)
{
    aMat->diffuse = aColor;
    aMat->duty |= 3;
}

void sdfMatSetSpecular(sdfMat_t *aMat, u32 aColor)
{
    aMat->specular = aColor;
    aMat->duty |= 3;
}

void sdfMatSetRefMap(sdfMat_t *aMat, u32 aColor)
{
    aMat->refmap = aColor;
    aMat->duty |= 3;
}

void sdfMatSetConstant(sdfMat_t *aMat, f32 aRatio)
{
    aMat->constant = aRatio;
    aMat->duty = aMat->duty | 3;
}

void sdfMatSetTex(sdfMat_t *aMat, sdfTexHandle_t *aHandle)
{
    aMat->texHandle = aHandle;
    aMat->duty |= 3;
}

sdfMatUvwParm_t *sdfMatAllocTexUvwParm(void)
{
    sdfMatUvwParm_t *parm;

    parm = sdfChipAllocDirect(sizeof(sdfMatUvwParm_t));
    /* Tile (1, 1), offset (0, 0), angle 0, written as the three quadword halves. */
    *(u64 *)&parm->tile[0] = 0x3F8000003F800000ULL;
    *(u64 *)&parm->offset[0] = 0;
    *(u32 *)&parm->angle = 0;
    return parm;
}

sdfMatUvwParm_t *sdfMatGetTexUvwParm(sdfMat_t *aMat)
{
    sdfMatUvwParm_t *parm = aMat->uvwParm;

    if (parm == NULL) {
        parm = sdfMatAllocTexUvwParm();
        aMat->uvwParm = parm;
    }
    return parm;
}

void sdfMatSetTexUvw(sdfMat_t *aMat, f32 aOffsetU, f32 aOffsetV, f32 aTileU, f32 aTileV,
                     f32 aAngle)
{
    f32 *values = sdfMatGetTexUvwParm(aMat)->offset;

    values[0] = aOffsetU;
    values[1] = aOffsetV;
    values[2] = aTileU;
    values[3] = aTileV;
    values[4] = aAngle;
    aMat->duty |= 0xC;
}

void sdfMatSetTexUvwByPtr(sdfMat_t *aMat, const f32 *aParm)
{
    f32 *values = sdfMatGetTexUvwParm(aMat)->offset;

    values[0] = aParm[0];
    values[1] = aParm[1];
    values[2] = aParm[2];
    values[3] = aParm[3];
    values[4] = aParm[4];
    aMat->duty |= 0xC;
}

void sdfMatSetMultiDiffuse(sdfMat_t *aMat, u32 aColor)
{
    aMat->multiTexDiffuse = aColor;
    aMat->duty |= 0x30;
}

void sdfMatSetMultiMethod(sdfMat_t *aMat, u32 aMethod)
{
    aMat->multiTexMethod = aMethod;
    aMat->duty |= 0x30;
}

void sdfMatSetMultiTex(sdfMat_t *aMat, sdfTexHandle_t *aHandle)
{
    aMat->multiTexHandle = aHandle;
    aMat->duty |= 0x30;
}

sdfMatUvwParm_t *sdfMatGetMultiTexUvwParm(sdfMat_t *aMat)
{
    sdfMatUvwParm_t *parm = aMat->multiTexUvwParm;

    if (parm == NULL) {
        parm = sdfMatAllocTexUvwParm();
        aMat->multiTexUvwParm = parm;
    }
    return parm;
}

void sdfMatSetMultiTexUvw(sdfMat_t *aMat, f32 aOffsetU, f32 aOffsetV, f32 aTileU, f32 aTileV,
                          f32 aAngle)
{
    f32 *values = sdfMatGetMultiTexUvwParm(aMat)->offset;

    values[0] = aOffsetU;
    values[1] = aOffsetV;
    values[2] = aTileU;
    values[3] = aTileV;
    values[4] = aAngle;
    aMat->duty |= 0x30;
}

void sdfMatSetMultiTexUvwByPtr(sdfMat_t *aMat, const f32 *aParm)
{
    f32 *values = sdfMatGetMultiTexUvwParm(aMat)->offset;

    values[0] = aParm[0];
    values[1] = aParm[1];
    values[2] = aParm[2];
    values[3] = aParm[3];
    values[4] = aParm[4];
    aMat->duty |= 0x30;
}

void sdfMatSetFaceDivParm(sdfMat_t *aMat, f32 aBias, f32 aSoften)
{
    aMat->fdBias = aBias;
    aMat->fdSoften = aSoften;
    aMat->duty = aMat->duty | 0xC0;
}

sdfMat_t *sdfMatNew(void)
{
    sdfMat_t *mat;
    u32 *packet;
    s32 i;

    sdfMatAllocCount++;
    mat = sdfChipAllocZeroDirect(sizeof(sdfMat_t));
    mat->duty = 0xFF;
    for (i = 0; i != 2; i++) {
        packet = sdfChipAllocDirect(sizeof(sdfMatPacket_t));
        mat->packet[i] = (sdfMatPacket_t *)packet;
        packet[0] = 0x6E05C000;
        packet[0x18 / 4] = 0x6005C005;
        packet[0x30 / 4] = 0;
        packet[0x34 / 4] = 0x640CC00A;
        ((sdfMatPacket_t *)packet)->vifcode3 = 0x0400000C;
        ((sdfMatPacket_t *)packet)->vifcode4 = 0x14000000;
    }
    mat->fdBias = 0;
    mat->fdSoften = 0;
    mat->ambient = 0x80808080;
    mat->diffuse = 0x80808080;
    mat->multiTexDiffuse = 0x80808080;
    return mat;
}

u8 *sdfMatSet(sdfMat_t *aMat, sdfItemArray_t *aTexList, u8 *aData)
{
    u32 flags;
    u8 *cursor;
    u32 packed;

    aMat->id = *(u32 *)aData;
    aMat->flags = *(u16 *)(aData + 4);
    flags = *(u16 *)(aData + 6);
    cursor = aData + 8;
    if (flags & 0x1) {
        sdfMatSetAmbient(aMat, *(u32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x2) {
        sdfMatSetDiffuse(aMat, *(u32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x4) {
        sdfMatSetTex(aMat, (sdfTexHandle_t *)((u32 *)aTexList->items)[*(u16 *)cursor]);
        cursor += 4;
    }
    if (flags & 0x8) {
        sdfMatSetTexUvw(aMat, ((f32 *)cursor)[0], ((f32 *)cursor)[1], ((f32 *)cursor)[2],
                        ((f32 *)cursor)[3], ((f32 *)cursor)[4]);
        cursor += 0x14;
    }
    if (flags & 0x10) {
        sdfMatSetMultiDiffuse(aMat, *(u32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x20) {
        packed = *(u32 *)cursor;
        cursor += 4;
        sdfMatSetMultiTex(aMat, (sdfTexHandle_t *)((u32 *)aTexList->items)[packed & 0xFFFF]);
        sdfMatSetMultiMethod(aMat, packed >> 16);
    }
    if (flags & 0x40) {
        sdfMatSetMultiTexUvw(aMat, ((f32 *)cursor)[0], ((f32 *)cursor)[1], ((f32 *)cursor)[2],
                             ((f32 *)cursor)[3], ((f32 *)cursor)[4]);
        cursor += 0x14;
    }
    if (flags & 0x80) {
        sdfMatSetSpecular(aMat, *(u32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x100) {
        sdfMatSetRefMap(aMat, *(u32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x200) {
        sdfMatSetConstant(aMat, *(f32 *)cursor);
        cursor += 4;
    }
    if (flags & 0x400) {
        sdfMatSetFaceDivParm(aMat, ((f32 *)cursor)[0], ((f32 *)cursor)[1]);
        cursor += 8;
    }
    return cursor;
}

void sdfMatFree(sdfMat_t *aMat)
{
    if (aMat == NULL) {
        return;
    }
    sdfMatAllocCount--;
    sdfChipFreeDirect(aMat->packet[0]);
    sdfChipFreeDirect(aMat->packet[1]);
    sdfChipFreeDirect(aMat->uvwParm);
    sdfChipFreeDirect(aMat->multiTexUvwParm);
    sdfChipFreeDirect(aMat);
}

void sdfMatFreeDelayed(sdfMat_t *aMat)
{
    if (aMat != NULL) {
        sdfDelayedResrcEntry(&sdfMatDelayedResrcCB2, aMat);
    }
}

sdfDmaTag_t *sdfMatMakePacket(sdfMat_t *aMat, sdfDmaTag_t *aTag, s32 aIdx)
{
    aTag->id = 0x30;
    aTag->addr = (void *)((u32)aMat->packet[aIdx] & 0x0FFFFFFF);
    aTag->qwc = 0xA;
    aTag->dat0 = 0;
    aTag->dat1 = 0;
    return (sdfDmaTag_t *)((u8 *)aTag + 0x10);
}

void sdfMatUpdatePacketBasic(sdfMat_t *aMat, sdfMatPacket_t *aPacket)
{
    sdfTexHandle_t *handle = aMat->texHandle;

    aPacket->ambient = aMat->ambient;
    aPacket->constant = aMat->constant;
    aPacket->diffuse = aMat->diffuse;
    aPacket->specularColor = aMat->specular;
    aPacket->refmapColor = aMat->refmap;
    if (handle != NULL) {
        aPacket->tex1Data = sdfTexGetTex1Reg(handle);
        aPacket->tex0Data = sdfTexGetTex0Reg(handle);
        aPacket->clampData = sdfTexGetClampReg(handle);
    }
}

void sdfMatSetUvwParm(sdfMatTexTm_t *aTm, const sdfMatUvwParm_t *aParm)
{
    f32 offsetU;
    f32 offsetV;
    f32 tileU;
    f32 tileV;
    f32 angle;
    f32 s;
    f32 c;
    f32 m0;
    f32 m1;
    f32 m2;
    f32 m3;

    if (aParm == NULL) {
        aTm->q[0] = 0x000000003F800000ULL;
        aTm->q[1] = 0x3F80000000000000ULL;
        aTm->q[2] = 0;
        return;
    }

    angle = -aParm->angle;
    s = sdfSin(angle);
    c = sdfCos(angle);
    offsetU = aParm->offset[0];
    offsetV = aParm->offset[1];
    tileU = aParm->tile[0];
    tileV = aParm->tile[1];

    m0 = c * tileU;
    m1 = s * tileV;
    m2 = -s * tileU;
    m3 = c * tileV;
    aTm->m[0] = m0;
    aTm->m[1] = m1;
    aTm->m[2] = m2;
    aTm->m[3] = m3;
    aTm->m[4] = 0.5f - m0 * (offsetU + 0.5f) - m2 * (0.5f - offsetV);
    aTm->m[5] = 0.5f - m1 * (offsetU + 0.5f) - m3 * (0.5f - offsetV);
}

void sdfMatUpdatePacketUvwParm(sdfMat_t *aMat, sdfMatPacket_t *aPacket)
{
    sdfMatSetUvwParm(&aPacket->texTm, aMat->uvwParm);
}

void sdfMatUpdatePacketMultiTex(sdfMat_t *aMat, sdfMatPacket_t *aPacket)
{
    sdfTexHandle_t *handle = aMat->multiTexHandle;
    u32 method;

    aPacket->mtexDiffuse = aMat->multiTexDiffuse;
    method = aMat->multiTexMethod;
    aPacket->mtexAlgo = method;
    aPacket->mtexAlpha = D_00529438[method];
    if (handle != NULL) {
        aPacket->mtexTex1Data = sdfTexGetTex1Reg(handle);
        aPacket->mtexTex0Data = sdfTexGetTex0Reg(handle);
        aPacket->mtexClampData = sdfTexGetClampReg(handle);
    }
    sdfMatSetUvwParm(&aPacket->mtexTexTm, aMat->multiTexUvwParm);
}

void sdfMatUpdatePacketFaceDiv(sdfMat_t *aMat, sdfMatPacket_t *aPacket)
{
    aPacket->fdBias = aMat->fdBias;
    aPacket->fdSoften = aMat->fdSoften;
}

void sdfMatUpdatePacket(sdfMat_t *aMat, s32 aIdx)
{
    u8 duty = aMat->duty;
    sdfMatPacket_t *packet = aMat->packet[aIdx];

    if ((duty >> aIdx) & 1) {
        sdfMatUpdatePacketBasic(aMat, packet);
    }
    if (duty & (4 << aIdx)) {
        sdfMatUpdatePacketUvwParm(aMat, packet);
    }
    if (duty & (0x10 << aIdx)) {
        sdfMatUpdatePacketMultiTex(aMat, packet);
    }
    if (duty & (0x40 << aIdx)) {
        sdfMatUpdatePacketFaceDiv(aMat, packet);
    }
    aMat->duty = duty & (0x55 << (aIdx ^ 1));
}

sdfItemArray_t *sdfMatListMake(sdfItemArray_t *aTexList, u32 *aData)
{
    u32 count = *aData;
    u8 *cursor = (u8 *)(aData + 1);
    sdfItemArray_t *list = sdfMatListNew(count >= 0x20 ? count : 0x20);

    while (count != 0) {
        sdfMat_t *mat = sdfMatNew();

        cursor = sdfMatSet(mat, aTexList, cursor);
        sdfMatListAdd(list, mat);
        count--;
    }
    return list;
}

void sdfMatListUpdatePacket(sdfItemArray_t *aList, s32 aIdx)
{
    s32 i;
    s32 count = aList->count;
    sdfMat_t **items = (sdfMat_t **)aList->items;

    for (i = 0; i < count; i++) {
        sdfMatUpdatePacket(items[i], aIdx);
    }
}

void sdfMatCopyStat(sdfMat_t *aDst, sdfMat_t *aSrc)
{
    sdfMatUvwParm_t *parm;

    aDst->duty = 0xFF;
    aDst->ambient = aSrc->ambient;
    aDst->diffuse = aSrc->diffuse;
    aDst->constant = aSrc->constant;
    aDst->multiTexDiffuse = aSrc->multiTexDiffuse;
    aDst->flags = aSrc->flags;
    aDst->texHandle = aSrc->texHandle;
    aDst->specular = aSrc->specular;
    aDst->refmap = aSrc->refmap;
    parm = aSrc->uvwParm;
    if (parm != NULL) {
        *sdfMatGetTexUvwParm(aDst) = *parm;
    }
    parm = aSrc->multiTexUvwParm;
    if (parm != NULL) {
        *sdfMatGetMultiTexUvwParm(aDst) = *parm;
    }
    aDst->fdBias = aSrc->fdBias;
    aDst->fdSoften = aSrc->fdSoften;
}

void sdfMatListCopyStat(sdfItemArray_t *aDst, sdfItemArray_t *aSrc)
{
    s32 i;
    s32 count = aDst->count;
    u32 *src = aSrc->items;
    u32 *dst = aDst->items;

    for (i = 0; i < count; i++) {
        sdfMatCopyStat((sdfMat_t *)dst[i], (sdfMat_t *)src[i]);
    }
}
