#include "common.h"

INCLUDE_ASM(const s32, "model/mdlManager", mdlInitManager);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSearchResrc);

INCLUDE_ASM(const s32, "model/mdlManager", mdlCheckLoading);

INCLUDE_ASM(const s32, "model/mdlManager", mdlRegLoading);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegLoading);

INCLUDE_ASM(const s32, "model/mdlManager", mdlRegist);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegistResrc);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegist);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegistAll);

INCLUDE_ASM(const s32, "model/mdlManager", mdlCreateMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFreeMotionCtrlr);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFreeMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegistResrcMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnRegistMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlRegistMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlRegistModelSet);

INCLUDE_ASM(const s32, "model/mdlManager", mdlWaitLoadEnd);

INCLUDE_ASM(const s32, "model/mdlManager", TbMbAbLoadEnd);

INCLUDE_ASM(const s32, "model/mdlManager", TbLoadCallback);

INCLUDE_ASM(const s32, "model/mdlManager", MbLoadCallback);

INCLUDE_ASM(const s32, "model/mdlManager", AbLoadCallback);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetFileName);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPrepare);

INCLUDE_ASM(const s32, "model/mdlManager", mdlLoadPrepare);

INCLUDE_ASM(const s32, "model/mdlManager", mdlUnlinkResrc);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetBlur);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFreeBlur);

INCLUDE_ASM(const s32, "model/mdlManager", mdlLinkBlur);

INCLUDE_ASM(const s32, "model/mdlManager", mdlLoad);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFree);

INCLUDE_ASM(const s32, "model/mdlManager", mdlLink);

INCLUDE_ASM(const s32, "model/mdlManager", mdlHideAllNode);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlLoopMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSingleMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSmoothLoopMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSmoothSingleMotion);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMotionCtrlr);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMotionNum);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMotionLengthEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMotionFrameEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlChkMotionEndEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMotionSpeedEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetMotionSpeedEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetPosition);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetPosition);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetRotate);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetRotate);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetScale);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetScale);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetColor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetEffectColor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetColor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetColorWithEffect);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetEffectScale);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetScreenOffset);

INCLUDE_ASM(const s32, "model/mdlManager", mdl_1DD9C0);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMotionPause);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMotionPauseOff);

INCLUDE_ASM(const s32, "model/mdlManager", mdlHasMotionEx);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMajorNum);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetMinorNum);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetNumOfMajor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetNumOfMinor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetNumOfCut);

INCLUDE_ASM(const s32, "model/mdlManager", mdlSetClutColor);

INCLUDE_ASM(const s32, "model/mdlManager", mdlGetFilebase);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackCheck);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackSetOld);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackSet);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFileCbFree);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFileLoadCallback);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFileLoad);

INCLUDE_ASM(const s32, "model/mdlManager", mdlPackDecode);

INCLUDE_ASM(const s32, "model/mdlManager", mdlFreeFileItem);

INCLUDE_ASM(const s32, "model/mdlManager", mdlReleaseHddFileList);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMakeHddFileListDummy);

INCLUDE_ASM(const s32, "model/mdlManager", mdlMakeHddFileList);
INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F628);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F630);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F634);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F638);

INCLUDE_SDATA(const s32, "model/mdlManager", D_0054F640);

