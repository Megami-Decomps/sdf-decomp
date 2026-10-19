#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

typedef struct
{
    u32 lsn;
    u32 unk4;
    s32 size;
} sdfCdImageEntry_t;

s32 sdfCreateSema(s32 aInit, s32 aMax, s32 aAttr);

s32 sceCdInit(s32 aMode);

s32 sceCdMmode(s32 aMode);

s32 sceCdStStop(void);

extern sdfCdImageEntry_t *sdfCdImageCurrFile;

extern s32 sdfCdImageFd;

extern s32 sdfCdTrayMode;

sdfCdImageEntry_t *sdfCdImageSearchFile(const char *aFileName, s32 *aOutLsn, s32 *aOutSize);

extern s32 WaitSema(s32 aSemaId);

extern s32 SignalSema(s32 aSemaId);

extern void *sdfCdImageDirData;

extern const char D_00549640[];

void FatalError(const char *aMessage, ...) __attribute__((noreturn));

s32 sceClose(s32 aFd);

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

extern void *memcpy(void *, const void *, unsigned int);

extern void sdfDelayThread(s32 delay);

void sdfCdWaitTrayClose(void);

extern u8 D_00552110;

extern s32 D_0055210C;

extern s32 sceCdStStart(s32 arg, u8 *options);

extern s32 sceCdStatus(void);

extern s32 sdfCdImageCurrRemain;

extern s32 sdfCdImageCurrActive;

extern u8 *sdfCdImageOffset;

extern u8 D_0058F7C0[0x800];

extern char D_00549658[];

extern s32 sceOpen();

extern s32 sceLseek(s32 fileHandle, s32 offset, s32 origin);

extern void *malloc(u32 size);

extern s32 sceRead(s32 fileHandle, s32 buffer, s32 byteCount);

extern s32 dds3ImgPath;

extern s32 sceCdSearchFile(void *, s32);

extern u32 sdfCdImageFileName;

extern u32 dds3ImgLsn;

extern s32 sceSifAllocIopHeap(s32);

extern void sceCdStInit(s32, s32, s32);

void sdfCdImageClose(void);

extern s32 (*p2sdSmgBuf)(void);

void *sdfCdImageGetDirData(void)
{
    return sdfCdImageDirData;
}

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdWaitTrayClose);

void sdfCdStStart(s32 request) {
    u8 options[4];
    s32 ready;
    options[0] = 0;
    if (D_00552110 == 1) {
        options[1] = 0;
    } else {
        options[1] = 1;
    }
    options[2] = 0;
    options[3] = 0;
    for (;;) {
        WaitSema(sdfCdImageFd);
        ready = sceCdStStart(request, options);
        SignalSema(sdfCdImageFd);
        if (ready != 0) {
            break;
        }
        if (sceCdStatus() == 1) {
            sdfCdWaitTrayClose();
        } else {
            sdfDelayThread(100);
        }
    }
    D_0055210C = request;
}

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdStRead);

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdImageSearchFileDirect);

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdImageSearchFile);

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdImageLoadStart);

