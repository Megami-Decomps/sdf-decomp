#include "common.h"
#include "sdf_types.h"
#include "sdf_resource.h"

struct SdfPacketPatchLink;

struct sdfPrimBitBltElem_t;

struct sdfLoadImagePacket_t;

struct sdfGsMemSlice_t;

struct sdfGrFrameSet_t;

struct sdfDrawTag_t;

struct sdfStoreImagePacket_t;

struct SdfPatchableResourcePacket;

struct sdfPreDrawTag_t;

/* The frame worker passes the node and queued buffer index. The registered
 * handlers project the node into their distinct complete packet owners. */
typedef void (*SdfPacketPatchCallback)();

#define SDF_DMA_QWC_MASK 0xFFFF

#define SDF_DMA_ADDRESS_MASK 0x0FFFFFFF

#define SDF_PATCHABLE_PACKET_BYTES 0x100

#define SDF_PATCHABLE_PACKET_TAIL_OFFSET 0xD0

#define SDF_DESCRIPTOR_PACKET_BYTES 0xB0

#define SDF_DESCRIPTOR_PACKET_TAIL_OFFSET 0x80

typedef struct SdfPacketPatchLink {
    struct SdfPacketPatchLink *next;
    SdfPacketPatchCallback patch;
} SdfPacketPatchLink;

/* The four A+D writes controlling a GS image transfer. */
typedef struct sdfPrimBitBltElem_t {
    sdfGrTagStdReg_t bitbltbuf;
    sdfGrTagStdReg_t trxpos;
    sdfGrTagStdReg_t trxreg;
    sdfGrTagStdReg_t trxdir;
} sdfPrimBitBltElem_t;

/* Host-to-local image transfer packet followed by its flush terminal tag (0xB0). */
typedef struct sdfLoadImagePacket_t {
    u64 header[4];
    sdfPrimBitBltElem_t transfer;
    u64 imageReferenceTag;
    u64 imageReferencePad;
    u64 imageGifTag0;
    u64 imageGifTag1;
    u64 flushTag0;
    u64 flushTag1;
    u64 flushGifTag;
    u64 flushRegister;
    u64 zero;
    u64 finishRegister;
} sdfLoadImagePacket_t;


/* Graph target: two color buffers followed by the auxiliary/depth buffer (0x14). */
typedef struct sdfGrFrameSet_t {
    s16 width;
    s16 unk2;
    s16 height;
    u8 bufferFormat;
    u8 auxiliaryFormat;
    sdfGsMemSlice_t *buffers[3];
} sdfGrFrameSet_t;


void sdfDrawTagLink2(struct sdfDrawTag_t *list, u32 packetAddress,
                          u32 rangeTailAddress);

/* Resource upload payload and its terminal primitive-reset tag (0xF0). */
typedef struct sdfStoreImagePacket_t {
    u64 header[14];
    sdfPrimBitBltElem_t transfer;
    s32 quadwordCount;
    u32 unkB4;
    u32 unkB8;
    u32 unkBC;
    u64 tail[6];
} sdfStoreImagePacket_t;

/* Metadata node followed by the resource-transfer payload (0x100). */
typedef struct SdfPatchableResourcePacket {
    SdfPacketPatchLink link;
    u32 unk08;
    u32 unk0C;
    sdfStoreImagePacket_t resourcePacket;
} SdfPatchableResourcePacket;

/* Complete linked-packet list, distinct from the 0x20-byte DMA list head. */
typedef struct sdfPreDrawTag_t {
    u32 next;
    SdfPacketPatchLink *head;
    SdfPacketPatchLink *tail;
    u32 pad;
} sdfPreDrawTag_t;

extern sdfGrFrameSet_t D_00529388;

s32 sdfGetTexelBitCount(u32 format);

typedef struct EffPacketParams {
    s16 parameterCount;
    s16 vertexCount;
    u16 primitive;
    u16 mask;
    u32 color;
    const u32 *parameters;
    u128 *positions;
    u128 *normals;
    f32 *texcoords;
    u32 *extraTexcoords;
    u32 *colors;
    void *(*allocate)(s32);
    f32 depth;
} EffPacketParams;

void sdfSetStoreImage(sdfStoreImagePacket_t *packet, sdfGsMemSlice_t *source,
    s32 sourceX, s32 sourceY, s32 width, s32 height, u32 resourceAddress, u32 arg7, u32 arg8);

