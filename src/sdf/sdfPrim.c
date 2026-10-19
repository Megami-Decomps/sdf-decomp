#include "common.h"
#include "sdf_types.h"

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

extern u8 D_0055188A;

extern u16 sdfReqDrawDelaySleepCount;

extern u32 sdfDrawFrameCount;

extern volatile u8 sdfDrawSyncFlag;

struct SdfGsGouraudTexturedVertex;

struct SdfGsTwoVertexPayload;

struct SdfGsTexturedVertex;

struct SdfGsGouraudVertex;

struct sdfPrimF3Elem_t;

struct sdfPrimF4Elem_t;

struct sdfPrimBoxLineElem_t;

struct sdfPrimTransElem_t;

struct sdfPrimTexflushPacket_t;

struct SdfPacketPatchLink;

struct sdfPrimGt3Elem_t;

struct sdfPrimGt4Elem_t;

struct SdfGsTwoVertexPacket;

struct sdfPrimSpriteElem_t;

struct sdfPrimFt3Elem_t;

struct sdfPrimFt4Elem_t;

struct sdfPrimG3Elem_t;

struct sdfPrimG4Elem_t;

struct sdfPrimBitBltElem_t;

struct sdfPrimF3Packet_t;

struct sdfPrimF4Packet_t;

struct sdfPrimBoxLinePacket_t;

struct sdfPrimTransPacket_t;

struct sdfPrimGt3Packet_t;

struct sdfPrimGt4Packet_t;

struct sdfPrimSpritePacket_t;

struct sdfPrimFt3Packet_t;

struct sdfPrimFt4Packet_t;

struct sdfPrimG3Packet_t;

struct sdfPrimG4Packet_t;

struct sdfPrimBitBltPacket_t;

struct sdfGsMemSlice_t;

struct sdfGrFrameSet_t;

struct sdfDrawTag_t;

struct sdfMemHandle_t;

struct SdfGraphCopyPacket;

struct sdfPreDrawTag_t;

/* The frame worker passes the node and queued buffer index. The registered
 * handlers project the node into their distinct complete packet owners. */
typedef void (*SdfPacketPatchCallback)();

/* Each GIF REGLIST vertex carries UV, its own RGBAQ, and packed XYZ2. */
typedef struct SdfGsGouraudTexturedVertex {
    u64 uv;
    u64 rgbaq;
    u64 xyz2;
} SdfGsGouraudTexturedVertex;

/* GIF REGLIST stream: PRIM, RGBAQ and two packed XYZ2 vertices. */
typedef struct SdfGsTwoVertexPayload {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    u64 rgbaq;
    u64 xyz2[2];
} SdfGsTwoVertexPayload;

/* GIF REGLIST stream: PRIM, RGBAQ, then UV/XYZ2 for each vertex. */
typedef struct SdfGsTexturedVertex {
    u64 uv;
    u64 xyz2;
} SdfGsTexturedVertex;

/* GIF REGLIST vertices carry their own RGBAQ and a packed XYZ2 position. */
typedef struct SdfGsGouraudVertex {
    u64 rgbaq;
    u64 xyz2;
} SdfGsGouraudVertex;

/* GIF stream for one flat-color triangle: tag/register list, PRIM, RGBAQ,
 * and the three XYZ2 values. */
typedef struct sdfPrimF3Elem_t {
    u64 gifTag;
    u64 gifRegisterList;
    u64 primitive;
    u64 rgbaq;
    u64 xyz2[3];
} sdfPrimF3Elem_t;

/* GIF stream for four flat-color vertices: PRIM, RGBAQ and four XYZ2 values. */
typedef struct sdfPrimF4Elem_t {
    u64 gifTag;
    u64 gifRegisterList;
    u64 primitive;
    u64 rgbaq;
    u64 xyz2[4];
} sdfPrimF4Elem_t;

/* GIF stream for a closed rectangle: its first XYZ2 value is repeated last. */
typedef struct sdfPrimBoxLineElem_t {
    u64 gifTag;
    u64 gifRegisterList;
    u64 primitive;
    u64 rgbaq;
    u64 xyz2[5];
} sdfPrimBoxLineElem_t;

#define SDF_PACKET_LIST_BYTES 0x20

#define SDF_GIF_ONE_REGISTER_WORD 0x10000000

#define SDF_GS_DEFAULT_ALPHA 0x44

#define SDF_GS_ALPHA_TEST 0x717FB

#define SDF_ALPHA_DMA_QWORDS 3

#define SDF_VIF_DIRECT_THREE_WORD 0x50000003

#define SDF_VIF_FLUSHE_HALFWORD 0x1000

#define SDF_GIF_TWO_AD_LOOPS_EOP 0x8002

extern volatile s8 sdfReqDrawDelayCount;

/* TEST and ALPHA form the two A+D writes shared by the blend builders. */
typedef struct sdfPrimTransElem_t {
    sdfGrTagStdReg_t test;
    sdfGrTagStdReg_t alpha;
} sdfPrimTransElem_t;

/* A single A+D write follows the texture-flush DMA/VIF/GIF header. */
typedef struct sdfPrimTexflushPacket_t {
    sdfGrPacketHeader_t header;
    sdfGrTagStdReg_t textureFlush;
} sdfPrimTexflushPacket_t;

typedef struct SdfPacketPatchLink {
    struct SdfPacketPatchLink *next;
    SdfPacketPatchCallback patch;
} SdfPacketPatchLink;

