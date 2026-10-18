#include "common.h"
#include "libsdf.h"

typedef struct sdfDevConsCB_t
{
    struct sdfDevConsCB_t *prev;
    struct sdfDevConsCB_t *next;
    u16 x;
    u16 y;
    s16 w;
    s16 h;
    s16 locX;
    s16 locY;
    u8 mode;
    u8 pad;
    u8 attr;
    u8 tabwidth;
    sdfMemHandle_t *bufferHandle;
    u8 *bufferPtr;
} sdfDevConsCB_t;

typedef struct sdfDevConsBuf_t
{
    u8 *cur;
    u8 *start;
    u8 *tag;
    u32 *dmaCh;
    u8 *buf[2];
    s32 bufSize;
    u16 bufIdx;
    u16 rowStep;
} sdfDevConsBuf_t;

extern void sdfDevConsDrawInit(sdfDevConsBuf_t *aBuf, void *aMem, s32 aSize);
extern void sdfDevConsDraw(sdfDevConsCB_t *aHndl, sdfDevConsBuf_t *aBuf);
extern void sdfDevConsDrawFlush(sdfDevConsBuf_t *aBuf);

extern void *memset(void *dst, s32 c, u32 size);
extern void *memmove(void *dst, const void *src, u32 size);
extern s32 vsprintf(char *buf, const char *fmt, void *args);

extern void *sdfChipAllocDirect(s32 size);
extern void sdfChipFreeDirect(void *ptr);
extern sdfMemHandle_t *sdfMemAllocDirect(s32 size);
extern void sdfMemFreeDirect(sdfMemHandle_t *handle);
extern void *sdfMemLock(sdfMemHandle_t *handle);
extern sdfTexHandle_t *sdfTexSet(void *file);

extern sdfTexHandle_t *sdfDebugFontHandle;
extern s32 alreadyInitDevCons;
extern u8 sdfDebugFontTex[];
extern s16 sdfDevConsFontUvTable[];

extern sdfDevConsCB_t *devConsTop;
extern u32 sdfDevConsColorTable[];

extern void sdfDevVprintf(u32 *aCB, const char *aFmt, void *aArgs);

void sdfDevConsInit(void)
{
    if (!alreadyInitDevCons) {
        alreadyInitDevCons = 1;
        sdfDebugFontHandle = sdfTexSet(sdfDebugFontTex);
    }
}

sdfTexHandle_t *sdfDevConsGetFontHandle(void)
{
    sdfDevConsInit();
    return sdfDebugFontHandle;
}

s16 *sdfDevConsGetFontUvTable(void)
{
    return sdfDevConsFontUvTable;
}

void sdfDevConsLinkToTop(sdfDevConsCB_t *aHndl)
{
    sdfDevConsCB_t *top;

    top = devConsTop;
    aHndl->prev = NULL;
    aHndl->next = top;
    if (top) {
        top->prev = aHndl;
    }
    devConsTop = aHndl;
}

void sdfDevConsUnlink(sdfDevConsCB_t *aHndl)
{
    sdfDevConsCB_t *prev;
    sdfDevConsCB_t *next;

    prev = aHndl->prev;
    next = aHndl->next;
    if (prev) {
        prev->next = next;
    } else {
        devConsTop = next;
    }
    if (next) {
        next->prev = prev;
    }
}

void sdfDevConsClose(sdfDevConsCB_t *aHndl)
{
    sdfDevConsUnlink(aHndl);
    sdfMemFreeDirect(aHndl->bufferHandle);
    sdfChipFreeDirect(aHndl);
}

void sdfDevConsClearInside(sdfDevConsCB_t *aHndl)
{
    aHndl->locX = 0;
    aHndl->locY = 0;
    memset(aHndl->bufferPtr, 0, aHndl->w * aHndl->h * 2);
}

void sdfDevConsClear(sdfDevConsCB_t *aHndl)
{
    sdfDevConsClearInside(aHndl);
}

sdfDevConsCB_t *sdfDevConsOpen(s32 aX, s32 aY, s32 aW, s32 aH)
{
    sdfDevConsCB_t *hndl;

    sdfDevConsInit();
    hndl = sdfChipAllocDirect(sizeof(sdfDevConsCB_t));
    hndl->x = aX;
    hndl->y = aY;
    hndl->w = aW;
    hndl->h = aH;
    hndl->tabwidth = 8;
    hndl->mode = 0;
    hndl->attr = 0;
    hndl->bufferHandle = sdfMemAllocDirect(aW * aH * 2);
    hndl->bufferPtr = sdfMemLock(hndl->bufferHandle);
    sdfDevConsClearInside(hndl);
    sdfDevConsLinkToTop(hndl);
    return hndl;
}

void sdfDevConsNewLine(sdfDevConsCB_t *aHndl)
{
    s32 lineSize;
    s32 size;

    if (aHndl->locY == aHndl->h - 1) {
        lineSize = aHndl->w * 2;
        size = lineSize * aHndl->locY;
        if (aHndl->locY > 0) {
            memmove(aHndl->bufferPtr, aHndl->bufferPtr + lineSize, size);
        }
        memset(aHndl->bufferPtr + size, 0, lineSize);
    } else {
        aHndl->locY++;
    }
    aHndl->locX = 0;
}

