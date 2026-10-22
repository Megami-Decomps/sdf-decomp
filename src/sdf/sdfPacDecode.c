#include "common.h"
#include "libsdf.h"

extern void *sdfChipAllocDirect(s32 aSize);
extern void *sdfChipAllocZeroDirect(s32 aSize);
extern void sdfChipFreeDirect(void *aPtr);

extern sdfMemHandle_t *sdfMemAllocDirect(s32 aSize);
extern sdfMemHandle_t *sdfMemAllocFromTailDirect(s32 aSize);
extern void *sdfMemLock(sdfMemHandle_t *aHandle);
extern void sdfMemFreeHandleDirect(sdfMemHandle_t **aHandlePtr);

extern sdfTexHandle_t *sdfTexSet(void *aTex);
extern sdfTexHandle_t *sdfTexSetDirect(void *aTex);
extern sdfItemArray_t *sdfTexListNew(s32 aInitCapa);

extern void sdfDecompInit(struct sdfDecomp_t *cb, u8 *dst);
extern s32 sdfDecomp(struct sdfDecomp_t *cb, const u8 *src, s32 length);
extern void sdfAddressResolver(s32 *aDataBody, s32 aOffset, u8 *aAddrInfoPtr, s32 aAddrInfoLeng);

s32 sdfPacStdCallback(sdfPacState_t *cb, s32 cause, sdfPacPacketHeader_t *hdr);
void sdfPacNewBlock(sdfPacState_t *cb);
void sdfPacEndBlock(sdfPacState_t *cb);
sdfPacWork_t *sdfPacNewResrc(sdfPacState_t *cb, const sdfPacPacketHeader_t *hdr);
void sdfPacMemDecoderFunc(sdfPacState_t *cb);
void sdfPacMemDecompDecoderFunc(sdfPacState_t *cb);
void sdfPacMemStayDecoderFunc(sdfPacState_t *cb);
void sdfPacMemDecoderFuncInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);
void sdfPacMemDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);
void sdfPacModelDecoderEndFunc(sdfPacState_t *cb);
void sdfPacModelDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);
void sdfPacMotionDecoderEndFunc(sdfPacState_t *cb);
void sdfPacMotionDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);
void sdfPacTmxDecoderFunc(sdfPacState_t *cb);
void sdfPacTmxDecompDecoderFunc(sdfPacState_t *cb);
void sdfPacTmxStayDecoderFunc(sdfPacState_t *cb);
void sdfPacTmxDecoderInitFunc(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr,
                              sdfPacBuf_t *tmxCb);
void sdfPacTmxDecoderEndFunc(sdfPacState_t *cb);
void sdfPacTmxDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);
void sdfPacTexListDecoderTmxBodyEndFunc(sdfPacState_t *cb);
void sdfPacTexListDecoder1(sdfPacState_t *cb);
void sdfPacTexListDecoder0(sdfPacState_t *cb);
void sdfPacTexListDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr);

void func_002C60E0(sdfPacState_t *aState);
void func_002C6D58(sdfPacState_t *aState);
void sdfPacTexListDecoderTmxHeaderEndFunc(sdfPacState_t *cb);
extern void sdfTexListAdd(sdfItemArray_t *aList, sdfTexHandle_t *aTexHandle);

void sdfPacInit(sdfPacState_t *cb, sdfPacCallbackFunc callback)
{
    memset(cb, 0, sizeof(*cb));
    cb->blockId = -1;
    if (callback != NULL) {
        cb->callback = callback;
    } else {
        cb->callback = sdfPacStdCallback;
    }
}

void sdfPacSetStay(sdfPacState_t *cb)
{
    cb->flags |= SDF_PAC_STATE_STAY;
}

void sdfPacSetFromTail(sdfPacState_t *cb)
{
    cb->flags |= SDF_PAC_STATE_FROMTAIL;
}

void sdfPacFree(sdfPacState_t *cb)
{
    sdfPacWork_t *cursor = cb->head;
    sdfPacWork_t *current = cursor;

    while (cursor != NULL) {
        cursor = cursor->next;
        sdfChipFreeDirect(current);
        current = cursor;
    }
}

