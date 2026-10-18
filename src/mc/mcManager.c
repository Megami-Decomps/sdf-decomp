#include "common.h"

#define MC_CARD_SLOT 0
#define MC_SYNC_POLL_MODE 1
#define MC_SYNC_COMPLETE 1
#define MC_SDK_COMMAND_SUCCESS 0
#define MC_SDK_SPECIAL_ERROR (-4)
#define MC_SDK_EXTENDED_ERROR_LIMIT (-10)

/* One poll attempt's result. PENDING means no terminal result yet; the SDK
 * poll values 0 and -1 both map to it. */
typedef enum McPollResult {
    MC_POLL_PENDING = 0,
    MC_POLL_SUCCESS = 1,
    MC_POLL_ERROR = -1,
    MC_POLL_SPECIAL_ERROR = -2
} McPollResult;

extern s32 sceMcSync(s32 mode, void *cmd, s32 *result);
extern s32 sceMcChdir(s32 port, s32 slot, const char *path, char *cwd);
extern s32 sceMcGetDir(s32 port, s32 slot, const char *path, u32 mode, s32 maxent, void *table);
extern s32 sceMcOpen(s32 port, s32 slot, const char *path, s32 flags);
extern s32 sceMcFormat(u32 port, s32 slot);
extern s32 sceMcMkdir(u32 port, s32 slot, const char *path);
extern s32 sceMcDelete(u32 port, s32 slot, const char *path);
extern s32 sceMcClose(s32 descriptor);
extern s32 sceMcRead(s32 descriptor, void *buffer, s32 size);
extern s32 sceMcWrite(s32 descriptor, const void *buffer, s32 size);

/* Each port's control block strides 40 bytes; the selected slot is at +19. */
typedef struct
{
    u8 unk00[19];
    u8 slotIndex;
    u8 unk14[20];
} sdfMcControl_t;

extern sdfMcControl_t sdfMcCb[];

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcFuncGetInfo);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcRegetInfo);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcFuncGetInfo1);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcGetFileInfoReady);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcGetFreeBytes);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcGetStatus);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcClearCardStat);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcSetCardStat);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcClearSlotFlags);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcSetSlotFlags);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcGetSlotFlags);

INCLUDE_ASM(const s32, "mc/mcManager", sdfMcGetSlotIndex);

void sdfMcSetSlotIndex(s32 port, s32 slot)
{
    sdfMcCb[port].slotIndex = slot;
}

void sdfMcFuncFormat(u32 port)
{
    sceMcFormat(port, MC_CARD_SLOT);
}

McPollResult sdfMcFuncFormat1(void)
{
    u32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult != MC_SYNC_COMPLETE) {
        return MC_POLL_PENDING;
    }
    if (commandResult == MC_SDK_COMMAND_SUCCESS) {
        return pollResult;
    }
    return MC_POLL_ERROR;
}

void sdfMcFuncChdir(u32 port, const char *path)
{
    sceMcChdir(port, MC_CARD_SLOT, path, 0);
}

McPollResult sdfMcFuncChdir1(void)
{
    s32 commandId;
    s32 commandResult;

    if (sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult) == MC_SYNC_COMPLETE) {
        if (commandResult == MC_SDK_COMMAND_SUCCESS) {
            return MC_POLL_SUCCESS;
        }
        return commandResult == MC_SDK_SPECIAL_ERROR ? MC_POLL_SPECIAL_ERROR : MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncMkdir(u32 port, const char *path)
{
    sceMcMkdir(port, MC_CARD_SLOT, path);
}

McPollResult sdfMcFuncMkdir1(void)
{
    u32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult == MC_SYNC_COMPLETE) {
        if (commandResult == MC_SDK_COMMAND_SUCCESS) {
            return pollResult;
        }
        if (commandResult < MC_SDK_EXTENDED_ERROR_LIMIT) {
            return MC_POLL_SPECIAL_ERROR;
        }
        return MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncGetDir(u32 port, const char *path, void *directoryEntries, s32 entryLimit)
{
    sceMcGetDir(port, MC_CARD_SLOT, path, 0, entryLimit, directoryEntries);
}

McPollResult sdfMcFuncGetDir1(s32 *commandResultOut)
{
    u32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult == MC_SYNC_COMPLETE) {
        if (commandResult >= MC_SDK_COMMAND_SUCCESS) {
            *commandResultOut = commandResult;
            return pollResult;
        }
        if (commandResult == MC_SDK_SPECIAL_ERROR) {
            return MC_POLL_SPECIAL_ERROR;
        }
        return MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncDelete(u32 port, const char *path)
{
    sceMcDelete(port, MC_CARD_SLOT, path);
}

McPollResult sdfMcFuncDelete1(void)
{
    s32 commandId;
    s32 commandResult;

    if (sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult) == MC_SYNC_COMPLETE) {
        if (commandResult == MC_SDK_COMMAND_SUCCESS) {
            return MC_POLL_SUCCESS;
        }
        return commandResult == MC_SDK_SPECIAL_ERROR ? MC_POLL_SPECIAL_ERROR : MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncOpen(u32 port, const char *path, s32 openFlags)
{
    sceMcOpen(port, MC_CARD_SLOT, path, openFlags);
}

McPollResult sdfMcFuncOpen1(s32 *commandResultOut)
{
    u32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult == MC_SYNC_COMPLETE) {
        if (commandResult >= MC_SDK_COMMAND_SUCCESS) {
            *commandResultOut = commandResult;
            return pollResult;
        }
        if (commandResult == MC_SDK_SPECIAL_ERROR) {
            return MC_POLL_SPECIAL_ERROR;
        }
        return MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncClose(s32 descriptor)
{
    sceMcClose(descriptor);
}

McPollResult sdfMcFuncClose1(void)
{
    s32 commandId;
    s32 commandResult;

    if (sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult) == MC_SYNC_COMPLETE) {
        if (commandResult == MC_SDK_COMMAND_SUCCESS) {
            return MC_POLL_SUCCESS;
        }
        return commandResult == MC_SDK_SPECIAL_ERROR ? MC_POLL_SPECIAL_ERROR : MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncRead(s32 descriptor, void *buffer, s32 size)
{
    sceMcRead(descriptor, buffer, size);
}

McPollResult sdfMcFuncRead1(void)
{
    u32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult == MC_SYNC_COMPLETE) {
        if (commandResult >= MC_SDK_COMMAND_SUCCESS) {
            return pollResult;
        }
        if (commandResult == MC_SDK_SPECIAL_ERROR) {
            return MC_POLL_SPECIAL_ERROR;
        }
        return MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}

void sdfMcFuncWrite(s32 descriptor, const void *buffer, s32 size)
{
    sceMcWrite(descriptor, buffer, size);
}

McPollResult sdfMcFuncWrite1(void)
{
    s32 commandId;
    s32 commandResult;
    s32 pollResult = sceMcSync(MC_SYNC_POLL_MODE, &commandId, &commandResult);

    if (pollResult == MC_SYNC_COMPLETE) {
        if (commandResult >= MC_SDK_COMMAND_SUCCESS) {
            return pollResult;
        }
        if (commandResult == MC_SDK_SPECIAL_ERROR) {
            return MC_POLL_SPECIAL_ERROR;
        }
        return MC_POLL_ERROR;
    }
    return MC_POLL_PENDING;
}
