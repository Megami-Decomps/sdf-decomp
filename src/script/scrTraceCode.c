#include "common.h"
#include "scr.h"

enum code_operation
{
    CODE_OPERATION_ADD = 0,
    CODE_OPERATION_SUB = 1,
    CODE_OPERATION_MUL = 2,
    CODE_OPERATION_DIV = 3,
    CODE_OPERATION_OR = 4,
    CODE_OPERATION_AND = 5,
    CODE_OPERATION_EQ = 6,
    CODE_OPERATION_NEQ = 7,
    CODE_OPERATION_S = 8,
    CODE_OPERATION_L = 9,
    CODE_OPERATION_SE = 10,
    CODE_OPERATION_LE = 11,
    CODE_OPERATION_MAX = 12
};

extern scrProcessWork_s *nowpwork;
extern scrCommandTable_t scrCommandTable[];

extern s32 PopInt(scrProcessWork_s *pw);
extern f32 PopFloat(scrProcessWork_s *pw);
extern s32 CodeFunc_2Operation(scrProcessWork_s *pw, s32 operation);

void PushInt(scrProcessWork_s *pw, s32 value)
{
    pw->StackType[pw->SP] = SCR_STACK_TYPE_INT;
    pw->StackData[pw->SP].i = value;
    pw->SP++;
}

void PushFloat(scrProcessWork_s *pw, f32 value)
{
    pw->StackType[pw->SP] = SCR_STACK_TYPE_FLOAT;
    pw->StackData[pw->SP].f = value;
    pw->SP++;
}

void PushStr(scrProcessWork_s *pw, u8 *str)
{
    pw->StackType[pw->SP] = SCR_STACK_TYPE_STRING;
    pw->StackData[pw->SP].i = (s32)str;
    pw->SP++;
}

void PushRet(scrProcessWork_s *pw, s32 cp)
{
    pw->StackType[pw->SP] = SCR_STACK_TYPE_RET;
    pw->StackData[pw->SP].i = cp;
    pw->SP++;
}

INCLUDE_RODATA(const s32, "script/scrTraceCode", scrCommandTable);

INCLUDE_ASM(const s32, "script/scrTraceCode", PopInt);

INCLUDE_ASM(const s32, "script/scrTraceCode", PopFloat);

