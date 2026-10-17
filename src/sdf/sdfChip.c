#include "common.h"

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocNextPage);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocGetFree);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocPageGrow);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocNewPage);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocDirect);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipAllocZeroDirect);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipFreeDirect);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipFreeDelayedDirect);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipCheck);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipInit);

INCLUDE_ASM(const s32, "sdf/sdfChip", sdfChipGetInfo);
