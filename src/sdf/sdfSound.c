#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

struct sdfGsMemSlice_t;

s32 strlen(const char *aStr);

s32 sdfSndCmdTrans(void);

s32 SleepThread(void);

extern s8 sdfSndBurstReqMode;

typedef struct
{
    u32 unk0;
    u8 unk4;
    u8 unk5;
    u16 unk6;
} p2sdLoadEntry_t;

extern p2sdLoadEntry_t LastLoad[];

s32 sdfSndSendCmd(s32 aCmd, s32 aParam, void *aExDataPtr, s32 aExDataSize);

extern s32 SignalSema(s32 aSemaId);

extern u32 p2sdPcmCallback;

extern u32 sdfSndMap[];

extern u32 p2sdPcmState;

extern u32 p2sdPcmBusy;

extern u8 D_00593890[];

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

typedef struct sndRingPacket_t {
    u32 transportHeader[3];
    u32 command;
    u32 sequence;
    u8 payload[108];
} sndRingPacket_t;

typedef struct sdfSndTrackPacket_t {
    u32 trackId;
    u32 unk4;
    u16 setting;
    u16 unkA;
    u32 unkC;
} sdfSndTrackPacket_t;

typedef struct sndSequenceVolumePanPacket_t {
    u32 trackId;
    u32 opaque04;
    u16 volume;
    u16 pan;
    u32 opaqueNotSent0C;
} sndSequenceVolumePanPacket_t;

typedef struct sndListenerState_t {
    s32 opaque00[2];
    s32 scaledValues[6];
} sndListenerState_t;

typedef struct sndFileRequestState_t {
    u8 path[0x100];
    s32 size;
    s32 pending;
    s32 data;
} sndFileRequestState_t;

typedef struct sndChannel_t {
    u8 index;
    u8 unk1;
    s16 unk2;
    u32 unk4;
    u32 iopWorkAddress;
    u32 unkC;
} sndChannel_t;

typedef struct sndIopBuffer_t {
    s32 size;
    u32 iopAddress;
} sndIopBuffer_t;

typedef struct {
    u8 values[24];
    u8 pad18[8];
} sndMidiStateGroup_t;

typedef struct sndMergedWork_t {
    s32 header;
    s32 bufferCount;
    u32 iopBufferAddress;
    u32 iopBufferBytes;
    sndChannel_t channels[16];
    sndIopBuffer_t buffers[16];
    p2sdLoadEntry_t slots[7];
    void *eeWorkAddress;
    u32 eeWorkByteCount;
    u32 opaque1D0;
    u32 opaque1D4;
    u32 opaque1D8;
    u32 opaque1DC;
    u8 unk1E0[0x380];
    sndMidiStateGroup_t stateGroups[2];
} sndMergedWork_t;

typedef struct sdfSoundRpcRequest_t {
    u8 opaque00[8];
    s32 destination;
    s32 byteCount;
} sdfSoundRpcRequest_t;

enum sdf_tex_resource {
    SDF_TEX_RESOURCE_FREE = 0,
    SDF_TEX_RESOURCE_FRAME = 1,
    SDF_TEX_RESOURCE_TEXTURE = 2,
    SDF_TEX_RESOURCE_CLUT = 3,
    SDF_TEX_RESOURCE_KNOWN_MODE_COUNT = 4
};

typedef struct {
    u32 control;
    u32 address;
    u32 vifCode0;
    u32 vifCode1;
} sdfDmaDumpTag_t;

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

extern void sdfSndTransToIOP(s32 aIOPAddr, void *aEEAddr, s32 aSize);

extern void *memcpy(void *, const void *, unsigned int);

extern void sdfDelayThread(s32 delay);

extern char * strcpy(char *, const char *);


extern sndRingPacket_t D_00590840[32];

extern u32 sdfSndSendCmd_cmd_id;

extern vu16 sdfSndCmdIndex;

extern vu16 sdfSndCmdTailIndex;

extern s32 sdfSndCmdBuf;

extern s32 WakeupThread(s32 thread);

extern void FlushCache(s32);

extern sndListenerState_t D_005907F0;

char *sdfSndMakeFilename(char *aDstName, const char *aSrcName);

extern s32 p2sdLoadCheckMidi(s32 MidiNo);

extern s32 p2sdSmgHandle;

extern s32 sprintf();

extern sndFileRequestState_t D_00593E20;

extern s32 D_00552120;

extern char D_00529BE8[];

extern s32 sifRpcServerSema;

extern char D_00551A20[];

extern char D_00551A28[];

extern char D_00551A30[];

extern char D_00551A38[];

extern char D_00551A40[];

extern char sdfDumpLineBuf[];

extern char D_00551A50[];

extern char D_00549B50[];

extern char D_00549B70[];

extern char D_00549B88[];

extern char D_00549BB0[];


extern char *sdfDmaTagNameTable[];

sdfGsMemSlice_t *sdfGsMemGetLinkHead(void);

extern char D_0054A088[];

extern char D_0054A100[];

extern char D_00551B40[];

extern char *D_00529CC8[];

sdfGsMemSlice_t *sdfGsMemGetLinkTail(void);

extern char *D_00529CD8[];

void sdfMemGetInfo(sdfMemInfo_t *aInfo);

void sdfChipGetInfo(sdfChipInfo_t *aInfo);

extern s32 (*p2sdSmgBuf)(void);

void p2sdLoadSMGFile(s32 MidiNo);

u32 *sdfFileRpcHandleCmd(u32 command, sdfSoundRpcRequest_t *request);

typedef struct sdfFileCmdHandle_t sdfFileCmdHandle_t;