void func_002C60E0(sdfPacState_t *aState)
{
    aState->phase = -1;
}

void sdfPacUseSrc(sdfPacState_t *cb, s32 size)
{
    cb->src += size;
    cb->srcLeft -= size;
    cb->srcUsed += size;
}

void sdfPacNewBlock(sdfPacState_t *cb)
{
    sdfPacPacketHeader_t *packet;
    u32 alignmentOffset;
    u32 packetBytes;
    u32 headerBytes;
    u32 payloadBytes;
    s32 (*callback)();
    s32 result;

    alignmentOffset = cb->srcUsed & 0x3F;
    if (alignmentOffset != 0) {
        cb->phase = 1;
        cb->dstLeft = 0x40 - alignmentOffset;
        return;
    }
    packet = (sdfPacPacketHeader_t *)cb->src;
    sdfPacUseSrc(cb, 0x10);
    packetBytes = packet->size;
    cb->blockId++;
    headerBytes = (packet->flags & SDF_PAC_EXTRA_MASK) + 0x10;
    payloadBytes = packetBytes - headerBytes;
    cb->dstLeft = payloadBytes;
    callback = (s32 (*)())cb->callback;
    result = callback(cb, 0, packet, payloadBytes);
    if (result == 1) {
        func_002C60E0(cb);
        return;
    }
    if (result == 2) {
        cb->phase = 1;
        cb->dstLeft = (cb->dstLeft + 0x3F) & ~0x3F;
    } else {
        cb->phase = 2;
    }
}

void sdfPacEndBlock(sdfPacState_t *cb)
{
    s32 result;
    sdfPacWork_t *tail;
    sdfPacWork_t *scan;
    sdfPacWork_t *next;

    result = cb->callback(cb, 1, (sdfPacPacketHeader_t *)cb->tail->header);
    if (result == 1) {
        func_002C60E0(cb);
        return;
    }
    if (result == 4) {
        tail = cb->tail;
        scan = cb->head;
        if (scan == tail) {
            cb->tail = NULL;
            cb->head = NULL;
        } else {
            while ((next = scan->next) != tail) {
                scan = next;
            }
            /* Retail clears the link but leaves the multi-item tail unchanged. */
            scan->next = NULL;
        }
        sdfChipFreeDirect(tail);
    }
    cb->phase = 0;
}

s32 sdfPacDecode(sdfPacState_t *cb, const u8 *data, s32 length)
{
    cb->src = data;
    cb->srcLeft = length;
    while (cb->srcLeft > 0) {
        switch ((s8)cb->phase) {
        case -1:
            return -1;
        case 0:
            sdfPacNewBlock(cb);
            break;
        case 1: {
            s32 len = cb->dstLeft;

            if (cb->srcLeft < len) {
                len = cb->srcLeft;
            }
            sdfPacUseSrc(cb, len);
            cb->dstLeft -= len;
            if (cb->dstLeft == 0) {
                cb->phase = 0;
            }
            break;
        }
        case 2:
            cb->decodeFunc(cb);
            break;
        }
    }
    return (s8)cb->phase;
}

sdfPacWork_t *sdfPacNewResrc(sdfPacState_t *cb, const sdfPacPacketHeader_t *hdr)
{
    s32 extra = hdr->flags & SDF_PAC_EXTRA_MASK;
    sdfPacWork_t *aWork = sdfChipAllocZeroDirect(extra + sizeof(sdfPacWork_t));

    aWork->state = cb;
    memcpy(aWork->header, hdr, extra + sizeof(*hdr));
    if (cb->tail == NULL) {
        cb->head = aWork;
    } else {
        cb->tail->next = aWork;
    }
    cb->tail = aWork;
    return aWork;
}

sdfPacWork_t *sdfPacFreeResrc(sdfPacWork_t *resrc)
{
    sdfPacState_t *aState = resrc->state;
    sdfPacWork_t **link = &aState->head;
    sdfPacWork_t *current = aState->head;
    sdfPacWork_t *previous = NULL;
    sdfPacWork_t *next;

    if (current != resrc) {
        do {
            previous = current;
            current = previous->next;
            link = &previous->next;
        } while (current != resrc);
    }
    next = resrc->next;
    *link = next;
    if (aState->tail == resrc) {
        aState->tail = previous;
    }
    sdfChipFreeDirect(resrc);
    return next;
}

