#include "common.h"
#include "libsdf.h"

extern void *sdfChipAllocDirect(s32 aSize);
extern void *sdfChipAllocZeroDirect(s32 aSize);
extern void sdfChipFreeDirect(void *aPtr);

extern sdfMemHandle_t *sdfMemAllocDirect(s32 aSize);
extern sdfMemHandle_t *sdfMemAllocFromTailDirect(s32 aSize);
extern void *sdfMemLock(sdfMemHandle_t *aHandle);
extern void sdfMemFreeHandleDirect(sdfMemHandle_t **aHandle);

extern sdfTexHandle_t *sdfTexSet(void *aData);
extern sdfTexHandle_t *sdfTexSetDirect(void *aData);
extern sdfItemArray_t *sdfTexListNew(s32 aCount);

extern void sdfDecompInit(struct sdfDecomp_t *aDecomp, u8 *aDst);
extern s32 sdfDecomp(struct sdfDecomp_t *aDecomp, const u8 *aSrc, s32 aLen);
extern void sdfAddressResolver(s32 *aBase, s32 aOffset, u8 *aTable, s32 aBytes);

s32 sdfPacStdCallback(sdfPacState_t *aState, s32 aStat, sdfPacPacketHeader_t *aPacket);
void sdfPacNewBlock(sdfPacState_t *aState);
void sdfPacEndBlock(sdfPacState_t *aState);
sdfPacWork_t *sdfPacNewResrc(sdfPacState_t *aState, const sdfPacPacketHeader_t *aPacket);
void sdfPacMemDecoderFunc(sdfPacState_t *aState);
void sdfPacMemDecompDecoderFunc(sdfPacState_t *aState);
void sdfPacMemStayDecoderFunc(sdfPacState_t *aState);
void sdfPacMemDecoderFuncInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);
void sdfPacMemDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);
void sdfPacModelDecoderEndFunc(sdfPacState_t *aState);
void sdfPacModelDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);
void sdfPacMotionDecoderEndFunc(sdfPacState_t *aState);
void sdfPacMotionDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);
void sdfPacTmxDecoderFunc(sdfPacState_t *aState);
void sdfPacTmxDecompDecoderFunc(sdfPacState_t *aState);
void sdfPacTmxStayDecoderFunc(sdfPacState_t *aState);
void sdfPacTmxDecoderInitFunc(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket,
                              sdfPacBuf_t *aBuf);
void sdfPacTmxDecoderEndFunc(sdfPacState_t *aState);
void sdfPacTmxDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);
void sdfPacTexListDecoderTmxBodyEndFunc(sdfPacState_t *aState);
void sdfPacTexListDecoder1(sdfPacState_t *aState);
void sdfPacTexListDecoder0(sdfPacState_t *aState);
void sdfPacTexListDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket);

void sdfPacInit(sdfPacState_t *aState, sdfPacCallbackFunc aCallback)
{
    memset(aState, 0, sizeof(*aState));
    aState->blockId = -1;
    if (aCallback != NULL) {
        aState->callback = aCallback;
    } else {
        aState->callback = sdfPacStdCallback;
    }
}

void sdfPacSetStay(sdfPacState_t *aState)
{
    aState->flags |= SDF_PAC_STATE_STAY;
}

void sdfPacSetFromTail(sdfPacState_t *aState)
{
    aState->flags |= SDF_PAC_STATE_FROMTAIL;
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacFree);

void func_002C60E0(sdfPacState_t *aState)
{
    aState->phase = -1;
}

void sdfPacUseSrc(sdfPacState_t *aState, s32 aLen)
{
    aState->src += aLen;
    aState->srcLeft -= aLen;
    aState->srcUsed += aLen;
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacNewBlock);

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacEndBlock);

s32 sdfPacDecode(sdfPacState_t *aState, const u8 *aSrc, s32 aLen)
{
    aState->src = aSrc;
    aState->srcLeft = aLen;
    while (aState->srcLeft > 0) {
        switch ((s8)aState->phase) {
        case -1:
            return -1;
        case 0:
            sdfPacNewBlock(aState);
            break;
        case 1: {
            s32 len = aState->dstLeft;

            if (aState->srcLeft < len) {
                len = aState->srcLeft;
            }
            sdfPacUseSrc(aState, len);
            aState->dstLeft -= len;
            if (aState->dstLeft == 0) {
                aState->phase = 0;
            }
            break;
        }
        case 2:
            aState->decodeFunc(aState);
            break;
        }
    }
    return (s8)aState->phase;
}

