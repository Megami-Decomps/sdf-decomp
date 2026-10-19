#ifndef LIBSDF_H
#define LIBSDF_H

/* sdf engine core: math, the chip and heap allocators, GS memory, textures,
 * materials and draw tags. Sizes and offsets are verified against SLUS_209.11;
 * see include/README.md for the evidence per struct. */

#include "common.h"
#include "sdf_types.h"

typedef struct sdfChipPageInfo_tag sdfChipPageInfo_tag;
typedef struct sdfChipCB_tag sdfChipCB_tag;
typedef struct sdfMemHandle_t sdfMemHandle_t;
typedef struct sdfGsMemSlice_t sdfGsMemSlice_t;
typedef struct sdfTexHandle_t sdfTexHandle_t;
typedef struct sdfTexPacket_t sdfTexPacket_t;
typedef struct sdfDrawTag_t sdfDrawTag_t;
typedef struct sdfPreDrawTag_t sdfPreDrawTag_t;
typedef struct sdfMat_t sdfMat_t;
typedef struct sdfMatUvwParm_t sdfMatUvwParm_t;
typedef struct sdfMatPacket_t sdfMatPacket_t;
typedef struct sdfPreDrawHook_t sdfPreDrawHook_t;
typedef struct sdfModelPacketHeader_t sdfModelPacketHeader_t;
typedef struct sdfItemArray_t sdfItemArray_t;
typedef struct sdfGrOt_t sdfGrOt_t;
typedef struct sdfPacState_t sdfPacState_t;

/* Bodies in sdf_lighting.h and draw3d.h; draw tags only need pointers here. */
struct sdf3DLightParallelPacket_t;
struct sdf3DFogPacket_t;

typedef struct
{
    f32 r;
    f32 g;
    f32 b;
    f32 a;
} sdfColor_t;

typedef struct
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} sdfColor32_t;


/* The id nibble of sdfDmaTag_t.id. REFE, REF and REFS name their data at
 * addr; the rest carry it after the tag. */
enum sdf_dma_tag_id
{
    SDF_DMA_TAG_ID_REFE = 0,
    SDF_DMA_TAG_ID_CNT = 1,
    SDF_DMA_TAG_ID_NEXT = 2,
    SDF_DMA_TAG_ID_REF = 3,
    SDF_DMA_TAG_ID_REFS = 4,
    SDF_DMA_TAG_ID_CALL = 5,
    SDF_DMA_TAG_ID_RET = 6,
    SDF_DMA_TAG_ID_END = 7
};


/* A resource whose free is deferred to the end of the frame: the free function
 * plus the head of the list of blocks waiting for it. */
typedef void (*sdfDelayedResrcFreeFunc)();

typedef struct
{
    sdfDelayedResrcFreeFunc freeFuncPtr;
    void *chunk;
} sdfDelayedResrcCB_t;

/* sdfChip: fixed-size chips carved out of 4KB pages. One control block per
 * size class, seven of them, selected by plzcw on the request size. */
struct sdfChipPageInfo_tag
{
    sdfChipPageInfo_tag *next;
    void *page;
    void *alloc; /* grow or free-list handler, called through +8 */
    sdfChipCB_tag *owner;
    void *freeLink;
    s16 allocCount;
    s16 growLeft;
};

struct sdfChipCB_tag
{
    sdfChipPageInfo_tag *currentPage;
    sdfChipPageInfo_tag *freeLink;
    s16 chipSize;
    s16 chipsPerPage;
};

/* Debug record of one chip allocation; names unconfirmed. */
typedef struct sdfChipRecord_t
{
    struct sdfChipRecord_t *next;
    void *ptr;
    char *file;
    s32 line;
} sdfChipRecord_t;

/* Five totals followed by the seven per-size-class allocation counters. */
typedef struct
{
    s32 total;
    s32 freeSize;
    s32 pageCount;
    s32 freePageCount;
    s32 fragmentedPageCount;
    s32 severalAllocCount[7];
} sdfChipInfo_t;

/* sdfMem: a doubly linked list over one arena. A block's size is the next
 * handle's ptr minus its own, so the list always ends with a sentinel. */
struct sdfMemHandle_t
{
    sdfMemHandle_t *prev;
    sdfMemHandle_t *next;
    void *ptr;
    u16 stat; /* sdf_mem_stat; sdfMemGetType returns it */
    u16 mlock;
};

/* sdfMemHandle_t.stat. sdfMemAllocDirect takes the first FREE handle, stops at
 * the END sentinel, and stamps the one it took USED; sdfMemFreeDirect puts it
 * back to FREE. */
