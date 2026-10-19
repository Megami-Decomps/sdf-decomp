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

typedef f32 (*sdfLightSources_t[3])[4];

struct sdfDrawTag_t;

struct sdf3DFogParam_t;

struct SdfCamera;

struct sdf3DViewportPacket_t;

struct DmaPacketHeader;

struct sdf3DFogPacket_t;

struct sdf3DScreenParam_t;

struct sdfTexPacket_t;

struct sdfGsMemSlice_t;

struct sdfTexHandle_t;

struct sdfMemHandle_t;

struct TMX_HEADER_t;

struct SdfGeneratedLookupTexture;

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

/* Shared fog/depth state consumed by sdf3DMakeFogPacket (0x14).
 * The viewer config serializes fogNear, depthNear, fogFar, depthFar, fogColor;
 * timeline payloads store fogFar before fogNear instead. */
typedef struct sdf3DFogParam_t {
    f32 fogNear;    /* 0x00: minimum fog range. */
    f32 fogFar;     /* 0x04: maximum fog range. */
    f32 depthNear;  /* 0x08: near depth used in the projection coefficients. */
    f32 depthFar;   /* 0x0C: far depth used in the projection coefficients. */
    u32 fogColor;   /* 0x10: packed GS FOGCOL value. */
} sdf3DFogParam_t;

/* Native 0x90-byte camera/viewport owner shared by both projection builders. */
typedef struct SdfCamera {
    u32 flags;       // 0x00: 1 half-height, 2 field-of-view projection
    f32 aspect;      // 0x04
    f32 scale;       // 0x08
    f32 fov;         // 0x0C
    f32 offsetX;     // 0x10
    f32 offsetY;     // 0x14
    f32 width;       // 0x18
    f32 height;      // 0x1C
    f32 top;         // 0x20
    f32 bottom;      // 0x24
    f32 nearZ;       // 0x28
    f32 farZ;        // 0x2C
    u8 matrix[0x40]; // 0x30
    f32 halfWidth;   // 0x70
    f32 halfHeight;  // 0x74
    f32 centerY;     // 0x78
    f32 one;         // 0x7C
    f32 originX;     // 0x80
    f32 originY;     // 0x84
    f32 bottomY;     // 0x88
    u32 zero;        // 0x8C
} SdfCamera;

/* Shared 0xD0 VIF packet: two stored matrices, three vector slots, and trailer. */
typedef struct sdf3DViewportPacket_t {
    u16 quadwords;        /* 0x00 */
    u8 unknown02[6];      /* 0x02 */
    u32 reservedWord;     /* 0x08 */
    u32 command;          /* 0x0C */
    u8 matrixA[0x40];     /* 0x10 */
    u8 matrixB[0x40];     /* 0x50 */
    u8 vecA[0x10];        /* 0x90 */
    u8 vecB[0x10];        /* 0xA0 */
    u8 vecC[0x10];        /* 0xB0 */
    u32 stmodCommand;     /* 0xC0 */
    u32 mscalCommand;     /* 0xC4 */
    u32 unknownC8;        /* 0xC8 */
    u32 unknownCC;        /* 0xCC */
} QWORD_ALIGNED sdf3DViewportPacket_t;

/* Shared 0x20-byte DMA/VIF reference header. Unrecovered bytes stay opaque. */
typedef struct DmaPacketHeader {
    u16 quadwords; /* 0x00 */
    u8 pad02[2]; /* 0x02 */
    u32 address; /* 0x04 */
    u32 tag; /* 0x08 */
    u32 command; /* 0x0C */
    u16 unknown10; /* 0x10 */
    u8 opaque12[6]; /* 0x12 */
    u32 unknown18; /* 0x18 */
    u32 unknown1C; /* 0x1C */
} QWORD_ALIGNED DmaPacketHeader;

/* Bind resource and asset release queues to their matching destruction callbacks. */
void sdfMatInitSystem(void);

/* Initialize the buffered object-list release queue with its object destructor. */
void sdfModelInitSystem(void);

extern u32 sdf3DLightDefault;

extern u32 sdf3DFogDefault;

extern f32 sdf3DEyeOffset[4];

extern u8 sdf3DScreenParam[];

#define SDF_DMA_ADDRESS_MASK 0x0FFFFFFF

#define SDF_DMA_QWORD_SHIFT 4

#define SDF_GS_FOGCOL_REGISTER 0x3D

#define SDF_VIF_DIRECT_COMMAND 0x50000000

#define SDF_VIF_FLUSHA_COMMAND 0x13000000