s32 CodeFunc_PushI(scrProcessWork_s *pw)
{
    pw->CP++;
    PushInt(pw, pw->Code[pw->CP].i);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushIS(scrProcessWork_s *pw)
{
    PushInt(pw, pw->Code[pw->CP].s.s1);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushF(scrProcessWork_s *pw)
{
    pw->CP++;
    PushFloat(pw, pw->Code[pw->CP].f);
    pw->CP++;
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PushFX);

s32 CodeFunc_PushLIX(scrProcessWork_s *pw)
{
    PushInt(pw, pw->LocalInt[pw->Code[pw->CP].s.s1]);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushLFX(scrProcessWork_s *pw)
{
    PushFloat(pw, pw->LocalFloat[pw->Code[pw->CP].s.s1]);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushSTR(scrProcessWork_s *pw)
{
    PushStr(pw, pw->StringData + pw->Code[pw->CP].s.s1);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushReg(scrProcessWork_s *pw)
{
    pw->StackType[pw->SP] = pw->StackType[27];
    pw->StackData[pw->SP].i = pw->StackData[27].i;
    pw->SP++;
    pw->CP++;
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopIX);

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_PopFX);

s32 CodeFunc_PopLIX(scrProcessWork_s *pw)
{
    pw->LocalInt[pw->Code[pw->CP].s.s1] = PopInt(pw);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PopLFX(scrProcessWork_s *pw)
{
    pw->LocalFloat[pw->Code[pw->CP].s.s1] = PopFloat(pw);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Proc(scrProcessWork_s *pw)
{
    pw->CP++;
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Comm);

s32 CodeFunc_End(scrProcessWork_s *pw)
{
    if (pw->SP == 0) {
        return CODE_RET_END;
    }

    pw->CP = PopInt(pw) + 1;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Jump(scrProcessWork_s *pw)
{
    pw->CP = pw->ProcTable[pw->Code[pw->CP].s.s1].CodeAddress;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Call(scrProcessWork_s *pw)
{
    PushRet(pw, pw->CP);
    pw->CP = pw->ProcTable[pw->Code[pw->CP].s.s1].CodeAddress;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Run(scrProcessWork_s *pw)
{
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Goto(scrProcessWork_s *pw)
{
    pw->CP = pw->LabelTable[pw->Code[pw->CP].s.s1].CodeAddress;
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_2Operation);

s32 CodeFunc_Add(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_ADD);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Sub(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_SUB);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Mul(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_MUL);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Div(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_DIV);
    pw->CP++;
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", CodeFunc_Minus);

s32 CodeFunc_Not(scrProcessWork_s *pw)
{
    switch ((s8)pw->StackType[pw->SP - 1]) {
    case SCR_STACK_TYPE_INT:
    case SCR_STACK_TYPE_INTVAR:
        PushInt(pw, PopInt(pw) == 0);
        break;
    case SCR_STACK_TYPE_FLOAT:
    case SCR_STACK_TYPE_FLOATVAR:
        PushInt(pw, PopFloat(pw) == 0.0f);
        break;
    case SCR_STACK_TYPE_RET:
        break;
    }

    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_Or(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_OR);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_And(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_AND);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_EQ(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_EQ);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_NEQ(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_NEQ);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_S(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_S);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_L(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_L);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_SE(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_SE);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_LE(scrProcessWork_s *pw)
{
    CodeFunc_2Operation(pw, CODE_OPERATION_LE);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_IF(scrProcessWork_s *pw)
{
    s32 cond;

    switch ((s8)pw->StackType[pw->SP - 1]) {
    case SCR_STACK_TYPE_INT:
    case SCR_STACK_TYPE_INTVAR:
        cond = PopInt(pw);
        break;
    case SCR_STACK_TYPE_FLOAT:
    case SCR_STACK_TYPE_FLOATVAR:
        cond = PopFloat(pw) != 0.0f;
        break;
    case SCR_STACK_TYPE_RET:
    default:
        cond = 0;
        break;
    }

    if (cond) {
        pw->CP++;
    } else {
        pw->CP = pw->LabelTable[pw->Code[pw->CP].s.s1].CodeAddress;
    }
    return CODE_RET_NEXT;
}

INCLUDE_ASM(const s32, "script/scrTraceCode", scrTraceCode);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetIntPara);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetFloatPara);

INCLUDE_ASM(const s32, "script/scrTraceCode", scrGetStrPara);

void scrSetIntReturnValue(s32 value)
{
    nowpwork->StackType[27] = SCR_STACK_TYPE_INT;
    nowpwork->StackData[27].i = value;
}

void scrSetFloatReturnValue(f32 value)
{
    nowpwork->StackType[27] = SCR_STACK_TYPE_FLOAT;
    nowpwork->StackData[27].f = value;
}

s32 scrGetProcedureCodeAddress(s32 no)
{
    return nowpwork->ProcTable[no].CodeAddress;
}

s32 scrGetLabelCodeAddress(s32 no)
{
    return nowpwork->LabelTable[no].CodeAddress;
}

s32 scrGetCodePointer(void)
{
    return nowpwork->CP;
}

void scrSetCodePointer(s32 cp)
{
    nowpwork->CP = cp;
}

s32 scrGetScriptTimer(void)
{
    return nowpwork->ScriptTimer;
}

s32 scrGetCommandTimer(void)
{
    return nowpwork->CommandTimer;
}

s32 scrGetMessageHandle(void)
{
    return nowpwork->MesHandle;
}

scrProcessWork_s *scrGetScriptHandle(void)
{
    return nowpwork;
}

void *scrGetData(void)
{
    return nowpwork->data;
}