/* PRIM followed by 3 individually textured and colored vertices. */
typedef struct sdfPrimGt3Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    SdfGsGouraudTexturedVertex vertices[3];
} sdfPrimGt3Elem_t;

/* PRIM followed by 4 individually textured and colored vertices. */
typedef struct sdfPrimGt4Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    SdfGsGouraudTexturedVertex vertices[4];
} sdfPrimGt4Elem_t;

/* DMA/VIF header followed by the complete two-vertex GIF payload. */
typedef struct SdfGsTwoVertexPacket {
    u64 dmaTag;
    u64 vifCommands;
    SdfGsTwoVertexPayload drawing;
} SdfGsTwoVertexPacket;

typedef struct sdfPrimSpriteElem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    u64 rgbaq;
    SdfGsTexturedVertex vertices[2];
} sdfPrimSpriteElem_t;

/* GIF REGLIST stream: PRIM, one RGBAQ, then UV/XYZ2 for each vertex. */
typedef struct sdfPrimFt3Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    u64 rgbaq;
    SdfGsTexturedVertex vertices[3];
} sdfPrimFt3Elem_t;

/* GIF REGLIST stream: PRIM, one RGBAQ, then UV/XYZ2 for each vertex. */
typedef struct sdfPrimFt4Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    u64 rgbaq;
    SdfGsTexturedVertex vertices[4];
} sdfPrimFt4Elem_t;

/* PRIM followed by three independently colored vertices. */
typedef struct sdfPrimG3Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    SdfGsGouraudVertex vertices[3];
} sdfPrimG3Elem_t;

/* PRIM followed by four independently colored vertices. */
typedef struct sdfPrimG4Elem_t {
    u64 gifTag;
    u64 gifRegisters;
    u64 primitive;
    SdfGsGouraudVertex vertices[4];
} sdfPrimG4Elem_t;

/* The four A+D writes controlling a GS image transfer. */
typedef struct sdfPrimBitBltElem_t {
    sdfGrTagStdReg_t bitbltbuf;
    sdfGrTagStdReg_t trxpos;
    sdfGrTagStdReg_t trxreg;
    sdfGrTagStdReg_t trxdir;
} sdfPrimBitBltElem_t;

/* The caller prepends DMA/VIF words to the triangle GIF stream. Allocation
 * reserves 0x50 bytes; the final qword is not written by these helpers. */
typedef struct sdfPrimF3Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimF3Elem_t drawing;
    u64 unwrittenAllocationWord;
} sdfPrimF3Packet_t;

/* The allocation contains the DMA/VIF prefix and the entire quad GIF stream. */
typedef struct sdfPrimF4Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimF4Elem_t drawing;
} sdfPrimF4Packet_t;

/* The caller prepends DMA/VIF words. The final allocated qword is not written. */
typedef struct sdfPrimBoxLinePacket_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimBoxLineElem_t drawing;
    u64 unwrittenAllocationWord;
} sdfPrimBoxLinePacket_t;

extern struct sdfMemHandle_t *sdfGrPacketBuffHandle;

/* Fixed packets allocated by the paired blend and composite-quad renderers. */
typedef struct sdfPrimTransPacket_t {
    sdfGrPacketHeader_t header;
    sdfPrimTransElem_t registers;
} sdfPrimTransPacket_t;

typedef struct sdfPrimGt3Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimGt3Elem_t drawing;
} sdfPrimGt3Packet_t;

typedef struct sdfPrimGt4Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimGt4Elem_t drawing;
    u64 unwrittenTail; /* Final allocated word is not written by the builder. */
} sdfPrimGt4Packet_t;

typedef struct sdfPrimSpritePacket_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimSpriteElem_t drawing;
} sdfPrimSpritePacket_t;

typedef struct sdfPrimFt3Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimFt3Elem_t drawing;
} sdfPrimFt3Packet_t;

typedef struct sdfPrimFt4Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimFt4Elem_t drawing;
} sdfPrimFt4Packet_t;

typedef struct sdfPrimG3Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimG3Elem_t drawing;
    u64 unwrittenTail; /* Allocation rounds the REGLIST payload up to a qword. */
} sdfPrimG3Packet_t;

typedef struct sdfPrimG4Packet_t {
    u64 dmaTag;
    u64 vifCommands;
    sdfPrimG4Elem_t drawing;
    u64 unwrittenTail; /* Allocation rounds the REGLIST payload up to a qword. */
} sdfPrimG4Packet_t;

/* DMA/VIF and GIF header followed by the complete image-transfer register block. */
typedef struct sdfPrimBitBltPacket_t {
    sdfGrPacketHeader_t header;
    sdfPrimBitBltElem_t transfer;
} sdfPrimBitBltPacket_t;

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

/* Graph target: two color buffers followed by the auxiliary/depth buffer (0x14). */
typedef struct sdfGrFrameSet_t {
    s16 width;
    s16 unk2;
    s16 height;
    u8 bufferFormat;
    u8 auxiliaryFormat;
    sdfGsMemSlice_t *buffers[3];
} sdfGrFrameSet_t;

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

/* General-heap allocation descriptor (0x10); links bound the represented span. */
typedef struct sdfMemHandle_t {
    struct sdfMemHandle_t *prev; /* 0x00 */
    struct sdfMemHandle_t *next; /* 0x04 */
    s32 ptr;                  /* 0x08: start of represented span */
    u16 stat;                 /* 0x0C: free, used, or end sentinel */
    s16 mlock;                /* 0x0E: -1 for sentinels */
} sdfMemHandle_t;