enum sdf_mem_stat
{
    SDF_MEM_STAT_FREE = 0,
    SDF_MEM_STAT_USED = 1,
    SDF_MEM_STAT_END = 2
};

/* The heap's control block, sdfMemCB. Both guard handles are embedded, which
 * is what lets the first and last block compute a size the same way; the HD
 * build holds pointers to them instead. */
typedef struct
{
    sdfMemHandle_t headGuard;
    sdfMemHandle_t tailGuard;
    void *allocPtr;
    s32 allocSize;
} sdfMemHandleCB_t;

typedef struct sdfMemRecord_t
{
    struct sdfMemRecord_t *next;
    sdfMemHandle_t *handle;
    char *file;
    s32 line;
} sdfMemRecord_t;

/* Six signed words written by sdfMemGetInfo. */
typedef struct
{
    s32 total;
    s32 freeSize;
    s32 maxFreeSize;
    s32 minFreeSize;
    s32 handleCount;
    s32 freeHandleCount;
} sdfMemInfo_t;

/* GS local memory, allocated in the same list-of-slices shape as sdfMem. */
struct sdfGsMemSlice_t
{
    sdfGsMemSlice_t *prev;
    sdfGsMemSlice_t *next;
    s32 type; /* 0 free */
    u32 addr;
    s32 size;
    s16 w;
    s16 h;
    /* The IDB says u32; retail loads it with lw in sdfSetLoadImage, so s32. */
    s32 pixelFormat;
};

struct sdfTexHandle_t
{
    sdfTexHandle_t *prev;
    sdfTexHandle_t *next;
    sdfTexHandle_t *origin; /* unconfirmed */
    s16 width;
    s16 height;
    sdfGsMemSlice_t *pixelGsHandle;
    sdfGsMemSlice_t *clutGsHandle;
    u8 numOfCLUT;
    u8 CLUTFormat;
    u8 pixelFormat;
    u8 numOfMipmap;
    u16 mipmapParam;  /* unconfirmed */
    u8 flags;         /* unconfirmed */
    u8 texWrapMode;   /* unconfirmed */
    u32 texId;        /* unconfirmed */
    u32 clutId;       /* unconfirmed */
    sdfTexPacket_t *packet;
    sdfTexPacket_t *packetCtx2; /* unconfirmed */
    void *clutData;
    u32 clutDataSize;
    u32 clutColor;
    /* The intensity table sdfTexSetMonoClut derives from the CLUT, allocated
     * on first use and written a byte per entry. */
    u8 *monoClutData;
};

struct sdfTexPacket_t
{
    sdfGifTag_t giftag;
    sdfGrTagStdReg_t tex1;
    sdfGrTagStdReg_t tex0;
    sdfGrTagStdReg_t clamp;
};

struct sdfDrawTag_t
{
    sdfDrawTag_t *next;
    sdfGrPacketHeader_t *head;
    sdfGrPacketHeader_t *tail;
    /* Sort key. sdfDrawTagInit sets it to 0xFFFF as an integer, so f32 is
     * the one field type here not pinned by the code. */
    f32 z;
    struct sdf3DLightParallelPacket_t *lightPacket;
    struct sdf3DFogPacket_t *fogPacket;
    void *texPacket[2];
};

/* A callback sdfGrDrawFrame runs before it sends the chain. */
struct sdfPreDrawHook_t
{
    sdfPreDrawHook_t *next;
    void *predrawFunc;
    u32 option0;
    u32 option1;
};

struct sdfPreDrawTag_t
{
    sdfPreDrawTag_t *next;
    sdfPreDrawHook_t *head;
    sdfPreDrawHook_t *tail;
    u32 pad;
};

/* One entry of a model's per-ordering-table packet list. */
struct sdfModelPacketHeader_t
{
    sdfModelPacketHeader_t *next;
    u8 type;
    u8 otIndex;
    u16 size;
    void *dataPtr;
    sdfMemHandle_t *memHandle;
};

struct sdfMatUvwParm_t
{
    f32 offset[2];
    f32 tile[2];
    f32 angle;
    f32 pad;
};

/* One material plus its two prebuilt VIF packets, one per texture context.
 * sdfhook gives this struct a trailing reserved[88] and 160 bytes; sdfMatNew
 * allocates 0x48, which is where the struct ends here. */
