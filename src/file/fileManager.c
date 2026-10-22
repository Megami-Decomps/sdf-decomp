#include "common.h"
#include "libsdf.h"
#include "kernel.h"

/* All three views share the 0x30-byte request header. Their differently
 * typed links distinguish transfer work from the queue and PAC callbacks. */
typedef void (*fileTransferCallback_t)(void *request, u32 arg);

#define FILE_REQUEST_FIELDS(Link, Callback) \
    u8 kind; \
    u8 state; \
    u8 retry; \
    u8 mode; \
    Link *next; \
    char *name; \
    void *dev; \
    s32 rest; \
    s32 size; \
    Callback callback; \
    void *arg; \
    sdfMemHandle_t *handle; \
    u32 top; \
    u32 ptr; \
    Link *donenext; \
    sdfPacState_t pac; \
    u16 pacrest; \
    u16 slot; \
    Link *cbnext;

typedef struct fileRequest {
    FILE_REQUEST_FIELDS(struct fileRequest, void *)
} fileRequest_t;

typedef struct fileTransferJob {
    FILE_REQUEST_FIELDS(struct fileTransferJob, fileTransferCallback_t)
} fileTransferJob_t;

typedef struct pacFileRequest {
    FILE_REQUEST_FIELDS(struct pacFileRequest, void *)
} pacFileRequest_t;

#undef FILE_REQUEST_FIELDS

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
    /* 0x10 */ fileTransferJob_t *done;
    /* 0x14 */ pacFileRequest_t *pacdone;
    /* 0x18 */ fileRequest_t *closewait;
    /* 0x1C */ u8 *buf;
    /* 0x20 */ fileSlot_t slots[4];
} fileQueue_t;


extern fileQueue_t fileQueueHead;
extern u32 fileCallbackStatus;
extern s32 (*p2sdSmgBuf)(void);
extern char D_00551650[];

void fileLinkQueue(fileRequest_t *req, s32 kind, const char *name, void *callback, void *arg);
void fileLinkCloseWait(fileQueue_t *cb, fileRequest_t *fileHandle);
void fileUnlinkCloseWait(fileQueue_t *cb, fileRequest_t *fileHandle);
fileRequest_t *pacLoadEx(const char *filename, u32 headOrTail, u32 callback, u32 pendCallback, u32 callbackParm);
s32 pacFileCheckEnd(const fileRequest_t *pacFileHandle);
void pacRead(fileRequest_t *pacHandle);
s32 pacLoadCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data);
s32 fileLoadCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data);
s32 fileSaveCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data);
void fileRead(fileTransferJob_t *fileHandle);
void fileExecQueue(void);
s32 fileManager(void);
s32 fileManagerProc(dds3ProcessID_t *procId);

dds3ProcessID_t *dds3StartProcess(char *v1, s32 dDS3_PRIORITY_DRAW_PB, s32 v2, s32 v3,
                                s32 (*drawExit)(dds3ProcessID_t *),
                                void (*deleteExit)(dds3ProcessID_t *), void *p);
s32 sdfCreateSema(s32 initCount, s32 maxCount, s32 option);
s32 WaitSema(s32 sema);
s32 SignalSema(s32 sema);
void *sdfChipAllocZeroDirect(u32 aSize);
char *sdfStrdupChip(const char *str);
void *sdfMemAllocDirect(u32 aSize);
void *sdfMemAlloc2Direct(u32 aSize);
u32 sdfMemLock(void *aHandle);
void sdfPacInit(void *cb, void *callback);
void sdfPacSetFromTail(void *cb);
void *sdfFileCmdReadOpen(const char *aFileName, void *aCallback, void *aUserData);
void *sdfFileCmdWriteOpen(const char *aFileName, void *aCallback, void *aUserData, u32 aMode);
s32 sdfFileCmdFileSize(void *aHandle);
s32 sdfFileCmdRead(void *aHandle, void *aBuffer, s32 aLength);
s32 sdfFileCmdWrite(void *aHandle, void *aBuffer, s32 aLength);
s32 sdfFileCmdClose(void *aHandle);
s32 sdfFileCmdFree(void *aHandle);
void sdfFileCmdThreadSetHiPrio(void);
s32 sdfPrintf(const char *str, ...);
void *memset(void *a, s32 b, u32 c);
void sdfChipFreeDirect(void *aPtr);
void sdfPacFree(sdfPacState_t *cb);
s32 sdfPacDecode(sdfPacState_t *cb, const u8 *data, s32 length);
void *sdfMemAllocFromTailDirect(u32 aSize);
void fileWrite(fileTransferJob_t *fileHandle);
void fileExecCallback(void);

