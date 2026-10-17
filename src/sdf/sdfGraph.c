#include "common.h"

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrSetFrameSetMode);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrSetScreen);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrFreeFrameSet);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrMakeFrameSet);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrSetDisp);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrGsReset);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDrawThIntHandler);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrReqCapture);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfGrCapture);

INCLUDE_ASM(const s32, "sdf/sdfGraph", sdfDeliverDrawStart);

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

