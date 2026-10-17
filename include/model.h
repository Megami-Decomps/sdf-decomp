#ifndef MODEL_H
#define MODEL_H

/* sdfModel: the node tree a loaded model file becomes, its per-frame draw
 * packet chains, and the four ordering tables they are filed into.
 * See include/README.md for the evidence per struct. */

#include "common.h"
#include "libsdf.h"

typedef struct sdfModelRootNode_t sdfModelRootNode_t;
typedef struct sdfModelNode_t sdfModelNode_t;
typedef struct sdfModelNodeDef_t sdfModelNodeDef_t;
typedef struct sdfModelDef_t sdfModelDef_t;
typedef struct sdfModelNodeList_t sdfModelNodeList_t;
typedef struct sdfModelTexList_t sdfModelTexList_t;
typedef struct sdfMatList_t sdfMatList_t;
typedef struct sdfModelMorphList_t sdfModelMorphList_t;
typedef struct sdfModelMorph_t sdfModelMorph_t;
typedef struct sdfModelShapeList_t sdfModelShapeList_t;

/* motion.h; a root owns a chain of controllers through motion. */
struct sdfMotionCtrlr_t;

/* Declared ahead so sdfModelNodeDef_t can point at it. */
struct sdfShapeHeader_t;

/* The axis-aligned box sdfModelCheckBounds tests against the screen matrix:
 * min[3] at 0, max[3] at 12, which is how it loads them. */
typedef struct
{
    sdfVector3_t min;
    sdfVector3_t max;
} sdfModelBounds_t;

/* The four ordering tables a model's draws can reach. sdfModelLinkOt picks one
 * per packet by sdfModelPacketHeader_t.otIndex. */
typedef struct
{
    sdfGrOt_t *ot[4];
} sdfModelOt_t;

/* The five lists below are all sdfItemArray_t (libsdf.h); each repeats the
 * header so items carries its element type. elemSize is 4 for the three
 * pointer lists and 16 for the morph list. */

struct sdfModelNodeList_t
{
    sdfMemHandle_t *memHandle;
    s16 count;
    s16 capacity;
    s16 elemSize;
    s16 growStep;
    sdfModelNode_t **items;
};

struct sdfModelTexList_t
{
    sdfMemHandle_t *memHandle;
    s16 count;
    s16 capacity;
    s16 elemSize;
    s16 growStep;
    sdfTexHandle_t **items;
};

struct sdfMatList_t
{
    sdfMemHandle_t *memHandle;
    s16 count;
    s16 capacity;
    s16 elemSize;
    s16 growStep;
    sdfMat_t **items;
};

/* One blend target and how far towards it the model currently is. */
typedef struct
{
    u32 targetId;
    f32 blend;
} sdfModelMorphParm_t;

/* A morph entry mixes at most two targets; sdfModelMakeMorphList zeroes both
 * for every element and sdfModelLinkShapedDrawPacket reads both. */
struct sdfModelMorph_t
{
    sdfModelMorphParm_t target[2];
};

struct sdfModelMorphList_t
{
    sdfMemHandle_t *memHandle;
    s16 count;
    s16 capacity;
    s16 elemSize;
    s16 growStep;
    sdfModelMorph_t *items;
};

/* One loaded model. sdfModelRootNew allocates 152 bytes; the HD build's
 * modelobj, oData, effpos_type and reversePB past zBias do not exist here. */
struct sdfModelRootNode_t
{
    sdfModelNodeList_t *nodeList;
    sdfModelNode_t *headNode;
    sdfModelTexList_t *texList;
    sdfMatList_t *matList;
    sdfModelMorphList_t *morphList;
    struct sdfMotionCtrlr_t *motion;
    u8 complexNodeId; /* unconfirmed; the mode itself is flags bit 0 */
    u8 flags;
    u16 drawAttr;
    u32 color;
    sdfMatrix4_t tm;
    sdfVector4_t rotate;
    sdfVector4_t scale;
    struct sdf3DLightParallelPacket_t *light;
    struct sdf3DFogPacket_t *fog;
    f32 faceDivSoften;
    f32 faceDivBias;
    void *extraData; /* the file's material def block, per sdfModelMakeRoot */
    f32 zBias;
};