extern sdfFileCmdHandle_t *sdfFileRpcArg;
extern sdfSoundRpcRequest_t *sdfFileRpcResult;
extern s32 sdfFileRpcCmdState;
/* This handler writes the first word of the RPC reply block. */
extern u32 sdfFileRpcQueue[];
/* The first cell stores the PCM base pointer. This unsized view keeps the
 * non-small-data address form; see docs/levers/addressing-and-types.md. */
extern u8 *p2sdPcmBuffer[];
extern sdfMemHandle_t *sdfMemAllocDirect(s32 size);
extern void *sdfMemLock(sdfMemHandle_t *handle);
extern void sdfMemFreeDirect(sdfMemHandle_t *handle);
extern sdfFileCmdHandle_t *sdfFileCmdReadOpen(
    const char *path,
    void (*callback)(sdfFileCmdHandle_t *, s32, s32, s32, s32),
    s32 callbackContext);
extern s32 sdfFileCmdClearError(sdfFileCmd_t *handle);
extern s32 sdfFileCmdFree(sdfFileCmd_t *handle);
extern s32 sdfFileCmdRead(void *handle, void *buffer, s32 byteCount);
extern s32 sdfFileCmdClose(void *handle);

void sdfGrFlushOtChain(u32 *source);

s32 sdfPrintf(const char *str, ...);

extern s32 sceSifAllocIopHeap(s32 size);
extern s32 sceSifQueryMaxFreeMemSize(void);
extern s32 sceSifQueryMemSize(void);
extern s32 sceSifQueryTotalFreeMemSize(void);
extern s32 printf(const char *format, ...);

s32 sdfSndIopAlloc(s32 size) {
    s32 address;

    address = sceSifAllocIopHeap(size);
    printf("--------\n");
    printf("sceSifAllocIopHeap         :%08x\n", size);
    printf("sceSifQueryMaxFreeMemSize  :%08x\n", sceSifQueryMaxFreeMemSize());
    printf("sceSifQueryMemSize         :%08x\n", sceSifQueryMemSize());
    printf("sceSifQueryTotalFreeMemSize:%08x\n", sceSifQueryTotalFreeMemSize());
    return address;
}

INCLUDE_ASM(const s32, "sdf/sdfSound", sdfSndCmdTrans);

void sdfSndCmdIFThread(void)
{
    for (;;) {
        SleepThread();
        if (sdfSndBurstReqMode == 0) {
            while (sdfSndCmdTrans() == 0) {
                ;
            }
        }
    }
}

typedef struct sdfSndInitThreadParam_t {
    s32 status;
    void (*entry)(void);
    void *stack;
    s32 stackSize;
    void *gp;
    s32 priority;
    u8 reserved[0x18];
} sdfSndInitThreadParam_t;

extern sdfSndInitThreadParam_t D_005907C0;
extern u8 D_00591840[];
extern u8 D_00592880[];
extern s32 D_005519D4;
extern s32 _gp;
extern s32 sceSifGetSreg(s32 index);
extern s32 CreateThread(sdfSndInitThreadParam_t *parameter);
extern s32 StartThread(s32 threadId, s32 argument);
extern s32 CreateThreadSimple(void *entryAddress, void *workspace, s32 stackBytes, s32 priority);
extern void SifRpcServerThreadProc(void);

void sdfSndInitCmdIF(void)
{
    s32 retries = 0;
    s32 threadId;

    while (sceSifGetSreg(0x1F) == 0) {
        retries++;
        if (retries == 0x5F5E100) {
            printf("sifSndInitCmdIF locked.\n");
            retries = 0;
        }
    }
    D_005907C0.entry = sdfSndCmdIFThread;
    D_005907C0.stack = D_00591840;
    D_005907C0.stackSize = 0x1000;
    /* Same descriptor order as CreateThreadSimple in sdfSystem.c. */
    D_005907C0.priority = 0x42;
    D_005907C0.gp = &_gp;
    sdfSndCmdIndex = 0;
    sdfSndCmdTailIndex = 0;
    threadId = CreateThread(&D_005907C0);
    sdfSndCmdBuf = threadId;
    StartThread(threadId, 0);
    threadId = CreateThreadSimple(SifRpcServerThreadProc, D_00592880, 0x1000, 0x43);
    D_005519D4 = threadId;
    StartThread(threadId, 0);
}

s32 sdfSndSendCmd(s32 aCmd, s32 aParam, void *aExDataPtr, s32 aExDataSize) {
    s32 nextWriteIndex;
    s32 wakeAttemptsRemaining;
    sndRingPacket_t *entry;
    u32 transferQuadwords;
    if (++sdfSndSendCmd_cmd_id == 0) {
        sdfSndSendCmd_cmd_id = 1;
    }
    nextWriteIndex = ((s16)sdfSndCmdTailIndex + 1) & (32 - 1);
    if (nextWriteIndex == (s16)sdfSndCmdIndex) {
        for (wakeAttemptsRemaining = 8; wakeAttemptsRemaining != 0; wakeAttemptsRemaining--) {
            do {
                WakeupThread(sdfSndCmdBuf);
            } while ((s16)sdfSndCmdIndex != nextWriteIndex);
        }
        return 0;
    }
    entry = &D_00590840[(s16)sdfSndCmdTailIndex];
    entry->sequence = sdfSndSendCmd_cmd_id;
    if (aExDataSize != 0) {
        memcpy(entry->payload, aExDataPtr, aExDataSize);
    }
    /* The SIF length includes the transport/command header and rounds up to quadwords. */
    transferQuadwords = (aExDataSize + 0x14 +
                        (16 - 1)) >> 4;
    entry->command = (aCmd << 16) |
                     (aParam & 0xFFFF) |
                     (transferQuadwords << 28);
    sdfSndCmdTailIndex = nextWriteIndex;
    if (sdfSndBurstReqMode != 0) {
        while (sdfSndCmdTrans() != 0) {
        }
    } else {
        WakeupThread(sdfSndCmdBuf);
    }
    return sdfSndSendCmd_cmd_id;
}

