#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

s32 strlen(const char *aStr);

typedef struct sifRpcClientDataPrefix_t {
    u8 opaque00[0x24];
    void *server;
} sifRpcClientDataPrefix_t;

typedef struct sifRpcClientContextView_t {
    sifRpcClientDataPrefix_t client;
    u8 opaque28[8];
} sifRpcClientContextView_t;

typedef void (*sifMRpcEnd_t)(void *argument);

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

s32 sceSifMCallRpc(sifRpcClientDataPrefix_t *client, s32 function, s32 mode, void *send, s32 sendSize,
                   void *receive, s32 receiveSize, sifMRpcEnd_t end, void *endArgument);

extern sifRpcClientContextView_t D_0058F310;

extern s8 sdfPrintIopFlag;

extern char D_00551908[];

extern s32 scePrintf(const char *, ...);

extern s32 vsprintf(char *destination, const char *format, void *args);

extern s32 (*p2sdSmgBuf)(void);

/* Copy a positive source/end span into the RPC reply buffer; otherwise return NULL. */
void *sdfPrintRpcFunc(s32 unused, sifCommand_t *packet) {
    s32 copyBytes = packet->end - packet->source;
    if (copyBytes > 0) {
        memcpy(packet, (void *)packet->source, copyBytes);
        return packet;
    }
    return NULL;
}

INCLUDE_ASM(const s32, "sdf/sdfPrint", sdfPrintRpcThread);

INCLUDE_ASM(const s32, "sdf/sdfPrint", sdf_2BD3E8);

INCLUDE_ASM(const s32, "sdf/sdfPrint", sdfPrintIopInit);

/* Print text through the bound IOP dev client, or locally when it is unbound. */
void sdfPuts(const char *text) {
    u8 buffer[0x13F];
    u8 *alignedBuffer;
    s32 length;
    if (sdfPrintIopFlag != 0) {
        length = strlen(text);
        if (length != 0) {
            alignedBuffer = (u8 *)(((u32)&buffer[0x3F]) & ~0x3F);
            memcpy(alignedBuffer, text, length);
            sceSifMCallRpc(&D_0058F310.client, 0x7D1, 0, alignedBuffer, length, NULL, 0, NULL, NULL);
        }
    } else {
        scePrintf(D_00551908, text);
    }
}

s32 sdfPrintf(const char *fmt, ...) {
    char buffer[0x100];
    __builtin_va_list args;
    s32 length;
    __builtin_stdarg_start(args, fmt);
    length = vsprintf(buffer, fmt, args);
    sdfPuts(buffer);
    return length;
}
INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfPrintIopFlag);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551901);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551908);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551910);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfCdTrayMode);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_0055191C);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileDefaultSrcType);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551921);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileSemaId);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551928);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551930);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551938);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551940);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551948);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdFreeList);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdCount);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551952);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551954);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551958);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdFreeCount);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdDefaultDevice);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdThreadPrioCheckCounter);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdThreadPrio);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551962);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfFileCmdDefaultCreateMode);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551966);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551968);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551970);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551978);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551980);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551988);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551990);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551998);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519A0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfDev9Ready);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519A9);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfCdClock);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519B1);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519B8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519B9);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", p2sdSmgHandle);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", p2sdPcmCallback);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", p2sdPcmState);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", p2sdPcmBusy);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfSndSendCmd_cmd_id);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519D4);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", p2sdSmgBuf);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519DC);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519E0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519E8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519F0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_005519F8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A00);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A08);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A10);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A18);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A20);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A28);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A30);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A38);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A40);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", sdfDumpLineBuf);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A4A);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A50);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A58);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A60);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A68);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A70);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A78);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A80);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A88);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A90);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551A98);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AA0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AA8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AB0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AB8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AC0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AC8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AD0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AD8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AE0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AE8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AF0);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551AF8);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B00);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B08);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B10);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B18);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B20);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B28);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B30);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B38);

INCLUDE_SDATA(const s32, "sdf/sdfPrint", D_00551B40);

