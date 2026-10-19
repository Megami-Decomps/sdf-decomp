#include "common.h"
#include "ps2asm.h"
#include "sdf_lighting.h"

/* Register selectors carried by GIF A+D writes in the two GS contexts. */
enum {
    SDF_GS_PRIM = 0x00,
    SDF_GS_RGBAQ = 0x01,
    SDF_GS_XYZ2 = 0x05,
    SDF_GS_TEX0_1 = 0x06,
    SDF_GS_TEX0_2 = 0x07,
    SDF_GS_CLAMP_1 = 0x08,
    SDF_GS_CLAMP_2 = 0x09,
    SDF_GIF_REGISTER_AD = 0x0E,
    SDF_GS_TEX1_1 = 0x14,
    SDF_GS_TEX1_2 = 0x15,
    SDF_GS_XYOFFSET_1 = 0x18,
    SDF_GS_XYOFFSET_2 = 0x19,
    SDF_GS_PRMODECONT = 0x1A,
    SDF_GS_TEXA = 0x3B,
    SDF_GS_TEXFLUSH = 0x3F,
    SDF_GS_SCISSOR_1 = 0x40,
    SDF_GS_SCISSOR_2 = 0x41,
    SDF_GS_ALPHA_1 = 0x42,
    SDF_GS_ALPHA_2 = 0x43,
    SDF_GS_DTHE = 0x45,
    SDF_GS_COLCLAMP = 0x46,
    SDF_GS_TEST_1 = 0x47,
    SDF_GS_TEST_2 = 0x48,
    SDF_GS_FRAME_1 = 0x4C,
    SDF_GS_FRAME_2 = 0x4D,
    SDF_GS_ZBUF_1 = 0x4E,
    SDF_GS_ZBUF_2 = 0x4F,
    SDF_GS_BITBLTBUF = 0x50,
    SDF_GS_TRXPOS = 0x51,
    SDF_GS_TRXREG = 0x52,
    SDF_GS_TRXDIR = 0x53
};

struct sdfDrawTag_t;

struct SdfDrawPacket;

struct SdfTextureDrawPacket;

struct sdfTexPacket_t;

struct sdfGsMemSlice_t;

struct sdfTexHandle_t;

struct TMX_HEADER_t;

/* Serialized byte consumed by the GS texture-state packet builder. */
typedef u8 SdfTextureClampMode;

/* DMA packet list cursors and endpoints (0x20); DDS1/2 game/code_002D33C8/0032C278.c. */
typedef struct sdfDrawTag_t {
    struct sdfDrawTag_t *next; /* CPU link between lists; head/tail are DMA addresses. */
    u32 head;
    u32 tail;
    u32 z;
    u32 lightPacket; /* Optional first DMA reference prefix. */
    u32 fogPacket;   /* Optional second DMA reference prefix. */
    u32 texPacket[2];
} sdfDrawTag_t;

/* Variable-sized packet header initialized by sdf2DMakePrimPacketHeader. */
typedef struct SdfDrawPacket {
    u16 quadwords;
    u8 pad02[6];
    u32 pad08;
    u32 command;
    u64 gifTag;
    u64 payloadHeader;
} SdfDrawPacket;

#define SDF_GIF_PRIM_SHIFT 47

#define SDF_DMA_QWORD_SHIFT 4

#define SDF_GIF_NREG_SHIFT 60

#define SDF_GIF_PRE_EOP_FLAGS 0x400000008000LL

#define SDF_VIF_DIRECT_COMMAND 0x50000000

/* Fixed-size texture packet; constructor allocation and writes cover 0x50. */
typedef struct SdfTextureDrawPacket {
    SdfDrawPacket header;
    u64 textureWordA;
    u64 registerAddressA;
    u64 textureWordB;
    u64 registerAddressB;
    u64 textureWordC;
    u64 registerAddressC;
} SdfTextureDrawPacket;

void sdfDrawTagLink(struct sdfDrawTag_t *list, u32 packetAddress);

/* Per-texture reference count and non-owning clone source (0x8). */
/* Texture buffer GPU command words (0x40); DDS1/2 game/code_002D10B0/00329F60.c. */
typedef struct sdfTexPacket_t {
    s32 gifTagWord; /* Low GIFtag word; NLOOP occupies bits 0-14. */
    u32 gifTagControl; /* High GIFtag control word; one packed register. */
    u64 gifTagRegisters; /* Packed register list: A+D. */
    u64 samplingState; /* GS TEX1 data, including minification/magnification filters. */
    u64 samplingRegister; /* GS TEX1_1/TEX1_2 register selector. */
    u64 textureState; /* GS TEX0 data. */
    u64 textureRegister; /* GS TEX0_1/TEX0_2 register selector. */
    u64 clampState; /* GS CLAMP data. */
    u64 clampRegister; /* GS CLAMP_1/CLAMP_2 register selector. */
} sdfTexPacket_t;