void sdfSndSyncCmd(s32 aCmdId)
{
}

s32 sdfSndSendSyncCmd(s32 aCmd, s32 aParam, void *aExDataPtr, s32 aExDataSize)
{
    s32 handle = sdfSndSendCmd(aCmd, aParam, aExDataPtr, aExDataSize);

    sdfSndSyncCmd(handle);
    return handle;
}

/* The SIF DMA request block, reused for every transfer. */
extern struct
{
    u32 source;
    s32 destination;
    s32 size;
    s32 attributes;
} sdfSndTransBuf;

extern s32 sceSifSetDma(void *aRequest, s32 aCount);

extern s32 sceSifDmaStat(s32 aId);

void sdfSndTransToIOP(s32 aIOPAddr, void *aEEAddr, s32 aSize)
{
    s32 dmaId;
    s32 delay;

    sdfSndTransBuf.source = (u32)aEEAddr;
    sdfSndTransBuf.destination = aIOPAddr;
    sdfSndTransBuf.size = aSize;
    sdfSndTransBuf.attributes = 0;
    FlushCache(0);
    while ((dmaId = sceSifSetDma(&sdfSndTransBuf, 1)) == 0) {
        /* Spin before retrying a full SIF queue. */
        delay = 0x186A0;
        while (delay != 0) {
            delay -= 0x271;
            __asm__ volatile("nop");
        }
    }
    while (sceSifDmaStat(dmaId) >= 0) {
    }
}

INCLUDE_ASM(const s32, "sdf/sdfSound", sdfSndLoadIRXModules);

INCLUDE_ASM(const s32, "sdf/sdfSound", sdfSndVoiceMapInit);

INCLUDE_ASM(const s32, "sdf/sdfSound", sdfSndSeqMapInit);

extern s32 sceSifInitIopHeap(void);
extern s32 D_0055213C;

void sdfSndInit(char *aVoiceInfo, s32 aVoiceCount, char *aSequenceInfo, s32 aSequenceCount)
{
    s32 iopAddress;

    sdfSndBurstReqMode = 1;
    sdfSndLoadIRXModules();
    sceSifInitIopHeap();
    sdfSndInitCmdIF();
    printf("SDF_SND_READ_BUFF_SIZE\n");
    sdfSndMap[2] = sdfSndIopAlloc(0x4000);
    sdfSndMap[3] = 0x4000;
    sdfSndVoiceMapInit(aVoiceInfo, aVoiceCount);
    sdfSndSeqMapInit((u32 *)aSequenceInfo, aSequenceCount);
    printf("sizeof(sdfSndMap)\n");
    iopAddress = sdfSndIopAlloc(0x5A0);
    sdfSndMap[0x72] = (u32)sdfSndMap;
    sdfSndMap[0x73] = 0x5A0;
    D_0055213C = iopAddress;
    sdfSndTransToIOP(iopAddress, sdfSndMap, 0x5A0);
    sdfSndSendSyncCmd(0, 0, &D_0055213C, 4);
    sdfSndBurstReqMode = 0;
}

/* Starts a track with the default 0x7f setting after preparing its identifier. */
void p2sdPlayBGM(s32 MidiNo) {
    sdfSndTrackPacket_t packet;
    p2sdLoadSMGFile(MidiNo);
    packet.trackId = MidiNo;
    packet.unk4 = 0;
    packet.setting = 0x7F;
    sdfSndSendCmd(0x20, 0, &packet, 0x10);
}

s32 p2sdSendStopCmd(s32 aId)
{
    s32 arg = aId;

    return sdfSndSendCmd(0x30, 0, &arg, 0x10);
}

/* Starts the alternate track command, leaving the other packet fields untouched. */
void p2sdResumeBgm(s32 trackId) {
    sdfSndTrackPacket_t packet;
    p2sdLoadSMGFile(trackId);
    packet.trackId = trackId;
    packet.setting = 0x7F;
    sdfSndSendCmd(0x130, 0, &packet, 0x10);
}

s32 p2sdStopSe(s32 aId)
{
    s32 arg = aId;

    return sdfSndSendCmd(0x30, 0, &arg, 0x10);
}

s32 p2sdStopAllBgm(void)
{
    return sdfSndSendCmd(0x1A0, 0, 0, 0);
}

s32 p2sd_2C1788(void)
{
    return sdfSndSendCmd(0x210, 0, 0, 0);
}

s32 p2sdStopAllSe(void)
{
    return sdfSndSendCmd(0x40, 0, 0, 0);
}

s32 p2sd_2C17D8(s32 aSeId)
{
    return sdfSndSendCmd(aSeId | 0x50, 0, 0, 0);
}

/* Send a NUL-terminated command string; the transport excludes the terminator. */
u32 sdfSndLoadHDFile(s32 aVcChNum, char *aFilename) {
    u32 length = strlen(aFilename);
    return sdfSndSendSyncCmd(aVcChNum | 0x70, 0, aFilename, length);
}

u32 sdfSndLoadBDFile(s32 aVcChNum, char *aFilename) {
    u32 length = strlen(aFilename);
    return sdfSndSendSyncCmd(aVcChNum | 0x60, 0, aFilename, length);
}

/* Send text with command bit 0x80; payload length excludes its NUL terminator. */
u32 sdfSndLoadSQFile(s32 aSqChNum, char *aFilename) {
    u32 length = strlen(aFilename);
    return sdfSndSendSyncCmd(aSqChNum | 0x80, 0, aFilename, length);
}

