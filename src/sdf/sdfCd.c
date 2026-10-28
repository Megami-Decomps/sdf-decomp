#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

typedef struct
{
    u32 nameOffset_;
    u32 offset_;
    s32 size_;
} sdfCdImageDirEntry_t;

s32 sdfCreateSema(s32 initCount, s32 maxCount, s32 option);

s32 sceCdInit(s32 aMode);

s32 sceCdMmode(s32 aMode);

s32 sceCdStStop(void);

extern sdfCdImageDirEntry_t *sdfCdImageCurrFile;

extern s32 sdfCdImageFd;

extern s32 sdfCdTrayMode;

sdfCdImageDirEntry_t *sdfCdImageSearchFile(const char *fname, s32 *lsn, u8 *modePtr);

extern s32 WaitSema(s32 aSemaId);

extern s32 SignalSema(s32 aSemaId);

extern void *sdfCdImageDirData;


void FatalError(const char *aMessage, ...) __attribute__((noreturn));

s32 sceClose(s32 a);

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


extern s32 sceOpen();

extern s32 sceLseek(s32 fileHandle, s32 offset, s32 origin);

extern void *malloc(u32 size);

extern s32 sceRead(s32 fileHandle, s32 buffer, s32 byteCount);

extern s32 dds3ImgPath;

extern s32 sceCdSearchFile(void *, const char *);

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

void sdfCdStStart(s32 lsn) {
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
        ready = sceCdStStart(lsn, options);
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
    D_0055210C = lsn;
}

extern s32 sceCdStRead(s32 sectors, void *buffer, s32 mode, s32 *status);
extern s32 printf(const char *format, ...);

void sdfCdStRead(s32 sectors, void *buffer)
{
    s32 status;
    s32 result;

    for (;;) {
        WaitSema(sdfCdImageFd);
        result = sceCdStRead(sectors, buffer, 1, &status);
        SignalSema(sdfCdImageFd);
        if (status == 0 && result == sectors) {
            break;
        }
        if (sceCdStatus() == 1) {
            sdfCdWaitTrayClose();
            sdfCdStStart(D_0055210C);
        } else {
            printf("CD read error!\n");
            WaitSema(sdfCdImageFd);
            sceCdStStop();
            SignalSema(sdfCdImageFd);
            sdfCdStStart(D_0055210C);
        }
    }
    D_0055210C += sectors;
}

extern char *strcpy(char *destination, const char *source);
extern char *strchr(const char *text, s32 character);
extern sdfCdImageDirEntry_t sdfCdImageSearchFileDirect_ent;

sdfCdImageDirEntry_t *sdfCdImageSearchFileDirect(const char *filename, s32 *lsn, u8 *mode)
{
    /* Same CD search scratch capacity as sdfCdImageInit. */
    u32 file[12];
    char path[256];
    char *name;
    char *cursor;
    s32 character;
    s32 status;
    s32 fd;

    strcpy(path, filename);
    name = strchr(path, ':') + 1;
    cursor = name;
    character = *cursor;
    while (character != 0) {
        if (character == '/') {
            *cursor = '\\';
        } else if (character >= 'a' && character <= 'z') {
            *cursor = character - 0x20;
        }
        cursor++;
        character = *cursor;
    }
    if (cursor[-2] != ';') {
        cursor[0] = ';';
        cursor[1] = '1';
        cursor[2] = 0;
    }
    for (;;) {
        WaitSema(sdfCdImageFd);
        if (sceCdSearchFile(file, name) != 0) {
            fd = sceOpen(path, 1);
            if (fd < 0) {
                SignalSema(sdfCdImageFd);
                continue;
            }
            sdfCdImageSearchFileDirect_ent.size_ = sceLseek(fd, 0, 2);
            sceClose(fd);
            SignalSema(sdfCdImageFd);
            break;
        }
        status = sceCdStatus();
        SignalSema(sdfCdImageFd);
        if (status == 1) {
            sdfCdWaitTrayClose();
        } else {
            return NULL;
        }
    }
    *lsn = file[0];
    if (mode != NULL) {
        *mode = 1;
    }
    return &sdfCdImageSearchFileDirect_ent;
}

