#include "common.h"
#include "ps2asm.h"

enum {
    SDF_TMX_MAGIC = 0x30584D54,
    SDF_STREAM_CHUNK_BYTES = 0x4000,
    SDF_STREAM_COMMAND_CREATE_STATE = 32,
    SDF_STREAM_COMMAND_CONTROL = 33,
    SDF_STREAM_COMMAND_READ = 34,
    SDF_STREAM_COMMAND_RELEASE_STATE = 35,
    SDF_IOP_MODULES_PER_GROUP = 2,
    SDF_KEY_TREE_PATH_LIMIT = 32,
    SDF_POOL_REBUILD_CHAIN = 0,
    SDF_POOL_PUSH_FREE = 1
};

struct SdfKeyTreeItem;

struct SdfKeyPoolNode;

struct sdfSortOtNode_tag;

struct SdfPool;

#define SDF_POOL_FREE_KIND 0xFFFF

/* Tree view of a pool node: the link word at +0 becomes the replaced-item
 * link, +0x4..+0xB stay opaque, and +0xC is compared as a float key. */
typedef struct SdfKeyTreeItem {
    struct SdfKeyTreeItem *replaced; /* 0x00 */
    u8 pad04[8];                     /* 0x04: opaque */
    f32 key;                         /* 0x0C */
} SdfKeyTreeItem;

extern void StartThread();

extern void SleepThread(void);

extern s32 sceSifLoadModule(const char *, s32, const char *);

extern char *strcpy(char *, const char *);

extern char *strcat(char *, const char *);

extern void sceMcInit(void);

extern void sdfGrJoinDrawTag(void *aFore, void *aBack);

/* Pool release logic tests the packet head address and kind at 0xC;
 * bytes at 0x8 remain opaque. The float-key tree insertion helper reads the
 * same +0xC word through SdfKeyTreeItem. DDS1/2 game/code_002EEA18/003478C0.c. */
typedef struct SdfKeyPoolNode {
    struct SdfKeyPoolNode *next; /* 0x00 */
    s32 packetHeadAddress;       /* 0x04: first DMA packet address. */
    u8 unk8[4];                  /* 0x08 */
    s32 kind;                    /* 0x0C */
} SdfKeyPoolNode;

typedef struct sdfSortOtNode_tag {
    struct sdfSortOtNode_tag *right; /* 0x00 */
    struct sdfSortOtNode_tag *left;  /* 0x04 */
    struct SdfKeyTreeItem *item;  /* 0x08 */
    s32 balance;                  /* 0x0C */
} sdfSortOtNode_tag;

/* A key-sorted draw-surface pool: an SdfPoolNode whose append and prepend
 * callbacks release a node into the pool and update its free list. */
typedef struct SdfPool {
    void *link;                                                /* 0x00: the pool entry's link */
    SdfKeyPoolNode *head;                                      /* 0x04: first node in the emitted chain */
    SdfKeyPoolNode *tail;                                      /* 0x08: last node in the emitted chain */
    SdfKeyPoolNode *freeHead;                                  /* 0x0C */
    void (*release)(struct SdfPool *, SdfKeyPoolNode *);       /* 0x10: the entry's append */
    void (*update)(struct SdfPool *, s32, SdfKeyPoolNode *);   /* 0x14: the entry's prepend */
    sdfSortOtNode_tag *keyTree[1];                                /* 0x18: first root; allocation continues */
} SdfPool;

/* Reserve size bytes from the active packet buffer, rounded up to 16 bytes.
 * Returns the prior cursor as a 32-bit address; the buffer owns this storage. */
s32 sdfGrAllocPacketBuff(s32 aSize);

extern s32 WaitSema(s32);

extern void FlushCache(s32);

extern s32 SignalSema(s32);

extern s32 sceSifSetDma(void *, s32);

extern s32 sceSifDmaStat(s32);

extern s32 GetThreadId(void);