/* Send a 12-byte sequence command; the trailing struct word is not transmitted. */
void p2sdPlaySE(s32 trackId, s32 volume, s32 pan) {
    sndSequenceVolumePanPacket_t packet;
    p2sdLoadSMGFile(trackId);
    packet.trackId = trackId;
    packet.volume = volume;
    packet.pan = (u8)pan;
    sdfSndSendCmd(0x90, 0, &packet, 0xC);
}

INCLUDE_ASM(const s32, "sdf/sdfSound", p2sdPlaySeDeg);

INCLUDE_ASM(const s32, "sdf/sdfSound", func_002C1990);

/* Converts world coordinates to the sound engine's one-tenth scale. */
void p2sdPlaySe3DEx(s32 MidiNo, s32 posx, f32 posy, f32 posz, f32 limit) {
    u32 packet[8];
    packet[0] = MidiNo;
    packet[1] = posx;
    packet[2] = (s32)(posy * 0.1f);
    packet[3] = (s32)(posz * 0.1f);
    packet[4] = (s32)(limit * 0.1f);
    sdfSndSendSyncCmd(0x170, 0, packet, 0x20);
}

/* Scale six world-space values and send them to the sound engine when they changed. */
void p2sdSetStandPos(f32 Cposx, f32 Cposy, f32 Cposz,
                                 f32 Pposx, f32 Pposy, f32 Pposz) {
    sndListenerState_t state;
    state.scaledValues[0] = Cposx * 0.1f;
    state.scaledValues[1] = Cposy * 0.1f;
    state.scaledValues[2] = Cposz * 0.1f;
    state.scaledValues[3] = Pposx * 0.1f;
    state.scaledValues[4] = Pposy * 0.1f;
    state.scaledValues[5] = Pposz * 0.1f;
    if (D_005907F0.scaledValues[0] != state.scaledValues[0] || D_005907F0.scaledValues[1] != state.scaledValues[1] ||
        D_005907F0.scaledValues[2] != state.scaledValues[2] || D_005907F0.scaledValues[3] != state.scaledValues[3] ||
        D_005907F0.scaledValues[4] != state.scaledValues[4] || D_005907F0.scaledValues[5] != state.scaledValues[5]) {
        sdfSndSendSyncCmd(0x160, 0, &state, 0x20);
        D_005907F0 = state;
    }
}

extern void InvalidDCache(void *start, void *end);

s32 p2sdLoadCheckMidi(s32 MidiNo)
{
    sndMergedWork_t *work = (sndMergedWork_t *)sdfSndMap;
    s32 bank = MidiNo >> 16;
    s32 i;

    InvalidDCache(work, (u8 *)work + 0x5A0);
    for (i = 0; i < 7; i++) {
        if (work->slots[i].unk0 == bank) {
            return 1;
        }
    }
    return (p2sdSmgHandle == bank) ? 2 : 0;
}

/* Make sure the MIDI bank named by the packed track id is resident, loading it if not. */
void p2sdLoadSMGFile(s32 MidiNo) {
    char name[0x10];
    char path[0x100];
    s32 status = p2sdLoadCheckMidi(MidiNo);
    s32 id;
    switch (status) {
    case 0:
        id = MidiNo >> 16;
        sprintf(name, "MIDI%04X.SMG", id);
        sdfSndMakeFilename(path, name);
        sdfSndSendSyncCmd(0xA0, 0, path, strlen(path) + 1);
        p2sdSmgHandle = id;
        break;
    case 1:
        break;
    case 2:
        do {
            sdfDelayThread(2);
            if (p2sdSmgBuf != NULL) {
                p2sdSmgBuf();
            }
        } while (p2sdLoadCheckMidi(MidiNo) != 1);
        break;
    }
}

u64 p2sd_2C1D78(const char *aName)
{
    return sdfSndSendSyncCmd(0xA0, 0, (void *)aName, strlen(aName));
}

void p2sdSetPcmBuffer(s32 data, s32 size) {
    s32 *packet;
    D_00593E20.size = size;
    D_00593E20.pending = 1;
    D_00593E20.data = data;
    if (data != 0 && size > 0) {
        packet = &D_00552120;
        *packet = data;
        sdfSndSendSyncCmd(0x1B0, 0, packet, 4);
    }
    p2sdSmgHandle = -1;
}

u64 p2sd_2C1E10(s32 aCh)
{
    return sdfSndSendSyncCmd((aCh & 0xF) | 0x1C0, 0, NULL, 0);
}

u64 p2sd_2C1E40(u8 aCh)
{
    return sdfSndSendSyncCmd(((aCh >> 3) & 0xF) | 0x140, 0, NULL, 0);
}

u64 func_002C1E70(u8 aCh)
{
    return sdfSndSendSyncCmd(((aCh >> 3) & 0xF) | 0x150, 0, NULL, 0);
}

void p2sd_2C1EA0(s32 id) {
    u32 packet[4];
    if (p2sdLoadCheckMidi(id) != 0) {
        packet[0] = id;
        sdfSndSendSyncCmd(0xB0, 0, packet, 0x10);
        id >>= 16;
        if (p2sdSmgHandle == id) {
            p2sdSmgHandle = -1;
        }
    }
}

u8 p2sd_2C1EF8(s32 aIndex)
{
    return LastLoad[aIndex].unk4;
}

u8 p2sd_2C1F10(s32 aIndex)
{
    return LastLoad[aIndex].unk5;
}

s32 p2sd_2C1F28(s32 aIndex)
{
    s32 remaining = LastLoad[aIndex].unk4 - LastLoad[aIndex].unk5;

    if (remaining > 0) {
        return remaining;
    }
    return 0;
}

