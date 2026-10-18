#include "common.h"

typedef struct fileRequest {
    /* 0x00 */ u8 kind;
    /* 0x01 */ u8 state;
    /* 0x02 */ u8 retry;
    /* 0x03 */ u8 mode;
    /* 0x04 */ struct fileRequest *next;
    /* 0x08 */ char *name;
    /* 0x0C */ void *dev;
    /* 0x10 */ s32 rest;
    /* 0x14 */ s32 size;
    /* 0x18 */ void (*callback)(struct fileRequest *, u32);
    /* 0x1C */ void *arg;
    /* 0x20 */ void *handle;
    /* 0x24 */ u32 top;
    /* 0x28 */ u32 ptr;
    /* 0x2C */ struct fileRequest *donenext;
    /* 0x30 */ u8 pac[0x38];
    /* 0x68 */ u16 pacrest;
    /* 0x6A */ u16 slot;
    /* 0x6C */ struct fileRequest *cbnext;
} fileRequest_t;

typedef struct fileSlot {
    /* 0x0 */ u32 size;
    /* 0x4 */ fileRequest_t *req;
} fileSlot_t;

typedef struct fileQueue {
    /* 0x00 */ s32 sema;
    /* 0x04 */ u8 readslot;
    /* 0x05 */ u8 nextslot;
    /* 0x06 */ u8 fullcnt;
    /* 0x07 */ u8 freecnt;
    /* 0x08 */ fileRequest_t *head;
    /* 0x0C */ fileRequest_t *tail;
    /* 0x10 */ fileRequest_t *done;
    /* 0x14 */ fileRequest_t *pacdone;
    /* 0x18 */ fileRequest_t *closewait;
    /* 0x1C */ u8 *buf;
    /* 0x20 */ fileSlot_t slots[4];
} fileQueue_t;

typedef struct sdfFileCmd {
    /* 0x00 */ u8 unk00[0x10];
    /* 0x10 */ char *path;
} sdfFileCmd_t;

extern fileQueue_t fileQueueHead;
extern u32 fileCallbackStatus;
extern void (*p2sdSmgBuf)(void);
extern char D_00551650[];
extern const char D_005468E8[];

void fileLinkQueue(fileRequest_t *req, s32 kind, const char *name, void *callback, void *arg);
void fileLinkCloseWait(fileQueue_t *q, fileRequest_t *req);
void fileUnlinkCloseWait(fileQueue_t *q, fileRequest_t *req);
fileRequest_t *pacLoadEx(const char *name, u32 tail, u32 dest, u32 callback, u32 arg);
s32 pacFileCheckEnd(const fileRequest_t *req);
void pacRead(fileRequest_t *req);
s32 pacLoadCallback(void *dev, s32 event, s32 unused, s32 len, fileRequest_t *req);
s32 fileLoadCallback(void *dev, s32 event, s32 unused, s32 len, fileRequest_t *req);
s32 fileSaveCallback(void *dev, s32 event, s32 unused, s32 len, fileRequest_t *req);
void fileRead(fileRequest_t *req);
void fileExecQueue(void);
void fileManager(void);
void fileManagerProc(void);

void dds3StartProcess(char *name, u32 prio, u32 a3, u32 a4, void (*proc)(void), u32 a6, u32 a7);
s32 sdfCreateSema(s32 init, s32 max, s32 attr);
void WaitSema(s32 sema);
void SignalSema(s32 sema);
void *sdfChipAllocZeroDirect(u32 size);
char *sdfStrdupChip(const char *str);
void *sdfMemAllocDirect(u32 size);
void *sdfMemAlloc2Direct(u32 size);
u32 sdfMemLock(void *handle);
void sdfPacInit(void *pac, void *dest);
void sdfPacSetFromTail(void *pac);
void *sdfFileCmdReadOpen(const char *name, void *callback, void *arg);
void *sdfFileCmdWriteOpen(const char *name, void *callback, void *arg, u32 mode);
void sdfFileCmdFileSize(void *dev);
void sdfFileCmdRead(void *dev, void *buf, s32 len);
void sdfFileCmdWrite(void *dev, void *buf, s32 len);
void sdfFileCmdClose(void *dev);
void sdfFileCmdFree(void *dev);
void sdfFileCmdThreadSetHiPrio(void);
void sdfPrintf(const char *fmt, ...);
void *memset(void *dst, s32 c, u32 len);

INCLUDE_ASM(const s32, "file/fileManager", pacFileHandleFree);

INCLUDE_ASM(const s32, "file/fileManager", pacFileHandleFree_);

INCLUDE_ASM(const s32, "file/fileManager", fileHandleFree);

void fileLinkQueue(fileRequest_t *req, s32 kind, const char *name, void *callback, void *arg) {
    fileQueue_t *q = &fileQueueHead;
    char *dup;
    s32 first;

    req->kind = kind;
    dup = sdfStrdupChip(name);
    req->callback = (void (*)(fileRequest_t *, u32))callback;
    req->name = dup;
    req->arg = arg;

    WaitSema(q->sema);
    if (q->tail == NULL) {
        q->head = req;
        first = 1;
    } else {
        q->tail->next = req;
        first = 0;
    }
    q->tail = req;
    SignalSema(q->sema);

    if (first != 0) {
        fileExecQueue();
    }
}

INCLUDE_ASM(const s32, "file/fileManager", fileUnlinkQueue);