void sdfLinkStoreImage(sdfDrawTag_t *list, sdfGsMemSlice_t *source, s32 sourceX,
                   s32 sourceY, s32 width, s32 height, s32 resourceAddress, s32 arg7,
                   s32 arg8, s32 (*allocate)(s32));

void sdfLinkStoreFrameImagePredraw(SdfPatchableResourcePacket *packet, s32 entryIndex);

void sdfLinkStoreFrameImage(sdfDrawTag_t *list, sdfPreDrawTag_t *linkedList, s32 sourceX, s32 sourceY,
                   s32 width, s32 height, s32 resourceAddress, s32 arg7, s32 arg8,
                   s32 (*allocatePacket)(s32));

void sdfSetLoadImage(sdfLoadImagePacket_t *packet, sdfGsMemSlice_t *source,
                   s64 destinationX, s64 destinationY, s32 transferWidth,
                   s32 transferHeight, u32 sourceAddress);

void sdfLinkLoadImage(sdfDrawTag_t *list, sdfGsMemSlice_t *source,
                   s32 destinationX, s32 destinationY, s32 width, s32 height,
                   s32 sourceAddress, s32 (*allocatePacket)(s32));

s32 sdfGrAllocPacketBuff(s32 size);

void sdfPreDrawTagLink(sdfPreDrawTag_t *list, SdfPacketPatchLink *node);

void sdfPrimSetBitBltElem(sdfPrimBitBltElem_t *packet, u32 destinationBufferAddress, s32 destinationBufferWidth,
                  s64 destinationFormat, s64 destinationX, s64 destinationY,
                  u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                  s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight, s32 transferDirection);

void sdfSetStoreImage(sdfStoreImagePacket_t *packet, sdfGsMemSlice_t *source,
    s32 sourceX, s32 sourceY, s32 width, s32 height, u32 resourceAddress, u32 arg7, u32 arg8) {
    s32 quadwordCount;

    packet->header[0] = 0x90000001ULL;
    packet->header[1] = 0x10000000ULL;
    packet->header[2] = 0;
    packet->header[3] = 0;
    packet->header[4] = 0x90000002ULL;
    packet->header[5] = 0x5000000206008000ULL;
    packet->header[6] = 0x1000000000000001ULL;
    packet->header[7] = 0xE;
    packet->header[8] = 0;
    packet->header[9] = 0x61;
    packet->header[10] = ((u64)((u32)packet->tail & SDF_DMA_ADDRESS_MASK) << 32) | 0xA0000005ULL;
    packet->header[11] = 0x5000000500000000ULL;
    packet->header[12] = 0x1000000000000004ULL;
    packet->header[13] = 0xE;
    sdfPrimSetBitBltElem(&packet->transfer, 0, 0, 0, 0, 0,
        source->addr, source->w, source->pixelFormat,
        sourceX, sourceY, width, height, 1);
    quadwordCount = (sdfGetTexelBitCount(source->pixelFormat) * width * height) >> 7;
    packet->quadwordCount = quadwordCount;
    packet->unkB4 = resourceAddress;
    packet->unkB8 = arg7;
    packet->unkBC = arg8;
    packet->tail[0] = 0x20000002ULL;
    packet->tail[1] = 0x5000000206000000ULL;
    packet->tail[2] = 0x1000000000008001ULL;
    packet->tail[3] = 0xE;
    packet->tail[4] = 0x46;
    packet->tail[5] = 0;
}

void sdfLinkStoreImage(sdfDrawTag_t *list, sdfGsMemSlice_t *source, s32 sourceX,
                   s32 sourceY, s32 width, s32 height, s32 resourceAddress, s32 arg7,
                   s32 arg8, s32 (*allocate)(s32)) {
    s32 packetAddress;

    if (allocate == NULL) {
        allocate = sdfGrAllocPacketBuff;
    }
    packetAddress = allocate(0xf0);
    sdfSetStoreImage((sdfStoreImagePacket_t *)packetAddress, source,
        sourceX, sourceY, width, height, resourceAddress, arg7, arg8);
    sdfDrawTagLink2(list, packetAddress, packetAddress + 0xc0);
}