u8 *p2sd_2C1F50(void)
{
    return D_00593890;
}

p2sdLoadEntry_t *p2sd_2C1F60(void)
{
    return LastLoad;
}

u32 p2sd_2C1F70(u32 *aOutValue)
{
    if (aOutValue != NULL) {
        *aOutValue = sdfSndMap[0x77];
    }
    return sdfSndMap[0x76];
}

/* Pack two 24-byte MIDI state groups into out; NULL leaves the state untouched. */
void p2sd_2C1F90(u8 *out) {
    s32 group;
    if (out == NULL) {
        return;
    }
    for (group = 0; group < 2; group++) {
        s32 byteIndex;
        for (byteIndex = 0; byteIndex < 24; byteIndex++) {
            *out++ = ((sndMergedWork_t *)sdfSndMap)->stateGroups[group].values[byteIndex];
        }
    }
}

/* Sends a prepared track identifier with the 0x17f setting. */
void p2sdPlayBGMFade(s32 trackId) {
    sdfSndTrackPacket_t packet;
    p2sdLoadSMGFile(trackId);
    packet.trackId = trackId;
    packet.unk4 = 0;
    packet.setting = 0x17F;
    sdfSndSendCmd(0x20, 0, &packet, 0x10);
}

s32 p2sdFadeOutBgm(s32 aId)
{
    struct
    {
        s32 id;
        s32 unk4;
        s16 fade;
    } arg;

    arg.id = aId;
    arg.unk4 = 0;
    arg.fade = 0x17F;
    return sdfSndSendCmd(0x20, 0, &arg, 0x10);
}

s32 p2sdStopBGM(s32 aId)
{
    s32 arg = aId;

    return sdfSndSendCmd(0xD0, 0, &arg, 0x10);
}

s32 p2sdBgmVolDown(void)
{
    return sdfSndSendCmd(0x180, 0, 0, 0);
}

s32 p2sdBgmVolUp(void)
{
    return sdfSndSendCmd(0x190, 0, 0, 0);
}

void p2sdSetBgmTrack(s32 aTrack)
{
    sdfSndSendCmd(0xE0 | ((aTrack + 1) & 0xF), 0, NULL, 0);
}

s32 p2sdLoadPcmFile(const char *name, u8 channel) {
    if (p2sdPcmState != 0) {
        return 1;
    }
    strcpy(D_00529BE8, name);
    sdfSndSendSyncCmd((channel >> 3) | 0xF0, 0, 0, 0);
    return 0;
}

u32 p2sdGetPcmState(void)
{
    return p2sdPcmState;
}

s32 p2sdSyncSound(void)
{
    return sdfSndSendSyncCmd(0x100, 0, 0, 0);
}

void p2sd_2C21A8(u8 channel) {
    sdfSndSendSyncCmd(((channel >> 3) & 0xF) | 0x110, 0, 0, 0);
}

s32 p2sd_2C21D8(u32 aCh)
{
    if (aCh > 16) {
        aCh = 16;
    }
    if (aCh == 0) {
        aCh = 1;
    }
    return sdfSndSendSyncCmd((aCh - 1) | 0x1D0, 0, NULL, 0);
}

u32 p2sdSetPcmCallback(u32 aCallback)
{
    if (p2sdPcmBusy != 0) {
        return 0;
    }
    p2sdPcmCallback = aCallback;
    return aCallback;
}

/* Send opcode 0x120 with the channel's group (bits 3-6) unless a command is
 * pending; returns nonzero when busy. */
s32 p2sdPlayPcm(u8 channel) {
    if (p2sdPcmBusy) {
        return 1;
    }
    channel = (channel >> 3) & 0xF;
    sdfSndSendSyncCmd(0x120 | channel, 0, 0, 0);
    return 0;
}

u32 p2sd_2C2280(void)
{
    return p2sdPcmBusy;
}

s32 p2sdSyncMovie(void)
{
    return sdfSndSendSyncCmd(0x100, 0, 0, 0);
}

/* Separate native copy of the command-0x110 channel-group sender. */
void p2sd_2C22B0(u8 channel) {
    sdfSndSendSyncCmd(((channel >> 3) & 0xF) | 0x110, 0, 0, 0);
}

s32 sdfFileRpcCallback(s32 unused, u32 event) {
    switch (event) {
    case 1:
        break;
    case 3:
        break;
    case 6:
        break;
    case 5:
        FlushCache(0);
        /* Fall through: flush and wake the waiting thread. */
    case 4:
        SignalSema(sifRpcServerSema);
        break;
    case 0:
    case 2:
    case 7:
        SignalSema(sifRpcServerSema);
        break;
    }
    return 0;
}

