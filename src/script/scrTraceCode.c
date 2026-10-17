#include "common.h"

INCLUDE_ASM(const s32, "script/scrTraceCode", PushInt);

INCLUDE_ASM(const s32, "script/scrTraceCode", PushFloat);

INCLUDE_ASM(const s32, "script/scrTraceCode", PushStr);

INCLUDE_ASM(const s32, "script/scrTraceCode", PushRet);

INCLUDE_RODATA(const s32, "script/scrTraceCode", scrCommandTable);

INCLUDE_ASM(const s32, "script/scrTraceCode", PopInt);

INCLUDE_ASM(const s32, "script/scrTraceCode", PopFloat);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushI);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushIS);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushF);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushFX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushLIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushLFX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushSTR);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushReg);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopFX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopLIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopLFX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Proc);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Comm);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_End);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Jump);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Call);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Run);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Goto);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_2Operation);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Add);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Sub);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Mul);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Div);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Minus);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Not);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Or);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_And);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_EQ);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_NEQ);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_S);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_L);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_SE);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_LE);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_IF);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrTraceCode);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetIntPara);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetFloatPara);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetStrPara);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrSetIntReturnValue);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrSetFloatReturnValue);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetProcedureCodeAddress);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetLabelCodeAddress);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetCodePointer);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrSetCodePointer);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetScriptTimer);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetCommandTimer);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetMessageHandle);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetScriptHandle);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetData);