/* Native VRAM range descriptor (0x1C), shared by textures, graph buffers and streams. */
typedef struct sdfGsMemSlice_t {
    struct sdfGsMemSlice_t *prev; /* 0x00 */
    struct sdfGsMemSlice_t *next; /* 0x04 */
    u32 type;                   /* 0x08: known categories above; unknown values remain valid */
    u32 addr;                   /* 0x0C: VRAM offset in 32-bit words */
    s32 size;                   /* 0x10: range length in 32-bit words */
    s16 w;                      /* 0x14 */
    s16 h;                      /* 0x16 */
    s32 pixelFormat;            /* 0x18: retail loads it with lw, so signed */
} sdfGsMemSlice_t;

/* Linked texture and its two buffers/resources (0x40); DDS1/2 sdf/sdfTex.c and game texture units. */
typedef struct sdfTexHandle_t {
    struct sdfTexHandle_t *prev;
    struct sdfTexHandle_t *next;
    struct sdfTexHandle_t *origin;
    s16 width;
    s16 height;
    sdfGsMemSlice_t *pixelGsHandle;
    sdfGsMemSlice_t *clutGsHandle;
    u8 numOfCLUT;
    u8 CLUTFormat;
    u8 pixelFormat;
    u8 numOfMipmap;
    u16 mipmapParam; /* Packed GS TEX1 L/K parameters. */
    u8 flags;
    SdfTextureClampMode texWrapMode; /* 0x1F: 0xFF leaves GS CLAMP state zero; otherwise low nibble is copied. */
    s32 texId;
    s32 clutId;
    sdfTexPacket_t *packet;
    sdfTexPacket_t *packetCtx2;
    u8 *clutData; /* 0x30: retained copy of the texture's palette colors. */
    s32 clutDataSize; /* 0x34: palette byte count from sdfTexGetPaletteByteSize. */
    s32 clutColor;
    u8 *monoClutData; /* 0x3C: lazily generated weighted-RGB intensity bytes. */
} sdfTexHandle_t;

/* Return packed 64-bit TEX0, TEX1, and CLAMP states from the primary GS packet. */
u64 sdfTexGetTex0Reg(sdfTexHandle_t *texture);

u64 sdfTexGetTex1Reg(sdfTexHandle_t *texture);

u64 sdfTexGetClampReg(sdfTexHandle_t *texture);

/* DMA/VIF transfer header and packed A+D GIF tag, excluding register payload. */
void sdfGrSetRegsPacketHeader(sdfGrPacketHeader_t *header, s32 payloadQwords);

u32 sdf2DGetTexPacketSize(sdfTexHandle_t *texture);

/* Serialized texture-resource header; payload begins after this 0x40-byte prefix. */
typedef struct TMX_HEADER_t {
    u8 fileType;
    u8 compression;    /* 0x01: high nibble adds payload offset bytes; low bits stay opaque. */
    s16 userId;        /* 0x02 */
    u32 fileSize;      /* 0x04 */
    u32 formatId;      /* 0x08 */
    s32 realSize;      /* 0x0C */
    u8 numOfCLUT;      /* 0x10 */
    u8 CLUTFormat;     /* 0x11 */
    s16 width;         /* 0x12 */
    s16 height;        /* 0x14 */
    u8 pixelFormat;    /* 0x16 */
    u8 numOfMipmap;    /* 0x17 */
    u16 mipmapParam;   /* 0x18 */
    u8 flags;          /* 0x1A */
    SdfTextureClampMode texWrapMode; /* 0x1B: serialized GS clamp mode */
    /* The IDB calls 0x1C onward userArea and comment. These two are what this
     * game puts in its user area; sdfTmx reads both as words. */
    s32 resourceKey;   /* 0x1C */
    s32 battleTextureSlot; /* 0x20: selects one of two battle-model texture headers */
    u8 pad24[0x1C];    /* 0x24 */
} TMX_HEADER_t;

/* Acquire a texture from a serialized texture-file header. */
struct sdfTexHandle_t *sdfTexSet(const TMX_HEADER_t *header);