/* Handle sound-file and PCM-buffer RPC commands. */
u32 *sdfFileRpcHandleCmd(u32 command, sdfSoundRpcRequest_t *request)
{
    char path[0x100];

    switch (command) {
    case 0: {
        sdfFileCmdHandle_t *state;

        if (D_00529BE8[0] == 0) {
            sdfFileRpcQueue[0] = 0;
            break;
        }
        p2sdPcmState = 1;
        sdfSndMakeFilename(path, D_00529BE8);
        state = sdfFileCmdReadOpen(path,
            (void (*)(sdfFileCmdHandle_t *, s32, s32, s32, s32))sdfFileRpcCallback, 0);
        sdfFileRpcArg = state;
        if (state != NULL) {
            WaitSema(sifRpcServerSema);
            state = sdfFileRpcArg;
            if (((sdfFileCmd_t *)state)->error != 0) {
                sdfFileCmdClearError((sdfFileCmd_t *)state);
                sdfFileCmdFree((sdfFileCmd_t *)sdfFileRpcArg);
                sdfFileRpcArg = NULL;
                state = NULL;
            }
        }
        sdfFileRpcQueue[0] = (u32)state;
        break;
    }
    case 1: {
        sdfMemHandle_t *allocation;
        void *buffer;

        sdfFileRpcResult = request;
        allocation = sdfMemAllocDirect(request->byteCount);
        buffer = sdfMemLock(allocation);
        if (buffer != NULL) {
            sdfFileCmdRead(sdfFileRpcArg, buffer, sdfFileRpcResult->byteCount);
            WaitSema(sifRpcServerSema);
            sdfSndTransToIOP(sdfFileRpcResult->destination, buffer, sdfFileRpcResult->byteCount);
            sdfMemFreeDirect(allocation);
            sdfFileRpcQueue[0] = sdfFileRpcResult->byteCount;
        } else {
            sdfFileRpcQueue[0] = 0;
        }
        break;
    }
    case 2:
        sdfFileCmdClose(sdfFileRpcArg);
        WaitSema(sifRpcServerSema);
        sdfFileCmdFree((sdfFileCmd_t *)sdfFileRpcArg);
        sdfFileRpcArg = NULL;
        p2sdPcmState = 0;
        sdfFileRpcQueue[0] = 0;
        break;
    case 3:
        sdfFileRpcCmdState = 0;
        sdfFileRpcQueue[0] = 0;
        break;
    case 4: {
        s32 bytes;

        sdfFileRpcResult = request;
        sdfSndTransToIOP(request->destination, p2sdPcmBuffer[0] + sdfFileRpcCmdState,
                         request->byteCount);
        bytes = sdfFileRpcResult->byteCount;
        sdfFileRpcQueue[0] = bytes;
        sdfFileRpcCmdState += bytes;
        break;
    }
    case 5:
        sdfFileRpcQueue[0] = 0;
        break;
    case 6: {
        u32 word = p2sdPcmCallback;

        if (word != 0) {
            p2sdPcmBusy = 1;
        }
        sdfFileRpcQueue[0] = word;
        break;
    }
    case 7: {
        void *buffer;

        sdfFileRpcResult = request;
        buffer = ((u8 *(*)(s32))p2sdPcmCallback)(request->byteCount);
        if (buffer != NULL) {
            sdfSndTransToIOP(sdfFileRpcResult->destination, buffer,
                             sdfFileRpcResult->byteCount);
            sdfFileRpcQueue[0] = sdfFileRpcResult->byteCount;
        } else {
            sdfFileRpcQueue[0] = -1;
        }
        break;
    }
    case 8:
        p2sdPcmBusy = 0;
        sdfFileRpcQueue[0] = 0;
        break;
    }
    return sdfFileRpcQueue;
}

INCLUDE_ASM(const s32, "sdf/sdfSound", SifRpcServerThreadProc);

INCLUDE_ASM(const s32, "sdf/sdfSound", sdf_2C2608);

INCLUDE_ASM(const s32, "sdf/sdfSound", sdf_2C2618);

INCLUDE_ASM(const s32, "sdf/sdfSound", sdf_2C27F8);

/* Print size bytes in address-aligned, sixteen-column hexadecimal rows. */
void sdfDumpHexLine(const u8 *data, s32 size) {
    s32 column = 0;
    s32 i;
    while (size > 0) {
        if (column == 0) {
            sdfPrintf(D_00551A20, data);
            column = (u32)data & 0xF;
            for (i = column; i > 0; i--) {
                sdfPrintf(D_00551A28);
            }
        } else if (column == 8) {
            sdfPrintf(D_00551A30);
        } else {
            sdfPrintf(D_00551A38);
        }
        sdfPrintf(D_00551A40, *data++);
        column++;
        if (column == 0x10) {
            column = 0;
            sdfPrintf(sdfDumpLineBuf);
        }
        size--;
    }
    if (column == 0) {
        sdfPrintf(sdfDumpLineBuf);
    } else {
        sdfPrintf(D_00551A50);
    }
}

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_00549B50);

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_00549B70);

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_00549B88);

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_00549BB0);

void sdfDumpDmaChain(sdfDmaDumpTag_t *source) {
    sdfDmaDumpTag_t *callStack[2];
    sdfDmaDumpTag_t *cursor;
    u32 tag;
    u32 tagAddress;
    s32 tagType;
    s32 tagCount;
    s32 depth = 0;
    s32 done = 0;
    sdfPrintf(D_00549B50, source);
    cursor = source;
    for (;;) {
        if ((u32)cursor & 0xF) {
            sdfPrintf(D_00549B70, cursor);
            return;
        }
        tag = cursor->control;
        tagAddress = cursor->address;
        tagType = (tag >> 28) & 7;
        tagCount = tag & 0xFFFF;
        sdfPrintf(D_00549B88, cursor, sdfDmaTagNameTable[tagType], tag, tagAddress, cursor->vifCode0, cursor->vifCode1);
        cursor++;
        switch (tagType) {
        case 0:
            done = 1;
            break;
        case 1:
            cursor += tagCount;
            break;
        case 2:
            cursor = (sdfDmaDumpTag_t *)tagAddress;
            break;
        case 3:
        case 4:
            break;
        case 5:
            if (depth == 2) {
                sdfPrintf(D_00549BB0);
                return;
            }
            callStack[depth++] = cursor;
            cursor = (sdfDmaDumpTag_t *)tagAddress;
            break;
        case 6:
            if (depth == 0) {
                done = 1;
            } else {
                cursor = callStack[--depth];
            }
            break;
        case 7:
            cursor += tagCount;
            done = 1;
            break;
        }
        if (done != 0) {
            sdfPrintf("<<< DMA normal end [%p] >>>\n", cursor);
            return;
        }
    }
}