/* One row per administrative mode. The entry receives the restore flag and
 * mode data; destroy and cleanup return scheduler results. Each row is 12 bytes
 * on the EE. */
typedef struct Dds3AdminModeDispatch {
    void (*entry)(s32 restoring, void *data);
    s32 (*destroy)(void);
    s32 (*cleanup)(void);
} Dds3AdminModeDispatch;

extern void *memcpy(void *, const void *, unsigned int);

sdfSortOtNode_tag *sdfSortOtAvlRotateLR(sdfSortOtNode_tag *node, sdfSortOtNode_tag *parent);

sdfSortOtNode_tag *sdfSortOtAvlRotateRL(sdfSortOtNode_tag *node, sdfSortOtNode_tag *parent);

void sdfSortOtAvlCheckBalance(sdfSortOtNode_tag **stack, s32 stack_depth, sdfSortOtNode_tag *node,
                   sdfSortOtNode_tag **root_p);

void sdfSortOtAvlRegist(sdfSortOtNode_tag **root_p, SdfKeyTreeItem *tag);

void sdfGrSortOtReg(SdfPool *aOt, SdfKeyPoolNode *aTag);

void sdfGrSortOtFixup(void *tag, SdfKeyPoolNode **head_p, SdfKeyPoolNode **tail_p);

void sdfGrSortOtTraverse(sdfSortOtNode_tag *node, SdfKeyPoolNode **head_p, SdfKeyPoolNode **tail_p);

void sdfGrSortOtSetup(SdfPool *aOt);

void sdfGrSortOtCallback(SdfPool *aOt, s32 aFunc, SdfKeyPoolNode *aData);

typedef struct _reent _reent;

typedef __builtin_va_list va_list;

typedef struct __sbuf {
    u8 *_base;
    s32 _size;
} __sbuf;

typedef struct __sFILE {
    u8 *_p;
    s32 _r;
    s32 _w;
    s16 _flags;
    s16 _file;
    __sbuf _bf;
    s32 _lbfsize;
    void *_cookie;
    s32 (*_read)(void *cookie, char *buf, s32 n);
    s32 (*_write)(void *cookie, const char *buf, s32 n);
    long (*_seek)(void *cookie, s32 offset, s32 whence);
    s32 (*_close)(void *cookie);
    __sbuf _ub;
    u8 *_up;
    s32 _ur;
    u8 _ubuf[3];
    u8 _nbuf[1];
    __sbuf _lb;
    s32 _blksize;
    s32 _offset;
    _reent *_data;
} FILE;

typedef struct __siov {
    void *iov_base;
    u32 iov_len;
} __siov;

typedef struct __suio {
    __siov *uio_iov;
    s32 uio_iovcnt;
    s32 uio_resid;
} __suio;

typedef struct _Bigint {
    struct _Bigint *next;
    s32 k;
    s32 maxwds;
    s32 sign;
    s32 wds;
    u32 x[1];
} _Bigint;

typedef struct _glue {
    struct _glue *next;
    s32 niobs;
    FILE *iobs;
} _glue;

typedef struct AtExitEntry {
    void *unk00;
    void *unk04;
    void *fn;
    void *unk0C;
    void *unk10;
    struct AtExitEntry *next;
} AtExitEntry;

typedef struct AtExitList {
    u32 **entries;
    s32 count;
} AtExitList;

typedef struct _atexit {
    struct _atexit *_next;
    s32 _ind;
    void (*_fns[32])(void);
} _atexit;

struct _reent {
    s32 _errno;
    FILE *_stdin;
    FILE *_stdout;
    FILE *_stderr;
    s32 _inc;
    char _emergency[25];
    s32 _current_category;
    const char *_current_locale;
    s32 __sdidinit;
    void (*__cleanup)(struct _reent *r);
    _Bigint *_result;
    s32 _result_k;
    _Bigint *_p5s;
    _Bigint **_freelist;
    u8 _unused[0xF8];
    _atexit *_atexit;
    _atexit _atexit0;
    void (**_sig_func)(s32 sig);
    struct _glue __sglue;
    FILE __sf[3];
};