/* Consume up to size disc-file bytes, retaining a partial sector; return the byte count. */
s32 sdfCdImageRead(void *buffer, s32 byteCount) {
    s32 available = sdfCdImageCurrRemain;
    u8 *destination = buffer;
    s32 bytes;
    s32 transferBytes;
    s32 t;
    if (available <= 0) {
        return 0;
    }
    if (byteCount > available) {
        byteCount = available;
    }
    if (byteCount == 0) {
        return 0;
    }
    bytes = byteCount;
    transferBytes = sdfCdImageCurrActive;
    if (transferBytes > 0) {
        if (transferBytes > byteCount) {
            transferBytes = byteCount;
        }
        memcpy(destination, sdfCdImageOffset, transferBytes);
        destination += transferBytes;
        sdfCdImageOffset += transferBytes;
        sdfCdImageCurrActive -= transferBytes;
        bytes -= transferBytes;
    }
    if (bytes > 0) {
        transferBytes = bytes;
        if (((u32)destination & 15) == 0) {
            t = transferBytes >> 11;
            t = transferBytes - t * 0x800;
            transferBytes -= t;
            if (transferBytes > 0) {
                sdfCdStRead(transferBytes >> 11, (s32)destination);
            }
            if (t > 0) {
                sdfCdStRead(1, (s32)D_0058F7C0);
                memcpy(destination + transferBytes, D_0058F7C0, t);
                sdfCdImageOffset = D_0058F7C0 + t;
                sdfCdImageCurrActive = 0x800 - t;
            }
        } else {
            while (transferBytes >= 0x800) {
                sdfCdStRead(1, (s32)D_0058F7C0);
                memcpy(destination, D_0058F7C0, 0x800);
                transferBytes -= 0x800;
                destination += 0x800;
            }
            if (transferBytes > 0) {
                sdfCdStRead(1, (s32)D_0058F7C0);
                memcpy(destination, D_0058F7C0, transferBytes);
                sdfCdImageOffset = D_0058F7C0 + transferBytes;
                sdfCdImageCurrActive = 0x800 - transferBytes;
            }
        }
    }
    sdfCdImageCurrRemain = available - byteCount;
    if (sdfCdImageCurrRemain == 0) {
        sdfCdImageClose();
    }
    return byteCount;
}

void sdfCdImageClose(void)
{
    if (sdfCdImageCurrFile != 0) {
        WaitSema(sdfCdImageFd);
        sceCdStStop();
        SignalSema(sdfCdImageFd);
        sdfCdImageCurrFile = NULL;
    }
}

s32 sdfCdImageCheckExist(const char *aFileName)
{
    s32 lsn;

    return sdfCdImageSearchFile(aFileName, &lsn, NULL) != 0;
}

s32 sdfCdImageGetFileSize(const char *aFileName)
{
    s32 lsn;
    sdfCdImageEntry_t *entry = sdfCdImageSearchFile(aFileName, &lsn, NULL);

    if (entry == NULL) {
        return -1;
    }
    return entry->size;
}

s32 sdfCdImageGetCurrFileSize(void)
{
    sdfCdImageEntry_t *file = sdfCdImageCurrFile;

    if (file == NULL) {
        FatalError(D_00549640);
    }
    return file->size;
}

/* Read a whole named file into the loader's allocated EE buffer, then close it. */
void sdfCdImageLoadDir(const char *fileName) {
    s32 fileHandle = sceOpen(fileName, 1);
    s32 fileBytes;
    void *buffer;
    if (fileHandle < 0) {
        FatalError(D_00549658, fileName);
    }
    fileBytes = sceLseek(fileHandle, 0, 2);
    sceLseek(fileHandle, 0, 0);
    buffer = malloc(fileBytes);
    sdfCdImageDirData = buffer;
    sceRead(fileHandle, (s32)buffer, fileBytes);
    sceClose(fileHandle);
}

/* Record the disc search path, preload a separate file, and initialize an aligned IOP buffer. */
void sdfCdImageInit(const char *discFileName, const char *preloadFileName) {
    u32 discFileRecord[12];
    s32 iopHeapAddress;
    dds3ImgPath = (u32)discFileName;
    sdfCdImageLoadDir(preloadFileName);
    if (sceCdSearchFile(discFileRecord, (s32)discFileName) == 0) {
        FatalError(D_00549658, discFileName);
    }
    dds3ImgLsn = discFileRecord[0];
    iopHeapAddress = sceSifAllocIopHeap(0x28010);
    sdfCdImageFileName = iopHeapAddress;
    sceCdStInit(0x50, 5, (iopHeapAddress + 15) & -16);
}

s32 sdfCdInit(void)
{
    sdfCdImageFd = sdfCreateSema(1, 0xFF, 0);
    sceCdInit(0);
    return sceCdMmode(sdfCdTrayMode);
}

INCLUDE_RODATA(const s32, "sdf/sdfCd", D_00549640);

INCLUDE_RODATA(const s32, "sdf/sdfCd", D_00549658);

