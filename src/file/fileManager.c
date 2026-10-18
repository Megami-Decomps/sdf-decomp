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
void fileLinkCloseWait(fileQueue_t *q, fileRequest_t *req);
void fileUnlinkCloseWait(fileQueue_t *q, fileRequest_t *req);
fileRequest_t *pacLoadEx(const char *name, u32 tail, u32 dest, u32 callback, u32 arg);
s32 pacFileCheckEnd(const fileRequest_t *req);
void pacRead(fileRequest_t *req);
s32 pacLoadCallback(void *dev, s32 event, s32 unused, s32 len, fileTransferJob_t *req);
s32 fileLoadCallback(void *dev, s32 event, s32 unused, s32 len, fileTransferJob_t *req);
s32 fileSaveCallback(void *dev, s32 event, s32 unused, s32 len, fileTransferJob_t *req);
void fileRead(fileTransferJob_t *req);
void fileExecQueue(void);
s32 fileManager(void);
s32 fileManagerProc(dds3ProcessID_t *process);

dds3ProcessID_t *dds3StartProcess(char *name, s32 priority, s32 flag, s32 runtime,
                                s32 (*process)(dds3ProcessID_t *),
                                void (*shutdown)(dds3ProcessID_t *), void *work);
s32 sdfCreateSema(s32 init, s32 max, s32 attr);
s32 WaitSema(s32 sema);
s32 SignalSema(s32 sema);
void *sdfChipAllocZeroDirect(u32 size);
char *sdfStrdupChip(const char *str);
void *sdfMemAllocDirect(u32 size);
void *sdfMemAlloc2Direct(u32 size);
u32 sdfMemLock(void *handle);
void sdfPacInit(void *pac, void *dest);
void sdfPacSetFromTail(void *pac);
void *sdfFileCmdReadOpen(const char *name, void *callback, void *arg);
void *sdfFileCmdWriteOpen(const char *name, void *callback, void *arg, u32 mode);
s32 sdfFileCmdFileSize(void *dev);
s32 sdfFileCmdRead(void *dev, void *buf, s32 len);
s32 sdfFileCmdWrite(void *dev, void *buf, s32 len);
s32 sdfFileCmdClose(void *dev);
s32 sdfFileCmdFree(void *dev);
void sdfFileCmdThreadSetHiPrio(void);
s32 sdfPrintf(const char *fmt, ...);
void *memset(void *dst, s32 c, u32 len);
void sdfChipFreeDirect(void *ptr);
void sdfPacFree(sdfPacState_t *state);
s32 sdfPacDecode(sdfPacState_t *state, const u8 *input, s32 size);
void *sdfMemAllocFromTailDirect(u32 size);
void fileWrite(fileTransferJob_t *req);
void fileExecCallback(void);

s32 pacFileHandleFree(fileRequest_t *request)
{
    if (request->state == 6) {
        void *deviceRequest = request->dev;
        if (deviceRequest != NULL) {
            sdfFileCmdFree(deviceRequest);
        }
        sdfPacFree(&request->pac);
        sdfChipFreeDirect(request->name);
        sdfChipFreeDirect(request);
        return 0;
    }
    return 1;
}

s32 pacFileHandleFree_(void *req) {
    return pacFileHandleFree(req);
}