sdfPacWork_t *sdfPacNewResrc(sdfPacState_t *aState, const sdfPacPacketHeader_t *aPacket)
{
    s32 extra = aPacket->flags & SDF_PAC_EXTRA_MASK;
    sdfPacWork_t *work = sdfChipAllocZeroDirect(extra + sizeof(sdfPacWork_t));

    work->state = aState;
    memcpy(work->header, aPacket, extra + sizeof(*aPacket));
    if (aState->tail == NULL) {
        aState->head = work;
    } else {
        aState->tail->next = work;
    }
    aState->tail = work;
    return work;
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacFreeResrc);

s32 sdfPacStdCallback(sdfPacState_t *aState, s32 aStat, sdfPacPacketHeader_t *aPacket)
{
    if (aStat == 0) {
        switch (aPacket->command) {
        case SDF_PAC_COMMAND_MEM:
            sdfPacMemDecoderInit(aState, aPacket);
            return 0;
        case SDF_PAC_COMMAND_TMX:
            sdfPacTmxDecoderInit(aState, aPacket);
            return 0;
        case SDF_PAC_COMMAND_MODEL:
            sdfPacModelDecoderInit(aState, aPacket);
            return 0;
        case SDF_PAC_COMMAND_MOTION:
            sdfPacMotionDecoderInit(aState, aPacket);
            return 0;
        case SDF_PAC_COMMAND_TEXLIST:
            sdfPacTexListDecoderInit(aState, aPacket);
            return 0;
        case SDF_PAC_COMMAND_END:
            return 1;
        default:
            return 3;
        }
    }
    return 0;
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacGetResrcFileName);

void sdfPacMemDecoderFunc(sdfPacState_t *aState)
{
    s32 len = aState->dstLeft;

    if (aState->srcLeft < len) {
        len = aState->srcLeft;
    }
    if (len != 0) {
        memcpy(aState->dst, aState->src, len);
        sdfPacUseSrc(aState, len);
        aState->dst += len;
        aState->dstLeft -= len;
        if (aState->dstLeft == 0) {
            aState->endFunc(aState);
        }
    }
}

void sdfPacMemDecompDecoderFunc(sdfPacState_t *aState)
{
    s32 len = aState->srcLeft;
    s32 done = sdfDecomp(aState->decomp, aState->src, len);

    sdfPacUseSrc(aState, len - aState->decomp->remain);
    if (done != 0) {
        sdfChipFreeDirect(aState->decomp);
        aState->endFunc(aState);
    }
}

void sdfPacMemStayDecoderFunc(sdfPacState_t *aState)
{
    s32 len = aState->dstLeft;

    if (aState->srcLeft < len) {
        len = aState->srcLeft;
    }
    if (len != 0) {
        sdfPacUseSrc(aState, len);
        aState->dstLeft -= len;
        if (aState->dstLeft == 0) {
            aState->endFunc(aState);
        }
    }
}

void sdfPacMemDecoderFuncInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    s32 size = aPacket->decodedSize;
    sdfPacWork_t *work;

    if (size == 0) {
        size = aPacket->size + (aPacket->flags & SDF_PAC_EXTRA_MASK) - sizeof(*aPacket);
    }
    if (aState->flags & SDF_PAC_STATE_STAY) {
        work = sdfPacNewResrc(aState, aPacket);
        work->data = (u8 *)(aPacket + 1);
        aState->decodeFunc = sdfPacMemStayDecoderFunc;
        return;
    }
    aState->phase = 2;
    work = sdfPacNewResrc(aState, aPacket);
    if (aState->flags & SDF_PAC_STATE_FROMTAIL) {
        work->resrc = sdfMemAllocFromTailDirect(size);
    } else {
        work->resrc = sdfMemAllocDirect(size);
    }
    work->data = sdfMemLock(work->resrc);
    aState->dst = work->data;
    switch (aPacket->flags & SDF_PAC_ENCODING_MASK) {
    case 0:
        aState->decodeFunc = sdfPacMemDecoderFunc;
        break;
    case 1: {
        struct sdfDecomp_t *decomp = sdfChipAllocDirect(sizeof(*decomp));

        aState->decomp = decomp;
        sdfDecompInit(decomp, aState->dst);
        aState->decodeFunc = sdfPacMemDecompDecoderFunc;
        break;
    }
    }
}

void sdfPacMemDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    sdfPacMemDecoderFuncInit(aState, aPacket);
    aState->endFunc = sdfPacEndBlock;
}

void sdfPacModelDecoderEndFunc(sdfPacState_t *aState)
{
    sdfPacWork_t *work = aState->tail;
    sdfPacReloc_t *reloc = (sdfPacReloc_t *)work->data;
    u8 *payload = reloc->payload;
    s32 bytes = reloc->tableBytes;

    work->data = payload;
    if (bytes != 0) {
        sdfAddressResolver((s32 *)payload, (s32)payload, payload + reloc->tableOffset, bytes);
        reloc->tableBytes = 0;
    }
    sdfPacEndBlock(aState);
}

void sdfPacModelDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    sdfPacMemDecoderFuncInit(aState, aPacket);
    aState->endFunc = sdfPacModelDecoderEndFunc;
}

void sdfPacMotionDecoderEndFunc(sdfPacState_t *aState)
{
    sdfPacWork_t *work = aState->tail;
    sdfPacReloc_t *reloc = (sdfPacReloc_t *)work->data;
    u8 *payload = reloc->payload;
    s32 bytes = reloc->tableBytes;

    work->data = payload;
    if (bytes != 0) {
        sdfAddressResolver((s32 *)payload, (s32)payload, payload + reloc->tableOffset, bytes);
        reloc->tableBytes = 0;
    }
    sdfPacEndBlock(aState);
}

void sdfPacMotionDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    sdfPacMemDecoderFuncInit(aState, aPacket);
    aState->endFunc = sdfPacMotionDecoderEndFunc;
}

void sdfPacTmxDecoderFunc(sdfPacState_t *aState)
{
    sdfPacBuf_t *buf = aState->buf;
    s32 len = buf->remain;

    if (aState->srcLeft < len) {
        len = aState->srcLeft;
    }
    if (len != 0) {
        memcpy(buf->cursor, aState->src, len);
        sdfPacUseSrc(aState, len);
        buf->cursor += len;
        buf->remain -= len;
        if (buf->remain == 0) {
            buf->tex = sdfTexSet(sdfMemLock(buf->memHandle));
            sdfMemFreeHandleDirect(&buf->memHandle);
            aState->endFunc(aState);
        }
    }
}

void sdfPacTmxDecompDecoderFunc(sdfPacState_t *aState)
{
    s32 len = aState->dstLeft;
    s32 done = sdfDecomp(aState->decomp, aState->src, len);

    sdfPacUseSrc(aState, len - aState->decomp->remain);
    if (done != 0) {
        sdfPacBuf_t *buf = aState->buf;

        buf->tex = sdfTexSet(sdfMemLock(buf->memHandle));
        sdfMemFreeHandleDirect(&buf->memHandle);
        sdfChipFreeDirect(aState->decomp);
        aState->endFunc(aState);
    }
}

void sdfPacTmxStayDecoderFunc(sdfPacState_t *aState)
{
    sdfPacBuf_t *buf = aState->buf;
    s32 len = buf->remain;

    if (aState->srcLeft < len) {
        len = aState->srcLeft;
    }
    if (len != 0) {
        sdfPacUseSrc(aState, len);
        buf->remain -= len;
        if (buf->remain == 0) {
            buf->tex = sdfTexSetDirect(buf->cursor);
            aState->endFunc(aState);
        }
    }
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacTmxDecoderInitFunc);

void sdfPacTmxDecoderEndFunc(sdfPacState_t *aState)
{
    aState->tail->resrc = aState->slot.buf->tex;
    sdfChipFreeDirect(aState->slot.buf);
    sdfPacEndBlock(aState);
}

void sdfPacTmxDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    sdfPacBuf_t *buf;

    sdfPacNewResrc(aState, aPacket);
    buf = sdfChipAllocDirect(sizeof(*buf));
    aState->slot.buf = buf;
    sdfPacTmxDecoderInitFunc(aState, aPacket, buf);
    aState->endFunc = sdfPacTmxDecoderEndFunc;
}

void sdfPacTexListDecoderTmxHeaderEndFunc(sdfPacState_t *aState)
{
    sdfPacTmxDecoderInitFunc(aState, &aState->slot.list->header, &aState->slot.list->buf);
    aState->endFunc = sdfPacTexListDecoderTmxBodyEndFunc;
}

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", func_002C6D58);

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacTexListDecoderTmxBodyEndFunc);

INCLUDE_ASM(const s32, "sdf/sdfPacDecode", sdfPacTexListDecoder1);

void sdfPacTexListDecoder0(sdfPacState_t *aState)
{
    u32 count;
    sdfPacList_t *list;
    sdfPacWork_t *work;

    if (aState->srcLeft != 0) {
        count = *(const u32 *)aState->src;
        sdfPacUseSrc(aState, 4);
        list = aState->slot.list;
        work = aState->tail;
        list->count = count;
        list->index = 0;
        work->resrc = sdfTexListNew(count);
        aState->dstLeft = ((count * 4 + 0x53) & -0x40) - 0x14;
        aState->decodeFunc = sdfPacTexListDecoder1;
    }
}

void sdfPacTexListDecoderInit(sdfPacState_t *aState, sdfPacPacketHeader_t *aPacket)
{
    sdfPacNewResrc(aState, aPacket);
    aState->slot.list = sdfChipAllocDirect(sizeof(sdfPacList_t));
    aState->decodeFunc = sdfPacTexListDecoder0;
}
