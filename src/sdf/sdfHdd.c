#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

extern s32 iSignalSema(s32 aSemaId);

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

extern s32 (*p2sdSmgBuf)(void);

INCLUDE_ASM(const s32, "sdf/sdfHdd", sdfHddPowerOffTh);

void sdfHddPowerOffHandler(s32 aSemaId)
{
    iSignalSema(aSemaId);
}

INCLUDE_ASM(const s32, "sdf/sdfHdd", sdfHddInit);