s32 fileHandleFree(fileRequest_t *req) {
    if (req->kind == 1) {
        return pacFileHandleFree(req);
    }
    if (req->state == 6) {
        if (req->dev != NULL) {
            sdfFileCmdFree((sdfFileCmd_t *)req->dev);
        }
        sdfChipFreeDirect(req->name);
        sdfChipFreeDirect(req);
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

void fileUnlinkQueue(fileRequest_t *node) {
    fileQueue_t *work = &fileQueueHead;
    fileRequest_t *previousNode;
    fileRequest_t *currentNode;
    s32 slotIndex;

    WaitSema(work->sema);
    for (slotIndex = 0; slotIndex != 4; slotIndex++) {
        if (work->slots[slotIndex].req == node) {
            work->slots[slotIndex].req = NULL;
        }
    }
    previousNode = NULL;
    currentNode = work->head;
    while (currentNode != NULL) {
        if (currentNode == node) {
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

    sdfPacInit(&req->pac, (void *)dest);
    if (tail != 0) {
        sdfPacSetFromTail(&req->pac);
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

sdfMemHandle_t *fileGetMemHandle(const fileRequest_t *req) {
    return req->handle;
}

u8 *fileGetLoadPtr(const fileRequest_t *req) {
    return (u8 *)req->top;
}

u32 fileGetLoadSize(const fileRequest_t *req) {
    return req->size;
}

u32 fileGetLoadRemain(fileRequest_t *req) {
    return req->rest;
}

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

void pacSync(fileRequest_t *req) {
    fileSync(req);
}

void fileSyncAll(void) {
    fileQueue_t *work = &fileQueueHead;
    while (work->head != 0 || work->closewait != 0) {
        fileManager();
    }
}

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

void pacRead(fileRequest_t *req) {
    fileQueue_t *work = &fileQueueHead;
    u8 slotIndex;
    s32 chunkBytes;

    WaitSema(work->sema);
    if (work->freecnt == 0) {
        SignalSema(work->sema);
        return;
    }
    if (req->state != 3) {
        SignalSema(work->sema);
        return;
    }
    req->state = 4;
    slotIndex = work->nextslot;
    if (slotIndex == 3) {
        work->nextslot = 0;
    } else {
        work->nextslot = slotIndex + 1;
    }
    req->slot = slotIndex;
    work->freecnt--;
    work->slots[slotIndex].req = req;
    chunkBytes = req->rest;
    if (chunkBytes > 0x10000) {
        chunkBytes = 0x10000;
    }
    SignalSema(work->sema);
    sdfFileCmdRead((sdfFileCmd_t *)req->dev,
                    work->buf + (slotIndex << 16), chunkBytes);
}

s32 pacLoadCallback(void *dev, s32 event, s32 unused, s32 byteCount, fileTransferJob_t *job) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (event) {
    case 2:
        job->dev = dev;
        job->state = 2;
        sdfFileCmdFileSize(dev);
        break;
    case 4:
        job->rest = byteCount;
        job->state = 3;
        job->size = byteCount;
        pacRead((fileRequest_t *)job);
        break;
    case 5:
        WaitSema(work->sema);
        if (job->pacrest != 0) {
            job->state = 5;
        } else {
            work->slots[job->slot].size = byteCount;
            job->rest -= byteCount;
            if (job->rest == 0) {
                job->state = 5;
                work->head = (fileRequest_t *)job->next;
                if (job->next == NULL) {
                    work->tail = NULL;
                }
            } else {
                job->state = 3;
            }
        }
        work->fullcnt++;
        if (job->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)job);
            SignalSema(work->sema);
            sdfFileCmdClose(dev);
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
        fileUnlinkCloseWait(work, (fileRequest_t *)job);
        SignalSema(work->sema);
        sdfFileCmdFree(dev);
        job->dev = NULL;
        job->state = 6;
        break;
    }
    return 0;
}

void fileRead(fileTransferJob_t *req) {
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

s32 fileLoadCallback(void *dev, s32 event, s32 unused, s32 byteCount, fileTransferJob_t *job) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (event) {
    case 2:
        job->dev = dev;
        job->state = event;
        sdfFileCmdFileSize(dev);
        break;
    case 4: {
        sdfMemHandle_t *handle;

        job->rest = byteCount;
        job->size = byteCount;
        if (job->mode == 0) {
            handle = sdfMemAlloc2Direct(byteCount);
            job->handle = handle;
            if (handle == NULL) {
                job->retry = 10;
                job->state = 7;
                break;
            }
        } else {
            handle = sdfMemAllocFromTailDirect(byteCount);
            job->handle = handle;
        }
        {
            u32 address = sdfMemLock(handle);

            job->ptr = address;
            job->top = address;
            job->state = 3;
        }
        fileRead(job);
        break;
    }
    case 5:
        WaitSema(work->sema);
        job->rest -= byteCount;
        if (job->rest == 0) {
            job->state = 5;
            if (work->head == (fileRequest_t *)job) {
                work->head = (fileRequest_t *)job->next;
                if (job->next == NULL) {
                    work->tail = NULL;
                }
            }
        } else {
            job->state = 3;
            job->ptr += byteCount;
        }
        if (job->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)job);
            SignalSema(work->sema);
            sdfFileCmdClose(dev);
            saved = fileCallbackStatus;
            fileCallbackStatus = 2;
            fileExecQueue();
            fileCallbackStatus = saved;
        } else {
            SignalSema(work->sema);
            fileRead(job);
        }
        break;
    case 7:
        sdfFileCmdFree(dev);
        job->dev = NULL;
        job->state = 6;
        WaitSema(work->sema);
        fileUnlinkCloseWait(work, (fileRequest_t *)job);
        if (job->callback != NULL) {
            fileTransferJob_t **link = &work->done;
            fileTransferJob_t *next;

            while ((next = *link) != NULL) {
                link = &next->donenext;
            }
            *link = job;
        }
        SignalSema(work->sema);
        break;
    }
    return 0;
}

void fileWrite(fileTransferJob_t *req) {
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

s32 fileSaveCallback(void *dev, s32 event, s32 unused, s32 byteCount, fileTransferJob_t *job) {
    fileQueue_t *work = &fileQueueHead;
    s32 saved;

    switch (event) {
    case 2:
        job->state = 3;
        fileWrite(job);
        break;
    case 6:
        WaitSema(work->sema);
        job->ptr += byteCount;
        job->rest -= byteCount;
        if (job->rest == 0) {
            job->state = 5;
            if (work->head == (fileRequest_t *)job) {
                work->head = (fileRequest_t *)job->next;
                if (job->next == NULL) {
                    work->tail = NULL;
                }
            }
        } else {
            job->state = 3;
        }
        if (job->state == 5) {
            fileLinkCloseWait(work, (fileRequest_t *)job);
            SignalSema(work->sema);
            sdfFileCmdClose(dev);
            saved = fileCallbackStatus;
            fileCallbackStatus = 3;
            fileExecQueue();
            fileCallbackStatus = saved;
        } else {
            SignalSema(work->sema);
            fileWrite(job);
        }
        break;
    case 7:
        sdfFileCmdFree(dev);
        job->dev = NULL;
        job->state = 6;
        WaitSema(work->sema);
        fileUnlinkCloseWait(work, (fileRequest_t *)job);
        if (job->callback != NULL) {
            fileTransferJob_t **link = &work->done;
            fileTransferJob_t *next;

            while ((next = *link) != NULL) {
                link = &next->donenext;
            }
            *link = job;
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

s32 fileManagerProc(dds3ProcessID_t *process) {
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