/* Metadata node plus the extended local-to-local image-copy payload (0x70). */
typedef struct SdfGraphCopyPacket {
    SdfPacketPatchLink link;
    u32 resourceIndexXor; /* Selects the source resource for the queued buffer. */
    u32 unk0C;
    sdfPrimBitBltPacket_t draw;
} SdfGraphCopyPacket;

/* Complete linked-packet list, distinct from the 0x20-byte DMA list head. */
typedef struct sdfPreDrawTag_t {
    u32 next;
    SdfPacketPatchLink *head;
    SdfPacketPatchLink *tail;
    u32 pad;
} sdfPreDrawTag_t;

extern s32 sdfGrPacketBuffSize;

extern s32 D_00551870[2];

extern s32 sdfGrPacketBuffPtr;

extern s32 sdfGrPacketBuffLimit;

extern s32 sdfDelayedResrcSemaId;

extern s32 sdfDrawReqQueueIndex;

extern sdfGrFrameSet_t D_00529388;

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

s32 sdfGrAllocPacketBuff(s32 size);

void sdfDrawTagInit(sdfDrawTag_t *list);

void sdfDrawTagLink(sdfDrawTag_t *list, u32 packet);

void sdfPreDrawTagLink(sdfPreDrawTag_t *list, SdfPacketPatchLink *node);

sdfDrawTag_t *sdfPrimAllocDrawTag(s32 (*allocator)(s32));

void sdfPrimLinkSimplePacket(s32 listAddress, void (*initialize)(s32), s32 packetBytes, s32 (*allocator)(s32));

void sdfPrimSetOpaqueElem(sdfPrimTransElem_t *packet);

void sdfPrimSetOpaque2Elem(sdfPrimTransElem_t *packet);

void sdfPrimSetOpaquePacket(sdfPrimTransPacket_t *packet);

void sdfPrimSetOpaque2Packet(sdfPrimTransPacket_t *packet);

void sdfPrimSetSemiTransElem(sdfPrimTransElem_t *packet);

void sdfPrimSetSemiTrans2Elem(sdfPrimTransElem_t *packet);

void sdfPrimSetSemiTransPacket(sdfPrimTransPacket_t *packet);

void sdfPrimSetSemiTrans2Packet(sdfPrimTransPacket_t *packet);

void sdfPrimSetAddTransElem(sdfPrimTransElem_t *packet);

void sdfPrimSetAddTrans2Elem(sdfPrimTransElem_t *packet);

void sdfPrimSetAddTransPacket(sdfPrimTransPacket_t *packet);

void sdfPrimSetAddTrans2Packet(sdfPrimTransPacket_t *packet);

void sdfPrimSetSubTransElem(sdfPrimTransElem_t *packet);

void sdfPrimSetSubTrans2Elem(sdfPrimTransElem_t *packet);

void sdfPrimSetSubTransPacket(sdfPrimTransPacket_t *packet);

void sdfPrimSetSubTrans2Packet(sdfPrimTransPacket_t *packet);

void sdfPrimSetTexflushElem(sdfGrTagStdReg_t *write);

void sdfPrimSetTexflushPacket(sdfPrimTexflushPacket_t *packet);

void sdfPrimSetBitBltElem(sdfPrimBitBltElem_t *packet, u32 destinationBufferAddress, s32 destinationBufferWidth,
                  s64 destinationFormat, s64 destinationX, s64 destinationY,
                  u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                  s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight, s32 transferDirection);

void sdfPrimSetBitBltPacket(sdfPrimBitBltPacket_t *packet, u32 destinationBufferAddress, s32 destinationBufferWidth,
                                    s64 destinationFormat, s64 destinationX, s64 destinationY,
                                    u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                                    s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight, s32 transferDirection);

void sdfPrimLinkBitBltPacket(s32 packetList, u32 destinationBufferAddress, s32 destinationBufferWidth,
                             s64 destinationFormat, s64 destinationX, s64 destinationY,
                             u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                             s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight,
                             s32 transferDirection, s32 (*allocator)(s32));

void sdfPrimFrameMovePredraw(SdfGraphCopyPacket *packet, s32 entryIndex);

void sdfPrimLinkFrameMovePacket(sdfDrawTag_t *drawList, sdfPreDrawTag_t *linkedList,
                   sdfGsMemSlice_t *destination, s32 destinationX, s32 destinationY,
                   s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight,
                   s32 resourceIndexXor, s32 (*allocPacket)(s32));

void sdfPrimSetSpriteElem(sdfPrimSpriteElem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 depth);

