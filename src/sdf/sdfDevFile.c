#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

void sdfMemFreeDirect(sdfMemHandle_t *aHandle);

void *sdfDevLoadFile(const char *aFileName, u32 *aOutData, u32 *aOutSize, s32 aAlign);

typedef struct
{
    u8 unk00[0x10];
    s32 resolveOffset;
    s32 resolveCount;
    u8 unk18[8];
} sdfBinaryHeader_t;

void sdfAddressResolver(void *aBase, void *aTarget, void *aTable, s32 aCount);

typedef u8 sdfDevOperationCode_t;

typedef s8 sdfDevStateCode_t;

/* Complete 0x40-byte request owner. sdfFileCmd_t describes only its opaque
 * 0x30-byte prefix for consumers outside the device queue implementation. */
typedef struct sdfFileCmdHandle_t {
    struct sdfFileCmdHandle_t *next;
    struct sdfFileCmdHandle_t *previous;
    struct sdfFileCmdHandle_t *workerNext;
    struct sdfFileCmdHandle_t *workerPrev;
    char *resolvedPath;
    u8 workerIndex;
    sdfDevOperationCode_t operation;
    sdfDevStateCode_t state;
    u8 pad17;
    s32 operationArg;
    s32 requestExtra;
    void *requestData;
    s32 options;
    s32 resourceId;
    s32 result;
    s32 transferred;
    u8 pad34[4];
    void (*callback)(struct sdfFileCmdHandle_t *, s32, s32, s32, s32);
    s32 callbackContext;
} sdfFileCmdHandle_t;

enum sdf_dev_state {
    SDF_DEV_STATE_OPENING = 2,
    SDF_DEV_STATE_SEEKING = 3,
    SDF_DEV_STATE_READING = 4,
    SDF_DEV_STATE_WRITING = 5,
    SDF_DEV_STATE_CLOSING = 6,
    SDF_DEV_STATE_ACTIVE = 7,
    SDF_DEV_STATE_COMPLETE = 8,
    SDF_DEV_STATE_INACTIVE = 9
};

enum sdf_dev_operation {
    SDF_DEV_OPERATION_NONE = 0,
    SDF_DEV_OPERATION_OPEN_READ = 1,
    SDF_DEV_OPERATION_OPEN_WRITE = 2,
    SDF_DEV_OPERATION_SEEK = 3,
    SDF_DEV_OPERATION_GET_SIZE = 4,
    SDF_DEV_OPERATION_READ = 5,
    SDF_DEV_OPERATION_WRITE = 6,
    SDF_DEV_OPERATION_CLOSE = 7,
    SDF_DEV_OPERATION_READ_BUFFER = 8,
    SDF_DEV_OPERATION_WRITE_BUFFER = 9
};

enum sdf_tex_resource {
    SDF_TEX_RESOURCE_FREE = 0,
    SDF_TEX_RESOURCE_FRAME = 1,
    SDF_TEX_RESOURCE_TEXTURE = 2,
    SDF_TEX_RESOURCE_CLUT = 3,
    SDF_TEX_RESOURCE_KNOWN_MODE_COUNT = 4
};

typedef struct sdfTextureOffsetListEntry_t {
    s32 textureHeaderOffset;
} sdfTextureOffsetListEntry_t;

typedef struct sdfTextureOffsetListHeader_t {
    u8 unknown00[0x10];
    s32 textureCount;
} sdfTextureOffsetListHeader_t;

enum sdf_stream_output {
    SDF_STREAM_OUTPUT_RGBA32 = 0,
    SDF_STREAM_OUTPUT_RGBA16 = 1,
    SDF_STREAM_OUTPUT_RGBA16_DITHERED = 2
};

enum sdf_stream_playback {
    SDF_STREAM_PLAYBACK_INITIAL = 0,
    SDF_STREAM_PLAYBACK_FIRST_COMPLETION = 1,
    SDF_STREAM_PLAYBACK_CADENCED = 2
};

u32 sdfMemLock(sdfMemHandle_t *allocation);

s32 printf(const char *format, ...);

extern char D_0054A2D8[];

