#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

extern void sdfDelayedResrcEntry(sdfDelayedResrcCB_t *cb, void *resource);

/* GS pixel storage modes, the values of sdfGsMemSlice_t.pixelFormat. */
enum sdf_psm
{
    SDF_PSMCT32 = 0x00,
    SDF_PSMCT24 = 0x01,
    SDF_PSMCT16 = 0x02,
    SDF_PSMCT16S = 0x0A,
    SDF_PSMT8 = 0x13,
    SDF_PSMT4 = 0x14,
    SDF_PSMT8H = 0x1B,
    SDF_PSMT4HL = 0x24,
    SDF_PSMT4HH = 0x2C,
    SDF_PSMZ32 = 0x30,
    SDF_PSMZ24 = 0x31,
    SDF_PSMZ16 = 0x32,
    SDF_PSMZ16S = 0x3A
};

extern s32 DIntr(void);

extern void EIntr(void);

extern void *sdfChipAllocDirect(u32 size);

extern void sdfChipFreeDirect(void *ptr);

extern void sdfDelayedResrcInitCB(sdfDelayedResrcCB_t *cb, sdfDelayedResrcFreeFunc freeFunc);

extern sdfGsMemSlice_t *gsMemHandleLinkHead;

extern sdfGsMemSlice_t *gsMemHandleLinkTail;

extern sdfDelayedResrcCB_t gsMemDelayedResrcCB;

extern void (*sdfGsErrorHandler)(s32 size, s32 type);

/* sdfImageSetRelease_t.freeMode: which heap owns the uploaded pixels. */
enum sdf_image_set_free_mode
{
    SDF_IMAGE_SET_FREE_NONE = 0,
    SDF_IMAGE_SET_FREE_MEM = 1,
    SDF_IMAGE_SET_FREE_CHIP = 2
};

s32 sdfGsMemGetBitCount(u32 pixelFormat);

sdfGsMemSlice_t *sdfGsMemAllocFromHead(s32 size, s32 type);

sdfGsMemSlice_t *sdfGsMemAllocFromTail(s32 size, s32 type);

s32 sdfGsMemCheckMerge(sdfGsMemSlice_t *slice);

void sdfGsMemFree(sdfGsMemSlice_t *slice);

s32 sdfGsMemGetBitCount(u32 pixelFormat)
{
    switch (pixelFormat) {
    case SDF_PSMCT32:
    case SDF_PSMCT24:
    case SDF_PSMT8H:
    case SDF_PSMT4HL:
    case SDF_PSMT4HH:
    case SDF_PSMZ32:
    case SDF_PSMZ24:
        return 0x20;
    case SDF_PSMCT16:
    case SDF_PSMCT16S:
    case SDF_PSMZ16:
    case SDF_PSMZ16S:
        return 0x10;
    case SDF_PSMT8:
        return 8;
    case SDF_PSMT4:
        return 4;
    default:
        return 0;
    }
}

s32 sdfGetTexelBitCount(u32 pixelFormat)
{
    switch (pixelFormat) {
    case SDF_PSMCT32:
    case SDF_PSMZ32:
        return 0x20;
    case SDF_PSMCT24:
    case SDF_PSMZ24:
        return 0x18;
    case SDF_PSMCT16:
    case SDF_PSMCT16S:
    case SDF_PSMZ16:
    case SDF_PSMZ16S:
        return 0x10;
    case SDF_PSMT8:
    case SDF_PSMT8H:
        return 8;
    case SDF_PSMT4:
    case SDF_PSMT4HL:
    case SDF_PSMT4HH:
        return 4;
    default:
        return 0;
    }
}

s32 sdfGsMemCheckHandle(sdfGsMemSlice_t *target) {
    sdfGsMemSlice_t *node = gsMemHandleLinkHead;

    if (node == NULL) {
        return 0;
    }
    do {
        if (node == (void *)target) {
            return 1;
        }
        node = node->next;
    } while (node != NULL);
    return 0;
}

sdfGsMemSlice_t *sdfGsMemAllocFromHead(s32 size, s32 type)
{
    sdfGsMemSlice_t *slice;
    sdfGsMemSlice_t *alloc;
    s32 reenable;

    reenable = DIntr();
    for (slice = gsMemHandleLinkHead;; slice = slice->next) {
        if (slice == NULL) {
            if (reenable != 0) {
                EIntr();
            }
            if (sdfGsErrorHandler != NULL) {
                sdfGsErrorHandler(size, type);
            }
        }
        if (slice->type != 0 || slice->size < size) {
            continue;
        }
        if (slice->size == size) {
            slice->type = type;
            if (reenable != 0) {
                EIntr();
            }
            return slice;
        }
        alloc = sdfChipAllocDirect(sizeof(*alloc));
        alloc->prev = slice->prev;
        if (alloc->prev == NULL) {
            gsMemHandleLinkHead = alloc;
        } else {
            slice->prev->next = alloc;
        }
        alloc->type = type;
        slice->prev = alloc;
        alloc->addr = slice->addr;
        alloc->size = size;
        alloc->next = slice;
        slice->addr += size;
        slice->size -= size;
        if (reenable != 0) {
            EIntr();
        }
        return alloc;
    }
}