s32 pacFileHandleFree(fileRequest_t *pacHandle)
{
    if (pacHandle->state == 6) {
        void *deviceRequest = pacHandle->dev;
        if (deviceRequest != NULL) {
            sdfFileCmdFree(deviceRequest);
        }
        sdfPacFree(&pacHandle->pac);
        sdfChipFreeDirect(pacHandle->name);
        sdfChipFreeDirect(pacHandle);
        return 0;
    }
    return 1;
}

s32 pacFileHandleFree_(void *pacHandle) {
    return pacFileHandleFree(pacHandle);
}

s32 fileHandleFree(fileRequest_t *fileHandle) {
    if (fileHandle->kind == 1) {
        return pacFileHandleFree(fileHandle);
    }
    if (fileHandle->state == 6) {
        if (fileHandle->dev != NULL) {
            sdfFileCmdFree((sdfFileCmd_t *)fileHandle->dev);
        }
        sdfChipFreeDirect(fileHandle->name);
        sdfChipFreeDirect(fileHandle);
        return 0;
    }
    return 1;
}

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

void fileUnlinkQueue(fileRequest_t *fileHandle) {
    fileQueue_t *work = &fileQueueHead;
    fileRequest_t *previousNode;
    fileRequest_t *currentNode;
    s32 slotIndex;

    WaitSema(work->sema);
    for (slotIndex = 0; slotIndex != 4; slotIndex++) {
        if (work->slots[slotIndex].req == fileHandle) {
            work->slots[slotIndex].req = NULL;
        }
    }
    previousNode = NULL;
    currentNode = work->head;
    while (currentNode != NULL) {
        if (currentNode == fileHandle) {
            if (currentNode->next == NULL) {
                work->tail = previousNode;
            }
            if (previousNode == NULL) {
                work->head = currentNode->next;
            } else {
                previousNode->next = currentNode->next;
            }
            break;
        }
        previousNode = currentNode;
        currentNode = currentNode->next;
    }
    SignalSema(work->sema);
}

void fileLinkCloseWait(fileQueue_t *cb, fileRequest_t *fileHandle) {
    fileHandle->next = cb->closewait;
    cb->closewait = fileHandle;
}

void fileUnlinkCloseWait(fileQueue_t *cb, fileRequest_t *fileHandle) {
    fileRequest_t **link = &cb->closewait;
    fileRequest_t *p;

    while ((p = *link) != fileHandle) {
        link = &p->next;
    }
    *link = fileHandle->next;
}

fileRequest_t *pacLoadEx(const char *filename, u32 headOrTail, u32 callback, u32 pendCallback, u32 callbackParm) {
    fileRequest_t *req = sdfChipAllocZeroDirect(sizeof(fileRequest_t));

    sdfPacInit(&req->pac, (void *)callback);
    if (headOrTail != 0) {
        sdfPacSetFromTail(&req->pac);
    }
    fileLinkQueue(req, 1, filename, (void *)pendCallback, (void *)callbackParm);
    return req;
}

fileRequest_t *pacLoad(const char *filename) {
    return pacLoadEx(filename, 0, 0, 0, 0);
}

fileRequest_t *pacLoadUseTail(const char *filename) {
    return pacLoadEx(filename, 1, 0, 0, 0);
}

fileRequest_t *fileLoadEx(const char *filename, u32 headOrTail, u32 endCallback, u32 callbackParm) {
    fileRequest_t *req = sdfChipAllocZeroDirect(0x30);

    req->mode = headOrTail;
    fileLinkQueue(req, 0, filename, (void *)endCallback, (void *)callbackParm);
    return req;
}

fileRequest_t *fileLoad(const char *filename) {
    return fileLoadEx(filename, 0, 0, 0);
}

fileRequest_t *fileLoadUseTail(const char *filename) {
    return fileLoadEx(filename, 1, 0, 0);
}

sdfMemHandle_t *fileGetMemHandle(const fileRequest_t *fileHandle) {
    return fileHandle->handle;
}

u8 *fileGetLoadPtr(const fileRequest_t *fileHandle) {
    return (u8 *)fileHandle->top;
}