extern _reent *D_0052BE28 __attribute__((section(".data")));

extern s32 _puts_r(_reent *r, const char *s);

extern s32 _raise_r(_reent *r, s32 sig);

extern void *_signal_r(_reent *r, s32 sig, void *handler);

extern s32 _setlocale_r(_reent *r, s32 category, const char *locale);

extern void *func_002DA308(_reent *r);

extern void _cleanup_r(_reent *r);

extern s32 fflush(void *fp);

extern s32 _fwalk(_reent *r, s32 (*func)(void *));

extern s32 libc_2DC670(void);

extern long _strtol_r(_reent *r, const char *s, char **end, s32 base);

extern long strtol(const char *s, char **end, s32 base);

extern s32 raise(s32 sig);

extern void func_001001C8(s32 code) __attribute__((noreturn));

extern u8 D_0054B8D8[] __attribute__((section(".data")));

extern s32 D_00552178 __attribute__((section(".data")));

extern s32 D_0052C28C __attribute__((section(".data")));

extern s32 D_0052C288 __attribute__((section(".data")));

extern s32 D_0052C378 __attribute__((section(".data")));

extern s32 _init_signal_r(_reent *r);

extern void gcc2_compiled__2(FILE *fp, s32 flags, s32 file, _reent *data);

extern _glue *__sfmoreglue(_reent *r, s32 n);

extern s32 __sigtramp_r(_reent *r, s32 sig);

extern void *libc_2D2648(void *fn);

extern void free(void *ptr);

extern void *malloc(u32 size);

extern void __malloc_lock(_reent *r);

extern void __malloc_unlock(_reent *r);

extern void *_malloc_r(_reent *r, u32 size);

extern void _free_r(_reent *r, void *ptr);

extern s32 SignalSema(s32 sema);

extern void *sbrk(s32 incr);

extern s32 close(s32 fd);

extern _Bigint *_Balloc(_reent *r, s32 k);

extern s32 registerAtExit(void *fn, void *entry);

extern s32 libc_2D25B0(void *fn, void *entry);

extern s32 write(s32 fd, const void *buf, s32 count);

extern s32 read(s32 fd, void *buf, s32 count);

extern long lseek(s32 fd, s32 offset, s32 whence);

extern s32 fstat(s32 fd, void *buf);

extern s32 kill(s32 pid, s32 sig);

extern long _read_r(_reent *r, s32 fd, char *buf, s32 n);

extern long _write_r(_reent *r, s32 fd, const char *buf, s32 n);

extern long _lseek_r(_reent *r, s32 fd, s32 offset, s32 whence);

extern s32 __sfvwrite(FILE *fp, __suio *uio);

extern s32 __sread(void *cookie, char *buf, s32 n);

extern s32 __swrite(void *cookie, const char *buf, s32 n);

extern long __sseek(void *cookie, s32 offset, s32 whence);

extern s32 __sclose(void *cookie);

extern s32 GetThreadId(void);

extern s32 WaitSema(s32 sema);

extern s32 vfprintf(FILE *fp, const char *fmt, va_list ap);

extern void *memset(void *a, s32 b, u32 c);

extern void *_calloc_r(_reent *r, u32 n, u32 size);

extern const double D_0054B930[];

extern void abort(void);

extern s32 initAtExit(void);

extern s32 atexit(void (*fn)(void));

extern void (**D_0052BB28)(void) __attribute__((section(".data")));

extern s32 D_0052BB2C __attribute__((section(".data")));

extern s32 D_0052C37C __attribute__((section(".data")));

extern u8 D_0052D940[] __attribute__((section(".data")));

extern void (*D_002F5600[])(void) __attribute__((section(".data")));

extern s32 D_0059AD58 __attribute__((section(".data")));

