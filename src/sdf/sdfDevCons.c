#include "common.h"
#include "libsdf.h"

#include "ps2asm.h"
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

/* SMT3 DrawAll and the DDS1 twin reserve this same 0x1040-byte stack
 * allocation: only the central 0x1000 bytes are passed as DMA storage. */
typedef struct
{
    u8 opaquePrefix[0x30];
    u8 data[0x1000];
    u8 opaqueSuffix[0x10];
} sdfDevConsStackPackets_t;

/* The four print-control words are signed XYZ and packed RGBA when drawing. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u32 color;
} sdfDevPrintCB_t;

typedef struct
{
    u16 quadwordCount;
    u8 reserved02;
    u8 tagKind;
    u32 nextAddress;
    u32 vifNop;
    u32 vifDirect;
    u64 setupTag;
    u64 setupRegisters;
    u64 textureClamp;
    u64 textureClampRegister;
    u64 textureFilter;
    u64 textureFilterRegister;
    u64 textureState;
    u64 textureStateRegister;
    u64 glyphTag;
    u64 glyphRegisters;
} sdfDevConsPacketHeader_t;

typedef struct
{
    u64 color;
    u64 firstUv;
    u64 firstPosition;
    u64 secondUv;
    u64 secondPosition;
} sdfDevConsGlyph_t;

extern s32 sceDmaSync(void *channel, s32 mode, s32 timeout);
extern void sceDmaSendN(void *channel, void *address, s32 quadwords);
extern s32 sceGsSyncPath(s32 mode, s32 timeout);
extern void *sceDmaGetChan(s32 channel);
extern u64 sdfTexGetTex0Reg(sdfTexHandle_t *texture);
extern void *sdfGrOpenPacketBuff(void);
extern void sdfGrClosePacketBuff(s32 cursor);
void sdfDevConsDrawBG(sdfDevConsCB_t *console, sdfDevConsBuf_t *buffers);

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
extern u32 sdfDevConsFontUvTable[];

extern sdfDevConsCB_t *devConsTop;
extern u32 sdfDevConsColorTable[];

extern void *sdfDevVprintf(u32 *aCB, const char *aFmt, void *aArgs);
void sdfDevSetPrintCB(u32 *aCB, u32 aPutFunc, u32 aFlushFunc, u32 aArg, s32 aMode);

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
    return (s16 *)sdfDevConsFontUvTable;
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

void sdfDevConsDrawFlush(sdfDevConsBuf_t *packetBuffers) {
    s32 packetBytes;
    u32 *channelRegisters;
    u32 nextBufferIndex;

    sdfDevConsTagClose(packetBuffers);
    packetBytes = packetBuffers->tag - packetBuffers->start;
    if ((packetBytes >> 3) != 0) {
        channelRegisters = packetBuffers->dmaCh;
        sceDmaSync(channelRegisters, 0, 0);
        *(volatile u32 *)channelRegisters = *(volatile u32 *)channelRegisters | 0x40; /* CHCR bit 6: tag-transfer enable */
        EE_SYNC_BARRIER();
        sceDmaSendN(channelRegisters, (void *)((u32)packetBuffers->start & 0x0FFFFFFF), packetBytes >> 4);
        nextBufferIndex = packetBuffers->bufIdx ^ 1;
        packetBuffers->bufIdx = nextBufferIndex;
        packetBuffers->tag = (u8 *)packetBuffers->buf[nextBufferIndex];
        packetBuffers->cur = packetBuffers->tag + 0x10;
        packetBuffers->start = packetBuffers->tag;
        sceGsSyncPath(0, 0);
    }
}

void sdfDevConsDrawBG(sdfDevConsCB_t *console, sdfDevConsBuf_t *packetBuffers) {
    vu64 *packet;
    u32 origin;
    u32 extent;

    sdfDevConsTagClose(packetBuffers);
    if (packetBuffers->bufSize < packetBuffers->tag - packetBuffers->start + 0x80) {
        sdfDevConsDrawFlush(packetBuffers);
    }
    packet = (vu64 *)packetBuffers->tag;
    origin = (u16)(console->x - 0x30) | ((u32)console->y << 16);
    extent = (u16)(console->w * 3 * 0x40 + 0x70) | ((u32)(console->h * packetBuffers->rowStep) << 16);
    if (console->mode & 4) {
        origin += 0xFFD00000;
        extent += 0x700000;
    } else {
        origin += 0xFFE80000;
        extent += 0x380000;
    }
    packet[0] = 0x5400000000008001ULL;
    packet[1] = 0x5510;
    packet[2] = 0x146;
    packet[3] = 0x30000000;
    packet[4] = origin;
    packet[5] = origin + extent;
    packet[6] = 0x156;
    packetBuffers->tag = (u8 *)packet + 0x40;
    packetBuffers->cur = packetBuffers->tag + 0x10;
}

