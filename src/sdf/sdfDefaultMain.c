#include "common.h"
#include "basic.h"
#include "kernel.h"
#include "libsdf.h"

extern u32 D_0054E8BC;

extern u32 sdfCdTrayMode;

extern u8 sdfPadManagerMode;

extern sdfThCB_t D_00551B88;

extern u8 D_00552310[];

extern void initFloatOnce(void);

extern void sceSifInitRpc(s32 mode);

extern s32 sceSifInitIopHeap(void);

extern s32 sdfCdInit(void);

extern s32 sceSifRebootIop(const char *image);

extern s32 sceSifSyncIop(void);

extern void sceSifLoadFileReset(void);

extern void sceFsReset(void);

extern s32 sceCdInit(s32 aMode);

extern s32 sceCdMmode(s32 aMode);

extern void sdfCdImageInit(const char *imageFile, const char *dirFile);

extern s32 sceSifLoadModule(const char *path, s32 argSize, const char *args);

extern void LoadMcModules(const char *prefix, s32 moduleGroup);

extern void dds3InitSubsystems(u32 heapSize, u32 initialHeapSize, u32 packetBufferBytes);

extern void sdfPadChangeMode(s32 aPadNum, u8 mode);

extern void sdfFileSetHostRoot(char *aRootPath);

extern void sdfFileSetDefaultSrc(s32 aType);

extern void sdfFileCmdStartThread(s32 deviceType);

extern void SdfIopInit(const char *driverPath, const char *modulePath);

extern void sdfStartFileRpcServer(void);

extern void sdfStartThreadAndWait(sdfThCB_t *thread, void (*entry)(void), void *stack,
                                  s32 stackSize, s32 priority, s32 arg);

extern void sdfSetMainThreadPriority(void);

extern void dds3KernelMain(void);

/* Named around `main` so cc1 does not emit the __main call it adds to a
   function of that name. */
s32 sdfDefaultMain(void) asm("main");

s32 sdfDefaultMain(void)
{
    initFloatOnce();
    sceSifInitRpc(0);
    sceSifInitIopHeap();
    sdfCdTrayMode = D_0054E8BC;
    sdfCdInit();
    while (sceSifRebootIop("cdrom0:\\IRX\\IOPRP250.IMG;1") == 0) {
    }
    while (sceSifSyncIop() == 0) {
    }
    sceSifInitRpc(0);
    sceSifInitIopHeap();
    sceSifLoadFileReset();
    sceFsReset();
    sceCdInit(0);
    sceCdMmode(D_0054E8BC);
    sdfCdImageInit("\\DDS3.IMG;1", "cdrom0:\\DDS3.DDT;1");
    while (sceSifLoadModule("cdrom0:\\IRX\\SIO2MAN.IRX;1", 0, NULL) < 0) {
    }
    while (sceSifLoadModule("cdrom0:\\IRX\\PADMAN.IRX;1", 0, NULL) < 0) {
    }
    LoadMcModules("cdrom0:\\IRX", 1);
    dds3InitSubsystems(0x1700000, 0x200000, 0x400000);
    sdfPadManagerMode = 1;
    sdfPadChangeMode(0, 3);
    sdfPadChangeMode(1, 3);
    sdfFileSetHostRoot("../../../dds3data");
    sdfFileSetDefaultSrc(3);
    sdfFileCmdStartThread(3);
    sdfFileCmdStartThread(0);
    SdfIopInit("cdrom0:\\USERIRX\\SDFIOP.IRX;1", "cdrom0:\\IRX\\MSIFRPC.IRX;1");
    sdfStartFileRpcServer();
    sdfStartThreadAndWait(&D_00551B88, dds3KernelMain, D_00552310, 0x4000, 0x60, 0);
    sdfSetMainThreadPriority();
    return 0;
}

INCLUDE_SDATA(const s32, "sdf/sdfDefaultMain", D_0054E8BC);

INCLUDE_SDATA(const s32, "sdf/sdfDefaultMain", dds3ClosedDrawTag);

