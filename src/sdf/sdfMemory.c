#include "common.h"

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemInit);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetType);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetTop);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetNext);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetSize);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetPtr);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemAllocDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemAllocFromTailDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemAlloc2Direct);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemReallocRecord);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemFreeMerge);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemFreeDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemFreePtrDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemFreeHandleDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemFreeDelayedDirect);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemLock);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemUnlock);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetHandleByPtr);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemGetInfo);

INCLUDE_ASM(const s32, "sdf/sdfMemory", sdfMemSearchHandle);

INCLUDE_ASM(const s32, "sdf/sdfMemory", SdfIopFileRpc);
INCLUDE_SDATA(const s32, "sdf/sdfMemory", sdfMemFullCallback);

INCLUDE_SDATA(const s32, "sdf/sdfMemory", sdfGrDrawFirstFlag);

INCLUDE_SDATA(const s32, "sdf/sdfMemory", sdfFrameEvenOdd);