void sdfPrimLinkSprite(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetF3Elem(sdfPrimF3Elem_t *dst, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 depth);

void sdfPrimLinkF3(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1,
                   s32 y1, s32 x2, s32 y2, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetF4Elem(sdfPrimF4Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 depth);

void sdfPrimLinkF4(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetG3Elem(sdfPrimG3Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 depth);

void sdfPrimLinkG3(sdfDrawTag_t *list, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetG4Elem(sdfPrimG4Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 x3, s32 y3, s32 color3, s32 depth);

void sdfPrimLinkG4(s32 list, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1,
                   s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 x3,
                   s32 y3, s32 color3, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetFt3Elem(sdfPrimFt3Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2, s32 u2, s32 v2, s32 depth);

void sdfPrimLinkFt3(s32 list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetFt4Elem(sdfPrimFt4Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2, s32 u2, s32 v2,
                   s32 x3, s32 y3, s32 u3, s32 v3, s32 depth);

void sdfPrimLinkFt4(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 x3, s32 y3, s32 u3, s32 v3, s32 depth,
                   s32 (*alloc)(s32));

void sdfPrimSetGt3Elem(sdfPrimGt3Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 color0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 color1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 color2, s32 depth);

void sdfPrimLinkGt3(s32 list, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 color0, s32 x1, s32 y1, s32 u1, s32 v1, s32 color1,
                   s32 x2, s32 y2, s32 u2, s32 v2, s32 color2, s32 depth,
                   s32 (*alloc)(s32));

void sdfPrimSetGt4Elem(sdfPrimGt4Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 color0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 color1, s32 x2, s32 y2, s32 u2,
                   s32 v2, s32 color2, s32 x3, s32 y3, s32 u3, s32 v3, s32 color3, s32 depth);

void sdfPrimLinkGt4(sdfDrawTag_t *list, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 color0, s32 x1, s32 y1, s32 u1, s32 v1, s32 color1,
                   s32 x2, s32 y2, s32 u2, s32 v2, s32 color2, s32 x3, s32 y3,
                   s32 u3, s32 v3, s32 color3, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetRectElem(SdfGsTwoVertexPayload *packet, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth);

void sdfPrimLinkRect(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetLineElem(SdfGsTwoVertexPayload *packet, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth);

void sdfPrimLinkLine(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32));

void sdfPrimSetBoxLineElem(sdfPrimBoxLineElem_t *dst, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth);

void sdfPrimLinkBoxLine(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32));

sdfDrawTag_t *sdfPrimAllocDrawTag(s32 (*allocator)(s32)) {
    sdfDrawTag_t *list;

    if (allocator == NULL) {
        allocator = sdfGrAllocPacketBuff;
    }
    list = (sdfDrawTag_t *)(u32)allocator(SDF_PACKET_LIST_BYTES);
    sdfDrawTagInit(list);
    return list;
}

void sdfPrimLinkSimplePacket(s32 listAddress, void (*initialize)(s32), s32 packetBytes, s32 (*allocator)(s32)) {
    s32 packetAddress;

    if (allocator == NULL) {
        allocator = sdfGrAllocPacketBuff;
    }
    packetAddress = allocator(packetBytes);
    initialize(packetAddress);
    sdfDrawTagLink(listAddress, packetAddress);
}

void sdfPrimSetOpaqueElem(sdfPrimTransElem_t *packet) {
    packet->test.data = SDF_GS_ALPHA_TEST;
    packet->test.reg = SDF_GS_TEST_1;
    packet->alpha.data = SDF_GS_DEFAULT_ALPHA;
    packet->alpha.reg = SDF_GS_ALPHA_1;
}

void sdfPrimSetOpaque2Elem(sdfPrimTransElem_t *packet) {
    packet->test.data = SDF_GS_ALPHA_TEST;
    packet->test.reg = SDF_GS_TEST_2;
    packet->alpha.data = SDF_GS_DEFAULT_ALPHA;
    packet->alpha.reg = SDF_GS_ALPHA_2;
}

void sdfPrimSetOpaquePacket(sdfPrimTransPacket_t *packet) {
    sdfPrimSetOpaqueElem(&packet->registers);
    packet->header.dmaWords.tag = SDF_ALPHA_DMA_QWORDS;
    packet->header.dmaWords.vif = (((u64)SDF_VIF_DIRECT_THREE_WORD << 16 | SDF_VIF_FLUSHE_HALFWORD) << 16);
    packet->header.giftag.parm = (((u64)SDF_GIF_ONE_REGISTER_WORD << 32) | SDF_GIF_TWO_AD_LOOPS_EOP);
    packet->header.giftag.regs = SDF_GIF_REGISTER_AD;
}

void sdfPrimSetOpaque2Packet(sdfPrimTransPacket_t *packet) {
    sdfPrimSetOpaque2Elem(&packet->registers);
    packet->header.dmaWords.tag = SDF_ALPHA_DMA_QWORDS;
    packet->header.dmaWords.vif = (((u64)SDF_VIF_DIRECT_THREE_WORD << 16 | SDF_VIF_FLUSHE_HALFWORD) << 16);
    packet->header.giftag.parm = (((u64)SDF_GIF_ONE_REGISTER_WORD << 32) | SDF_GIF_TWO_AD_LOOPS_EOP);
    packet->header.giftag.regs = SDF_GIF_REGISTER_AD;
}

void sdfPrimSetSemiTransElem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x717FB;
    packet->test.reg = 0x47;
    packet->alpha.data = 0x44;
    packet->alpha.reg = 0x42;
}

void sdfPrimSetSemiTrans2Elem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x717FB;
    packet->test.reg = 0x48;
    packet->alpha.data = 0x44;
    packet->alpha.reg = 0x43;
}

void sdfPrimSetSemiTransPacket(sdfPrimTransPacket_t *packet) {
    sdfPrimSetSemiTransElem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetSemiTrans2Packet(sdfPrimTransPacket_t *packet) {
    sdfPrimSetSemiTrans2Elem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetAddTransElem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x71801;
    packet->test.reg = 0x47;
    packet->alpha.data = 0x48;
    packet->alpha.reg = 0x42;
}

void sdfPrimSetAddTrans2Elem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x71801;
    packet->test.reg = 0x48;
    packet->alpha.data = 0x48;
    packet->alpha.reg = 0x43;
}

void sdfPrimSetAddTransPacket(sdfPrimTransPacket_t *packet) {
    sdfPrimSetAddTransElem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetAddTrans2Packet(sdfPrimTransPacket_t *packet) {
    sdfPrimSetAddTrans2Elem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetSubTransElem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x71801;
    packet->test.reg = 0x47;
    packet->alpha.data = 0x42;
    packet->alpha.reg = 0x42;
}

void sdfPrimSetSubTrans2Elem(sdfPrimTransElem_t *packet) {
    packet->test.data = 0x71801;
    packet->test.reg = 0x48;
    packet->alpha.data = 0x42;
    packet->alpha.reg = 0x43;
}

void sdfPrimSetSubTransPacket(sdfPrimTransPacket_t *packet) {
    sdfPrimSetSubTransElem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetSubTrans2Packet(sdfPrimTransPacket_t *packet) {
    sdfPrimSetSubTrans2Elem(&packet->registers);
    packet->header.dmaWords.tag = 3;
    packet->header.dmaWords.vif = (((u64)0x50000003 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8002);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetTexflushElem(sdfGrTagStdReg_t *write) {
    write->data = 0;
    write->reg = SDF_GS_TEXFLUSH;
}

void sdfPrimSetTexflushPacket(sdfPrimTexflushPacket_t *packet) {
    sdfPrimSetTexflushElem(&packet->textureFlush);
    packet->header.dmaWords.tag = 2;
    packet->header.dmaWords.vif = (((u64)0x50000002 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8001);
    packet->header.giftag.regs = 0xE;
}

void sdfPrimSetBitBltElem(sdfPrimBitBltElem_t *packet, u32 destinationBufferAddress, s32 destinationBufferWidth,
                  s64 destinationFormat, s64 destinationX, s64 destinationY,
                  u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                  s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight, s32 transferDirection) {
    packet->bitbltbuf.data = (destinationFormat << 56) |
        ((u64)(destinationBufferWidth >> 6) << 48) |
        ((u64)(destinationBufferAddress >> 6) << 32) |
        (sourceFormat << 24) | ((sourceBufferWidth >> 6) << 16) |
        (sourceBufferAddress >> 6);
    packet->bitbltbuf.reg = SDF_GS_BITBLTBUF;
    packet->trxpos.data = (destinationY << 48) | (destinationX << 32) |
        (sourceY << 16) | sourceX;
    packet->trxpos.reg = SDF_GS_TRXPOS;
    packet->trxreg.data = transferWidth | ((u64)transferHeight << 32);
    packet->trxreg.reg = SDF_GS_TRXREG;
    packet->trxdir.data = transferDirection;
    packet->trxdir.reg = SDF_GS_TRXDIR;
}

void sdfPrimSetBitBltPacket(sdfPrimBitBltPacket_t *packet, u32 destinationBufferAddress, s32 destinationBufferWidth,
                                    s64 destinationFormat, s64 destinationX, s64 destinationY,
                                    u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                                    s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight, s32 transferDirection) {
    packet->header.dmaWords.tag = 5;
    packet->header.dmaWords.vif = (((u64)0x50000005 << 16 | 0x1000) << 16);
    packet->header.giftag.parm = (((u64)0x10000000 << 32) | 0x8004);
    packet->header.giftag.regs = 0xE;
    sdfPrimSetBitBltElem(&packet->transfer, destinationBufferAddress, destinationBufferWidth,
                  destinationFormat, destinationX, destinationY, sourceBufferAddress, sourceBufferWidth,
                  sourceFormat, sourceX, sourceY, transferWidth, transferHeight, transferDirection);
}

void sdfPrimLinkBitBltPacket(s32 packetList, u32 destinationBufferAddress, s32 destinationBufferWidth,
                             s64 destinationFormat, s64 destinationX, s64 destinationY,
                             u32 sourceBufferAddress, s32 sourceBufferWidth, s32 sourceFormat,
                             s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight,
                             s32 transferDirection, s32 (*allocator)(s32)) {
    s32 packetAddress;

    if (allocator == NULL) {
        allocator = sdfGrAllocPacketBuff;
    }
    packetAddress = allocator(0x60);
    sdfPrimSetBitBltPacket((sdfPrimBitBltPacket_t *)packetAddress, destinationBufferAddress, destinationBufferWidth,
                                    destinationFormat, destinationX, destinationY, sourceBufferAddress, sourceBufferWidth,
                                    sourceFormat, sourceX, sourceY, transferWidth, transferHeight, transferDirection);
    sdfDrawTagLink(packetList, packetAddress);
}

void sdfPrimFrameMovePredraw(SdfGraphCopyPacket *packet, s32 entryIndex) {
    packet->draw.transfer.bitbltbuf.data =
        (packet->draw.transfer.bitbltbuf.data & ~0x3FFF) |
        (u64)(u32)(D_00529388.buffers[entryIndex ^ packet->resourceIndexXor]->addr >> 6);
}

void sdfPrimLinkFrameMovePacket(sdfDrawTag_t *drawList, sdfPreDrawTag_t *linkedList,
                   sdfGsMemSlice_t *destination, s32 destinationX, s32 destinationY,
                   s32 sourceX, s32 sourceY, s32 transferWidth, s32 transferHeight,
                   s32 resourceIndexXor, s32 (*allocPacket)(s32)) {
    SdfGraphCopyPacket *packet;
    sdfPrimBitBltPacket_t *drawPacket;

    if (allocPacket == NULL) {
        allocPacket = sdfGrAllocPacketBuff;
    }
    packet = (SdfGraphCopyPacket *)allocPacket(0x70);
    packet->resourceIndexXor = resourceIndexXor;
    packet->link.patch = sdfPrimFrameMovePredraw;
    drawPacket = &packet->draw;

    sdfPrimSetBitBltPacket(
        drawPacket, destination->addr, destination->w,
        destination->pixelFormat, destinationX, destinationY, D_00529388.buffers[0]->addr,
        D_00529388.width, D_00529388.bufferFormat, sourceX, sourceY, transferWidth,
        transferHeight, 2);
    sdfPreDrawTagLink(linkedList, &packet->link);
    sdfDrawTagLink(drawList, (s32)drawPacket);
}

void sdfPrimSetSpriteElem(sdfPrimSpriteElem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0x6400000000008001ULL;
    packet->gifRegisters = 0x535310;
    packet->primitive = (u32)(primitive | 0x116);
    packet->rgbaq = (u32)color | ((u64)0xFE00 << 46);
    packet->vertices[0].uv = (u0 & 0xFFFF) | (v0 << 16);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].uv = (u1 & 0xFFFF) | (v1 << 16);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
}

void sdfPrimLinkSprite(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimSpritePacket_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimSpritePacket_t *)alloc(0x50);
    packet->dmaTag = 0x20000004;
    packet->vifCommands = (((u64)0x50000004 << 16) | 0x1000) << 16;
    sdfPrimSetSpriteElem(&packet->drawing, color, primitive, x0, y0, u0, v0, x1, y1, u1, v1, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetF3Elem(sdfPrimF3Elem_t *dst, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 depth) {
    u64 depthHigh = (u64)depth << 32;
    u32 first = ((u32)x0 & 0xFFFF) | ((u32)y0 << 16);
    u32 second = ((u32)x1 & 0xFFFF) | ((u32)y1 << 16);
    u32 third = ((u32)x2 & 0xFFFF) | ((u32)y2 << 16);

    dst->gifTag = 0x6400000000008001ULL;
    dst->gifRegisterList = 0xF55510;
    dst->primitive = (u32)primitive | 0x104;
    dst->rgbaq = (u32)color | 0x3F80000000000000ULL;
    dst->xyz2[0] = first | depthHigh;
    dst->xyz2[1] = second | depthHigh;
    dst->xyz2[2] = third | depthHigh;
}

void sdfPrimLinkF3(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1,
                   s32 y1, s32 x2, s32 y2, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimF3Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimF3Packet_t *)alloc(0x50);
    packet->dmaTag = 0x20000004;
    packet->vifCommands = (((u64)0x50000004 << 16) | 0x1000) << 16;
    sdfPrimSetF3Elem(&packet->drawing, color, primitive, x0, y0, x1, y1, x2, y2, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetF4Elem(sdfPrimF4Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0x6400000000008001ULL;
    packet->gifRegisterList = 0x555510;
    packet->primitive = (u32)(primitive | 0x104);
    packet->rgbaq = (u32)color | ((u64)0xFE00 << 46);
    packet->xyz2[0] = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->xyz2[1] = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->xyz2[2] = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
    packet->xyz2[3] = (u32)((x3 & 0xFFFF) | (y3 << 16)) | depthHigh;
}

void sdfPrimLinkF4(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 x1, s32 y1, s32 x2, s32 y2, s32 x3, s32 y3, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimF4Packet_t *packet;

    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimF4Packet_t *)alloc(0x50);
    packet->dmaTag = 0x20000004ULL;
    packet->vifCommands = 0x5000000410000000ULL;
    sdfPrimSetF4Elem(&packet->drawing, color, primitive, x0, y0, x1, y1, x2, y2, x3, y3, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetG3Elem(sdfPrimG3Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0x8400000000008001ULL;
    packet->gifRegisters = 0xFFFFFFFFF5151510ULL;
    packet->primitive = (u32)(primitive | 0x10C);
    packet->vertices[0].rgbaq = (u32)color0 | ((u64)0xFE00 << 46);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].rgbaq = (u32)color1 | ((u64)0xFE00 << 46);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].rgbaq = (u32)color2 | ((u64)0xFE00 << 46);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
}

void sdfPrimLinkG3(sdfDrawTag_t *list, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimG3Packet_t *packet;

    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimG3Packet_t *)alloc(0x60);
    packet->dmaTag = 0x20000005ULL;
    packet->vifCommands = 0x5000000510000000ULL;
    sdfPrimSetG3Elem(&packet->drawing, primitive, x0, y0, color0, x1, y1, color1, x2, y2, color2, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetG4Elem(sdfPrimG4Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1, s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 x3, s32 y3, s32 color3, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0xA400000000008001ULL;
    packet->gifRegisters = 0xF515151510ULL;
    packet->primitive = (u32)(primitive | 0x10C);
    packet->vertices[0].rgbaq = (u32)color0 | ((u64)0xFE00 << 46);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].rgbaq = (u32)color1 | ((u64)0xFE00 << 46);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].rgbaq = (u32)color2 | ((u64)0xFE00 << 46);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
    packet->vertices[3].rgbaq = (u32)color3 | ((u64)0xFE00 << 46);
    packet->vertices[3].xyz2 = (u32)((x3 & 0xFFFF) | (y3 << 16)) | depthHigh;
}

void sdfPrimLinkG4(s32 list, s32 primitive, s32 x0, s32 y0, s32 color0, s32 x1,
                   s32 y1, s32 color1, s32 x2, s32 y2, s32 color2, s32 x3,
                   s32 y3, s32 color3, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimG4Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimG4Packet_t *)alloc(0x70);
    packet->dmaTag = 0x20000006;
    packet->vifCommands = (((u64)0x50000006 << 16) | 0x1000) << 16;
    sdfPrimSetG4Elem(&packet->drawing, primitive, x0, y0, color0, x1, y1, color1, x2, y2, color2, x3, y3, color3, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetFt3Elem(sdfPrimFt3Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2, s32 u2, s32 v2, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0x8400000000008001ULL;
    packet->gifRegisters = 0x53535310;
    packet->primitive = (u32)(primitive | 0x114);
    packet->rgbaq = (u32)color | ((u64)0xFE00 << 46);
    packet->vertices[0].uv = (u0 & 0xFFFF) | (v0 << 16);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].uv = (u1 & 0xFFFF) | (v1 << 16);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].uv = (u2 & 0xFFFF) | (v2 << 16);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
}

void sdfPrimLinkFt3(s32 list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimFt3Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimFt3Packet_t *)alloc(0x60);
    packet->dmaTag = 0x20000005;
    packet->vifCommands = (((u64)0x50000005 << 16) | 0x1000) << 16;
    sdfPrimSetFt3Elem(&packet->drawing, color, primitive, x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetFt4Elem(sdfPrimFt4Elem_t *packet, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2, s32 u2, s32 v2,
                   s32 x3, s32 y3, s32 u3, s32 v3, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0xA400000000008001ULL;
    packet->gifRegisters = 0x5353535310ULL;
    packet->primitive = (u32)(primitive | 0x114);
    packet->rgbaq = (u32)color | ((u64)0xFE00 << 46);
    packet->vertices[0].uv = (u0 & 0xFFFF) | (v0 << 16);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].uv = (u1 & 0xFFFF) | (v1 << 16);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].uv = (u2 & 0xFFFF) | (v2 << 16);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
    packet->vertices[3].uv = (u3 & 0xFFFF) | (v3 << 16);
    packet->vertices[3].xyz2 = (u32)((x3 & 0xFFFF) | (y3 << 16)) | depthHigh;
}

void sdfPrimLinkFt4(sdfDrawTag_t *list, s32 color, s32 primitive, s32 x0, s32 y0, s32 u0,
                   s32 v0, s32 x1, s32 y1, s32 u1, s32 v1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 x3, s32 y3, s32 u3, s32 v3, s32 depth,
                   s32 (*alloc)(s32)) {
    sdfPrimFt4Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimFt4Packet_t *)alloc(0x70);
    packet->dmaTag = 0x20000006;
    packet->vifCommands = (((u64)0x50000006 << 16) | 0x1000) << 16;
    sdfPrimSetFt4Elem(&packet->drawing, color, primitive, x0, y0, u0, v0, x1, y1, u1, v1, x2, y2, u2, v2, x3, y3, u3, v3, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetGt3Elem(sdfPrimGt3Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 color0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 color1, s32 x2, s32 y2,
                   s32 u2, s32 v2, s32 color2, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0xA400000000008001ULL;
    packet->gifRegisters = 0x5135135130ULL;
    packet->primitive = (u32)(primitive | 0x11C);
    packet->vertices[0].uv = (u0 & 0xFFFF) | (v0 << 16);
    packet->vertices[0].rgbaq = (u32)color0 | ((u64)0xFE00 << 46);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].uv = (u1 & 0xFFFF) | (v1 << 16);
    packet->vertices[1].rgbaq = (u32)color1 | ((u64)0xFE00 << 46);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].uv = (u2 & 0xFFFF) | (v2 << 16);
    packet->vertices[2].rgbaq = (u32)color2 | ((u64)0xFE00 << 46);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
}

void sdfPrimLinkGt3(s32 list, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 color0, s32 x1, s32 y1, s32 u1, s32 v1, s32 color1,
                   s32 x2, s32 y2, s32 u2, s32 v2, s32 color2, s32 depth,
                   s32 (*alloc)(s32)) {
    sdfPrimGt3Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimGt3Packet_t *)alloc(0x70);
    packet->dmaTag = 0x20000006;
    packet->vifCommands = (((u64)0x50000006 << 16) | 0x1000) << 16;
    sdfPrimSetGt3Elem(&packet->drawing, primitive, x0, y0, u0, v0, color0, x1, y1, u1, v1, color1, x2, y2, u2, v2, color2, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetGt4Elem(sdfPrimGt4Elem_t *packet, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0, s32 color0,
                   s32 x1, s32 y1, s32 u1, s32 v1, s32 color1, s32 x2, s32 y2, s32 u2,
                   s32 v2, s32 color2, s32 x3, s32 y3, s32 u3, s32 v3, s32 color3, s32 depth) {
    u64 depthHigh = (u64)depth << 32;

    packet->gifTag = 0xE400000000008001ULL;
    packet->gifRegisters = 0xF5135135135130ULL;
    packet->primitive = (u32)(primitive | 0x11C);
    packet->vertices[0].uv = (u0 & 0xFFFF) | (v0 << 16);
    packet->vertices[0].rgbaq = (u32)color0 | ((u64)0xFE00 << 46);
    packet->vertices[0].xyz2 = (u32)((x0 & 0xFFFF) | (y0 << 16)) | depthHigh;
    packet->vertices[1].uv = (u1 & 0xFFFF) | (v1 << 16);
    packet->vertices[1].rgbaq = (u32)color1 | ((u64)0xFE00 << 46);
    packet->vertices[1].xyz2 = (u32)((x1 & 0xFFFF) | (y1 << 16)) | depthHigh;
    packet->vertices[2].uv = (u2 & 0xFFFF) | (v2 << 16);
    packet->vertices[2].rgbaq = (u32)color2 | ((u64)0xFE00 << 46);
    packet->vertices[2].xyz2 = (u32)((x2 & 0xFFFF) | (y2 << 16)) | depthHigh;
    packet->vertices[3].uv = (u3 & 0xFFFF) | (v3 << 16);
    packet->vertices[3].rgbaq = (u32)color3 | ((u64)0xFE00 << 46);
    packet->vertices[3].xyz2 = (u32)((x3 & 0xFFFF) | (y3 << 16)) | depthHigh;
}

void sdfPrimLinkGt4(sdfDrawTag_t *list, s32 primitive, s32 x0, s32 y0, s32 u0, s32 v0,
                   s32 color0, s32 x1, s32 y1, s32 u1, s32 v1, s32 color1,
                   s32 x2, s32 y2, s32 u2, s32 v2, s32 color2, s32 x3, s32 y3,
                   s32 u3, s32 v3, s32 color3, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimGt4Packet_t *packet;
    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimGt4Packet_t *)alloc(0x90);
    packet->dmaTag = 0x20000008;
    packet->vifCommands = (((u64)0x50000008 << 16) | 0x1000) << 16;
    sdfPrimSetGt4Elem(&packet->drawing, primitive, x0, y0, u0, v0, color0, x1, y1, u1, v1, color1, x2, y2, u2, v2, color2, x3, y3, u3, v3, color3, depth);
    sdfDrawTagLink(list, (s32)packet);
}

void sdfPrimSetRectElem(SdfGsTwoVertexPayload *packet, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth) {
    u64 depthHigh = (u64)(u32)depth << 32;
    u32 first = ((u32)left & 0xFFFF) | ((u32)top << 16);
    u32 second = ((u32)right & 0xFFFF) | ((u32)bottom << 16);

    packet->gifTag = 0x4400000000008001ULL;
    packet->gifRegisters = 0x5510;
    packet->primitive = (u32)primitive | 0x106;
    packet->rgbaq = (u32)color | 0x3F80000000000000ULL;
    packet->xyz2[0] = first | depthHigh;
    packet->xyz2[1] = second | depthHigh;
}

void sdfPrimLinkRect(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32)) {
    SdfGsTwoVertexPacket *packet;

    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (SdfGsTwoVertexPacket *)alloc(0x40);
    packet->dmaTag = 0x20000003ULL;
    packet->vifCommands = 0x5000000310000000ULL;
    sdfPrimSetRectElem(&packet->drawing, color, primitive, left, top, right, bottom, depth);
    sdfDrawTagLink(list, (u32)packet);
}

void sdfPrimSetLineElem(SdfGsTwoVertexPayload *packet, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth) {
    u64 depthHigh = (u64)(u32)depth << 32;
    u32 first = ((u32)left & 0xFFFF) | ((u32)top << 16);
    u32 second = ((u32)right & 0xFFFF) | ((u32)bottom << 16);

    packet->gifTag = 0x4400000000008001ULL;
    packet->gifRegisters = 0x5510;
    packet->primitive = (u32)primitive | 0x101;
    packet->rgbaq = (u32)color | 0x3F80000000000000ULL;
    packet->xyz2[0] = first | depthHigh;
    packet->xyz2[1] = second | depthHigh;
}

void sdfPrimLinkLine(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32)) {
    SdfGsTwoVertexPacket *packet;

    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (SdfGsTwoVertexPacket *)alloc(0x40);
    packet->dmaTag = 0x20000003ULL;
    packet->vifCommands = 0x5000000310000000ULL;
    sdfPrimSetLineElem(&packet->drawing, color, primitive, left, top, right, bottom, depth);
    sdfDrawTagLink(list, (u32)packet);
}

void sdfPrimSetBoxLineElem(sdfPrimBoxLineElem_t *dst, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth) {
    u64 depthHigh = (u64)(u32)depth << 32;
    u32 x0 = (u32)left & 0xFFFF;
    u32 x1 = (u32)right & 0xFFFF;
    u32 y0 = (u32)top << 16;
    u32 y1 = (u32)bottom << 16;

    dst->gifTag = 0x8400000000008001ULL;
    dst->gifRegisterList = (s32)0xF5555510;
    dst->primitive = (u32)primitive | 0x102;
    dst->rgbaq = (u32)color | 0x3F80000000000000ULL;
    dst->xyz2[0] = (x0 | y0) | depthHigh;
    dst->xyz2[1] = (x1 | y0) | depthHigh;
    dst->xyz2[2] = (x1 | y1) | depthHigh;
    dst->xyz2[3] = (x0 | y1) | depthHigh;
    dst->xyz2[4] = (x0 | y0) | depthHigh;
}

void sdfPrimLinkBoxLine(sdfDrawTag_t *list, s32 color, s32 primitive, s32 left, s32 top, s32 right, s32 bottom, s32 depth, s32 (*alloc)(s32)) {
    sdfPrimBoxLinePacket_t *packet;

    if (alloc == NULL) {
        alloc = sdfGrAllocPacketBuff;
    }
    packet = (sdfPrimBoxLinePacket_t *)alloc(0x60);
    packet->dmaTag = 0x20000005ULL;
    packet->vifCommands = 0x5000000510000000ULL;
    sdfPrimSetBoxLineElem(&packet->drawing, color, primitive, left, top, right, bottom, depth);
    sdfDrawTagLink(list, (s32)packet);
}