u32 fileGetLoadSize(const fileRequest_t *fileHandle) {
    return fileHandle->size;
}

u32 fileGetLoadRemain(fileRequest_t *fileHandle) {
    return fileHandle->rest;
}

s32 pacFileCheckEnd(const fileRequest_t *pacFileHandle) {
    s32 end;

    if (pacFileHandle->kind == 1) {
        end = 0;
        if (pacFileHandle->pacrest != 0) {
            end = pacFileHandle->state == 6;
        }
        return end;
    }
    return pacFileHandle->state == 6;
}

s32 pacCheckEnd(const fileRequest_t *pacHandle) {
    s32 end = 0;

    if (pacHandle->pacrest != 0) {
        end = pacHandle->state == 6;
    }
    return end;
}

void fileSync(fileRequest_t *fileHandle) {
    while (pacFileCheckEnd(fileHandle) == 0) {
        sdfFileCmdThreadSetHiPrio();
        fileManager();
    }
}

void pacSync(fileRequest_t *pacFileHandle) {
    fileSync(pacFileHandle);
}

void fileSyncAll(void) {
    fileQueue_t *work = &fileQueueHead;
    while (work->head != 0 || work->closewait != 0) {
        fileManager();
    }
}

fileRequest_t *fileSaveEx(const char *filename, const void *buff, s32 size, s32 endCallback, s32 callbackParm) {
    fileRequest_t *req = sdfChipAllocZeroDirect(0x30);

    req->top = (u32)buff;
    req->ptr = (u32)buff;
    req->size = size;
    req->rest = size;
    fileLinkQueue(req, 2, filename, (void *)endCallback, (void *)callbackParm);
    return req;
}

fileRequest_t *fileSave(const char *filename, const void *buff, s32 size) {
    return fileSaveEx(filename, buff, size, 0, 0);
}

void pacRead(fileRequest_t *pacHandle) {
    fileQueue_t *work = &fileQueueHead;
    u8 slotIndex;
    s32 chunkBytes;

    WaitSema(work->sema);
    if (work->freecnt == 0) {
        SignalSema(work->sema);
        return;
    }
    if (pacHandle->state != 3) {
        SignalSema(work->sema);
        return;
    }
    pacHandle->state = 4;
    slotIndex = work->nextslot;
    if (slotIndex == 3) {
        work->nextslot = 0;
    } else {
        work->nextslot = slotIndex + 1;
    }
    pacHandle->slot = slotIndex;
    work->freecnt--;
    work->slots[slotIndex].req = pacHandle;
    chunkBytes = pacHandle->rest;
    if (chunkBytes > 0x10000) {
        chunkBytes = 0x10000;
    }
    SignalSema(work->sema);
    sdfFileCmdRead((sdfFileCmd_t *)pacHandle->dev,
                    work->buf + (slotIndex << 16), chunkBytes);
}

s32 pacLoadCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (cause) {
    case 2:
        data->dev = fileHandle;
        data->state = 2;
        sdfFileCmdFileSize(fileHandle);
        break;
    case 4:
        data->rest = length;
        data->state = 3;
        data->size = length;
        pacRead((fileRequest_t *)data);
        break;
    case 5:
        WaitSema(work->sema);
        if (data->pacrest != 0) {
            data->state = 5;
        } else {
            work->slots[data->slot].size = length;
            data->rest -= length;
            if (data->rest == 0) {
                data->state = 5;
                work->head = (fileRequest_t *)data->next;
                if (data->next == NULL) {
                    work->tail = NULL;
                }
            } else {
                data->state = 3;
            }
        }
        work->fullcnt++;
        if (data->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)data);
            SignalSema(work->sema);
            sdfFileCmdClose(fileHandle);
        } else {
            SignalSema(work->sema);
        }
        saved = fileCallbackStatus;
        fileCallbackStatus = 1;
        fileExecQueue();
        fileCallbackStatus = saved;
        break;
    case 7:
        WaitSema(work->sema);
        fileUnlinkCloseWait(work, (fileRequest_t *)data);
        SignalSema(work->sema);
        sdfFileCmdFree(fileHandle);
        data->dev = NULL;
        data->state = 6;
        break;
    }
    return 0;
}