struct sdfTexHandle_t *sdfTexSetDirect(const TMX_HEADER_t *header);

/* Reserve size bytes from the active packet buffer, rounded up to 16 bytes.
 * Returns the prior cursor as a 32-bit address; the buffer owns this storage. */
s32 sdfGrAllocPacketBuff(s32 size);

SdfTextureDrawPacket *sdf2DMakeTexPacket(SdfTextureDrawPacket *drawPacket, sdfTexHandle_t *texture, s32 contextOffset);

s32 sdf2DLinkTexPacket(sdfDrawTag_t *packetList, sdfTexHandle_t *texture, s32 contextOffset);

u32 sdf2DMakeStdPacket(u32 packetAddress, s32 packetBytes);

s32 sdf2DGetPrimPacketSize(s32 registerCount, s32 loopCount);

SdfDrawPacket *sdf2DMakePrimPacketHeader(SdfDrawPacket *packet, s32 primitiveFlags, s32 registerCount, s64 registerList, s32 loopCount);

SdfDrawPacket *sdf2DAllocSprite(s32 loopCount);

u32 sdf2DGetTexPacketSize(sdfTexHandle_t *texture) {
    return 5 << SDF_DMA_QWORD_SHIFT;
}

SdfTextureDrawPacket *sdf2DMakeTexPacket(SdfTextureDrawPacket *drawPacket, sdfTexHandle_t *texture, s32 contextOffset) {
    drawPacket->header.quadwords = 4;
    drawPacket->header.gifTag = 0x1000000000008003ULL;
    drawPacket->header.command = 0x50000004;
    drawPacket->header.pad08 = 0;
    drawPacket->header.payloadHeader = SDF_GIF_REGISTER_AD;
    drawPacket->textureWordA = sdfTexGetTex1Reg(texture);
    drawPacket->registerAddressA = contextOffset + 0x14;
    drawPacket->textureWordB = sdfTexGetTex0Reg(texture);
    drawPacket->registerAddressB = contextOffset + 6;
    drawPacket->textureWordC = sdfTexGetClampReg(texture);
    drawPacket->registerAddressC = contextOffset + 8;
    return drawPacket;
}

s32 sdf2DLinkTexPacket(sdfDrawTag_t *packetList, sdfTexHandle_t *texture, s32 contextOffset) {
    s32 packetBytes = sdf2DGetTexPacketSize(texture);
    SdfTextureDrawPacket *packet = (SdfTextureDrawPacket *)sdfGrAllocPacketBuff(packetBytes);
    s32 packetAddress = (s32)sdf2DMakeTexPacket(packet, texture, contextOffset);
    sdfDrawTagLink(packetList, packetAddress);
    return packetAddress;
}

u32 sdf2DMakeStdPacket(u32 packetAddress, s32 packetBytes) {
    sdfGrSetRegsPacketHeader((sdfGrPacketHeader_t *)packetAddress, (packetBytes >> SDF_DMA_QWORD_SHIFT) - 2);
    return packetAddress;
}

s32 sdf2DGetPrimPacketSize(s32 registerCount, s32 loopCount) {
    return (registerCount * loopCount + 2) << SDF_DMA_QWORD_SHIFT;
}

void *sdf2DGetPrimPtr(SdfDrawPacket *packet) {
    return packet + 1;
}

SdfDrawPacket *sdf2DMakePrimPacketHeader(SdfDrawPacket *packet, s32 primitiveFlags, s32 registerCount, s64 registerList, s32 loopCount) {
    s32 packetQuadwords = registerCount * loopCount + 1;
    s64 gifTag = loopCount | ((s64)registerCount << SDF_GIF_NREG_SHIFT);

    gifTag |= (s64)primitiveFlags << SDF_GIF_PRIM_SHIFT;
    gifTag |= SDF_GIF_PRE_EOP_FLAGS;
    packet->gifTag = gifTag;
    packet->command = packetQuadwords | SDF_VIF_DIRECT_COMMAND;
    packet->payloadHeader = registerList;
    packet->pad08 = 0;
    packet->quadwords = packetQuadwords;
    return packet;
}

SdfDrawPacket *sdf2DAllocSprite(s32 loopCount) {
    SdfDrawPacket *packet = (SdfDrawPacket *)sdfGrAllocPacketBuff(sdf2DGetPrimPacketSize(5, loopCount));
    sdf2DMakePrimPacketHeader(packet, 0x156, 5, 0x53531, loopCount);
    return packet;
}
