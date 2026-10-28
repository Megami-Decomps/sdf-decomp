#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

extern s32 iSignalSema(s32 aSemaId);

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

extern s32 (*p2sdSmgBuf)(void);

extern u8 sdfHddValid;

extern u8 sdfDev9Ready;

extern char D_00551990[];

extern char D_00551998[];

extern char D_005519A0[];

extern char D_00529B60[];

extern char D_00529B70[];

extern u8 D_0058FFC0[];

extern s32 printf(const char *format, ...);

extern s32 WaitSema(s32 aSemaId);

extern s32 sceDevctl(const char *device, s32 cmd, void *arg, s32 arglen, void *buf,
                     s32 buflen);

extern s32 sceCdPowerOff(s32 *result);

extern void sdfDev9LoadModule(void);

extern s32 sceSifLoadStartModule(const char *path, s32 arglen, char *args, s32 *result);

extern s32 sdfCreateSema(s32 initial, s32 max, s32 option);

extern s32 CreateThreadSimple(void *entry, void *stack, s32 stackSize, s32 priority);

extern s32 StartThread(s32 threadId, void *arg);

extern s32 sceCdPOffCallback(void *handler, s32 arg);

extern s32 sceFsInit(void);

extern s32 sceMount(char *mountPoint, char *device, s32 flag, void *arg, s32 arglen);

void sdfHddPowerOffHandler(s32 aArg);

void sdfHddPowerOffTh(s32 aSemaId)
{
    s32 result;

    while (1) {
        WaitSema(aSemaId);
        printf("power off request has come.\n");
        sceDevctl(D_00551990, 0x5003, NULL, 0, NULL, 0);
        sceDevctl(D_00551998, 0x4806, NULL, 0, NULL, 0);
        sceCdPowerOff(&result);
    }
}

void sdfHddPowerOffHandler(s32 aArg)
{
    iSignalSema(aArg);
}

void sdfHddInit(void)
{
    s32 result;
    s32 sema;

    sdfHddValid = 0;
    sdfDev9LoadModule();
    if (sdfDev9Ready == 0) {
        return;
    }
    if (sceSifLoadStartModule("cdrom0:\\IRX\\ATAD.IRX;1", 0, NULL, &result) < 0) {
        printf("cdrom0:\\IRX\\ATAD.IRX;1 could't load.\n");
        return;
    }
    if (result != 0) {
        printf("cdrom0:\\IRX\\ATAD.IRX;1 resident fail.\n");
        return;
    }
    if (sceSifLoadStartModule("cdrom0:\\IRX\\HDD.IRX;1", 0xC, D_00529B60, &result) < 0) {
        printf("cdrom0:\\IRX\\HDD.IRX;1 could't load.\n");
        return;
    }
    if (result != 0) {
        printf("cdrom0:\\IRX\\HDD.IRX;1 resident fail.\n");
        return;
    }
    if (sceSifLoadStartModule("cdrom0:\\IRX\\PFS.IRX;1", 0x12, D_00529B70, &result) < 0) {
        printf("cdrom0:\\IRX\\PFS.IRX;1 could't load.\n");
        return;
    }
    if (result != 0) {
        printf("cdrom0:\\IRX\\PFS.IRX;1 resident fail.[%d]\n", result);
        return;
    }
    sema = sdfCreateSema(0, 1, 0);
    StartThread(CreateThreadSimple(sdfHddPowerOffTh, D_0058FFC0, 0x800, 1), (void *)sema);
    sceCdPOffCallback(sdfHddPowerOffHandler, sema);
    sceFsInit();
    sceMount(D_005519A0, "hdd0:dds3,", 0, NULL, 0);
    sdfHddValid = 1;
}