extern AtExitEntry *D_0059AD60 __attribute__((section(".data")));

typedef struct SdfUnpacked {
    s32 kind;
    s32 sign;
    s32 exp;
    u32 mant;
} SdfUnpacked;

extern void sdfUnpackFloat(const void *x, SdfUnpacked *u);

extern f32 sdfPackFloat(const SdfUnpacked *u);

extern long dptoul(s32 kind, s32 sign, s32 exp, long mant);

#define va_start(ap, last) __builtin_stdarg_start((ap), last)

extern s32 func_002D6678(_reent *r, FILE *fp, const char *fmt, va_list ap);

typedef union f32_bits {
    f32 f;
    u32 u;
} f32_bits;

typedef struct SdfUnpackedD {
    s32 kind;
    s32 sign;
    s32 exp;
    long mant;
} SdfUnpackedD;

extern long libc_2D34A0(const SdfUnpackedD *u);

extern void sdfUnpackDouble(const double *x, SdfUnpackedD *u);

extern SdfUnpackedD *libc_2D1590(const SdfUnpackedD *a, const SdfUnpackedD *b, SdfUnpackedD *r);

extern long libc_2D3638(const SdfUnpackedD *a, const SdfUnpackedD *b);

sdfSortOtNode_tag *sdfSortOtAvlRotateLR(sdfSortOtNode_tag *node, sdfSortOtNode_tag *parent) {
    sdfSortOtNode_tag *childRoot = node;
    sdfSortOtNode_tag *pivot;

    if (childRoot->balance < 0) {
        pivot = childRoot->left;
        parent->right = pivot->left;
        childRoot->left = pivot->right;
        pivot->left = parent;
        pivot->right = childRoot;
        if (pivot->balance > 0) {
            parent->balance = -1;
            childRoot->balance = 0;
        } else {
            parent->balance = 0;
            childRoot->balance = 1;
        }
        pivot->balance = 0;
    } else {
        parent->balance = 0;
        parent->right = childRoot->left;
        childRoot->balance = 0;
        childRoot->left = parent;
        pivot = childRoot;
    }
    return pivot;
}

sdfSortOtNode_tag *sdfSortOtAvlRotateRL(sdfSortOtNode_tag *node, sdfSortOtNode_tag *parent) {
    sdfSortOtNode_tag *childRoot = node;
    sdfSortOtNode_tag *pivot;

    if (childRoot->balance > 0) {
        pivot = childRoot->right;
        parent->left = pivot->right;
        childRoot->right = pivot->left;
        pivot->right = parent;
        pivot->left = childRoot;
        if (pivot->balance > 0) {
            parent->balance = -1;
            childRoot->balance = 0;
        } else {
            parent->balance = 0;
            childRoot->balance = 1;
        }
        pivot->balance = 0;
    } else {
        parent->balance = 0;
        parent->left = childRoot->right;
        childRoot->balance = 0;
        childRoot->right = parent;
        pivot = childRoot;
    }
    return pivot;
}

void sdfSortOtAvlCheckBalance(sdfSortOtNode_tag **stack, s32 stack_depth, sdfSortOtNode_tag *node,
                   sdfSortOtNode_tag **root_p) {
    sdfSortOtNode_tag *parent;
    sdfSortOtNode_tag *replacement = NULL;
    s32 balance;

    if (stack_depth > 0) {
        do {
            parent = stack[--stack_depth];
            if (node == parent->right) {
                balance = ++parent->balance;
            } else {
                balance = --parent->balance;
            }
            if (balance == 0) {
                return;
            }
            if (balance > 1) {
                replacement = sdfSortOtAvlRotateLR(node, parent);
                break;
            }
            if (balance < -1) {
                replacement = sdfSortOtAvlRotateRL(node, parent);
                break;
            }
            node = parent;
        } while (stack_depth > 0);
    }
    if (stack_depth > 0) {
        sdfSortOtNode_tag *ancestor = stack[--stack_depth];
        if (ancestor->right == parent) {
            ancestor->right = replacement;
        } else {
            ancestor->left = replacement;
        }
    } else if (replacement != NULL) {
        *root_p = replacement;
    }
}