void sdfDevConsPutInside(sdfDevConsCB_t *aHndl, s32 aChar, s32 aAttr)
{
    s32 cnt;
    s32 off;

    switch (aChar) {
    case '\t':
        off = (aHndl->locY * aHndl->w + aHndl->locX) * 2;
        cnt = aHndl->locX % aHndl->tabwidth;
        if (cnt == 0) {
            cnt = aHndl->tabwidth;
        }
        do {
            aHndl->bufferPtr[off] = 0;
            aHndl->bufferPtr[off + 1] = aAttr;
            off += 2;
            aHndl->locX++;
            if (aHndl->locX == aHndl->w) {
                sdfDevConsNewLine(aHndl);
                return;
            }
        } while (--cnt);
        return;
    case '\n':
        sdfDevConsNewLine(aHndl);
        return;
    case '\r':
        aHndl->locX = 0;
        return;
    }

    if (aChar == ' ') {
        aChar = 0;
    } else if (aChar < ' ' || aChar > 0x7F) {
        aChar = 0x7F;
    }
    off = (aHndl->locY * aHndl->w + aHndl->locX) * 2;
    aHndl->bufferPtr[off] = aChar;
    aHndl->bufferPtr[off + 1] = aAttr;
    aHndl->locX++;
    if (aHndl->locX == aHndl->w) {
        sdfDevConsNewLine(aHndl);
    }
}

void sdfDevConsPut(sdfDevConsCB_t *aHndl, s32 aChar, s32 aAttr)
{
    sdfDevConsPutInside(aHndl, aChar, aAttr);
}

s32 sdfDevConsPrintf(sdfDevConsCB_t *aHndl, const char *aFmt, ...)
{
    char buf[0x200];
    __builtin_va_list args;
    s32 count = 0;
    s32 ch;

    __builtin_stdarg_start(args, aFmt);
    vsprintf(buf, aFmt, args);
    ch = buf[0];
    if (ch != 0) {
        do {
            count++;
            sdfDevConsPutInside(aHndl, ch, aHndl->attr);
            ch = buf[count];
        } while (ch != 0);
    }
    return count;
}

void sdfDevConsLocate(sdfDevConsCB_t *aHndl, s32 aX, s32 aY)
{
    aHndl->locX = aX;
    aHndl->locY = aY;
}

void sdfDevConsAttribute(sdfDevConsCB_t *aHndl, s32 aAttr)
{
    aHndl->attr = aAttr;
}

s32 sdfDevConsGetMode(sdfDevConsCB_t *aHndl)
{
    return aHndl->mode;
}

void sdfDevConsSetMode(sdfDevConsCB_t *aHndl, s32 aMode)
{
    aHndl->mode = aMode;
}

void sdfDevConsTagClose(sdfDevConsBuf_t *aBuf)
{
    u8 *cur = aBuf->cur;
    volatile u64 *tag = (volatile u64 *)aBuf->tag;
    s32 regCount = ((cur - aBuf->tag) >> 3) - 2;

    if (regCount != 0) {
        if (regCount & 1) {
            cur += 8;
        }
        regCount /= 5;
        aBuf->tag = cur;
        aBuf->cur = cur + 0x10;
        tag[0] = (s64)regCount | 0x5400000000008000ULL;
        tag[1] = 0x53531;
    }
}

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevConsDrawFlush);

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevConsDrawBG);

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevConsDraw);

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevConsDrawInit);

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevConsDrawAll);

INCLUDE_ASM(const s32, "sdf/sdfDevCons", sdfDevVprintf);

void sdfDevPrintfEx(u32 *aCB, const char *aFmt, ...)
{
    __builtin_va_list args;

    __builtin_stdarg_start(args, aFmt);
    sdfDevVprintf(aCB, aFmt, args);
}

void sdfDevPrintf(u32 aPutFunc, u32 aFlushFunc, u32 aArg, s32 aMode, const char *aFmt, ...)
{
    u32 cb[4];
    __builtin_va_list args;

    sdfDevSetPrintCB(cb, aPutFunc, aFlushFunc, aArg, aMode);
    __builtin_stdarg_start(args, aFmt);
    sdfDevVprintf(cb, aFmt, args);
}

void func_002BD300(u32 *aCB, s32 aMode)
{
    aCB[3] = sdfDevConsColorTable[aMode];
}

void sdfDevSetPrintCB(u32 *aCB, u32 aPutFunc, u32 aFlushFunc, u32 aArg, s32 aMode)
{
    aCB[0] = aPutFunc;
    aCB[1] = aFlushFunc;
    aCB[2] = aArg;
    func_002BD300(aCB, aMode);
}

INCLUDE_SDATA(const s32, "sdf/sdfDevCons", devConsTop);

INCLUDE_SDATA(const s32, "sdf/sdfDevCons", alreadyInitDevCons);