void fileRead(fileTransferJob_t *fileHandle) {
    fileQueue_t *q = &fileQueueHead;

    WaitSema(q->sema);
    if (fileHandle->state != 3) {
        SignalSema(q->sema);
        return;
    }
    fileHandle->state = 4;
    SignalSema(q->sema);
    sdfFileCmdRead(fileHandle->dev, (void *)fileHandle->ptr, fileHandle->rest <= 0x8000 ? fileHandle->rest : 0x8000);
}

s32 fileLoadCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (cause) {
    case 2:
        data->dev = fileHandle;
        data->state = cause;
        sdfFileCmdFileSize(fileHandle);
        break;
    case 4: {
        sdfMemHandle_t *handle;

        data->rest = length;
        data->size = length;
        if (data->mode == 0) {
            handle = sdfMemAlloc2Direct(length);
            data->handle = handle;
            if (handle == NULL) {
                data->retry = 10;
                data->state = 7;
                break;
            }
        } else {
            handle = sdfMemAllocFromTailDirect(length);
            data->handle = handle;
        }
        {
            u32 address = sdfMemLock(handle);

            data->ptr = address;
            data->top = address;
            data->state = 3;
        }
        fileRead(data);
        break;
    }
    case 5:
        WaitSema(work->sema);
        data->rest -= length;
        if (data->rest == 0) {
            data->state = 5;
            if (work->head == (fileRequest_t *)data) {
                work->head = (fileRequest_t *)data->next;
                if (data->next == NULL) {
                    work->tail = NULL;
                }
            }
        } else {
            data->state = 3;
            data->ptr += length;
        }
        if (data->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)data);
            SignalSema(work->sema);
            sdfFileCmdClose(fileHandle);
            saved = fileCallbackStatus;
            fileCallbackStatus = 2;
            fileExecQueue();
            fileCallbackStatus = saved;
        } else {
            SignalSema(work->sema);
            fileRead(data);
        }
        break;
    case 7:
        sdfFileCmdFree(fileHandle);
        data->dev = NULL;
        data->state = 6;
        WaitSema(work->sema);
        fileUnlinkCloseWait(work, (fileRequest_t *)data);
        if (data->callback != NULL) {
            fileTransferJob_t **link = &work->done;
            fileTransferJob_t *next;

            while ((next = *link) != NULL) {
                link = &next->donenext;
            }
            *link = data;
        }
        SignalSema(work->sema);
        break;
    }
    return 0;
}

void fileWrite(fileTransferJob_t *fileHandle) {
    fileQueue_t *q = &fileQueueHead;

    WaitSema(q->sema);
    if (fileHandle->state != 3) {
        SignalSema(q->sema);
        return;
    }
    fileHandle->state = 4;
    SignalSema(q->sema);
    sdfFileCmdWrite(fileHandle->dev, (void *)fileHandle->ptr, fileHandle->rest <= 0x8000 ? fileHandle->rest : 0x8000);
}

s32 fileSaveCallback(void *fileHandle, s32 cause, s32 buffer, s32 length, fileTransferJob_t *data) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (cause) {
    case 2:
        data->state = 3;
        fileWrite(data);
        break;
    case 6:
        WaitSema(work->sema);
        data->ptr += length;
        data->rest -= length;
        if (data->rest == 0) {
            data->state = 5;
            if (work->head == (fileRequest_t *)data) {
                work->head = (fileRequest_t *)data->next;
                if (data->next == NULL) {
                    work->tail = NULL;
                }
            }
        } else {
            data->state = 3;
        }
        if (data->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)data);
            SignalSema(work->sema);
            sdfFileCmdClose(fileHandle);
            saved = fileCallbackStatus;
            fileCallbackStatus = 3;
            fileExecQueue();
            fileCallbackStatus = saved;
        } else {
            SignalSema(work->sema);
            fileWrite(data);
        }
        break;
    case 7:
        sdfFileCmdFree(fileHandle);
        data->dev = NULL;
        data->state = 6;
        WaitSema(work->sema);
        fileUnlinkCloseWait(work, (fileRequest_t *)data);
        if (data->callback != NULL) {
            fileTransferJob_t **link = &work->done;
            fileTransferJob_t *next;

            while ((next = *link) != NULL) {
                link = &next->donenext;
            }
            *link = data;
        }
        SignalSema(work->sema);
        break;
    }
    return 0;
}

