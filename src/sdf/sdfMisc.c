#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

void sdfMemFreeDirect(sdfMemHandle_t *aHandle);

void sdfChipFreeDirect(void *aPtr);

enum SdfFileIOCmdStat {
    SDF_FILE_CMD_STAT_INIT = 0,
    SDF_FILE_CMD_STAT_DORMANT = 1,
    SDF_FILE_CMD_STAT_OPEN = 2,
    SDF_FILE_CMD_STAT_SEEK = 3,
    SDF_FILE_CMD_STAT_READ = 4,
    SDF_FILE_CMD_STAT_WRITE = 5,
    SDF_FILE_CMD_STAT_CLOSE = 6,
    SDF_FILE_CMD_STAT_SUSPEND = 7,
    SDF_FILE_CMD_STAT_DONE = 8,
    SDF_FILE_CMD_STAT_ERROR = 9,
    SDF_FILE_CMD_STAT_CANCEL = 10
};

enum SdfFileIOCmd {
    SDF_FILE_CMD_NONE = 0,
    SDF_FILE_CMD_READOPEN = 1,
    SDF_FILE_CMD_WRITEOPEN = 2,
    SDF_FILE_CMD_SEEK = 3,
    SDF_FILE_CMD_FILESIZE = 4,
    SDF_FILE_CMD_READ = 5,
    SDF_FILE_CMD_WRITE = 6,
    SDF_FILE_CMD_CLOSE = 7,
    SDF_FILE_CMD_READFILE = 8,
    SDF_FILE_CMD_WRITEFILE = 9
};

enum sdf_tex_resource {
    SDF_TEX_RESOURCE_FREE = 0,
    SDF_TEX_RESOURCE_FRAME = 1,
    SDF_TEX_RESOURCE_TEXTURE = 2,
    SDF_TEX_RESOURCE_CLUT = 3,
    SDF_TEX_RESOURCE_KNOWN_MODE_COUNT = 4
};

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

void *sdfChipAllocDirect(s32 size);

u32 sdfMemLock(sdfMemHandle_t *allocation);

sdfMemHandle_t *sdfMemAllocDirect(s32 size);

extern s32 (*p2sdSmgBuf)(void);

/* Allocate element storage with a fixed element stride and growth increment. */
sdfItemArray_t *sdfItemArrayNew(s32 elementCount, s32 elementStride, s32 growthStep) {
    sdfItemArray_t *request = sdfChipAllocDirect(sizeof(*request));
    request->growStep = growthStep;
    request->count = 0;
    request->capacity = elementCount;
    request->elemSize = elementStride;
    if (elementCount != 0) {
        request->memHandle = sdfMemAllocDirect(elementStride * elementCount);
        request->items = (void *)sdfMemLock(request->memHandle);
    } else {
        request->memHandle = 0;
        request->items = 0;
    }
    return request;
}

void sdfItemArrayFree(sdfItemArray_t *aArray)
{
    sdfMemFreeDirect(aArray->memHandle);
    sdfChipFreeDirect(aArray);
}

INCLUDE_ASM(const s32, "sdf/sdfMisc", sdfItemArrayGrow);
