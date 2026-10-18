#include "common.h"
#include "libsdf.h"
#include "draw.h"

typedef struct
{
    u64 pmode;
    u64 smode2;
    u32 dispfb;
    u32 dispfbHi;
    u64 display;
    u64 bgcolor;
} sceGsDispEnv;

extern void sceGsResetGraph(s32 aMode, s32 aInter, s32 aOmode, s32 aFfmode);
extern void sceGsSetDefDispEnv(sceGsDispEnv *aEnv, s32 aPsm, s32 aWidth, s32 aHeight, s32 aDx,
                               s32 aDy);
extern void sceGsPutDispEnv(sceGsDispEnv *aEnv);

extern void sdfGsMemFree(sdfGsMemSlice_t *aSlice);
extern sdfGsMemSlice_t *sdfGsAllocFrameBuff(s32 aWidth, s32 aHeight, s32 aPsm);

extern sdfGrScreenDef_t sdfGrFrameModes[4];
extern sdfGrFrameSet_t D_00529388;
extern sceGsDispEnv D_00580FF0;

extern u8 reqMakeFrameBuff;
extern u8 sdfGrScreenMode;
extern u8 sdfGrGsResetDone;
extern s8 sdfDispFrameId[2];
extern volatile s8 sdfGrDrawBusy;
extern s32 sdfGrCaptureReq;
extern s32 sdfGrCaptureTarget;
extern s8 sdfGrCaptureBuff;

void sdfGrSetFrameSetMode(sdfGrFrameSet_t *aSet, s32 aMode, s32 aFree);
void sdfGrFreeFrameSet(sdfGrFrameSet_t *aSet);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrSetFrameSetMode);

void sdfGrSetScreen(s32 aMode)
{
    sdfGrScreenMode = aMode;
    sdfGrSetFrameSetMode(&D_00529388, aMode, 0);
    reqMakeFrameBuff = 1;
}

void sdfGrFreeFrameSet(sdfGrFrameSet_t *aSet)
{
    sdfGsMemFree(aSet->frameHandle[0]);
    aSet->frameHandle[0] = NULL;
    sdfGsMemFree(aSet->frameHandle[1]);
    aSet->frameHandle[1] = NULL;
    sdfGsMemFree(aSet->zbuffHandle);
    aSet->zbuffHandle = NULL;
}

void sdfGrMakeFrameSet(sdfGrFrameSet_t *aSet)
{
    s16 width;
    s16 height;
    u8 format;

    sdfGrFreeFrameSet(aSet);
    width = aSet->w;
    height = aSet->buffH;
    aSet->zbuffHandle = sdfGsAllocFrameBuff(width, height, aSet->zbuffFormat);
    format = aSet->pixelFormat;
    aSet->frameHandle[0] = sdfGsAllocFrameBuff(width, height, format);
    aSet->frameHandle[1] = sdfGsAllocFrameBuff(width, height, format);
}

void sdfGrSetDisp(s32 aBuffId)
{
    sdfGsMemSlice_t *buff = D_00529388.frameHandle[aBuffId];
    u32 fbp;

    sceGsSetDefDispEnv((sceGsDispEnv *)(((u32)&D_00580FF0 & 0x0FFFFFFF) | 0x20000000), 0,
                       D_00529388.w, D_00529388.buffH, 0, 0);
    fbp = (buff->addr >> 11) & 0x1FF;
    D_00580FF0.dispfb = (D_00580FF0.dispfb & ~0x1FF) | fbp;
    sceGsPutDispEnv(&D_00580FF0);
}

void sdfGrGsReset(void)
{
    s32 inter;
    s32 ffmode;

    switch (sdfGrFrameModes[sdfGrScreenMode].fieldFrameMode) {
    case 0:
        inter = 0;
        ffmode = 1;
        break;
    case 1:
        inter = 1;
        ffmode = 0;
        break;
    default:
        inter = 1;
        ffmode = 1;
        break;
    }
    sceGsResetGraph(sdfGrGsResetDone, inter, 2, ffmode);
    sdfGrGsResetDone = 1;
}

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDrawThIntHandler);

void sdfGrReqCapture(s32 aReq, s32 aTarget)
{
    sdfGrCaptureReq = aReq;
    sdfGrCaptureTarget = aTarget;
    sdfGrCaptureBuff = 1;
}

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrCapture);

void sdfDeliverDrawStart(s32 aOldBuffId, s32 aNextBuffId)
{
    if (sdfDispFrameId[0] == aOldBuffId) {
        sdfDispFrameId[0] = aOldBuffId ^ 1;
    }
    if (sdfDispFrameId[1] == aOldBuffId) {
        sdfDispFrameId[1] = aOldBuffId ^ 1;
    }
    sdfGrDrawBusy = aNextBuffId;
}

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDeliverDrawEnd);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDrawThread);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDispThread);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrInit);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrInitNewFrame);
INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrScreenMode);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrGsResetDone);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfFrameId);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", dds3GsSema);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrCaptureBuff);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfDispThreadFlag);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrCaptureReq);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrCaptureTarget);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", dds3GsThreadId);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfDispFrameId);

INCLUDE_SDATA(const s32, "sdf/sdfGraph", sdfGrDrawBusy);