s32 sdfPacStdCallback(sdfPacState_t *cb, s32 cause, sdfPacPacketHeader_t *hdr)
{
    if (cause == 0) {
        switch (hdr->command) {
        case DDS3_FT_EFFECT:
            sdfPacMemDecoderInit(cb, hdr);
            return 0;
        case DDS3_FT_TMX:
            sdfPacTmxDecoderInit(cb, hdr);
            return 0;
        case DDS3_FT_MODEL:
            sdfPacModelDecoderInit(cb, hdr);
            return 0;
        case DDS3_FT_MOTION:
            sdfPacMotionDecoderInit(cb, hdr);
            return 0;
        case DDS3_FT_TEXLIST:
            sdfPacTexListDecoderInit(cb, hdr);
            return 0;
        case DDS3_FT_END:
            return 1;
        default:
            return 3;
        }
    }
    return 0;
}

char *sdfPacGetResrcFileName(sdfPacWork_t *resrc)
{
    sdfPacPacketHeader_t *packet = (sdfPacPacketHeader_t *)resrc->header;
    s32 extra = packet->flags & SDF_PAC_EXTRA_MASK;

    if (extra <= 0) {
        return NULL;
    }
    return (char *)(packet + 1);
}

void sdfPacMemDecoderFunc(sdfPacState_t *cb)
{
    s32 len = cb->dstLeft;

    if (cb->srcLeft < len) {
        len = cb->srcLeft;
    }
    if (len != 0) {
        memcpy(cb->dst, cb->src, len);
        sdfPacUseSrc(cb, len);
        cb->dst += len;
        cb->dstLeft -= len;
        if (cb->dstLeft == 0) {
            cb->endFunc(cb);
        }
    }
}

void sdfPacMemDecompDecoderFunc(sdfPacState_t *cb)
{
    s32 len = cb->srcLeft;
    s32 done = sdfDecomp(cb->decomp, cb->src, len);

    sdfPacUseSrc(cb, len - cb->decomp->remain);
    if (done != 0) {
        sdfChipFreeDirect(cb->decomp);
        cb->endFunc(cb);
    }
}

void sdfPacMemStayDecoderFunc(sdfPacState_t *cb)
{
    s32 len = cb->dstLeft;

    if (cb->srcLeft < len) {
        len = cb->srcLeft;
    }
    if (len != 0) {
        sdfPacUseSrc(cb, len);
        cb->dstLeft -= len;
        if (cb->dstLeft == 0) {
            cb->endFunc(cb);
        }
    }
}

void sdfPacMemDecoderFuncInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    s32 size = hdr->decodedSize;
    sdfPacWork_t *aWork;

    if (size == 0) {
        size = hdr->size + (hdr->flags & SDF_PAC_EXTRA_MASK) - sizeof(*hdr);
    }
    if (cb->flags & SDF_PAC_STATE_STAY) {
        aWork = sdfPacNewResrc(cb, hdr);
        aWork->data = (u8 *)(hdr + 1);
        cb->decodeFunc = sdfPacMemStayDecoderFunc;
        return;
    }
    cb->phase = 2;
    aWork = sdfPacNewResrc(cb, hdr);
    if (cb->flags & SDF_PAC_STATE_FROMTAIL) {
        aWork->resrc = sdfMemAllocFromTailDirect(size);
    } else {
        aWork->resrc = sdfMemAllocDirect(size);
    }
    aWork->data = sdfMemLock(aWork->resrc);
    cb->dst = aWork->data;
    switch (hdr->flags & SDF_PAC_ENCODING_MASK) {
    case 0:
        cb->decodeFunc = sdfPacMemDecoderFunc;
        break;
    case 1: {
        struct sdfDecomp_t *decomp = sdfChipAllocDirect(sizeof(*decomp));

        cb->decomp = decomp;
        sdfDecompInit(decomp, cb->dst);
        cb->decodeFunc = sdfPacMemDecompDecoderFunc;
        break;
    }
    }
}

void sdfPacMemDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    sdfPacMemDecoderFuncInit(cb, hdr);
    cb->endFunc = sdfPacEndBlock;
}

void sdfPacModelDecoderEndFunc(sdfPacState_t *cb)
{
    sdfPacWork_t *aWork = cb->tail;
    sdfPacReloc_t *reloc = (sdfPacReloc_t *)aWork->data;
    u8 *payload = reloc->payload;
    s32 bytes = reloc->tableBytes;

    aWork->data = payload;
    if (bytes != 0) {
        sdfAddressResolver((s32 *)payload, (s32)payload, payload + reloc->tableOffset, bytes);
        reloc->tableBytes = 0;
    }
    sdfPacEndBlock(cb);
}

void sdfPacModelDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    sdfPacMemDecoderFuncInit(cb, hdr);
    cb->endFunc = sdfPacModelDecoderEndFunc;
}

void sdfPacMotionDecoderEndFunc(sdfPacState_t *cb)
{
    sdfPacWork_t *aWork = cb->tail;
    sdfPacReloc_t *reloc = (sdfPacReloc_t *)aWork->data;
    u8 *payload = reloc->payload;
    s32 bytes = reloc->tableBytes;

    aWork->data = payload;
    if (bytes != 0) {
        sdfAddressResolver((s32 *)payload, (s32)payload, payload + reloc->tableOffset, bytes);
        reloc->tableBytes = 0;
    }
    sdfPacEndBlock(cb);
}

void sdfPacMotionDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    sdfPacMemDecoderFuncInit(cb, hdr);
    cb->endFunc = sdfPacMotionDecoderEndFunc;
}

void sdfPacTmxDecoderFunc(sdfPacState_t *cb)
{
    sdfPacBuf_t *buf = cb->buf;
    s32 len = buf->remain;

    if (cb->srcLeft < len) {
        len = cb->srcLeft;
    }
    if (len != 0) {
        memcpy(buf->cursor, cb->src, len);
        sdfPacUseSrc(cb, len);
        buf->cursor += len;
        buf->remain -= len;
        if (buf->remain == 0) {
            buf->tex = sdfTexSet(sdfMemLock(buf->memHandle));
            sdfMemFreeHandleDirect(&buf->memHandle);
            cb->endFunc(cb);
        }
    }
}

void sdfPacTmxDecompDecoderFunc(sdfPacState_t *cb)
{
    s32 len = cb->dstLeft;
    s32 done = sdfDecomp(cb->decomp, cb->src, len);

    sdfPacUseSrc(cb, len - cb->decomp->remain);
    if (done != 0) {
        sdfPacBuf_t *buf = cb->buf;

        buf->tex = sdfTexSet(sdfMemLock(buf->memHandle));
        sdfMemFreeHandleDirect(&buf->memHandle);
        sdfChipFreeDirect(cb->decomp);
        cb->endFunc(cb);
    }
}

void sdfPacTmxStayDecoderFunc(sdfPacState_t *cb)
{
    sdfPacBuf_t *buf = cb->buf;
    s32 len = buf->remain;

    if (cb->srcLeft < len) {
        len = cb->srcLeft;
    }
    if (len != 0) {
        sdfPacUseSrc(cb, len);
        buf->remain -= len;
        if (buf->remain == 0) {
            buf->tex = sdfTexSetDirect(buf->cursor);
            cb->endFunc(cb);
        }
    }
}