INCLUDE_ASM(const s32, "sdf/sdfCd", sdfCdImageSearchFile);

s32 sdfCdImageLoadStart(const char *filename)
{
    s32 lsn;
    sdfCdImageDirEntry_t *entry = sdfCdImageSearchFile(filename, &lsn, &D_00552110);

    if (entry == NULL) {
        return -1;
    }
    sdfCdImageCurrRemain = entry->size_;
    sdfCdImageCurrFile = entry;
    sdfCdImageCurrActive = 0;
    sdfCdStStart(lsn);
    return entry->size_;
}

/* Consume up to size disc-file bytes, retaining a partial sector; return the byte count. */
s32 sdfCdImageRead(void *buff, s32 length) {
    s32 available = sdfCdImageCurrRemain;
    u8 *destination = buff;
    s32 bytes;
    s32 transferBytes;
    s32 t;
    if (available <= 0) {
        return 0;
    }
    if (length > available) {
        length = available;
    }
    if (length == 0) {
        return 0;
    }
    bytes = length;
    transferBytes = sdfCdImageCurrActive;
    if (transferBytes > 0) {
        if (transferBytes > length) {
            transferBytes = length;
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
                sdfCdStRead(transferBytes >> 11, destination);
            }
            if (t > 0) {
                sdfCdStRead(1, D_0058F7C0);
                memcpy(destination + transferBytes, D_0058F7C0, t);
                sdfCdImageOffset = D_0058F7C0 + t;
                sdfCdImageCurrActive = 0x800 - t;
            }
        } else {
            while (transferBytes >= 0x800) {
                sdfCdStRead(1, D_0058F7C0);
                memcpy(destination, D_0058F7C0, 0x800);
                transferBytes -= 0x800;
                destination += 0x800;
            }
            if (transferBytes > 0) {
                sdfCdStRead(1, D_0058F7C0);
                memcpy(destination, D_0058F7C0, transferBytes);
                sdfCdImageOffset = D_0058F7C0 + transferBytes;
                sdfCdImageCurrActive = 0x800 - transferBytes;
            }
        }
    }
    sdfCdImageCurrRemain = available - length;
    if (sdfCdImageCurrRemain == 0) {
        sdfCdImageClose();
    }
    return length;
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

s32 sdfCdImageCheckExist(const char *fname)
{
    s32 lsn;

    return sdfCdImageSearchFile(fname, &lsn, NULL) != 0;
}

s32 sdfCdImageGetFileSize(const char *fname)
{
    s32 lsn;
    sdfCdImageDirEntry_t *entry = sdfCdImageSearchFile(fname, &lsn, NULL);

    if (entry == NULL) {
        return -1;
    }
    return entry->size_;
}

s32 sdfCdImageGetCurrFileSize(void)
{
    sdfCdImageDirEntry_t *file = sdfCdImageCurrFile;

    if (file == NULL) {
        FatalError("file didn't open.");
    }
    return file->size_;
}

/* Read a whole named file into the loader's allocated EE buffer, then close it. */
void sdfCdImageLoadDir(const char *dirFile) {
    s32 fileHandle = sceOpen(dirFile, 1);
    s32 fileBytes;
    void *buffer;
    if (fileHandle < 0) {
        FatalError("%s couldn't open.", dirFile);
    }
    fileBytes = sceLseek(fileHandle, 0, 2);
    sceLseek(fileHandle, 0, 0);
    buffer = malloc(fileBytes);
    sdfCdImageDirData = buffer;
    sceRead(fileHandle, (s32)buffer, fileBytes);
    sceClose(fileHandle);
}

/* Record the disc search path, preload a separate file, and initialize an aligned IOP buffer. */
void sdfCdImageInit(const char *imageFile, const char *dirFile) {
    u32 discFileRecord[12];
    s32 iopHeapAddress;
    dds3ImgPath = (u32)imageFile;
    sdfCdImageLoadDir(dirFile);
    if (sceCdSearchFile(discFileRecord, imageFile) == 0) {
        FatalError("%s couldn't open.", imageFile);
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

