#include "common.h"

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionNodeNew);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionNodeFree);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionNodeBaseInit);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionCtrlrNew);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionCtrlrFree);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionChangeBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionChange);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionNext);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionPause);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotionPauseOff);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_Destroy);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_SetCut);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGetKey);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyFloat);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyXYZ);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyColor);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyUvwSub);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotInterpolateKeyUvw);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenNode);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelInit);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelLinerPos);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerPosSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerPosBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelLinerRotXYZ);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerRotXYZSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerRotXYZBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelLinerScale);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerScaleSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLinerScaleBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelLerpRotQuat);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLerpRotQuatSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelLerpRotQuatBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenModelVisible);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelVisibleSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelVisibleBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelVisibleInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelPosInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelRotInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_modelScaleInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatNode);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatInit);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatDiffuse);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matDiffuseSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matDiffuseBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matDiffuseInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatAmbient);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matAmbientSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matAmbientBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matAmbientInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatSpecular);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matSpecularSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matSpecularBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matSpecularInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatRefMap);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matRefMapSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matRefMapBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matRefMapInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatConstant);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matConstantSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matConstantBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matConstantInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatUvw);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvwSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvwBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvwInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiUvw);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvwSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvwBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvwInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiDiffuse);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiDiffuseSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiDiffuseBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiDiffuseInitBlend);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatUvw2);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvw2Seek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matUvw2BlendSeek);

void sdfMotCLBK_matUvw2InitBlend(void) {
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMatMultiUvw2);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvw2Seek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_matMultiUvw2BlendSeek);

void sdfMotCLBK_matMultiUvw2InitBlend(void) {
}

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMorphNode);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMorphInit);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotGenMorph);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphBlendSeek);

INCLUDE_ASM(const s32, "sdf/sdfMotion", sdfMotCLBK_morphInitBlend);