sdfGsMemSlice_t *sdfGsMemAllocFromTail(s32 size, s32 type)
{
    sdfGsMemSlice_t *slice;
    sdfGsMemSlice_t *alloc;
    s32 reenable;

    reenable = DIntr();
    for (slice = gsMemHandleLinkTail;; slice = slice->prev) {
        if (slice == NULL) {
            if (reenable != 0) {
                EIntr();
            }
            if (sdfGsErrorHandler != NULL) {
                sdfGsErrorHandler(size, type);
            }
        }
        if (slice->type != 0 || slice->size < size) {
            continue;
        }
        if (slice->size == size) {
            slice->type = type;
            if (reenable != 0) {
                EIntr();
            }
            return slice;
        }
        alloc = sdfChipAllocDirect(sizeof(*alloc));
        alloc->next = slice->next;
        if (alloc->next == NULL) {
            gsMemHandleLinkTail = alloc;
        } else {
            slice->next->prev = alloc;
        }
        alloc->type = type;
        slice->next = alloc;
        alloc->addr = slice->addr + (slice->size - size);
        alloc->size = size;
        alloc->prev = slice;
        slice->size -= size;
        if (reenable != 0) {
            EIntr();
        }
        return alloc;
    }
}

sdfGsMemSlice_t *sdfGsMemAllocRect(s32 w, s32 h, s32 pixelFormat, s32 type, s32 fromHead)
{
    s32 bitCount = sdfGsMemGetBitCount(pixelFormat);
    s32 words = (w * h * bitCount) >> 5;
    sdfGsMemSlice_t *slice;

    if (fromHead != 0) {
        slice = sdfGsMemAllocFromHead(words, type);
    } else {
        slice = sdfGsMemAllocFromTail(words, type);
    }
    slice->w = w;
    slice->h = h;
    slice->pixelFormat = pixelFormat;
    return slice;
}

s32 sdfGsMemCheckMerge(sdfGsMemSlice_t *slice)
{
    sdfGsMemSlice_t *below = slice->next;

    if (below == NULL || below->type != 0) {
        return 0;
    }
    slice->size += below->size;
    if ((slice->next = below->next) != NULL) {
        below->next->prev = slice;
    } else {
        gsMemHandleLinkTail = slice;
    }
    sdfChipFreeDirect(below);
    return 1;
}

void sdfGsMemCheckLinkConsistence(void) {
    sdfGsMemSlice_t *node;

    node = gsMemHandleLinkHead;
    while (node->next != NULL) {
        node = node->next;
    }
    node = gsMemHandleLinkTail;
    while (node->prev != NULL) {
        node = node->prev;
    }
}

void sdfGsMemFree(sdfGsMemSlice_t *slice)
{
    s32 reenable;

    if (slice == NULL) {
        return;
    }
    reenable = DIntr();
    slice->type = 0;
    sdfGsMemCheckMerge(slice);
    if (slice->prev != NULL) {
        if (slice->prev->type == 0) {
            sdfGsMemCheckMerge(slice->prev);
        }
    }
    if (reenable != 0) {
        EIntr();
    }
}

void sdfGsMemFreeDelayed(sdfGsMemSlice_t *texture) {
    sdfDelayedResrcEntry(&gsMemDelayedResrcCB, texture);
}

void sdfGsMemInit(void)
{
    sdfGsMemSlice_t *slice;

    slice = sdfChipAllocDirect(sizeof(sdfGsMemSlice_t));
    slice->size = 0x100000;
    slice->prev = NULL;
    slice->next = NULL;
    slice->type = 0;
    slice->addr = 0;
    gsMemHandleLinkHead = slice;
    gsMemHandleLinkTail = slice;
    sdfDelayedResrcInitCB(&gsMemDelayedResrcCB, sdfGsMemFree);
}

sdfGsMemSlice_t *sdfGsAllocFrameBuff(s32 w, s32 h, s32 pixelFormat)
{
    s32 rowWidth = (w + 0x3F) & -0x40;
    s32 rowCount = (h + 0x1F) & -0x20;
    sdfGsMemSlice_t *slice;

    switch (pixelFormat) {
    case SDF_PSMCT16:
    case SDF_PSMCT16S:
    case SDF_PSMZ16:
    case SDF_PSMZ16S:
        rowWidth >>= 1;
        break;
    case SDF_PSMCT32:
    case SDF_PSMCT24:
    case SDF_PSMZ32:
    case SDF_PSMZ24:
        break;
    }
    slice = sdfGsMemAllocFromHead((rowWidth * rowCount + 0x7FF) & -0x800, 1);
    slice->w = w;
    slice->h = h;
    slice->pixelFormat = pixelFormat;
    return slice;
}

sdfGsMemSlice_t *sdfGsMemGetLinkHead(void) {
    return gsMemHandleLinkHead;
}

sdfGsMemSlice_t *sdfGsMemGetLinkTail(void) {
    return gsMemHandleLinkTail;
}

INCLUDE_SDATA(const s32, "sdf/sdfGsMemory", sdfGsErrorHandler);

