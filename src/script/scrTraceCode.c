#include "common.h"
#include "newdata.h"
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
extern const scrCommandTable_t scrCommandTable[];
extern s32 (*codefunc[])(scrProcessWork_s *pw);

extern s32 PopInt(scrProcessWork_s *pw);
extern f32 PopFloat(scrProcessWork_s *pw);
extern void CodeFunc_2Operation(scrProcessWork_s *pw, s32 operation);

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

s32 PopInt(scrProcessWork_s *pw)
{
    s32 stackIndex = pw->SP;

    pw->SP = stackIndex - 1;
    switch ((s8)pw->StackType[stackIndex - 1]) {
    case SCR_STACK_TYPE_INT:
    case SCR_STACK_TYPE_RET:
        return pw->StackData[pw->SP].i;
    case SCR_STACK_TYPE_FLOAT:
        return pw->StackData[pw->SP].f;
    case SCR_STACK_TYPE_INTVAR:
        return DDS3_GBWK->integers[pw->StackData[pw->SP].i];
    case SCR_STACK_TYPE_FLOATVAR:
        return DDS3_GBWK->floats[pw->StackData[pw->SP].i];
    }
    return 0;
}

f32 PopFloat(scrProcessWork_s *pw)
{
    s32 stackIndex = pw->SP;

    pw->SP = stackIndex - 1;
    switch ((s8)pw->StackType[stackIndex - 1]) {
    case SCR_STACK_TYPE_INT:
        return (f32)pw->StackData[pw->SP].i;
    case SCR_STACK_TYPE_RET:
        return 0.0f;
    case SCR_STACK_TYPE_FLOAT:
        return pw->StackData[pw->SP].f;
    case SCR_STACK_TYPE_INTVAR:
        return (f32)DDS3_GBWK->integers[pw->StackData[pw->SP].i];
    case SCR_STACK_TYPE_FLOATVAR:
        return DDS3_GBWK->floats[pw->StackData[pw->SP].i];
    }
    return 0.0f;
}

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