struct sdfMat_t
{
    u32 id;
    u16 flags;
    u8 duty;
    u8 pad;
    sdfMatPacket_t *packet[2];
    u32 ambient;
    u32 diffuse;
    u32 multiTexDiffuse;
    f32 constant;
    u32 specular;
    u32 specularParm;
    u32 refmap;
    sdfTexHandle_t *texHandle;
    sdfTexHandle_t *multiTexHandle;
    u32 multiTexMethod;
    sdfMatUvwParm_t *uvwParm;
    sdfMatUvwParm_t *multiTexUvwParm;
    f32 fdBias;
    f32 fdSoften;
};

/* A texture coordinate transform in a material packet. sdfMatSetUvwParm fills
 * it either as six floats or, for the identity case, as three quadword halves. */
typedef union
{
    f32 m[6];
    u64 q[3];
} sdfMatTexTm_t;

/* The VIF packet a material keeps per context: the GS registers
 * sdfMatUpdatePacketBasic copies out of the texture handle, interleaved with
 * the VIF codes that upload them. sdfMatNew allocates 0xA0 for it and writes
 * VIF codes at 0x98 and 0x9C, which is the tail sdfhook's 128-byte version
 * does not model. */
struct sdfMatPacket_t
{
    u32 vifcode0;
    u32 ambient;
    u32 diffuse;
    u32 mtexDiffuse;
    u32 specularColor;
    u32 refmapColor;
    u32 vifcode1;
    f32 constant;
    u32 mtexAlpha;
    u32 mtexAlgo;
    f32 fdBias;
    f32 fdSoften;
    u32 pad;
    u32 vifcode2;
    u64 tex1Data;
    u64 tex0Data;
    u64 clampData;
    u64 mtexTex1Data;
    u64 mtexTex0Data;
    u64 mtexClampData;
    /* sdfMatSetUvwParm writes one transform per texture: a 2x2 then an offset. */
    sdfMatTexTm_t texTm;
    sdfMatTexTm_t mtexTexTm;
    u32 vifcode3;
    u32 vifcode4;
};

/* Every growable list in the engine: a 16-byte header plus one element block in
 * sdfMem. sdfItemArrayNew allocates the header, sdfItemArrayGrow reallocs the
 * block by growStep elements and relocks it. model.h and motion.h declare the
 * same six fields per element type, which is how the engine's own headers do it. */
struct sdfItemArray_t
{
    sdfMemHandle_t *memHandle;
    s16 count;
    s16 capacity;
    s16 elemSize;
    s16 growStep;
    void *items;
};

/* One ordering table: the draw tags filed into it this frame, plus the two
 * callbacks that file and flush them. sdfGrOtTable is 85 of these back to back,
 * which is what fixes the 32-byte stride. */
struct sdfGrOt_t
{
    sdfGrOt_t *next;
    sdfDrawTag_t *tagHead;
    sdfDrawTag_t *tagTail;
    sdfDrawTag_t *noSortTag; /* name unconfirmed; sdfGrOtInit zeroes it */
    void (*regFunc)(sdfGrOt_t *ot, sdfDrawTag_t *tag);
    void *callbackFunc;
    void *data; /* name unconfirmed; sdfGrOtInit zeroes it */
    u32 pad;
};

/* Four quadwords: colour, direction, position and the falloff parameters. */
typedef struct
{
    sdfVector4_t color;
    sdfVector4_t v;
    sdfVector4_t p;
    f32 param[4];
} sdf3DLight_t;

/* The .pac stream decoder. sdfPacInit memsets 56 bytes, and sdfPacDecode runs
 * phase as a three-state machine: 0 starts the next packet, 1 skips bytes, 2
 * hands the input to decodeFunc. */

/* The 16-byte header every packet in a .pac stream starts with. The low nibble
 * of flags is the encoding, the high nibble the extra header bytes. */
typedef struct
{
    u8 command;
    u8 flags;
    u16 kind;
    s32 size;
    u32 tag;
    s32 decodedSize;
} sdfPacPacketHeader_t;

/* sdfPacPacketHeader_t.flags. */
enum sdf_pac_flag
{
    SDF_PAC_ENCODING_MASK = 0xF,
    SDF_PAC_EXTRA_MASK = 0xF0
};

/* sdfPacPacketHeader_t.command, as sdfPacStdCallback dispatches on it. */
enum DDS3_FILE_TYPE
{
    DDS3_FT_EFFECT = 1,
    DDS3_FT_TMX = 2,
    DDS3_FT_MODEL = 6,
    DDS3_FT_MOTION = 8,
    DDS3_FT_TEXLIST = 9,
    DDS3_FT_END = 0xFF
};