void sdfDevConsDraw(sdfDevConsCB_t *console, sdfDevConsBuf_t *buffers) {
    u8 *cells;
    vu64 *writeCursor;
    u8 *packetStart;
    s32 row;
    s32 column;
    s32 y;
    s32 cellStep;
    u64 top;
    s32 character;
    s32 attribute;
    u64 glyph;
    u64 color;

    if ((u8)(console->mode & 1)) {
        return;
    }
    buffers->rowStep = (console->mode & 4) ? 0xC0 : 0x60;
    if (console->mode & 2) {
        sdfDevConsDrawBG(console, buffers);
    }
    cellStep = ((u32)buffers->rowStep << 16) | 0xC0;
    row = 0;
    writeCursor = (vu64 *)buffers->cur;
    packetStart = buffers->start;
    cells = console->bufferPtr;
    y = console->y;
    do {
        top = (u32)console->x | ((u32)y << 16);
        column = 0;
        do {
            character = cells[0];
            attribute = cells[1];
            cells += 2;
            if (character >= 0x20) {
                if (buffers->bufSize < (u8 *)writeCursor - packetStart + 0x50) {
                    buffers->cur = (u8 *)writeCursor;
                    sdfDevConsDrawFlush(buffers);
                    writeCursor = (vu64 *)buffers->cur;
                    packetStart = buffers->start;
                }
                glyph = sdfDevConsFontUvTable[character - 0x20];
                color = sdfDevConsColorTable[attribute];
                writeCursor[0] = color;
                writeCursor[1] = glyph;
                writeCursor[2] = top;
                writeCursor[3] = glyph + 0xC000C0;
                writeCursor[4] = top + cellStep;
                writeCursor += 5;
            }
            column++;
            top += 0xC0;
        } while (column != console->w);
        row++;
        y += buffers->rowStep;
    } while (row != console->h);
    buffers->cur = (u8 *)writeCursor;
}

void sdfDevConsDrawInit(sdfDevConsBuf_t *packetBuffers, void *storageBase, s32 storageCapacityBytes) {
    u32 ringAddress;
    u8 *ring;
    vu64 *packet;
    u64 textureState;
    s32 halfBufferBytes = storageCapacityBytes >> 1;

    ringAddress = ((u32)storageBase & 0x0FFFFFFF) | 0x30000000;
    ring = (u8 *)ringAddress;
    packetBuffers->bufSize = halfBufferBytes;
    packetBuffers->buf[0] = ring;
    packetBuffers->buf[1] = ring + halfBufferBytes;
    packetBuffers->start = ring;
    packetBuffers->bufIdx = 0;
    textureState = sdfTexGetTex0Reg(sdfDebugFontHandle);
    packet = (vu64 *)ring;
    packet[0] = 0x10AB400000008005ULL;
    packet[1] = 0xE;
    packet[2] = 0x44;
    packet[3] = 0x42;
    packet[4] = 0x3000D;
    packet[5] = 0x47;
    packet[6] = 0;
    packet[7] = 8;
    packet[8] = 1;
    packet[9] = 0x14;
    packet[10] = textureState;
    packet[11] = 6;
    packetBuffers->tag = ring + 0x60;
    packetBuffers->cur = packetBuffers->tag + 0x10;
    packetBuffers->dmaCh = sceDmaGetChan(2);
}

void sdfDevConsDrawAll(void) {
    sdfDevConsStackPackets_t storage;
    sdfDevConsBuf_t packetBuffers;
    sdfDevConsCB_t *console;

    if (devConsTop != NULL) {
        sdfDevConsDrawInit(&packetBuffers, storage.data, 0x1000);
        for (console = devConsTop; console != NULL; console = console->next) {
            sdfDevConsDraw(console, &packetBuffers);
        }
        sdfDevConsDrawFlush(&packetBuffers);
    }
}

void *sdfDevVprintf(u32 *inputWords, const char *format, void *arguments) {
    sdfDevPrintCB_t *input = (sdfDevPrintCB_t *)inputWords;
    char text[0x200];
    s32 formattedCount;
    u8 *start;
    u8 *cursor;
    sdfDevConsPacketHeader_t *header;
    u8 *textCursor;
    s32 character;
    s32 glyphCount;
    u32 color;
    u64 position;
    s32 quadwordCount;

    sdfDevConsInit();
    formattedCount = vsprintf(text, format, arguments);
    start = (u8 *)sdfGrOpenPacketBuff();
    if (formattedCount == 0) {
        *(u128 *)start = 0;
        cursor = start + 0x10;
    } else {
        header = (sdfDevConsPacketHeader_t *)start;
        cursor = start + sizeof(*header);
        glyphCount = 0;
        /* This dual-use command carries x, y, depth and RGBA in its four words. */
        position = (u32)((u16)input->x | ((u32)(u16)input->y << 16)) |
                   ((u64)(u32)input->z << 32);
        color = input->color;
        textCursor = (u8 *)text;
        while ((character = *textCursor++) != 0) {
            if (character >= 0x20) {
                sdfDevConsGlyph_t *glyph = (sdfDevConsGlyph_t *)cursor;
                u32 uv = sdfDevConsFontUvTable[character - 0x20];

                glyph->color = color;
                glyph->firstUv = uv;
                glyph->firstPosition = position;
                glyph->secondUv = (u32)(uv + 0x00C000C0);
                glyph->secondPosition = position + 0x006000C0;
                cursor += sizeof(*glyph);
                glyphCount++;
            }
            /* Control bytes consume the same horizontal advance as glyphs. */
            position += 0xC0;
        }
        if (glyphCount & 1) {
            *(u64 *)cursor = 0;
            cursor += sizeof(u64);
        }
        quadwordCount = ((s32)(cursor - start) >> 4) - 1;
        input->x = (u16)position;
        header->quadwordCount = quadwordCount;
        header->vifNop = 0;
        header->vifDirect = 0x50000000 | quadwordCount;
        header->setupTag = 0x10AB400000000003ULL;
        header->setupRegisters = 0xEEE;
        header->textureClamp = 0;
        header->textureClampRegister = 8;
        header->textureFilter = 1;
        header->textureFilterRegister = 0x14;
        header->textureState = sdfTexGetTex0Reg(sdfDebugFontHandle);
        header->textureStateRegister = 6;
        header->glyphTag = 0x5400000000008000ULL | glyphCount;
        header->glyphRegisters = 0x53531;
    }
    sdfGrClosePacketBuff((s32)cursor);
    return start;
}

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