#define SDF_VIF_ITOP_MATRIX 0x04000002

#define SDF_VIF_MSCAL_COMMAND 0x14000000

extern u8 vu1MicroPacket[];

extern u8 sdf3DScreenTm[];

extern u8 sdfCameraTm[];

/* DMA/VIF prefix, left/right projection terms, then a one-register GIF write
 * and the VU execution command. */
typedef struct sdf3DFogPacket_t {
    u64 dmaTag;
    u64 vifUnpackCode;
    f32 right;
    f32 left;
    f32 mid;
    f32 scale;
    u64 gifTag;
    u64 gifRegister;
    u64 registerValue;
    u64 fogColorRegister;
    u32 mscalCommand;
    u32 reservedA;
    u32 reservedB;
    u32 reservedC;
} sdf3DFogPacket_t;

extern u8 D_002F5600[];

extern u64 sdf3DRefMapParam;

extern u8 D_002E8ED0[];

extern u8 D_002ECF10[];

/* Complete static projection record, including the inverse-origin cache. */
typedef struct sdf3DScreenParam_t {
    SdfCamera camera;
    f32 inverseOrigin[4]; /* 0x90 */
} sdf3DScreenParam_t;

void sdfDrawTagLinkRef(struct sdfDrawTag_t *list, u32 packetAddress);

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
    sdfGsMemSlice_t *primaryResource;
    sdfGsMemSlice_t *secondaryResource;
    u8 paletteCount;
    u8 clutFormat;
    u8 pixelFormat;
    u8 maxMipLevel;
    u16 lodParameters; /* Packed GS TEX1 L/K parameters. */
    u8 unk1E;
    SdfTextureClampMode clampMode; /* 0x1F: 0xFF leaves GS CLAMP state zero; otherwise low nibble is copied. */
    s32 resourceKey;
    s32 battleTextureSlot;
    sdfTexPacket_t *primaryBuffer;
    sdfTexPacket_t *secondaryBuffer;
    u8 *paletteData; /* 0x30: retained copy of the texture's palette colors. */
    s32 paletteDataSize; /* 0x34: palette byte count from sdfTexGetPaletteByteSize. */
    s32 unk38;
    u8 *intensityMap; /* 0x3C: lazily generated weighted-RGB intensity bytes. */
} sdfTexHandle_t;

/* Return packed 64-bit TEX0, TEX1, and CLAMP states from the primary GS packet. */
u64 sdfTexGetTex0Reg(sdfTexHandle_t *texture);

/* General-heap allocation descriptor (0x10); links bound the represented span. */
typedef struct sdfMemHandle_t {
    struct sdfMemHandle_t *prev; /* 0x00 */
    struct sdfMemHandle_t *next; /* 0x04 */
    s32 ptr;                  /* 0x08: start of represented span */
    u16 stat;                 /* 0x0C: free, used, or end sentinel */
    s16 mlock;                /* 0x0E: -1 for sentinels */
} sdfMemHandle_t;

u32 sdfMemLock(struct sdfMemHandle_t *allocation);

void sdfMemFreeDirect(struct sdfMemHandle_t *allocation);

/* Serialized texture-resource header; payload begins after this 0x40-byte prefix. */
typedef struct TMX_HEADER_t {
    u8 unk00;
    u8 payloadOffsetFlags; /* 0x01: high nibble adds payload offset bytes; low bits stay opaque. */
    u8 pad02[2];       /* 0x02 */
    u32 byteCount;     /* 0x04 */
    u32 magic;         /* 0x08 */
    u8 pad0C[4];       /* 0x0C */
    u8 paletteCount;   /* 0x10 */
    u8 clutFormat;     /* 0x11 */
    s16 width;         /* 0x12 */
    s16 height;        /* 0x14 */
    u8 pixelFormat;    /* 0x16 */
    u8 maxMipLevel;    /* 0x17 */
    u16 lodParameters; /* 0x18 */
    u8 unk1A;          /* 0x1A */
    SdfTextureClampMode clampMode; /* 0x1B: serialized GS clamp mode */
    s32 resourceKey;   /* 0x1C */
    s32 battleTextureSlot; /* 0x20: selects one of two battle-model texture headers */
    u8 pad24[0x1C];    /* 0x24 */
} TMX_HEADER_t;

/* Acquire a texture from a serialized texture-file header. */
struct sdfTexHandle_t *sdfTexSet(const TMX_HEADER_t *header);

struct sdfTexHandle_t *sdfTexSetDirect(const TMX_HEADER_t *header);