void sdfSortOtAvlRegist(sdfSortOtNode_tag **root_p, SdfKeyTreeItem *tag) {
    sdfSortOtNode_tag *path[SDF_KEY_TREE_PATH_LIMIT];
    s32 depth = 0;
    sdfSortOtNode_tag **link = root_p;
    sdfSortOtNode_tag *cur = *root_p;
    sdfSortOtNode_tag *node;
    f32 key = tag->key;

    while (cur != NULL) {
        if (key == cur->item->key) {
            tag->replaced = cur->item;
            cur->item = tag;
            return;
        }
        path[depth] = cur;
        depth++;
        if (cur->item->key < key) {
            link = &cur->right;
        } else {
            link = &cur->left;
        }
        cur = *link;
    }
    tag->replaced = NULL;
    node = sdfGrAllocPacketBuff(0x10);
    node->right = NULL;
    node->left = NULL;
    node->item = tag;
    *link = node;
    node->balance = 0;
    if (depth != 0) {
        sdfSortOtAvlCheckBalance(path, depth, node, root_p);
    }
}

void sdfGrSortOtReg(SdfPool *aOt, SdfKeyPoolNode *aTag) {
    if (aTag->packetHeadAddress != 0) {
        if (aTag->kind == SDF_POOL_FREE_KIND) {
            aTag->next = aOt->freeHead;
            aOt->freeHead = aTag;
        } else {
            /* The key tree views the pool node's +0xC word as its float key. */
            sdfSortOtAvlRegist(aOt->keyTree, (SdfKeyTreeItem *)aTag);
        }
    }
}

void sdfGrSortOtFixup(void *tag, SdfKeyPoolNode **head_p, SdfKeyPoolNode **tail_p) {
    SdfKeyPoolNode *current;
    SdfKeyPoolNode *next;

    if (tag != NULL) {
        if (*tail_p == NULL) {
            *head_p = tag;
        } else {
            sdfGrJoinDrawTag(*tail_p, tag);
        }
        current = tag;
        while ((next = current->next) != NULL) {
            sdfGrJoinDrawTag(current, next);
            current = next;
        }
        *tail_p = current;
    }
}

void sdfGrSortOtTraverse(sdfSortOtNode_tag *node, SdfKeyPoolNode **head_p, SdfKeyPoolNode **tail_p) {
    sdfSortOtNode_tag *next;

    do {
        if (node->right != NULL) {
            sdfGrSortOtTraverse(node->right, head_p, tail_p);
        }
        sdfGrSortOtFixup(node->item, head_p, tail_p);
        next = node->left;
        node = next;
    } while (next != NULL);
}

void sdfGrSortOtSetup(SdfPool *aOt) {
    SdfKeyPoolNode *head = NULL;
    SdfKeyPoolNode *tail = NULL;

    sdfGrSortOtFixup(aOt->freeHead, &head, &tail);
    if (aOt->keyTree[0] != NULL) {
        sdfGrSortOtTraverse(aOt->keyTree[0], &head, &tail);
    }
    aOt->head = head;
    aOt->tail = tail;
}

void sdfGrSortOtCallback(SdfPool *aOt, s32 aFunc, SdfKeyPoolNode *aData) {
    switch (aFunc) {
    case SDF_POOL_REBUILD_CHAIN:
        sdfGrSortOtSetup(aOt);
        break;
    case SDF_POOL_PUSH_FREE:
        if (aData->packetHeadAddress != 0) {
            aData->next = aOt->freeHead;
            aOt->freeHead = aData;
        }
        break;
    }
}

void sdfModelSetOtCallbacks(SdfPool *aOt) {
    aOt->release = sdfGrSortOtReg;
    aOt->update = sdfGrSortOtCallback;
}