void sdfPacTmxDecoderInitFunc(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr,
                             sdfPacBuf_t *tmxCb)
{
    cb->buf = tmxCb;
    tmxCb->remain = hdr->size - 0x10;
    if (cb->flags & SDF_PAC_STATE_STAY) {
        cb->decodeFunc = sdfPacTmxStayDecoderFunc;
        tmxCb->cursor = (u8 *)(cb->src - 0x10);
        return;
    }
    if (cb->flags & SDF_PAC_STATE_FROMTAIL) {
        tmxCb->memHandle = sdfMemAllocFromTailDirect(hdr->size);
    } else {
        tmxCb->memHandle = sdfMemAllocDirect(hdr->size);
    }
    tmxCb->cursor = sdfMemLock(tmxCb->memHandle);
    memcpy(tmxCb->cursor, hdr, 0x10);
    tmxCb->cursor += 0x10;
    switch (hdr->flags) {
    case 0:
        cb->decodeFunc = sdfPacTmxDecoderFunc;
        break;
    case 1: {
        struct sdfDecomp_t *decomp = sdfChipAllocDirect(sizeof(*decomp));

        cb->decompBuffer = (u8 *)decomp;
        sdfDecompInit(decomp, tmxCb->cursor);
        cb->decodeFunc = sdfPacTmxDecompDecoderFunc;
        break;
    }
    }
}

void sdfPacTmxDecoderEndFunc(sdfPacState_t *cb)
{
    cb->tail->resrc = cb->slot.buf->tex;
    sdfChipFreeDirect(cb->slot.buf);
    sdfPacEndBlock(cb);
}

void sdfPacTmxDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    sdfPacBuf_t *buf;

    sdfPacNewResrc(cb, hdr);
    buf = sdfChipAllocDirect(sizeof(*buf));
    cb->slot.buf = buf;
    sdfPacTmxDecoderInitFunc(cb, hdr, buf);
    cb->endFunc = sdfPacTmxDecoderEndFunc;
}

void sdfPacTexListDecoderTmxHeaderEndFunc(sdfPacState_t *cb)
{
    sdfPacTmxDecoderInitFunc(cb, &cb->slot.list->header, &cb->slot.list->buf);
    cb->endFunc = sdfPacTexListDecoderTmxBodyEndFunc;
}

void func_002C6D58(sdfPacState_t *aState)
{
    aState->dst = (u8 *)&aState->slot.list->header;
    aState->dstLeft = 0x10;
    aState->decodeFunc = sdfPacMemDecoderFunc;
    aState->endFunc = sdfPacTexListDecoderTmxHeaderEndFunc;
}

void sdfPacTexListDecoderTmxBodyEndFunc(sdfPacState_t *cb)
{
    sdfPacList_t *list = cb->slot.list;

    sdfTexListAdd(cb->tail->resrc, list->buf.tex);
    {
        s32 nextIndex = list->index + 1;

        list->index = nextIndex;
        if (nextIndex == list->count) {
            sdfChipFreeDirect(list);
            sdfPacEndBlock(cb);
        } else {
            func_002C6D58(cb);
        }
    }
}

void sdfPacTexListDecoder1(sdfPacState_t *cb)
{
    s32 skipBytes = cb->srcLeft;

    if (cb->dstLeft < skipBytes) {
        skipBytes = cb->dstLeft;
    }
    sdfPacUseSrc(cb, skipBytes);
    {
        s32 bytesRemaining = cb->dstLeft - skipBytes;

        cb->dstLeft = bytesRemaining;
        if (bytesRemaining != 0) {
            return;
        }
    }
    func_002C6D58(cb);
}

void sdfPacTexListDecoder0(sdfPacState_t *cb)
{
    u32 count;
    sdfPacList_t *list;
    sdfPacWork_t *aWork;

    if (cb->srcLeft != 0) {
        count = *(const u32 *)cb->src;
        sdfPacUseSrc(cb, 4);
        list = cb->slot.list;
        aWork = cb->tail;
        list->count = count;
        list->index = 0;
        aWork->resrc = sdfTexListNew(count);
        cb->dstLeft = ((count * 4 + 0x53) & -0x40) - 0x14;
        cb->decodeFunc = sdfPacTexListDecoder1;
    }
}

void sdfPacTexListDecoderInit(sdfPacState_t *cb, sdfPacPacketHeader_t *hdr)
{
    sdfPacNewResrc(cb, hdr);
    cb->slot.list = sdfChipAllocDirect(sizeof(sdfPacList_t));
    cb->decodeFunc = sdfPacTexListDecoder0;
}