/* Generated 128x64 grayscale power lookup texture: a serialized texture
 * header, 256 RGBA palette words, then the full 64x128 index image. */
typedef struct SdfGeneratedLookupTexture {
    TMX_HEADER_t header; /* 0x0000 */
    u32 palette[256];            /* 0x0040 */
    u8 indices[64][128];         /* 0x0440 */
} SdfGeneratedLookupTexture;

extern sdfTexHandle_t *D_005518B8;

extern sdfTexHandle_t *sdf3DSpecularTex;

extern sdfTexHandle_t *D_005518C8;

extern void sdfTmxSetHeader(TMX_HEADER_t *, s32, s32, s32, s32, s32);

extern s32 vu1Tex0Slot0[];

extern s32 vu1Tex0Slot2[];

extern s32 vu1Tex0Slot1[];

extern void *memcpy(void *, const void *, unsigned int);

void ReverseMatrix(void);

void MultMatrixRev(const void *matrix);

void sdf3DMakeViewportPacket(sdf3DViewportPacket_t *packet, sdf3DScreenParam_t *node, void *transformMatrix);

void sdf3DSetCamera(sdf3DScreenParam_t *node, void *transformMatrix);

void sdf3DMakeFogPacket(sdf3DFogPacket_t *packet, sdf3DFogParam_t *projectionParams);

void sdf3DMakeDirectRefPacket(DmaPacketHeader *packet, u32 sourceAddress, s32 payloadBytes);

void sdf3DLinkInitPacket(sdfDrawTag_t *packetList, DmaPacketHeader *packet);

void sdf3DInitSpecularTex2(void);

void sdf3DInit(void);

void sdf3DMakeViewportPacket(sdf3DViewportPacket_t *packet, sdf3DScreenParam_t *node, void *transformMatrix) {
    packet->quadwords = 0xC;
    packet->command = 0x6C0BC000;
    packet->reservedWord = 0;
    SDF_VU0_LOADMATRIX(transformMatrix);
    SDF_VU0_STOREMATRIX(packet->matrixA);
    MultMatrixRev(node->camera.matrix);
    SDF_VU0_STOREMATRIX(packet->matrixB);
    SDF_VU0_LOADVECTOR(&node->camera.halfWidth);
    SDF_VU0_STOREVECTOR(packet->vecA);
    SDF_VU0_LOADVECTOR(&node->camera.originX);
    SDF_VU0_STOREVECTOR(packet->vecB);
    SDF_VU0_LOADMATRIX(transformMatrix);
    ReverseMatrix();
    SDF_VU0_GET_ROW3();
    SDF_VU0_STOREVECTOR(packet->vecC);
    SDF_VU0_STOREVECTOR(node->inverseOrigin);
    packet->stmodCommand = SDF_VIF_ITOP_MATRIX;
    packet->mscalCommand = SDF_VIF_MSCAL_COMMAND;
    packet->unknownC8 = 0;
    packet->unknownCC = 0;
}

void sdf3DSetCamera(sdf3DScreenParam_t *node, void *transformMatrix) {
    SDF_VU0_LOADMATRIX(transformMatrix);
    SDF_VU0_STOREMATRIX(sdfCameraTm);
    MultMatrixRev(node->camera.matrix);
    SDF_VU0_LOADVECTOR(node->inverseOrigin);
    SDF_VU0_STOREMATRIX(sdf3DScreenTm);
    SDF_VU0_APPLYMATRIX_XYZ1();
    SDF_VU0_STOREVECTOR(sdf3DEyeOffset);
    memcpy(sdf3DScreenParam, node, sizeof(*node));
}

void sdf3DSetDefaultLight(u8 *sourceObject) {
    sdf3DLightDefault = (u32)sourceObject;
}

void sdf3DSetDefaultFog(u8 *sourceObject) {
    sdf3DFogDefault = (u32)sourceObject;
}

INCLUDE_ASM(const s32, "sdf/sdf3D", sdf3DMakeScreenParam);

void sdf3DMakeLightParallelPacket(sdf3DLightParallelPacket_t *packet, sdfLightSources_t sources, f32 *ambient);

INCLUDE_ASM(const s32, "sdf/sdf3D", sdf3DMakeLightParallelPacket);