extern char *D_00529C68[];
extern s32 D_00529C88[];
extern char D_00551A58[];
extern char D_00551A60[];
extern char D_00551B10[];
extern char D_00551B18[];

void sdfGrFlushOtChain(u32 *source) {
    u32 *returnStack[2];
    u32 *cursor;
    u32 *wordCursor = NULL;
    s32 dataMode = 0;
    s32 tagId = 0;
    s32 depth = 0;
    s32 n = 0;
    s32 directRow;
    s32 phase;
    s32 wordsLeft;
    u32 tagAddress;
    s32 qwc;
    s32 done;
    u32 value;

    sdfPrintf("<<< DMA-VIF dump start [%p] >>>\n", source);
    cursor = source;
    directRow = 0;
    tagAddress = 0;
    qwc = 0;
    phase = 0;
    wordsLeft = 0;
    done = 0;
    for (;;) {
        if (wordsLeft == 0) {
            switch (phase) {
            case 0: {
                u32 tag;
                if ((u32)cursor & 0xF) {
                    sdfPrintf(D_00549B70, cursor);
                    return;
                }
                tag = cursor[0];
                tagAddress = cursor[1];
                tagId = (tag >> 28) & 7;
                qwc = tag & 0xFFFF;
                sdfPrintf("\n%08X [DMA %s] : %08X %08X\n",
                                           cursor, sdfDmaTagNameTable[tagId], tag, tagAddress);
                wordCursor = cursor + 2;
                cursor += 4;
                wordsLeft = 2;
                phase = 1;
                break;
            }
            case 1:
                wordsLeft = qwc * 4;
                switch (tagId) {
                case 0:
                case 3:
                case 4:
                    wordCursor = (u32 *)tagAddress;
                    break;
                default:
                    wordCursor = cursor;
                    break;
                }
                phase = 2;
                break;
            case 2:
                switch (tagId) {
                case 0:
                case 7:
                    done = 1;
                    break;
                case 1:
                    cursor += qwc * 4;
                    break;
                case 2:
                    cursor = (u32 *)tagAddress;
                    break;
                case 3:
                case 4:
                    break;
                case 5:
                    if (depth == 2) {
                        sdfPrintf("\ncall stack nest overflow!\n");
                        return;
                    }
                    returnStack[depth++] = cursor + qwc * 4;
                    cursor = (u32 *)tagAddress;
                    break;
                case 6:
                    if (depth == 0) {
                        done = 1;
                    } else {
                        depth--;
                        cursor = returnStack[depth];
                    }
                    break;
                }
                phase = 0;
                break;
            }
        } else {
            value = *wordCursor++;
            wordsLeft--;
            switch (dataMode) {
            case 0: {
                u32 opcode = (value >> 24) & 0x7F;
                u32 num = (value >> 16) & 0xFF;
                u32 immediate = value & 0xFFFF;
                s32 interrupt = (value >> 31) ? 'i' : ' ';
                s32 bits;
                s32 format;
                s32 unpackFormat;

                sdfPrintf("%08X %c%08X : ", (u32)(wordCursor - 1),
                                           interrupt, value);
                switch (opcode) {
                case 0x00:
                    sdfPrintf("VIF NOP\n");
                    break;
                case 0x01:
                    sdfPrintf("VIF STCYCL (CL=%d, WL=%d)\n", (s32)(immediate & 0xFF),
                                    (s32)(immediate >> 8));
                    break;
                case 0x02:
                    sdfPrintf("VIF OFFSET (%04X)\n", immediate & 0x3FF);
                    break;
                case 0x03:
                    sdfPrintf("VIF BASE   (%04X)\n", immediate & 0x3FF);
                    break;
                case 0x04:
                    sdfPrintf("VIF ITOP   (%04X)\n", immediate & 0x3FF);
                    break;
                case 0x05:
                    sdfPrintf("VIF STMOD  (%d)\n", (s32)(immediate & 3));
                    break;
                case 0x06:
                    sdfPrintf("VIF MSKPATH3 (%s)\n",
                                    (immediate & 0x8000) ? D_00551A58 : D_00551A60);
                    break;
                case 0x07:
                    sdfPrintf("VIF MARK   (%04X)\n", immediate);
                    break;
                case 0x10:
                    sdfPrintf("VIF FLUSHE\n");
                    break;
                case 0x11:
                    sdfPrintf("VIF FLUSH\n");
                    break;
                case 0x13:
                    sdfPrintf("VIF FLUSHA\n");
                    break;
                case 0x14:
                    sdfPrintf("VIF MSCAL  (%04X)\n", immediate);
                    break;
                case 0x15:
                    sdfPrintf("VIF MSCALF (%04X)\n", immediate);
                    break;
                case 0x17:
                    sdfPrintf("VIF MSCNT\n");
                    break;
                case 0x20:
                    dataMode = 1;
                    sdfPrintf("VIF STMASK\n");
                    break;
                case 0x30:
                    dataMode = 2;
                    n = 4;
                    sdfPrintf("VIF STROW\n");
                    break;
                case 0x31:
                    dataMode = 3;
                    n = 4;
                    sdfPrintf("VIF STCOL\n");
                    break;
                case 0x4A:
                    n = num * 2;
                    sdfPrintf("VIF MPG   (%04X)\n", immediate);
                    if (n == 0) {
                        n = 0x200;
                    }
                    dataMode = 4;
                    break;
                case 0x50:
                    n = immediate * 4;
                    if (n == 0) {
                        n = 0x40000;
                    }
                    sdfPrintf("VIF DIRECT (%06X bytes)\n", (u32)(n * 4));
                    dataMode = 5;
                    directRow = 0;
                    break;
                case 0x51:
                    n = immediate * 4;
                    if (n == 0) {
                        n = 0x40000;
                    }
                    sdfPrintf("VIF DIRECTHL (%06X bytes)\n", (u32)(n * 4));
                    dataMode = 6;
                    directRow = 0;
                    break;
                default:
                    if ((opcode & 0x60) == 0x60) {
                        unpackFormat = opcode & 0xF;
                        if (((char **)((u8 *)sdfDmaTagNameTable + 0x20))[unpackFormat] == NULL) {
                            sdfPrintf("illegal unpack type !\n");
                            return;
                        }
                        format = (((s32)immediate >> 13) & 6) | (((s32)opcode >> 4) & 1);
                        dataMode = 7;
                        sdfPrintf("VIF UNPACK%s %s,%04X,%04X\n",
                                                   D_00529C68[format],
                                                   ((char **)((u8 *)sdfDmaTagNameTable + 0x20))[unpackFormat],
                                                   immediate & 0x3FF, num);
                        bits = D_00529C88[unpackFormat];
                        n = (bits * (s32)num + 0x1F) >> 5;
                    } else {
                        sdfPrintf("unkown vif command\n");
                        return;
                    }
                    break;
                }
                break;
            }
            case 1:
                sdfPrintf("    STMASK : %08X\n", value);
                dataMode = 0;
                break;
            case 2:
                n--;
                sdfPrintf("    STROW : %08X\n", value);
                if (n == 0) {
                    dataMode = 0;
                }
                break;
            case 3:
                n--;
                sdfPrintf("    STCOL : %08X\n", value);
                if (n == 0) {
                    dataMode = 0;
                }
                break;
            case 4:
                if (--n == 0) {
                    dataMode = 0;
                }
                break;
            case 5:
                if (directRow == 0) {
                    sdfPrintf(D_00551B10);
                } else {
                    sdfPrintf(D_00551A38);
                }
                directRow++;
                sdfPrintf(D_00551B18, value);
                if (directRow == 4) {
                    directRow = 0;
                    sdfPrintf(sdfDumpLineBuf);
                }
                if (--n == 0) {
                    if (directRow != 0) {
                        sdfPrintf(sdfDumpLineBuf);
                    }
                    dataMode = 0;
                }
                break;
            case 6:
                if (--n == 0) {
                    dataMode = 0;
                }
                break;
            case 7:
                if (--n == 0) {
                    dataMode = 0;
                }
                break;
            }
        }
        if (done != 0) {
            sdfPrintf("<<< DMA-VIF normal end [%p] >>>\n", (void *)wordCursor);
            return;
        }
    }
}