/* sdfPacState_t.flags, set by sdfPacSetStay and sdfPacSetFromTail. */
enum sdf_pac_state_flag
{
    SDF_PAC_STATE_STAY = 1,
    SDF_PAC_STATE_FROMTAIL = 2
};

/* One queued resource: the decoded block plus the packet header it came from.
 * sdfPacNewResrc allocates 32 bytes plus the header's extra bytes. */
typedef struct sdfPacWork_t
{
    struct sdfPacWork_t *next;
    sdfPacState_t *state;
    void *resrc;
    u8 *data;
    u8 header[16];
} sdfPacWork_t;

/* The relocation record a model or motion payload starts with; its own data
 * follows at 16 and the pointer table sits tableOffset bytes into it. */
typedef struct
{
    s32 tableOffset;
    s32 tableBytes;
    u32 pad[2];
    u8 payload[1];
} sdfPacReloc_t;

/* The decode target for one texture: the sdfMem block it lands in and how many
 * bytes of it are still to come. */
typedef struct
{
    sdfTexHandle_t *tex;
    sdfMemHandle_t *memHandle;
    u8 *cursor;
    s32 remain;
} sdfPacBuf_t;

/* A texture list packet in progress: the entry count, the entry being decoded,
 * and the header and buffer that entry reuses. */
typedef struct
{
    u32 count;
    s32 index;
    u32 pad[2];
    sdfPacPacketHeader_t header;
    sdfPacBuf_t buf;
} sdfPacList_t;

typedef s32 (*sdfPacCallbackFunc)(sdfPacState_t *aState, s32 aStat,
                                  sdfPacPacketHeader_t *aPacket);
typedef void (*sdfPacDecodeFunc)(sdfPacState_t *aState);

struct sdfPacState_t
{
    u8 phase;
    u8 flags;
    s16 blockId;
    sdfPacCallbackFunc callback;
    sdfPacDecodeFunc decodeFunc;
    sdfPacDecodeFunc endFunc;
    const u8 *src;
    s32 srcLeft;
    s32 srcUsed;
    u8 *dst;
    s32 dstLeft;
    union
    {
        struct sdfDecomp_t *decomp;
        u8 *decompBuffer;
    };
    sdfPacBuf_t *buf;
    union
    {
        sdfPacBuf_t *buf;
        sdfPacList_t *list;
    } slot;
    sdfPacWork_t *head;
    sdfPacWork_t *tail;
};

/* The LZ decoder sdfDecompInit sets up in 32 chip bytes; sdfDecomp returns
 * nonzero when the stream ends and leaves the unconsumed byte count at 12. */
struct sdfDecomp_t
{
    u32 pad[3];
    s32 remain;
    u32 pad2[4];
};

/* File-command prefix shared by the queue and device helpers. */
typedef struct sdfFileCmd_t
{
    u8 unk00[0x10];
    char *path;
    u8 unk14[2];
    s8 stat;
    u8 unk17[0x11];
    s32 fd;
    s32 error;
} sdfFileCmd_t;

/* Command header shared by sdfPktInit and SIF packet formatting. */
typedef struct sifCommand_t
{
    s32 source;
    s32 end;
    s32 argument;
    u32 command;
} sifCommand_t;

/* SDK thread registration node shared by graph and system management. */
typedef struct sdfThCB_t
{
    struct sdfThCB_t *next;
    s32 thId;
} sdfThCB_t;

typedef u8 sdfTextureClampMode_t;

/* Native 0x40-byte TMX header; sdfTexSetEx and resource loaders consume it. */
typedef struct TMX_HEADER_t
{
    u8 fileType;
    u8 compression;
    s16 userId;
    u32 fileSize;
    u32 formatId;
    s32 realSize;
    u8 numOfCLUT;
    u8 CLUTFormat;
    s16 width;
    s16 height;
    u8 pixelFormat;
    u8 numOfMipmap;
    u16 mipmapParam;
    u8 flags;
    sdfTextureClampMode_t texWrapMode;
    /* The IDB calls 0x1C onward userArea and comment. These two are what this
     * game puts in its user area; sdfTmx reads both as words. */
    s32 resourceKey;
    s32 battleTextureSlot;
    u8 pad24[0x1C];
} TMX_HEADER_t;

/* Serialized texture-offset table passed with an in-memory scene resource.
 * Event loading only forwards this pointer; its contents remain opaque here. */
typedef struct sdfTextureOffsetListHeader_t sdfTextureOffsetListHeader_t;

#endif /* LIBSDF_H */