void sdf3DMakeFogPacket(sdf3DFogPacket_t *packet, sdf3DFogParam_t *projectionParams) {
    f32 rangeMax = projectionParams->fogFar;
    f32 rangeMin = projectionParams->fogNear;
    f32 nearZ = projectionParams->depthNear;
    f32 farZ = projectionParams->depthFar;
    f32 depthRange = farZ - nearZ;
    s32 fogColor = projectionParams->fogColor;
    packet->dmaTag = 0x20000004;
    packet->vifUnpackCode = 0x6C03C00013000000ULL;
    packet->gifTag = 0x1000000000008001ULL;
    packet->gifRegister = SDF_GIF_REGISTER_AD;
    packet->registerValue = (u32)fogColor;
    packet->fogColorRegister = SDF_GS_FOGCOL_REGISTER;
    packet->mscalCommand = 0x14000014;
    packet->reservedC = 0;
    packet->right = rangeMax;
    packet->left = rangeMin;
    packet->reservedA = 0;
    packet->reservedB = 0;
    packet->mid = (((rangeMax - rangeMin) * (farZ + nearZ)) / depthRange + (rangeMax + rangeMin)) * 0.5f;
    packet->scale = ((farZ * nearZ) * (rangeMin - rangeMax)) / depthRange;
}

void sdf3DMakeDirectRefPacket(DmaPacketHeader *packet, u32 sourceAddress, s32 payloadBytes) {
    s32 quadwordCount = (payloadBytes + 15) >> SDF_DMA_QWORD_SHIFT;
    packet->quadwords = quadwordCount;
    packet->address = sourceAddress & SDF_DMA_ADDRESS_MASK;
    packet->tag = SDF_VIF_FLUSHA_COMMAND;
    packet->command = quadwordCount | SDF_VIF_DIRECT_COMMAND;
    packet->unknown10 = 0;
    packet->unknown18 = 0;
    packet->unknown1C = 0;
}

void sdf3DLinkInitPacket(sdfDrawTag_t *packetList, DmaPacketHeader *packet) {
    packet->address = (u32)vu1MicroPacket & SDF_DMA_ADDRESS_MASK;
    packet->quadwords = (D_002F5600 - vu1MicroPacket) >> SDF_DMA_QWORD_SHIFT;
    packet->tag = 0;
    packet->command = 0;
    packet->unknown10 = 0;
    packet->unknown18 = 0;
    packet->unknown1C = 0;
    sdfDrawTagLinkRef(packetList, (u32)packet);
}

void sdf3DInitSpecularTex2(void) {
    struct sdfMemHandle_t *allocation = sdfMemAllocDirect(0x2440);
    SdfGeneratedLookupTexture *texture = (SdfGeneratedLookupTexture *)(u32)sdfMemLock(allocation);
    s32 i;
    s32 j;

    sdfTmxSetHeader(&texture->header, 0x80, 0x40, 0x13, 1, 0);
    texture->header.byteCount = 0x2440;
    texture->header.clampMode = 5;
    /* The physical CLUT slot j decodes to the logical intensity i. */
    for (j = 0; j != 0x100; j++) {
        i = ((j & 8) << 1) | ((j & 0x10) >> 1) | (j & ~0x18);
        texture->palette[j] = i | (i << 16) | (i << 8) | 0x80000000;
    }
    memset(&texture->indices[0][1], 0xFF, 0x7F);
    texture->indices[0][0] = 0;
    for (i = 0; i != 0x80; i++) {
        f32 power = i * (1.0f / 127.0f);
        f32 value = power;
        for (j = 1; j != 0x40; j++) {
            texture->indices[j][i] = (u8)(s32)(value * 255.0f + 0.5f);
            value *= power;
        }
    }
    sdf3DSpecularTex = sdfTexSet(&texture->header);
    sdfMemFreeDirect(allocation);
}

void sdf3DInit(void) {
    s64 textureWord;
    sdf3DInitSpecularTex2();
    textureWord = sdfTexGetTex0Reg(sdf3DSpecularTex);
    sdf3DRefMapParam = textureWord;
    vu1Tex0Slot0[0] = textureWord;
    vu1Tex0Slot0[1] = textureWord >> 32;
    D_005518B8 = sdfTexSetDirect((TMX_HEADER_t *)D_002E8ED0);
    textureWord = sdfTexGetTex0Reg(D_005518B8);
    vu1Tex0Slot2[0] = textureWord;
    vu1Tex0Slot2[1] = textureWord >> 32;
    D_005518C8 = sdfTexSetDirect((TMX_HEADER_t *)D_002ECF10);
    textureWord = sdfTexGetTex0Reg(D_005518C8);
    vu1Tex0Slot1[0] = textureWord;
    vu1Tex0Slot1[1] = textureWord >> 32;
    sdfModelInitSystem();
    sdfMatInitSystem();
}