/* One joint. sdfModelNodeNew allocates 256 bytes; prev and next are the
 * circular sibling links, children points at one child. */
struct sdfModelNode_t
{
    sdfModelNode_t *prev;
    sdfModelNode_t *next;
    sdfModelNode_t *parent;
    sdfModelNode_t *children;
    sdfModelRootNode_t *root;
    u16 flags;
    u16 drawAttr;
    u32 id;
    u32 color;
    /* unconfirmed: nothing in the model code reads these two. The draw
     * packets' own sdfModelPacketHeader_t.memHandle is what gets freed. */
    sdfMemHandle_t *drawPacketMemHandle[2];
    /* One packet chain per frame buffer, indexed by sdfFrameId. */
    sdfModelPacketHeader_t *drawPacket[2];
    /* 256 bytes from sdfModelAllocTmPacket: two 128-byte VU1 transform
     * packets, one per frame buffer. */
    void *tmPacket;
    sdfModelBounds_t *bounds;
    sdfModelNodeDef_t *shape;
    u32 pad0;
    void *userData; /* unconfirmed */
    u32 pad1[3];
    sdfVector4_t rotate; /* quaternion, from the def's euler angles */
    sdfVector4_t pos;
    sdfVector4_t scale;
    sdfMatrix4_t localNoScaledTm;
    sdfMatrix4_t tm;
};

/* The file-resident node definition sdfModelNodeBaseSet copies into a node.
 * sdfModelMakeRoot strides the array by 80. */
struct sdfModelNodeDef_t
{
    u16 format;
    u16 pad0;
    u32 pad1;
    u32 id;
    s32 parentIndex; /* negative for a root-level node */
    sdfVector4_t rotate;
    sdfVector4_t pos;
    sdfVector4_t scale;
    sdfModelBounds_t *bounds;
    /* format 0: three shape headers, one per ordering table slot.
     * format 1: shape[0] is a null-terminated array of header pointers and
     * each header's own otIndex picks the table. */
    struct sdfShapeHeader_t *shape[3];
};

/* The head of a model file's data, as sdfFixupModelBinary returns it. */
struct sdfModelDef_t
{
    /* Node-def block: the node count, then the sdfModelNodeDef_t array at +16. */
    void *modelDef;
    /* Material-def block: the material count, then that many variable-length
     * material defs, which sdfMatSet walks. */
    void *matDef;
    u32 morphCount;
    void *extraData;
};

/* One blend target's VIF stream, as sdfModelLinkShapedDrawPacket REFs it. */
typedef struct
{
    void *dataPtr;
    u16 qwc;
    u16 pad;
} sdfModelShapeEntry_t;

/* The per-target streams of one shaped mesh. next and count close the chain
 * with one more REF of their own. */
struct sdfModelShapeList_t
{
    void *next;
    u16 count;
    u16 pad;
    sdfModelShapeEntry_t entry[1];
};

/* Shape headers. The engine's own name for these is not in either source, so
 * the three struct names below are this repo's; the field names follow the HD
 * metadata's sdfShapeClass, which merges the header and one packet. */

/* What sdfModelNodeDef_t.shape points at: a count, the ordering-table slot the
 * node's packets default to, and one pointer per packet. */
struct sdfShapeHeader_t
{
    u16 numOfPacket;
    u16 priority;
    void *packet[1];
};

/* Packet type 1: one material and one mesh body, both REF'd straight out. */
typedef struct
{
    u8 packetType;
    u8 mpgType;
    u16 packetPrio;
    u16 packetSize; /* qwc */
    u16 matIndex;
    void *dataBody;
} sdfShapePacket_t;