void sdfDumpGsMemory(void) {
    char buf[8];
    sdfGsMemSlice_t *head;
    sdfGsMemSlice_t *node;
    sdfGsMemSlice_t *walk;
    char *name;
    sdfPrintf(D_0054A088);
    head = sdfGsMemGetLinkHead();
    node = head;
    while (node != 0) {
        if ((u32)node->type < SDF_TEX_RESOURCE_KNOWN_MODE_COUNT) {
            name = D_00529CC8[node->type];
        } else {
            sdfPrintf(buf, D_00551B40, node->type);
            name = buf;
        }
        sdfPrintf(D_0054A100, node, node->prev, node->next, name, node->addr, node->size);
        walk = head;
        while (walk != node) {
            walk = walk->next;
        }
        node = node->next;
    }
}

void sdfDumpGsMemory2(void) {
    char buf[8];
    sdfGsMemSlice_t *head;
    sdfGsMemSlice_t *node;
    sdfGsMemSlice_t *walk;
    char *name;
    sdfPrintf(D_0054A088);
    head = sdfGsMemGetLinkTail();
    node = head;
    while (node != 0) {
        if ((u32)node->type < SDF_TEX_RESOURCE_KNOWN_MODE_COUNT) {
            name = D_00529CD8[node->type];
        } else {
            sdfPrintf(buf, D_00551B40, node->type);
            name = buf;
        }
        sdfPrintf(D_0054A100, node, node->prev, node->next, name, node->addr, node->size);
        walk = head;
        while (walk != node) {
            walk = walk->prev;
        }
        node = node->prev;
    }
}

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_0054A088);

INCLUDE_RODATA(const s32, "sdf/sdfSound", D_0054A100);

void sdfDumpMemory(void) {
    sdfMemInfo_t info;
    sdfMemGetInfo(&info);
    sdfPrintf(" <<< memory information >>>\n             total : 0x%06X\n        free total : 0x%06X\n     max free size : 0x%06X\n     min free size : 0x%06X\n      handle total : %d\n free handle count : %d\n\n",
                    info.total, info.freeSize,
                    info.maxFreeSize, info.minFreeSize,
                    info.handleCount, info.freeHandleCount);
}

/* Print the chip heap totals and how many cells are in use per size class (1..16, 17..32, ...). */
void sdfDumpChipMemory(void) {
    sdfChipInfo_t stats;
    u32 limit = 16;
    s32 i = 0;
    u32 first;
    u32 *used;
    sdfChipGetInfo(&stats);
    sdfPrintf(" <<< chip memory information >>>\n                 total : 0x%06X\n            free total : 0x%06X\n            page count : %d\n       free page count : %d\n fragmented page count : %d\n", stats.total, stats.freeSize, stats.pageCount, stats.freePageCount, stats.fragmentedPageCount);
    sdfPrintf("\n several size alloc count...\n");
    first = 0;
    used = stats.severalAllocCount;
    do {
        sdfPrintf("       for %3d .. %4d : %d\n", first, limit, *used++);
        i++;
        first = limit + 1;
        limit *= 2;
    } while (i != 7);
    sdfPrintf(sdfDumpLineBuf, first);
}