void sdfMemUnlock(sdfMemHandle_t *allocation);

void sdfTexListAdd(sdfItemArray_t *list, sdfTexHandle_t *texture);

sdfMemHandle_t *sdfMemAllocDirect(s32 size);

extern sdfItemArray_t *sdfTexListNew(u32 capacity);

sdfTexHandle_t *sdfTexSet(const TMX_HEADER_t *header);

extern s32 (*p2sdSmgBuf)(void);

void sdfFileClose(void *aHandle);

u32 sdfFileLength(void *aHandle);

/* Open a path using the shared reply callback and wait for its initial completion. */ sdfFileCmdHandle_t *sdfFileOpen(const char *path);

s32 sdfFileRead(void *aHandle, void *aBuffer, s32 aSize);

/* Read a named file through the dev RPC into a freshly allocated block; returns the block's handle.
 * outData receives the block address, outSize the file size; without outData the block is released. */
void *sdfDevLoadFile(const char *name, u32 *outData, u32 *outSize, s32 extra) {
    sdfFileCmdHandle_t *state = sdfFileOpen(name);
    s32 size = sdfFileLength(state);
    sdfMemHandle_t *handle = sdfMemAllocDirect(size + extra);
    u32 address = sdfMemLock(handle);
    sdfFileRead(state, (void *)address, size);
    sdfFileClose(state);
    if (outData != NULL) {
        *outData = address;
    } else {
        sdfMemUnlock(handle);
    }
    if (outSize != NULL) {
        *outSize = size;
    }
    return handle;
}

void *sdfDevLoadFileEx(const char *aFileName, u32 *aOutData, u32 *aOutSize)
{
    return sdfDevLoadFile(aFileName, aOutData, aOutSize, 0);
}

sdfTexHandle_t *sdfDevLoadTex(const char *filename) {
    u32 dataAddress;
    sdfMemHandle_t *allocation;
    sdfTexHandle_t *texture;

    printf(D_0054A2D8, filename);
    allocation = sdfDevLoadFileEx(filename, &dataAddress, 0);
    texture = sdfTexSet((void *)dataAddress);
    sdfMemFreeDirect(allocation);
    return texture;
}

sdfItemArray_t *sdfTexSetTB(const sdfTextureOffsetListHeader_t *resource) {
    s32 i = 0;
    s32 count = resource->textureCount;
    sdfItemArray_t *handle = sdfTexListNew(count);
    const sdfTextureOffsetListEntry_t *entry;
    if (count != i) {
        entry = (const sdfTextureOffsetListEntry_t *)((const u8 *)resource + sizeof(*resource));
        do {
            i++;
            sdfTexListAdd(handle, sdfTexSet((TMX_HEADER_t *)(
                (void *)((const u8 *)resource + entry->textureHeaderOffset))));
            entry++;
        } while (i != count);
    }
    return handle;
}

sdfItemArray_t *sdfDevLoadTexList(const char *filename) {
    u32 dataAddress;
    sdfMemHandle_t *allocation;
    sdfItemArray_t *texture;

    printf(D_0054A2D8, filename);
    allocation = sdfDevLoadFileEx(filename, &dataAddress, 0);
    texture = sdfTexSetTB((void *)dataAddress);
    sdfMemFreeDirect(allocation);
    return texture;
}

void *sdfFixupModelBinary(sdfBinaryHeader_t *aHeader)
{
    void *body = aHeader + 1;

    sdfAddressResolver(body, body, (u8 *)body + aHeader->resolveOffset, aHeader->resolveCount);
    return body;
}

INCLUDE_ASM(const s32, "sdf/sdfDevFile", sdfDevLoadModel);

void *sdfFixupMotionBinary(sdfBinaryHeader_t *aHeader)
{
    void *body = aHeader + 1;

    sdfAddressResolver(body, body, (u8 *)body + aHeader->resolveOffset, aHeader->resolveCount);
    return body;
}

INCLUDE_ASM(const s32, "sdf/sdfDevFile", sdfDevLoadMotion);

INCLUDE_RODATA(const s32, "sdf/sdfDevFile", D_0054A2D8);