void sdfLinkStoreFrameImagePredraw(SdfPatchableResourcePacket *packet, s32 entryIndex) {
    packet->resourcePacket.transfer.bitbltbuf.data =
        (packet->resourcePacket.transfer.bitbltbuf.data & ~0x3FFF) |
        (u64)(u32)(D_00529388.buffers[entryIndex]->addr >> 6);
}

void sdfLinkStoreFrameImage(sdfDrawTag_t *list, sdfPreDrawTag_t *linkedList, s32 sourceX, s32 sourceY,
                   s32 width, s32 height, s32 resourceAddress, s32 arg7, s32 arg8,
                   s32 (*allocatePacket)(s32)) {
    SdfPatchableResourcePacket *packet;

    if (allocatePacket == NULL) {
        allocatePacket = sdfGrAllocPacketBuff;
    }
    packet = (SdfPatchableResourcePacket *)allocatePacket(SDF_PATCHABLE_PACKET_BYTES);
    packet->link.patch = sdfLinkStoreFrameImagePredraw;
    sdfSetStoreImage(&packet->resourcePacket,
        D_00529388.buffers[0], sourceX, sourceY, width, height,
        resourceAddress, arg7, arg8);
    sdfPreDrawTagLink(linkedList, &packet->link);
    sdfDrawTagLink2(list, (u32)&packet->resourcePacket,
        (u32)packet + SDF_PATCHABLE_PACKET_TAIL_OFFSET);
}

void sdfSetLoadImage(sdfLoadImagePacket_t *packet, sdfGsMemSlice_t *source,
                   s64 destinationX, s64 destinationY, s32 transferWidth,
                   s32 transferHeight, u32 sourceAddress) {
    s32 qwc;

    packet->header[0] = 0x10000006;
    packet->header[1] = 0x5000000611000000;
    packet->header[2] = 0x1000000000000004;
    packet->header[3] = 0xE;
    sdfPrimSetBitBltElem(&packet->transfer, source->addr, source->w,
                  source->pixelFormat, destinationX, destinationY, 0, 0, 0, 0, 0,
                  transferWidth, transferHeight, 0);

    qwc = (sdfGetTexelBitCount(source->pixelFormat) * transferWidth * transferHeight) >> 7;
    packet->imageReferenceTag = 0x0800000000000000 | qwc;
    packet->imageReferencePad = 0;
    packet->imageGifTag0 = (u32)((qwc & SDF_DMA_QWC_MASK) | 0x30000000) |
                           ((u64)(sourceAddress & SDF_DMA_ADDRESS_MASK) << 32);
    packet->imageGifTag1 = (u64)(0x51000000 | qwc) << 32;
    packet->flushTag0 = 0x20000002;
    packet->flushTag1 = 0x5000000200000000;
    packet->flushGifTag = 0x1000000000008001;
    packet->flushRegister = 0xE;
    packet->zero = 0;
    packet->finishRegister = 0x3F;
}

void sdfLinkLoadImage(sdfDrawTag_t *list, sdfGsMemSlice_t *source,
                   s32 destinationX, s32 destinationY, s32 width, s32 height,
                   s32 sourceAddress, s32 (*allocatePacket)(s32)) {
    s32 packetAddress;
    if (allocatePacket == NULL) {
        allocatePacket = sdfGrAllocPacketBuff;
    }
    packetAddress = allocatePacket(SDF_DESCRIPTOR_PACKET_BYTES);
    sdfSetLoadImage((sdfLoadImagePacket_t *)packetAddress, source,
                  destinationX, destinationY, width, height, sourceAddress);
    sdfDrawTagLink2(list, packetAddress, packetAddress + SDF_DESCRIPTOR_PACKET_TAIL_OFFSET);
}
INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDelayedResrcSemaId);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrPacketBuffHandle);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrPacketBuffSize);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_00551870);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrPacketBuffPtr);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrPacketBuffLimit);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_00551880);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDrawFrameFlag);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDrawSyncFlag);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDrawFrameCounter);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_00551888);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrDrawFrameId);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_0055188A);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfReqDrawDelayCount);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfReqDrawDelaySleepCount);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_0055188E);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDrawReqQueueIndex);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfGrDrawFrameTagHead);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfDrawFrameCount);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", corePktSoftenBias);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", D_0055189D);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfMatAllocCount);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfCoreDrawBuffHandle);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfFaceDivSoften);

INCLUDE_SDATA(const s32, "sdf/sdfGsMemIO", sdfFaceDivBias);

