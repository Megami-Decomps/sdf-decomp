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

extern void sdfGrJoinDrawTag(void *left, void *right);

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
s32 sdfGrAllocPacketBuff(s32 size);

/* One row per administrative mode. The entry receives the restore flag and
 * mode data; destroy and cleanup return scheduler results. Each row is 12 bytes
 * on the EE. */
typedef struct Dds3AdminModeDispatch {
    void (*entry)(s32 restoring, void *data);
    s32 (*destroy)(void);
    s32 (*cleanup)(void);
} Dds3AdminModeDispatch;

sdfSortOtNode_tag *sdfSortOtAvlRotateLR(sdfSortOtNode_tag *heavyChild, sdfSortOtNode_tag *parent);

sdfSortOtNode_tag *sdfSortOtAvlRotateRL(sdfSortOtNode_tag *heavyChild, sdfSortOtNode_tag *parent);

void sdfSortOtAvlCheckBalance(sdfSortOtNode_tag **path, s32 depth, sdfSortOtNode_tag *child,
                   sdfSortOtNode_tag **root);

void sdfSortOtAvlRegist(sdfSortOtNode_tag **tree, SdfKeyTreeItem *item);

void sdfGrSortOtReg(SdfPool *pool, SdfKeyPoolNode *node);

void sdfGrSortOtFixup(void *first, SdfKeyPoolNode **head, SdfKeyPoolNode **tail);

void sdfGrSortOtTraverse(sdfSortOtNode_tag *node, SdfKeyPoolNode **head, SdfKeyPoolNode **tail);

void sdfGrSortOtSetup(SdfPool *pool);

void sdfGrSortOtCallback(SdfPool *pool, s32 mode, SdfKeyPoolNode *freeNode);

extern void (**D_0052BB28)(void) __attribute__((section(".data")));

extern void (*D_002F5600[])(void) __attribute__((section(".data")));

typedef union f32_bits {
    f32 f;
    u32 u;
} f32_bits;

sdfSortOtNode_tag *sdfSortOtAvlRotateLR(sdfSortOtNode_tag *heavyChild, sdfSortOtNode_tag *parent) {
    sdfSortOtNode_tag *childRoot = heavyChild;
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

sdfSortOtNode_tag *sdfSortOtAvlRotateRL(sdfSortOtNode_tag *heavyChild, sdfSortOtNode_tag *parent) {
    sdfSortOtNode_tag *childRoot = heavyChild;
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

void sdfSortOtAvlCheckBalance(sdfSortOtNode_tag **path, s32 depth, sdfSortOtNode_tag *child,
                   sdfSortOtNode_tag **root) {
    sdfSortOtNode_tag *parent;
    sdfSortOtNode_tag *replacement = NULL;
    s32 balance;

    if (depth > 0) {
        do {
            parent = path[--depth];
            if (child == parent->right) {
                balance = ++parent->balance;
            } else {
                balance = --parent->balance;
            }
            if (balance == 0) {
                return;
            }
            if (balance > 1) {
                replacement = sdfSortOtAvlRotateLR(child, parent);
                break;
            }
            if (balance < -1) {
                replacement = sdfSortOtAvlRotateRL(child, parent);
                break;
            }
            child = parent;
        } while (depth > 0);
    }
    if (depth > 0) {
        sdfSortOtNode_tag *ancestor = path[--depth];
        if (ancestor->right == parent) {
            ancestor->right = replacement;
        } else {
            ancestor->left = replacement;
        }
    } else if (replacement != NULL) {
        *root = replacement;
    }
}

void sdfSortOtAvlRegist(sdfSortOtNode_tag **tree, SdfKeyTreeItem *item) {
    sdfSortOtNode_tag *path[SDF_KEY_TREE_PATH_LIMIT];
    s32 depth = 0;
    sdfSortOtNode_tag **link = tree;
    sdfSortOtNode_tag *cur = *tree;
    sdfSortOtNode_tag *node;
    f32 key = item->key;

    while (cur != NULL) {
        if (key == cur->item->key) {
            item->replaced = cur->item;
            cur->item = item;
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
    item->replaced = NULL;
    node = sdfGrAllocPacketBuff(0x10);
    node->right = NULL;
    node->left = NULL;
    node->item = item;
    *link = node;
    node->balance = 0;
    if (depth != 0) {
        sdfSortOtAvlCheckBalance(path, depth, node, tree);
    }
}

void sdfGrSortOtReg(SdfPool *pool, SdfKeyPoolNode *node) {
    if (node->packetHeadAddress != 0) {
        if (node->kind == SDF_POOL_FREE_KIND) {
            node->next = pool->freeHead;
            pool->freeHead = node;
        } else {
            /* The key tree views the pool node's +0xC word as its float key. */
            sdfSortOtAvlRegist(pool->keyTree, (SdfKeyTreeItem *)node);
        }
    }
}

void sdfGrSortOtFixup(void *first, SdfKeyPoolNode **head, SdfKeyPoolNode **tail) {
    SdfKeyPoolNode *current;
    SdfKeyPoolNode *next;

    if (first != NULL) {
        if (*tail == NULL) {
            *head = first;
        } else {
            sdfGrJoinDrawTag(*tail, first);
        }
        current = first;
        while ((next = current->next) != NULL) {
            sdfGrJoinDrawTag(current, next);
            current = next;
        }
        *tail = current;
    }
}

void sdfGrSortOtTraverse(sdfSortOtNode_tag *node, SdfKeyPoolNode **head, SdfKeyPoolNode **tail) {
    sdfSortOtNode_tag *next;

    do {
        if (node->right != NULL) {
            sdfGrSortOtTraverse(node->right, head, tail);
        }
        sdfGrSortOtFixup(node->item, head, tail);
        next = node->left;
        node = next;
    } while (next != NULL);
}

void sdfGrSortOtSetup(SdfPool *pool) {
    SdfKeyPoolNode *head = NULL;
    SdfKeyPoolNode *tail = NULL;

    sdfGrSortOtFixup(pool->freeHead, &head, &tail);
    if (pool->keyTree[0] != NULL) {
        sdfGrSortOtTraverse(pool->keyTree[0], &head, &tail);
    }
    pool->head = head;
    pool->tail = tail;
}

void sdfGrSortOtCallback(SdfPool *pool, s32 mode, SdfKeyPoolNode *freeNode) {
    switch (mode) {
    case SDF_POOL_REBUILD_CHAIN:
        sdfGrSortOtSetup(pool);
        break;
    case SDF_POOL_PUSH_FREE:
        if (freeNode->packetHeadAddress != 0) {
            freeNode->next = pool->freeHead;
            pool->freeHead = freeNode;
        }
        break;
    }
}

void sdfModelSetOtCallbacks(SdfPool *pool) {
    pool->release = sdfGrSortOtReg;
    pool->update = sdfGrSortOtCallback;
}