s32 CodeFunc_PushIX(scrProcessWork_s *pw)
{
    PushInt(pw, DDS3_GBWK->integers[pw->Code[pw->CP].s.s1]);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PushFX(scrProcessWork_s *pw)
{
    PushFloat(pw, DDS3_GBWK->floats[pw->Code[pw->CP].s.s1]);
    pw->CP++;
    return CODE_RET_NEXT;
}

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

s32 CodeFunc_PopIX(scrProcessWork_s *pw)
{
    DDS3_GBWK->integers[pw->Code[pw->CP].s.s1] = PopInt(pw);
    pw->CP++;
    return CODE_RET_NEXT;
}

s32 CodeFunc_PopFX(scrProcessWork_s *pw)
{
    f32 value = PopFloat(pw);

    DDS3_GBWK->floats[pw->Code[pw->CP].s.s1] = value;
    pw->CP++;
    return CODE_RET_NEXT;
}

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

/* A pending command yields without consuming its arguments. */
s32 CodeFunc_Comm(scrProcessWork_s *pw)
{
    s32 commandCP = pw->CP;
    s32 commandIndex = pw->Code[commandCP].s.s1;

    nowpwork = pw;
    if (scrCommandTable[commandIndex].comm() == 0) {
        return CODE_RET_SYNC;
    }
    pw->SP -= scrCommandTable[commandIndex].paranum;
    if (commandCP == pw->CP) {
        pw->CP = commandCP + 1;
    }
    return CODE_RET_NEXT;
}

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

extern s32 CodeFunc_2Operation_i1;
extern s32 CodeFunc_2Operation_i2;
extern s32 CodeFunc_2Operation_ri;
extern f32 CodeFunc_2Operation_f1;
extern f32 CodeFunc_2Operation_f2;
extern f32 CodeFunc_2Operation_rf;
extern s32 CodeFunc_2Operation_t1;
extern s32 CodeFunc_2Operation_t2;

/* Recovered symbol names for ee-gcc's fptodp/dpcmp runtime entry points. */
extern f64 sdfFloatToULong(f32 value) __attribute__((const));
extern s32 dpdiv(f64 left, f64 right) __attribute__((const));

void CodeFunc_2Operation(scrProcessWork_s *pw, s32 operatorCode)
{
    CodeFunc_2Operation_t1 = (s8)pw->StackType[pw->SP - 1];
    CodeFunc_2Operation_t2 = (s8)pw->StackType[pw->SP - 2];
    if ((CodeFunc_2Operation_t1 == 0 || CodeFunc_2Operation_t1 == 2) && (CodeFunc_2Operation_t2 == 0 || CodeFunc_2Operation_t2 == 2)) {
        CodeFunc_2Operation_i1 = PopInt(pw);
        CodeFunc_2Operation_i2 = PopInt(pw);
        switch (operatorCode) {
        case 0:
            CodeFunc_2Operation_ri = CodeFunc_2Operation_i1 + CodeFunc_2Operation_i2;
            break;
        case 1:
            CodeFunc_2Operation_ri = CodeFunc_2Operation_i1 - CodeFunc_2Operation_i2;
            break;
        case 2:
            CodeFunc_2Operation_ri = CodeFunc_2Operation_i1 * CodeFunc_2Operation_i2;
            break;
        case 3:
            CodeFunc_2Operation_ri = CodeFunc_2Operation_i1 / CodeFunc_2Operation_i2;
            break;
        case 4:
            if (CodeFunc_2Operation_i1 != 0 || CodeFunc_2Operation_i2 != 0) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 5:
            if (CodeFunc_2Operation_i1 != 0 && CodeFunc_2Operation_i2 != 0) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 6:
            if (CodeFunc_2Operation_i1 == CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 7:
            if (CodeFunc_2Operation_i1 != CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 8:
            if (CodeFunc_2Operation_i1 < CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 9:
            if (CodeFunc_2Operation_i1 > CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 10:
            if (CodeFunc_2Operation_i1 <= CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        case 11:
            if (CodeFunc_2Operation_i1 >= CodeFunc_2Operation_i2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            break;
        }
        PushInt(pw, CodeFunc_2Operation_ri);
    } else {
        CodeFunc_2Operation_f1 = PopFloat(pw);
        CodeFunc_2Operation_f2 = PopFloat(pw);
        switch (operatorCode) {
        case 0:
            CodeFunc_2Operation_rf = CodeFunc_2Operation_f1 + CodeFunc_2Operation_f2;
            break;
        case 1:
            CodeFunc_2Operation_rf = CodeFunc_2Operation_f1 - CodeFunc_2Operation_f2;
            break;
        case 2:
            CodeFunc_2Operation_rf = CodeFunc_2Operation_f1 * CodeFunc_2Operation_f2;
            break;
        case 3:
            CodeFunc_2Operation_rf = CodeFunc_2Operation_f1 / CodeFunc_2Operation_f2;
            break;
        case 4:
            if (dpdiv(sdfFloatToULong(CodeFunc_2Operation_f1), 0.0) != 0 ||
                dpdiv(sdfFloatToULong(CodeFunc_2Operation_f2), 0.0) != 0) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 5:
            if (dpdiv(sdfFloatToULong(CodeFunc_2Operation_f1), 0.0) != 0 &&
                dpdiv(sdfFloatToULong(CodeFunc_2Operation_f2), 0.0) != 0) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 6:
            if (CodeFunc_2Operation_f1 == CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 7:
            if (CodeFunc_2Operation_f1 != CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 8:
            if (CodeFunc_2Operation_f1 < CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 9:
            if (CodeFunc_2Operation_f1 > CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 10:
            if (CodeFunc_2Operation_f1 <= CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        case 11:
            if (CodeFunc_2Operation_f1 >= CodeFunc_2Operation_f2) {
                CodeFunc_2Operation_ri = 1;
            } else {
                CodeFunc_2Operation_ri = 0;
            }
            PushInt(pw, CodeFunc_2Operation_ri);
            return;
        }
        PushFloat(pw, CodeFunc_2Operation_rf);
    }
}


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

s32 CodeFunc_Minus(scrProcessWork_s *pw)
{
    switch ((s8)pw->StackType[pw->SP - 1]) {
    case SCR_STACK_TYPE_INT:
        pw->StackData[pw->SP - 1].i = -pw->StackData[pw->SP - 1].i;
        break;
    case SCR_STACK_TYPE_FLOAT:
        pw->StackData[pw->SP - 1].f = -pw->StackData[pw->SP - 1].f;
        break;
    case SCR_STACK_TYPE_INTVAR:
        DDS3_GBWK->integers[pw->StackData[pw->SP - 1].i] =
            -DDS3_GBWK->integers[pw->StackData[pw->SP - 1].i];
        break;
    case SCR_STACK_TYPE_FLOATVAR:
        DDS3_GBWK->floats[pw->StackData[pw->SP - 1].i] =
            -DDS3_GBWK->floats[pw->StackData[pw->SP - 1].i];
        break;
    case SCR_STACK_TYPE_RET:
        break;
    }
    pw->CP++;
    return CODE_RET_NEXT;
}

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

/* Continue until an instruction completes the procedure or yields a tick. */
s32 scrTraceCode(scrProcessWork_s *pw)
{
    s32 result;

    while (1) {
        result = codefunc[pw->Code[pw->CP].s.s0](pw);
        if (result == CODE_RET_END) {
            pw->CommandTimer = 0;
            return SCR_RESULT_END;
        }
        if (result == CODE_RET_SYNC) {
            break;
        }
        if (result == CODE_RET_NEXT) {
            pw->CommandTimer = 0;
        }
    }
    pw->CommandTimer++;
    pw->ScriptTimer++;
    return SCR_RESULT_CONTINUE;
}

s32 scrGetIntPara(s32 parameterIndex)
{
    scrProcessWork_s *pw = nowpwork;
    s32 stackIndex = pw->SP - parameterIndex - 1;

    switch ((s8)pw->StackType[stackIndex]) {
    case SCR_STACK_TYPE_INT:
    case SCR_STACK_TYPE_RET:
        return pw->StackData[stackIndex].i;
    case SCR_STACK_TYPE_FLOAT:
        return pw->StackData[stackIndex].f;
    case SCR_STACK_TYPE_INTVAR:
        return DDS3_GBWK->integers[pw->StackData[stackIndex].i];
    case SCR_STACK_TYPE_FLOATVAR:
        return DDS3_GBWK->floats[pw->StackData[stackIndex].i];
    }
    return 0;
}

f32 scrGetFloatPara(s32 parameterIndex)
{
    scrProcessWork_s *pw = nowpwork;
    s32 stackIndex = pw->SP - parameterIndex - 1;

    switch ((s8)pw->StackType[stackIndex]) {
    case SCR_STACK_TYPE_INT:
    case SCR_STACK_TYPE_RET:
        return (f32)pw->StackData[stackIndex].i;
    case SCR_STACK_TYPE_FLOAT:
        return pw->StackData[stackIndex].f;
    case SCR_STACK_TYPE_INTVAR:
        return (f32)DDS3_GBWK->integers[pw->StackData[stackIndex].i];
    case SCR_STACK_TYPE_FLOATVAR:
        return DDS3_GBWK->floats[pw->StackData[stackIndex].i];
    }
    return 0.0f;
}

u8 *scrGetStrPara(s32 parameterIndex)
{
    scrProcessWork_s *pw = nowpwork;
    s32 stackIndex = pw->SP - parameterIndex - 1;
    s32 stackType = (s8)pw->StackType[stackIndex];

    if (stackType >= 0) {
        if (stackType >= SCR_STACK_TYPE_STRING) {
            if (stackType == SCR_STACK_TYPE_STRING) {
                return (u8 *)pw->StackData[stackIndex].i;
            }
        }
    }
    return NULL;
}

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