/* Packet type 2: the same header, then one bone id per skinned joint.
 * sdfModelSetOneSkinDrawPacket reads boneCount as a word and
 * sdfModelGetDrawPacketSize as a halfword, so only the low half is in use. */
typedef struct
{
    u8 packetType;
    u8 mpgType;
    u16 packetPrio;
    u16 packetSize;
    u16 matIndex;
    void *dataBody;
    u32 boneCount;
    u16 boneId[1];
} sdfShapeOneSkinPacket_t;

/* Packet type 3: a morph entry plus one shape list per blend group. */
typedef struct
{
    u8 packetType;
    u8 mpgType;
    u16 packetPrio;
    u16 morphIndex;
    u16 matIndex;
    u16 listCount;
    u16 pad;
    sdfModelShapeList_t *shapeList[1];
} sdfShapeShapedPacket_t;

/* The per-draw colour and face-division terms VU1 reads, closing a node's
 * draw. sdfModelLinkOt writes every field of it. */
typedef struct QWORD_ALIGNED
{
    sdfDmaTag_t next;
    u32 unpackColor;
    u32 color;
    u32 unpackParm;
    f32 zBias;
    f32 faceDivSoften;
    f32 faceDivBias;
    u32 itop;
    u32 mscal;
} sdfModelDrawParmPacket_t;

/* What sdfModelLinkOt allocates per node that reaches an ordering table. */
typedef struct QWORD_ALIGNED
{
    sdfDmaTag_t tmRef;
    sdfModelDrawParmPacket_t parm;
} sdfModelNodeDrawPacket_t;

/* sdfModelSetNormalDrawPacket: one material REF then one mesh body REF. */
typedef struct QWORD_ALIGNED
{
    sdfDmaTag_t matRef;
    sdfDmaTag_t dataRef;
} sdfModelNormalDrawPacket_t;

/* sdfModelRootNode_t.flags. */
enum sdf_model_root_flag
{
    SDF_MODEL_ROOT_FLAG_COMPLEXNODEID = 1,  /* sdfModelSearchNode searches by id */
    SDF_MODEL_ROOT_FLAG_OWNFACEDIV = 2,     /* use the root's own faceDiv terms */
    SDF_MODEL_ROOT_FLAG_TEMPPACKET = 4,     /* also link the temporary packets */
    SDF_MODEL_ROOT_FLAG_NOLINKEDPACKET = 8, /* suppress the type 1 and 2 paths */
    SDF_MODEL_ROOT_FLAG_DREW = 16           /* set if anything reached a table */
};

/* sdfModelNode_t.flags. Either hidden bit suppresses the node; sdfModelLinkOt
 * tests them as one 0x11 mask. */
enum sdf_model_node_flag
{
    SDF_MODEL_NODE_FLAG_HIDDEN = 1,
    SDF_MODEL_NODE_FLAG_OWNCOLOR = 2,
    SDF_MODEL_NODE_FLAG_HIDDEN2 = 16
};

/* sdfModelPacketHeader_t.type, as sdfModelLinkOt switches on it. */
enum sdf_model_packet_type
{
    SDF_MODEL_PACKET_TYPE_END = 0,
    SDF_MODEL_PACKET_TYPE_DMACALL = 1,
    SDF_MODEL_PACKET_TYPE_SHAPED = 2,
    SDF_MODEL_PACKET_TYPE_GENERATED = 3
};

/* sdfShapePacket_t.packetType, as sdfModelLinkDrawPacket dispatches on it.
 * 4 through 8 all build a generated packet; 5 and 7 first resolve the skinned
 * nodes. Only the three named below have a path of their own. */
enum sdf_shape_packet_type
{
    SDF_SHAPE_PACKET_TYPE_NORMAL = 1,
    SDF_SHAPE_PACKET_TYPE_ONESKIN = 2,
    SDF_SHAPE_PACKET_TYPE_SHAPED = 3
};

#endif /* MODEL_H */