void fileLinkCloseWait(fileQueue_t *q, fileRequest_t *req) {
    req->next = q->closewait;
    q->closewait = req;
}

void fileUnlinkCloseWait(fileQueue_t *q, fileRequest_t *req) {
    fileRequest_t **link = &q->closewait;
    fileRequest_t *p;

    while ((p = *link) != req) {
        link = &p->next;
    }
    *link = req->next;
}

fileRequest_t *pacLoadEx(const char *name, u32 tail, u32 dest, u32 callback, u32 arg) {
    fileRequest_t *req = sdfChipAllocZeroDirect(sizeof(fileRequest_t));

    sdfPacInit(req->pac, (void *)dest);
    if (tail != 0) {
        sdfPacSetFromTail(req->pac);
    }
    fileLinkQueue(req, 1, name, (void *)callback, (void *)arg);
    return req;
}

fileRequest_t *pacLoad(const char *name) {
    return pacLoadEx(name, 0, 0, 0, 0);
}

fileRequest_t *pacLoadUseTail(const char *name) {
    return pacLoadEx(name, 1, 0, 0, 0);
}

fileRequest_t *fileLoadEx(const char *name, u32 mode, u32 callback, u32 arg) {
    fileRequest_t *req = sdfChipAllocZeroDirect(0x30);

    req->mode = mode;
    fileLinkQueue(req, 0, name, (void *)callback, (void *)arg);
    return req;
}

fileRequest_t *fileLoad(const char *name) {
    return fileLoadEx(name, 0, 0, 0);
}

fileRequest_t *fileLoadUseTail(const char *name) {
    return fileLoadEx(name, 1, 0, 0);
}

INCLUDE_ASM(const s32, "file/fileManager", fileGetMemHandle);

INCLUDE_ASM(const s32, "file/fileManager", fileGetLoadPtr);

INCLUDE_ASM(const s32, "file/fileManager", fileGetLoadSize);

INCLUDE_ASM(const s32, "file/fileManager", fileGetLoadRemain);

s32 pacFileCheckEnd(const fileRequest_t *req) {
    s32 end;

    if (req->kind == 1) {
        end = 0;
        if (req->pacrest != 0) {
            end = req->state == 6;
        }
        return end;
    }
    return req->state == 6;
}

s32 pacCheckEnd(const fileRequest_t *req) {
    s32 end = 0;

    if (req->pacrest != 0) {
        end = req->state == 6;
    }
    return end;
}

void fileSync(fileRequest_t *req) {
    while (pacFileCheckEnd(req) == 0) {
        sdfFileCmdThreadSetHiPrio();
        fileManager();
    }
}

INCLUDE_ASM(const s32, "file/fileManager", pacSync);

INCLUDE_ASM(const s32, "file/fileManager", fileSyncAll);

fileRequest_t *fileSaveEx(const char *name, const void *src, s32 size, s32 callback, s32 arg) {
    fileRequest_t *req = sdfChipAllocZeroDirect(0x30);

    req->top = (u32)src;
    req->ptr = (u32)src;
    req->size = size;
    req->rest = size;
    fileLinkQueue(req, 2, name, (void *)callback, (void *)arg);
    return req;
}

fileRequest_t *fileSave(const char *name, const void *src, s32 size) {
    return fileSaveEx(name, src, size, 0, 0);
}

INCLUDE_ASM(const s32, "file/fileManager", pacRead);

INCLUDE_ASM(const s32, "file/fileManager", pacLoadCallback);

void fileRead(fileRequest_t *req) {
    fileQueue_t *q = &fileQueueHead;

    WaitSema(q->sema);
    if (req->state != 3) {
        SignalSema(q->sema);
        return;
    }
    req->state = 4;
    SignalSema(q->sema);
    sdfFileCmdRead(req->dev, (void *)req->ptr, req->rest <= 0x8000 ? req->rest : 0x8000);
}

INCLUDE_ASM(const s32, "file/fileManager", fileLoadCallback);

void fileWrite(fileRequest_t *req) {
    fileQueue_t *q = &fileQueueHead;

    WaitSema(q->sema);
    if (req->state != 3) {
        SignalSema(q->sema);
        return;
    }
    req->state = 4;
    SignalSema(q->sema);
    sdfFileCmdWrite(req->dev, (void *)req->ptr, req->rest <= 0x8000 ? req->rest : 0x8000);
}

INCLUDE_ASM(const s32, "file/fileManager", fileSaveCallback);

INCLUDE_ASM(const s32, "file/fileManager", fileExecQueue);

INCLUDE_ASM(const s32, "file/fileManager", fileExecCallback);

INCLUDE_ASM(const s32, "file/fileManager", fileManager);

INCLUDE_ASM(const s32, "file/fileManager", fileManagerProc);

INCLUDE_ASM(const s32, "file/fileManager", fileSysInit);

INCLUDE_SDATA(const s32, "file/fileManager", fileCallbackStatus);

INCLUDE_SDATA(const s32, "file/fileManager", D_0055164C);

INCLUDE_SDATA(const s32, "file/fileManager", D_00551650);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546900);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546918);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546930);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546948);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546960);

INCLUDE_RODATA(const s32, "file/fileManager", D_00546978);

INCLUDE_RODATA(const s32, "file/fileManager", D_00548500);

INCLUDE_RODATA(const s32, "file/fileManager", D_005485C8);

INCLUDE_RODATA(const s32, "file/fileManager", D_00548690);

INCLUDE_RODATA(const s32, "file/fileManager", D_00548758);