void fileExecQueue(void) {
    fileQueue_t *work = &fileQueueHead;
    fileTransferJob_t *job;
    s32 locked;

    WaitSema(work->sema);
    job = (fileTransferJob_t *)work->head;
    if (job == NULL) {
        SignalSema(work->sema);
        return;
    }
    locked = 1;
    switch (job->kind) {
    case 0:
        switch (job->state) {
        case 0:
            job->state = 1;
            locked = 0;
            SignalSema(work->sema);
            job->dev = sdfFileCmdReadOpen(job->name, fileLoadCallback, job);
            break;
        case 7:
            job->handle = sdfMemAlloc2Direct(job->size);
            if (job->handle == NULL) {
                sdfPrintf("alloc retry for %s\n",
                                            ((sdfFileCmd_t *)job->dev)->path);
                job->retry--;
            } else {
                u32 address = sdfMemLock(job->handle);

                job->ptr = address;
                job->top = address;
                job->state = 3;
                fileRead(job);
            }
            break;
        }
        break;
    case 1:
        switch (job->state) {
        case 0:
            job->state = 1;
            locked = 0;
            SignalSema(work->sema);
            job->dev = sdfFileCmdReadOpen(job->name, pacLoadCallback, job);
            break;
        case 3:
            locked = 0;
            SignalSema(work->sema);
            pacRead((fileRequest_t *)job);
            break;
        }
        break;
    case 2:
        if (job->state == 0) {
            job->state = 1;
            locked = 0;
            SignalSema(work->sema);
            job->dev = sdfFileCmdWriteOpen(job->name, fileSaveCallback, job, 0x180);
        }
        break;
    }
    if (locked != 0) {
        SignalSema(work->sema);
    }
}

void fileExecCallback(void) {
    fileQueue_t *work = &fileQueueHead;
    fileTransferJob_t *node;

    WaitSema(work->sema);
    while ((node = work->done) != NULL) {
        work->done = node->donenext;
        SignalSema(work->sema);
        node->callback(node, (u32)node->arg);
        WaitSema(work->sema);
    }
    SignalSema(work->sema);
}

s32 fileManager(void) {
    fileQueue_t *work = &fileQueueHead;
    u32 fullcnt;
    pacFileRequest_t **link;
    pacFileRequest_t *req;

    fileExecCallback();
    fullcnt = work->fullcnt;
    while (fullcnt != 0) {
        u32 slot = work->readslot;
        const u8 *input = work->buf + (slot << 16);
        fileTransferJob_t *job = (fileTransferJob_t *)work->slots[slot].req;
        u8 kind = job->kind;

        if (kind == 1) {
            pacFileRequest_t *req = (pacFileRequest_t *)job;
            if (req != NULL &&
                sdfPacDecode(&req->pac, input, work->slots[slot].size) == -1) {
                fileUnlinkQueue((fileRequest_t *)req);
                req->pacrest = kind;
                if (req->callback != NULL) {
                    pacFileRequest_t **append = &work->pacdone;
                    pacFileRequest_t *next;

                    while ((next = *append) != NULL) {
                        append = &next->cbnext;
                    }
                    *append = req;
                }
            }
            WaitSema(work->sema);
            slot++;
            if (slot == 4) {
                slot = 0;
            }
            work->readslot = slot;
            work->freecnt++;
            work->fullcnt--;
            SignalSema(work->sema);
            fullcnt = work->fullcnt;
        }
    }
    fileExecQueue();
    link = &work->pacdone;
    while ((req = *link) != NULL) {
        if (req->state == 6) {
            *link = req->cbnext;
            ((void (*)(pacFileRequest_t *, void *))req->callback)(req, req->arg);
        } else {
            link = &req->cbnext;
        }
    }
    return work->head != NULL;
}

s32 fileManagerProc(dds3ProcessID_t *procId) {
    fileManager();
    return 0;
}

void fileSysInit(void) {
    memset(&fileQueueHead, 0, 0x40);
    fileQueueHead.freecnt = 4;
    fileQueueHead.sema = sdfCreateSema(1, 0x7F, 0);
    fileQueueHead.buf = (u8 *)sdfMemLock(sdfMemAllocDirect(0x40000));
    dds3StartProcess(D_00551650, 0x384, 1, 0, fileManagerProc, 0, 0);
    p2sdSmgBuf = fileManager;
}

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

INCLUDE_SDATA(const s32, "file/fileManager", fileCallbackStatus);

INCLUDE_SDATA(const s32, "file/fileManager", D_0055164C);

INCLUDE_SDATA(const s32, "file/fileManager", D_00551650);

