#include "common.h"
#include "draw3d.h"
#include "ps2asm.h"

typedef struct
{
    u128 *pVertex;
    u32 *pVC;
    s32 count;
    s32 rec;
    u32 color;
} dds3TrackPolyList_t;

typedef struct dds3TrackPoly_t
{
    u16 type;
    u16 bucket;
    s32 number;
    s32 face;
    s32 rec;
    sdfMemHandle_t *hMem;
    dds3TrackPolyList_t *list;
    u128 *pVertex;
    u32 *pVC;
    sdfMat_t *pMat;
    struct dds3TrackPoly_t *next;
    s32 unk28;
} dds3TrackPoly_t;


/* Polygon owners follow the variable-sized attribute arrays in one allocation. */
typedef struct dds3Polygon_t
{
    s16 flags;
    u8 unk02[2];
    s32 capacity;
    s32 unk08;
    s32 drawCount;
    sdfMemHandle_t *allocation;
    sdf3DPrimPolyParm_t request;
    sdfMat_t *asset;
    u128 *positions;
    u32 (*texcoords)[2];
    u128 *normals;
    u32 *colors;
    struct dds3Polygon_t *next;
    s32 d_next;
} dds3Polygon_t;

typedef struct
{
    s32 count;
    u32 color;
    u128 *positions;
    u32 *colors;
    sdfMat_t *asset;
    sdfMemHandle_t *allocation;
} dds3TrianglePoly_t;

enum trackTopology
{
    TRACK_TOPOLOGY_PAIR,
    TRACK_TOPOLOGY_TRIANGLE,
    TRACK_TOPOLOGY_SIX_VECTOR,
    TRACK_TOPOLOGY_FOUR_VECTOR,
    TRACK_TOPOLOGY_FIVE_VECTOR
};

/* Packed color words written in the vertex order used by each cell geometry. */
typedef struct trackTriangleColors_t {
    s32 edge0;  /* 0x00 */
    s32 middle; /* 0x04 */
    s32 edge1;  /* 0x08 */
} trackTriangleColors_t;

extern sdfMemHandle_t *sdfMemAllocDirect(u32 aSize);
extern s32 sdfMemLock(sdfMemHandle_t *aHandle);
extern void sdfMemFreeDirect(sdfMemHandle_t *aHandle);
extern sdfMat_t *sdfMatNew(void);
extern void sdfMatFreeDelayed(sdfMat_t *aMat);
extern void sdfMatSetConstant(sdfMat_t *aMat, f32 aRatio);
extern void sdfMatSetTex(sdfMat_t *aMat, sdfTexHandle_t *aTex);
extern s32 sdfGrAllocPacketBuff(s32 aSize);
extern void sdfDrawTagInit(sdfDrawTag_t *a);
extern void sdfDrawTagLink(sdfDrawTag_t *aTag, u32 aPacket);
extern void sdf3DPrimLinkResetTm(sdfDrawTag_t *aTag, s32 aAllocator);
extern void sdf3DPrimLinkMat(sdfDrawTag_t *aTag, sdfMat_t *aMat, s32 aAllocator);
extern void *effPkt3DPrimMakePoly(sdf3DPrimPolyParm_t *aParm);
extern f32 D_003175B0[4];
extern f32 dds3TrackPolyVCType2Param[4];
extern f32 D_003175D0[4];
extern f32 D_003175E0[4];
extern f32 dds3TrackPolyVCAcuteParam[4];
extern void (*VCSetFunc[])(void *, void *, void *);
extern u32 D_00317380[];
extern u32 D_00317400[];
extern u32 D_00317540[];
extern u32 D_00317470[];
extern u32 D_003174F0[];
extern sdfGrOt_t *D_00317640[];
extern sdfGrOt_t D_002F64B8;

void dds3TrackPolyListInit(dds3TrackPoly_t *hTrackPoly, s32 number);
extern dds3TrackPoly_t *dds3TrackPolyTag;

typedef struct dds3SpPolyHead_t {
    f32 origin[4];
    u32 entryCount; /* 0x10: number of trailing per-entry records. */
    s32 duration;
    u32 templateSize; /* Byte length copied from the source template. */
    u8 pad1C[4];
    f32 matrix[16];
    u32 color;
    u8 loop;
    u8 pad65;
    u16 drawBucket; /* 0x66: selected particle draw bucket. */
    u32 startColorRampFrames;
    u32 endColorRampFrames;
    f32 matrixCopy[16]; /* Initialized from matrix when the owner is created. */
    u16 handler;
    u16 active; /* Cleared when every entry finishes. */
    u8 padB4[0xC];
} dds3SpPolyHead_t;


typedef struct dds3SpPolyRingRecord_t {
    s32 age;
    f32 radiusStep;
} dds3SpPolyRingRecord_t;

typedef struct dds3SpPolySphereRecord_t {
    s32 age;
    u32 radius; /* Native initializer converts a float to u32. */
} dds3SpPolySphereRecord_t;

typedef struct dds3SpPolyRollRecord_t {
    s32 age;
    f32 radius;
    f32 radiusStep;
    f32 rotationXRadians;
    f32 rotationYRadians;
} dds3SpPolyRollRecord_t;


typedef struct dds3SpPolyOld_t {
    f32 origin[4];
    u16 entryCount;
    u8 pad12[2];
    s32 duration;
    s32 headerSize;
    u8 pad1C[4];
    f32 matrix[16];
    u32 color;
    u8 templateLoop;
    u8 pad65;
    u16 drawBucket;
    u32 startColorRampFrames;
    u32 endColorRampFrames;
    f32 matrixCopy[16];
    u16 handler;
    u16 active;
    u8 padB4[0xC];
    u8 loop;
    u8 padC1[3];
    s32 spawnDelayStep;
    u16 segments;
    u8 padCA[2];
    f32 radius;
    f32 pairDisplacement;
    u8 padD4[8];
    dds3TrackPoly_t *strip;
    s32 *ages;
} dds3SpPolyOld_t;


typedef struct dds3SpPolyRing_t {
    dds3SpPolyHead_t head;
    u32 spawnDelayStep;
    u32 segments;
    f32 radialWidth;
    f32 initialRadius;
    f32 targetRadius;
    f32 initialRadiusJitter;
    f32 targetRadiusJitter;
    f32 liftStep;
    u8 padE0[4];
    void *dispatchArgB;             /* +0xE4 */
    u8 padE8[4];
    void *dispatchArgA;             /* +0xEC */
    struct dds3TrackPoly_t *strip;         /* +0xF0 */
    dds3SpPolyRingRecord_t *records;         /* +0xF4 */
    struct sdfMemHandle_t *allocation;  /* +0xF8 */
    u8 padFC[4];
} dds3SpPolyRing_t;


typedef struct dds3SpPolyOldCellParam_t {
    u8 pad00[8];
    u16 verticesPerCell; /* 0x08 */
    u8 pad0A[0xA];
    void *dispatchArgA;
    void *dispatchArgB;          /* 0x14: forwarded as two opaque callback arguments */
    struct dds3TrackPoly_t *cellSystem; /* 0x1C: allocated cell system */
    u32 *cellWords;       /* 0x20: inline per-cell word array */
    u8 pad24[0xC];
} dds3SpPolyOldCellParam_t;

/* Fixed 0xF0 descriptor prefix; variable per-cell words begin at +0xF0. */
typedef struct dds3SpPolyOldDesc_t {
    u8 pad00[0x10];
    s32 count;            /* 0x10 */
    u8 pad14[4];
    s32 headerSize;       /* 0x18: serialized-header byte count */
    u8 pad1C[0xA4];
    dds3SpPolyOldCellParam_t cells; /* 0xC0 */
} dds3SpPolyOldDesc_t;


enum {
    POLY_INACTIVE_ENTRY_AGE = -0xFFFFFF,
    POLY_RESET_ENTRY_AGE = 0xFFFFFF0,
    POLY_RESET_DURATION = 0xFFFFFFF
};

typedef struct {
    dds3SpPolyHead_t head;
    u32 spawnDelayStep;
    u32 segments;
    f32 radialWidth;
    f32 radius;
    f32 radiusJitter;
    void *dispatchArgB;            /* 0xD4 */
    u8 padD8[4];
    void *dispatchArgA;            /* 0xDC */
    dds3TrackPoly_t *strip;              /* 0xE0 */
    dds3SpPolySphereRecord_t *records;        /* 0xE4 */
    struct sdfMemHandle_t *allocation; /* 0xE8 */
    u8 padEC[4];
} dds3SpPolySphereRing_t; /* 0xF0, followed by eight-byte records */

typedef struct {
    dds3SpPolyHead_t head;
    u32 spawnDelayStep;
    u32 segments;
    f32 radialWidth;
    f32 initialRadius;
    f32 targetRadius;
    f32 initialRadiusJitter;
    f32 targetRadiusJitter;
    u32 spawnDelayGroupSize;
    f32 rotationXDegrees;
    f32 rotationStepDegrees;
    void *dispatchArgB;            /* 0xE8 */
    u8 padEC[4];
    void *dispatchArgA;            /* 0xF0 */
    dds3TrackPoly_t *strip;              /* 0xF4 */
    dds3SpPolyRollRecord_t *records; /* 0xF8 */
    struct sdfMemHandle_t *allocation; /* 0xFC */
} dds3SpPolyRollRing_t; /* 0x100, followed by 20-byte records */

/* The ring kinds share the template prefix and their leading parameter words;
 * the creation paths clamp the segment count through this shared view. */
typedef union {
    dds3SpPolyRing_t band;
    dds3SpPolySphereRing_t arc;
    dds3SpPolyRollRing_t rotating;
} dds3SpPolyRingUnion_t;


typedef struct
{
    void *(*func)(void *params);
    void (*callback)(void *work);
    void (*destroy)(void *work);
} dds3SpPolyEntry_t;

typedef struct
{
    u8 unk00[0x64];
    u8 loop;
    u8 unk65[0x4B];
    u16 handler;
    u16 active;
    u8 unkB4[0xC];
    u8 oldLoop;
} dds3SpPolyLoopWork_t;

extern void *memcpy(void *a, const void *b, u32 c);
extern void *memset(void *a, s32 b, u32 c);
extern dds3SpPolyEntry_t dds3SpPolyTable[];
extern void (*D_003176A0[])(void *work);
extern void (*D_003176B0[])(f32 scale, void *work);
extern f32 D_00317650[4];
extern f32 D_00317660[4];
extern f32 dds3PolygonTriangleParam[4];
extern u8 dds3EffectBase[];
extern f32 dds3GetRandFloatA(void *name);
extern void *sdfChipAllocDirect(u32 aSize);
extern void sdfChipFreeDirect(void *aPtr);
extern f32 sdfCos(f32 aRadius);
extern f32 sdfSin(f32 aRadius);
extern void SetRotateMatrixX(f32 aAngle);
extern void SetRotateMatrixY_B(f32 aAngle);
extern void MultMatrixRevNoArg(void);
extern void MultMatrixNoArg(void);
extern void dds3SpPoly_RingBuffInit(dds3SpPolyRing_t *pRing, s32 number);

dds3SpPolyOldDesc_t *dds3SpPoly_Ring_Old_Create(dds3SpPolyOldDesc_t *src);
void dds3SpPoly_Ring_Old_Delete(dds3SpPolyOld_t *hData);
void dds3SpPoly_Ring_Old_BuffInit(dds3SpPolyOld_t *hData);
void dds3SpPoly_Ring_Old_SetScale(f32 hData, dds3SpPolyOld_t *scale);
void dds3SpPoly_Ring_Old_Calc(dds3SpPolyOld_t *pRing);
void dds3SpPoly_Ring_Old_Draw(dds3SpPolyOld_t *hData);
void dds3SpPoly_Ring_Old_New(dds3SpPolyOld_t *pRing, s32 number);
void dds3SpPoly_Ring_Old_Move(dds3SpPolyOld_t *pRing, s32 number);
u32 dds3SpPoly_Color(u32 frame, u32 frame_end, u32 abgr);
dds3SpPolyRing_t *dds3SpPoly_RingCreate(dds3SpPolyHead_t *templateHead);
void dds3SpPoly_RingDelete(dds3SpPolyRing_t *hData);
void dds3SpPoly_RingAllBuffInit(dds3SpPolyRing_t *band);
void dds3SpPoly_RingMove(dds3SpPolyRing_t *pRing, s32 number);
void dds3SpPoly_RingSetScale(f32 hData, dds3SpPolyRing_t *scale);
void dds3SpPoly_RingDraw(dds3SpPolyRing_t *hData);
dds3SpPolySphereRing_t *dds3SpPoly_SphereRingCreate(dds3SpPolySphereRing_t *source);
void dds3SpPoly_SphereRingDelete(dds3SpPolySphereRing_t *hData);
void dds3SpPoly_SphereRingAllBuffInit(dds3SpPolySphereRing_t *arc);
void dds3SpPoly_SphereRingBuffInit(dds3SpPolySphereRing_t *pSphereRing, s32 number);
void dds3SpPoly_SphereRingMove(dds3SpPolySphereRing_t *pSphereRing, s32 number);
void dds3SpPoly_SphereRingSetScale(f32 hData, dds3SpPolySphereRing_t *scale);
void dds3SpPoly_SphereRingDraw(dds3SpPolySphereRing_t *hData);
dds3SpPolyRollRing_t *dds3SpPoly_RollRingCreate(dds3SpPolyHead_t *templateHead);
void dds3SpPoly_RollRingDelete(dds3SpPolyRollRing_t *hData);
void dds3SpPoly_RollRingAllBuffInit(dds3SpPolyRollRing_t *rotatingBand);
void dds3SpPoly_RollRingBuffInit(dds3SpPolyRollRing_t *pRollRing, u32 number);
void dds3SpPoly_RollRingMove(dds3SpPolyRollRing_t *pRollRing, s32 number);
void dds3SpPoly_RollRingSetScale(f32 hData, dds3SpPolyRollRing_t *scale);
void dds3SpPoly_RollRingSetEndless(dds3SpPolyRollRing_t *pRollRing);
void *dds3SpPolyCreate(s32 index, void *arg);
void *dds3SpPolyCopy(dds3SpPolyHead_t *effect);
void dds3SpPolyDelete(dds3SpPolyHead_t *hData);
void dds3SpPolyDraw(dds3SpPolyHead_t *hData);
void dds3SpPolyReset(dds3SpPolyHead_t *hData);
void dds3SpPolySetScale(dds3SpPolyHead_t *hData, f32 scale);
u16 dds3SpPolyGetState(dds3SpPolyHead_t *hData);
void dds3SpPolySetPosition(void *hData, void *pPos);
void dds3SpPolySetRotateMatrix(dds3SpPolyHead_t *hData, void *pMatrix);
void dds3SpPolySetRGBA(dds3SpPolyHead_t *hData, u32 abgr);
void dds3SpPolySetLoop(dds3SpPolyLoopWork_t *hData, u8 loop);
u8 dds3SpPolyGetLoop(dds3SpPolyLoopWork_t *hData);
void dds3PolygonSetColor(dds3Polygon_t *hPolygon, u32 color);
void dds3PolygonTriangleSet(dds3Polygon_t *hPolygon, const f32 *center);
void dds3TrackPolyColorSet(dds3TrackPoly_t *hTrackPoly, s32 number, u32 color);
extern dds3Polygon_t *hDrawListTop;
extern u8 D_005698A0[];
extern sdf3DPrimPolyParm_t D_005698B0;

extern void sdfRandomizeByTime(void *aCB);

void dds3TrackPolyInit(void);

void dds3PolygonManagerInit(void)
{
    hDrawListTop = 0;
    dds3TrackPolyInit();
    sdfRandomizeByTime(D_005698A0);
}

void dds3PolygonManagerExit(void) {
}

dds3Polygon_t *dds3PolygonCreate(s32 face, s16 type, s16 attr) {
    s32 vertexCount;
    s32 bytes;
    sdfMemHandle_t *allocation;
    u8 *buffer;
    dds3Polygon_t *record;
    sdfMat_t *asset;

    if (type & 8) vertexCount = face + 2;
    else vertexCount = face * 3;
    bytes = vertexCount * sizeof(u128);
    if (type & 1) bytes += vertexCount * sizeof(record->texcoords[0]);
    if (type & 2) bytes += vertexCount * sizeof(u128);
    if (type & 4) bytes += vertexCount * sizeof(u32);
    allocation = sdfMemAllocDirect(bytes + sizeof(*record));
    buffer = (u8 *)sdfMemLock(allocation);
    record = (dds3Polygon_t *)(buffer + bytes);
    memset(record, 0, sizeof(*record));
    record->positions = (u128 *)buffer;
    /* Allocation and layout reuse one byte extent; DDS1 cc1probe:
     * sandbox_ClusterX/ctor_reentry_F4_probe/decisions.log:16561-16660.
     * The saved span ties allocation at 1250, below vertex count at 3500;
     * a separate span instead has priority 3846 and takes the count's home. */
    bytes = vertexCount * sizeof(u128);
    buffer += bytes;
    if (type & 1) {
        record->texcoords = (u32 (*)[2])buffer;
        buffer += vertexCount * sizeof(record->texcoords[0]);
    }
    if (type & 2) record->normals = (u128 *)buffer;
    if (type & 4) record->colors = (u32 *)buffer;
    record->flags = type;
    record->capacity = vertexCount;
    record->allocation = allocation;
    record->drawCount = 0;
    memset(&record->request, 0, sizeof(record->request));
    record->request.color = 0x80808080;
    if (!(attr & 1)) record->request.attr = 0x4000;
    asset = sdfMatNew();
    record->asset = asset;
    if (!(attr & 2)) sdfMatSetConstant(asset, 1.0f);
    return record;
}

void dds3PolygonDelete(dds3Polygon_t *hPolygon) {
    hPolygon->flags = 1;
    sdfMatFreeDelayed(hPolygon->asset);
    sdfMemFreeDirect(hPolygon->allocation);
}

void dds3PolygonDraw(dds3Polygon_t *hPolygon) {
    hPolygon->next = hDrawListTop;
    hDrawListTop = hPolygon;
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonLinkOt);

void dds3PolygonSetTex(dds3Polygon_t *hPolygon, sdfTexHandle_t *hTex) {
    sdfMatSetTex(hPolygon->asset, hTex);
}

void dds3TrackPolyInit(void)
{
    memset(&D_005698B0, 0, sizeof(D_005698B0));
    D_005698B0.attr = 0x4000;
}

dds3TrackPoly_t *dds3TrackPolyCreate(s32 number, s32 face, s32 rec, u32 type) {
    s32 total;
    sdfMemHandle_t *allocation;
    s32 base;
    s32 cellsSize;
    dds3TrackPoly_t *system;
    s32 i;

    if (type == TRACK_TOPOLOGY_FIVE_VECTOR) {
        face = face * 5 + 5;
    } else if (type == TRACK_TOPOLOGY_FOUR_VECTOR) {
        face = face * 4 + 4;
    } else if (type == TRACK_TOPOLOGY_SIX_VECTOR) {
        face = face * 6 + 6;
    } else {
        face = face * 2;
        if (rec != 0) {
            if (face % rec != 0) {
                face = face / rec + 2;
            } else {
                face = face / rec;
            }
        }
        face &= ~1;
        face += 2;
        if (type == TRACK_TOPOLOGY_TRIANGLE) {
            face += face >> 1;
        }
    }
    total = number * face;
    cellsSize = (total + number) * 0x14;
    allocation = sdfMemAllocDirect(cellsSize + 0x2C);
    base = sdfMemLock(allocation);
    system = (dds3TrackPoly_t *)(base + cellsSize);
    memset(system, 0, 0x2C);
    system->pVertex = (u128 *)base;
    base += total * 0x10;
    system->pVC = (u32 *)base;
    base += total * 4;
    system->list = (dds3TrackPolyList_t *)base;
    for (i = 0; i < number; i++) {
        dds3TrackPolyList_t *cell = (dds3TrackPolyList_t *)(i * sizeof(dds3TrackPolyList_t) + (s32)system->list);
        cell->pVertex = (u128 *)((u8 *)system->pVertex + i * face * 0x10);
        cell->pVC = system->pVC + i * face;
        dds3TrackPolyListInit(system, i);
    }
    system->pMat = sdfMatNew();
    sdfMatSetConstant(system->pMat, 1.0f);
    system->type = type;
    system->number = number;
    system->bucket = 2;
    system->face = face;
    system->rec = rec;
    system->hMem = allocation;
    system->next = 0;
    system->unk28 = 0;
    return system;
}

void dds3TrackPolyDelete(dds3TrackPoly_t *hTrackPoly) {
    sdfMatFreeDelayed(hTrackPoly->pMat);
    sdfMemFreeDirect(hTrackPoly->hMem);
}

void dds3TrackPolyListInit(dds3TrackPoly_t *hTrackPoly, s32 number)
{
    dds3TrackPolyList_t *list = (dds3TrackPolyList_t *)(number * 20 + (u32)hTrackPoly->list);

    list->color = 0x80808080;
    list->rec = 0;
    list->count = 0;
}

void dds3TrackPolyDraw(dds3TrackPoly_t *node) {
    node->next = dds3TrackPolyTag;
    dds3TrackPolyTag = node;
}

void dds3TrackPolySet(dds3TrackPoly_t *hTrackPoly, s32 number, const u128 *pV) {
    dds3TrackPolyList_t *cell = &hTrackPoly->list[number];
    u128 *vertex;
    s32 shiftCount;
    s32 i;

    if (cell->rec == 0) {
        shiftCount = hTrackPoly->face - 2;
        vertex = cell->pVertex + shiftCount;
        for (i = 0; i < shiftCount; i++) {
            vertex--;
            SDF_VU0_COPYVECTOR(vertex + 2, vertex);
        }
        cell->rec = hTrackPoly->rec;
        if (cell->count < shiftCount + 2) {
            cell->count += 2;
        }
    } else {
        cell->rec--;
        vertex = cell->pVertex;
    }
    SDF_VU0_COPYVECTOR(vertex, pV);
    SDF_VU0_COPYVECTOR(vertex + 1, pV + 1);
}

void dds3TrackPolyMove(dds3TrackPoly_t *hTrackPoly, s32 number, const u128 *pMove) {
    dds3TrackPolyList_t *cell = hTrackPoly->list + number;
    s32 count = hTrackPoly->face;
    u128 *vertex = cell->pVertex;
    s32 i;
    SDF_VU0_LOADVECTOR_B_BARRIER(pMove);
    if (count > 0) {
        i = count;
        do {
            SDF_VU0_LOADVECTOR(vertex);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR(vertex);
            i--;
            vertex++;
        } while (i != 0);
    }
}

void dds3TrackPolySet2(dds3TrackPoly_t *hTrackPoly, s32 number, const u128 *pV) {
    dds3TrackPolyList_t *cell = &hTrackPoly->list[number];
    u128 *vertex;
    s32 shiftCount;
    s32 i;

    if (cell->rec == 0) {
        shiftCount = hTrackPoly->face - 3;
        vertex = cell->pVertex + shiftCount;
        for (i = 0; i < shiftCount; i++) {
            vertex--;
            SDF_VU0_COPYVECTOR(vertex + 3, vertex);
        }
        cell->rec = hTrackPoly->rec;
        if (cell->count < shiftCount + 3) {
            cell->count += 3;
        }
    } else {
        cell->rec--;
        vertex = cell->pVertex;
    }
    SDF_VU0_COPYVECTOR(vertex, pV);
    SDF_VU0_COPYVECTOR(vertex + 1, pV + 1);
    SDF_VU0_COPYVECTOR(vertex + 2, pV + 2);
}

void dds3TrackPolyMove2(dds3TrackPoly_t *hTrackPoly, s32 number, const u128 *pMove) {
    dds3TrackPolyList_t *cell = hTrackPoly->list + number;
    s32 count = cell->count / 3;
    u128 *vertex = cell->pVertex;
    s32 i;
    SDF_VU0_LOADVECTOR_B(pMove);
    if (count > 0) {
        i = count;
        do {
            SDF_VU0_LOADVECTOR(vertex);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR(vertex);
            SDF_VU0_LOADVECTOR(vertex + 1);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR(vertex + 1);
            SDF_VU0_LOADVECTOR(vertex + 2);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR(vertex + 2);
            i--;
            vertex += 3;
        } while (i != 0);
    }
}

void dds3TrackPolyVCRefresh_ParType1(dds3TrackPoly_t *hTrackPoly, s32 index);

void dds3TrackPolyVCRefresh_ParType1(dds3TrackPoly_t *hTrackPoly, s32 index) {
    u32 startCarrier[4];
    u32 cellCarrier[4];
    u32 packed[4];
    dds3TrackPolyList_t *cell = (dds3TrackPolyList_t *)((u8 *)(index * sizeof(dds3TrackPolyList_t)) + (u32)hTrackPoly->list);
    u32 *vc = cell->pVC;
    u32 perCell = cell->count >> 1;
    u32 color = vc[0];
    u32 j;

    if (perCell < 2) {
        return;
    }
    startCarrier[0] = color & 0xFF000000;
    MMI_RGBA_UNPACK(startCarrier, 1.0f / 128.0f);
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(D_003175B0);
    VU0_LERP_VF10_VF11(1.0f / (f32)perCell);
    SDF_VU0_COPYVECTOR_A_TO_C();
    cellCarrier[0] = color;
    MMI_RGBA_UNPACK(cellCarrier, 1.0f / 128.0f);
    for (j = 0; j < perCell; j++) {
        u32 packedColor;

        SDF_VU0_COPYVECTOR_A_TO_B();
        MMI_RGBA_PACK_UNIT(packedColor, 0x43000000);
        packed[0] = packedColor;
        vc[0] = packedColor;
        vc[1] = packedColor;
        SDF_VU0_SUBVECTOR_B_C();
        SDF_VU0_COPYVECTOR_B_TO_A();
        vc += 2;
    }
}


void dds3TrackPolyVCRefresh_ParType2(dds3TrackPoly_t *hTrackPoly, s32 index);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCRefresh_ParType2);

void dds3TrackPolyVCSet_Normal(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out)
{
    s32 perCell = hTrackPoly->face >> 1;
    s32 count = hTrackPoly->number;
    s32 i;

    for (i = 0; i < count; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = abgr_in;
            vc[1] = abgr_out;
            vc += 2;
        }
    }
}

void dds3TrackPolyVCSet_Acute(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    u32 startCarrier[4];
    u32 cellCarrier[4];
    u32 packed[4];
    u32 perCell = hTrackPoly->face >> 1;
    u32 cellCount = hTrackPoly->number;
    s32 i;

    startCarrier[0] = abgr_in & 0xFF000000;
    MMI_RGBA_UNPACK(startCarrier, 1.0f / 128.0f);
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(D_003175D0);
    VU0_LERP_VF10_VF11(1.0f / (f32)perCell);
    SDF_VU0_COPYVECTOR_A_TO_C();
    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        cellCarrier[0] = abgr_in;
        MMI_RGBA_UNPACK_NOMEM(cellCarrier, 0x3C000000);
        for (j = 0; j < perCell; j++) {
            u32 color;

            SDF_VU0_COPYVECTOR_A_TO_B();
            MMI_RGBA_PACK_UNIT(color, 0x43000000);
            packed[0] = color;
            vc[0] = color;
            vc[1] = color;
            SDF_VU0_SUBVECTOR_B_C();
            SDF_VU0_COPYVECTOR_B_TO_A();
            vc += 2;
        }
    }
}


INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet_AcuteW);

void dds3TrackPolyVCSet2_Normal(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 3;
    s32 i;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[1] = abgr_in;
            vc[2] = abgr_out;
            vc[0] = abgr_out;
            vc += 3;
        }
    }
}


INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet2_Acute);

void dds3TrackPolyVCSet2_AcuteW(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = (hTrackPoly->face / 3) >> 1;
    u32 firstAlpha = abgr_in & 0xFF000000;
    u32 secondAlpha = abgr_out & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    s32 i;

    abgr_in &= 0x00FFFFFF;
    abgr_out &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = 0;
        u32 rampSecond = 0;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[1] = abgr_in | (rampFirst & 0xFF000000);
            vc[2] = abgr_out | (rampSecond & 0xFF000000);
            vc[0] = abgr_out | (rampSecond & 0xFF000000);
            rampFirst += firstStep;
            rampSecond += secondStep;
            vc += 3;
        }
        rampFirst = firstAlpha;
        rampSecond = secondAlpha;
        for (j = 0; j < perCell; j++) {
            rampFirst -= firstStep;
            rampSecond -= secondStep;
            vc[1] = abgr_in | (rampFirst & 0xFF000000);
            vc[2] = abgr_out | (rampSecond & 0xFF000000);
            vc[0] = abgr_out | (rampSecond & 0xFF000000);
            vc += 3;
        }
    }
}


void dds3TrackPolyVC3Set_Normal(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 6;
    s32 i;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = abgr_2;
            vc[1] = abgr_1;
            vc[2] = abgr_0;
            vc[3] = abgr_0;
            vc[4] = abgr_1;
            vc[5] = abgr_2;
            vc += 6;
        }
    }
}


void dds3TrackPolyVC3Set_Acute(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 6;
    u32 firstAlpha = abgr_0 & 0xFF000000;
    u32 secondAlpha = abgr_1 & 0xFF000000;
    u32 thirdAlpha = abgr_2 & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 thirdStep = thirdAlpha / perCell;
    s32 i;

    abgr_0 &= 0x00FFFFFF;
    abgr_1 &= 0x00FFFFFF;
    abgr_2 &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = firstAlpha;
        u32 rampSecond = secondAlpha;
        u32 rampThird = thirdAlpha;
        s32 j;

        for (j = 0; j < perCell; j++) {
            rampFirst -= firstStep;
            rampSecond -= secondStep;
            rampThird -= thirdStep;
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_0 | (rampFirst & 0xFF000000);
            vc[4] = abgr_1 | (rampSecond & 0xFF000000);
            vc[5] = abgr_2 | (rampThird & 0xFF000000);
            vc += 6;
        }
    }
}


void dds3TrackPolyVC3Set_AcuteW(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = (hTrackPoly->face / 6) >> 1;
    u32 firstAlpha = abgr_0 & 0xFF000000;
    u32 secondAlpha = abgr_1 & 0xFF000000;
    u32 thirdAlpha = abgr_2 & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 thirdStep = thirdAlpha / perCell;
    s32 i;

    abgr_0 &= 0x00FFFFFF;
    abgr_1 &= 0x00FFFFFF;
    abgr_2 &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = 0;
        u32 rampSecond = 0;
        u32 rampThird = 0;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_0 | (rampFirst & 0xFF000000);
            vc[4] = abgr_1 | (rampSecond & 0xFF000000);
            vc[5] = abgr_2 | (rampThird & 0xFF000000);
            rampFirst += firstStep;
            rampSecond += secondStep;
            rampThird += thirdStep;
            vc += 6;
        }
        rampFirst = firstAlpha;
        rampSecond = secondAlpha;
        rampThird = thirdAlpha;
        for (j = 0; j < perCell; j++) {
            rampFirst -= firstStep;
            rampSecond -= secondStep;
            rampThird -= thirdStep;
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_0 | (rampFirst & 0xFF000000);
            vc[4] = abgr_1 | (rampSecond & 0xFF000000);
            vc[5] = abgr_2 | (rampThird & 0xFF000000);
            vc += 6;
        }
    }
}


void dds3TrackPolyVCSet4_Normal(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    s32 perCell = hTrackPoly->face >> 2;
    s32 cellCount = hTrackPoly->number;

    s32 cellIndex;

    for (cellIndex = 0; cellIndex < cellCount; cellIndex++) {
        u32 *vertices = hTrackPoly->list[cellIndex].pVC;
        s32 i;

        for (i = 0; i < perCell; i++) {
            vertices[2] = abgr_in;
            vertices[1] = abgr_in;
            vertices[3] = abgr_out;
            vertices[0] = abgr_out;
            vertices += 4;
        }
    }
}

void dds3TrackPolyVCSet4_Acute(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    s32 perCell = hTrackPoly->face >> 2;
    u32 firstAlpha = abgr_in & 0xFF000000;
    u32 secondAlpha = abgr_out & 0xFF000000;
    s32 cellCount = hTrackPoly->number;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 firstRGB = abgr_in & 0x00FFFFFF;
    u32 secondRGB = abgr_out & 0x00FFFFFF;
    s32 i;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 alphaA = firstAlpha;
        u32 alphaB = secondAlpha;
        s32 j;

        for (j = 0; j < perCell; j++) {
            alphaA -= firstStep;
            alphaB -= secondStep;
            vc[2] = firstRGB | (alphaA & 0xFF000000);
            vc[1] = firstRGB | (alphaA & 0xFF000000);
            vc[3] = secondRGB | (alphaB & 0xFF000000);
            vc[0] = secondRGB | (alphaB & 0xFF000000);
            vc += 4;
        }
    }
}


void dds3TrackPolyVCSet4_AcuteW(dds3TrackPoly_t *hTrackPoly, u32 abgr_in, u32 abgr_out) {
    s32 perCell = hTrackPoly->face >> 3;
    u32 firstAlpha = abgr_in & 0xFF000000;
    u32 secondAlpha = abgr_out & 0xFF000000;
    s32 cellCount = hTrackPoly->number;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 firstRGB = abgr_in & 0x00FFFFFF;
    u32 secondRGB = abgr_out & 0x00FFFFFF;
    s32 i;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 alphaA = 0;
        u32 alphaB = 0;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[2] = firstRGB | (alphaA & 0xFF000000);
            vc[1] = firstRGB | (alphaA & 0xFF000000);
            vc[3] = secondRGB | (alphaB & 0xFF000000);
            vc[0] = secondRGB | (alphaB & 0xFF000000);
            alphaA += firstStep;
            alphaB += secondStep;
            vc += 4;
        }
        alphaA = firstAlpha;
        alphaB = secondAlpha;
        for (j = 0; j < perCell; j++) {
            alphaA -= firstStep;
            alphaB -= secondStep;
            vc[2] = firstRGB | (alphaA & 0xFF000000);
            vc[1] = firstRGB | (alphaA & 0xFF000000);
            vc[3] = secondRGB | (alphaB & 0xFF000000);
            vc[0] = secondRGB | (alphaB & 0xFF000000);
            vc += 4;
        }
    }
}


void dds3TrackPolyVC5Set_Normal(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 5;
    s32 i;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = abgr_2;
            vc[1] = abgr_1;
            vc[2] = abgr_0;
            vc[3] = abgr_1;
            vc[4] = abgr_2;
            vc += 5;
        }
    }
}


void dds3TrackPolyVC5Set_Acute(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 5;
    u32 firstAlpha = abgr_0 & 0xFF000000;
    u32 secondAlpha = abgr_1 & 0xFF000000;
    u32 thirdAlpha = abgr_2 & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 thirdStep = thirdAlpha / perCell;
    s32 i;

    abgr_0 &= 0x00FFFFFF;
    abgr_1 &= 0x00FFFFFF;
    abgr_2 &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = firstAlpha;
        u32 rampSecond = secondAlpha;
        u32 rampThird = thirdAlpha;
        s32 j;

        for (j = 0; j < perCell; j++) {
            rampFirst -= firstStep;
            rampSecond -= secondStep;
            rampThird -= thirdStep;
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_1 | (rampSecond & 0xFF000000);
            vc[4] = abgr_2 | (rampThird & 0xFF000000);
            vc += 5;
        }
    }
}


void dds3TrackPolyVC5Set_AcuteR(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = hTrackPoly->face / 5;
    u32 firstAlpha = abgr_0 & 0xFF000000;
    u32 secondAlpha = abgr_1 & 0xFF000000;
    u32 thirdAlpha = abgr_2 & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 thirdStep = thirdAlpha / perCell;
    s32 i;

    abgr_0 &= 0x00FFFFFF;
    abgr_1 &= 0x00FFFFFF;
    abgr_2 &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = 0;
        u32 rampSecond = 0;
        u32 rampThird = 0;
        s32 j;

        for (j = 0; j < perCell; j++) {
            rampFirst += firstStep;
            rampSecond += secondStep;
            rampThird += thirdStep;
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_1 | (rampSecond & 0xFF000000);
            vc[4] = abgr_2 | (rampThird & 0xFF000000);
            vc += 5;
        }
    }
}


void dds3TrackPolyVC5Set_AcuteW(dds3TrackPoly_t *hTrackPoly, u32 abgr_0, u32 abgr_1, u32 abgr_2) {
    s32 cellCount = hTrackPoly->number;
    s32 perCell = (hTrackPoly->face / 5) >> 1;
    u32 firstAlpha = abgr_0 & 0xFF000000;
    u32 secondAlpha = abgr_1 & 0xFF000000;
    u32 thirdAlpha = abgr_2 & 0xFF000000;
    u32 firstStep = firstAlpha / perCell;
    u32 secondStep = secondAlpha / perCell;
    u32 thirdStep = thirdAlpha / perCell;
    s32 i;

    abgr_0 &= 0x00FFFFFF;
    abgr_1 &= 0x00FFFFFF;
    abgr_2 &= 0x00FFFFFF;

    for (i = 0; i < cellCount; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        u32 rampFirst = 0;
        u32 rampSecond = 0;
        u32 rampThird = 0;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_1 | (rampSecond & 0xFF000000);
            vc[4] = abgr_2 | (rampThird & 0xFF000000);
            rampFirst += firstStep;
            rampSecond += secondStep;
            rampThird += thirdStep;
            vc += 5;
        }
        rampFirst = firstAlpha;
        rampSecond = secondAlpha;
        rampThird = thirdAlpha;
        for (j = 0; j < perCell; j++) {
            rampFirst -= firstStep;
            rampSecond -= secondStep;
            rampThird -= thirdStep;
            vc[0] = abgr_2 | (rampThird & 0xFF000000);
            vc[1] = abgr_1 | (rampSecond & 0xFF000000);
            vc[2] = abgr_0 | (rampFirst & 0xFF000000);
            vc[3] = abgr_1 | (rampSecond & 0xFF000000);
            vc[4] = abgr_2 | (rampThird & 0xFF000000);
            vc += 5;
        }
    }
}


void dds3TrackPolySetMode(dds3TrackPoly_t *hTrackPoly, u16 mode) {
    hTrackPoly->bucket = mode;
}

void dds3TrackPolyVCSet(dds3TrackPoly_t *hTrackPoly, s32 type, void *abgr_in, void *abgr_out) {
    u16 id = hTrackPoly->type;

    VCSetFunc[id * 3 + type](hTrackPoly, abgr_in, abgr_out);
}

void dds3TrackPolyLinkOt(void) {
    sdfDrawTag_t *lists[5];
    sdfDrawTag_t **slot;
    sdfDrawTag_t *list;
    sdfDrawTag_t *specialList;
    s32 count;
    s32 i;
    s32 remaining;
    dds3TrackPoly_t *system;
    dds3TrackPolyList_t *cell;
    u64 *packet;

    if (dds3TrackPolyTag == NULL) {
        return;
    }
    memset(lists, 0, sizeof(lists));
    for (system = dds3TrackPolyTag; system != NULL; system = system->next) {
        slot = &lists[system->bucket];
        list = *slot;
        if (list == 0) {
            *slot = (sdfDrawTag_t *)sdfGrAllocPacketBuff(0x20);
            sdfDrawTagInit(*slot);
            sdf3DPrimLinkResetTm(*slot, 0);
            list = *slot;
        }
        sdf3DPrimLinkMat(list, system->pMat, 0);
        count = system->number;
        if (system->type == TRACK_TOPOLOGY_PAIR) {
            D_005698B0.faceIndex = D_00317380;
            for (i = 0; i < count; i++) {
                cell = &system->list[i];
                D_005698B0.faceCount = 16;
                D_005698B0.pointCount = 18;
                remaining = cell->count;
                D_005698B0.vertex = (sdfVector4_t *)cell->pVertex;
                D_005698B0.vertexColor = cell->pVC;
                D_005698B0.color = cell->color;
                while (remaining >= 18) {
                    remaining -= 16;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                    D_005698B0.vertex += 16;
                    D_005698B0.vertexColor += 16;
                }
                if (remaining >= 4) {
                    D_005698B0.faceCount = remaining - 2;
                    D_005698B0.pointCount = remaining;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                }
            }
        } else if (system->type == TRACK_TOPOLOGY_TRIANGLE) {
            D_005698B0.faceIndex = D_00317400;
            for (i = 0; i < count; i++) {
                cell = &system->list[i];
                D_005698B0.faceCount = 16;
                D_005698B0.pointCount = 15;
                remaining = cell->count;
                D_005698B0.vertex = (sdfVector4_t *)cell->pVertex;
                D_005698B0.vertexColor = cell->pVC;
                D_005698B0.color = cell->color;
                while (remaining >= 15) {
                    remaining -= 12;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                    D_005698B0.vertex += 12;
                    D_005698B0.vertexColor += 12;
                }
                if (remaining >= 6) {
                    D_005698B0.faceCount = (remaining / 3) * 4 - 4;
                    D_005698B0.pointCount = remaining;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                }
            }
        } else if (system->type == TRACK_TOPOLOGY_FIVE_VECTOR) {
            D_005698B0.faceIndex = D_00317540;
            for (i = 0; i < count; i++) {
                cell = &system->list[i];
                D_005698B0.faceCount = 8;
                D_005698B0.pointCount = 10;
                remaining = cell->count;
                D_005698B0.vertex = (sdfVector4_t *)cell->pVertex;
                D_005698B0.vertexColor = cell->pVC;
                D_005698B0.color = cell->color;
                while (remaining >= 10) {
                    remaining -= 5;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                    D_005698B0.vertex += 5;
                    D_005698B0.vertexColor += 5;
                }
            }
        } else if (system->type == TRACK_TOPOLOGY_SIX_VECTOR) {
            D_005698B0.faceIndex = D_00317470;
            for (i = 0; i < count; i++) {
                cell = &system->list[i];
                D_005698B0.faceCount = 10;
                D_005698B0.pointCount = 12;
                remaining = cell->count;
                D_005698B0.vertex = (sdfVector4_t *)cell->pVertex;
                D_005698B0.vertexColor = cell->pVC;
                D_005698B0.color = cell->color;
                while (remaining >= 12) {
                    remaining -= 6;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                    D_005698B0.vertex += 6;
                    D_005698B0.vertexColor += 6;
                }
            }
        } else {
            D_005698B0.faceIndex = D_003174F0;
            for (i = 0; i < count; i++) {
                cell = &system->list[i];
                D_005698B0.faceCount = 12;
                D_005698B0.pointCount = 16;
                remaining = cell->count;
                D_005698B0.vertex = (sdfVector4_t *)cell->pVertex;
                D_005698B0.vertexColor = cell->pVC;
                D_005698B0.color = cell->color;
                while (remaining >= 16) {
                    remaining -= 12;
                    sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&D_005698B0));
                    D_005698B0.vertex += 12;
                    D_005698B0.vertexColor += 12;
                }
            }
        }
    }
    for (i = 0; i < 4; i++) {
        if (lists[i] != 0) {
            ((void (*)(sdfGrOt_t *, sdfDrawTag_t *))D_00317640[i]->regFunc)(D_00317640[i], lists[i]);
        }
    }
    if (lists[4] != 0) {
        specialList = (sdfDrawTag_t *)sdfGrAllocPacketBuff(0x20);
        sdfDrawTagInit(specialList);
        packet = (u64 *)sdfGrAllocPacketBuff(0x30);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x8000000026ULL;
        packet[5] = 0x42;
        sdfDrawTagLink(specialList, (u32)packet);
        D_002F64B8.regFunc(&D_002F64B8, specialList);
        packet = (u64 *)sdfGrAllocPacketBuff(0x30);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x42;
        packet[5] = 0x42;
        sdfDrawTagLink(lists[4], (u32)packet);
        D_002F64B8.regFunc(&D_002F64B8, lists[4]);
    }
    dds3TrackPolyTag = NULL;
}

dds3TrianglePoly_t *dds3TrianglePolyCreate(s32 number) {
    s32 points = number * 3;
    s32 colorBytes = points * 4;
    sdfMemHandle_t *allocation = sdfMemAllocDirect((colorBytes + points) * 4 + 0x18);
    s32 base = sdfMemLock(allocation);
    u8 *vertices = (u8 *)base + points * 16;
    dds3TrianglePoly_t *block = (dds3TrianglePoly_t *)(vertices + colorBytes);
    block->color = 0x80808080;
    block->count = number;
    block->colors = (u32 *)vertices;
    block->allocation = allocation;
    block->positions = (u128 *)base;
    block->asset = sdfMatNew();
    sdfMatSetConstant(block->asset, 1.0f);
    return block;
}

void dds3TrianglePolyDelete(dds3TrianglePoly_t *hTriPoly) {
    sdfMatFreeDelayed(hTriPoly->asset);
    sdfMemFreeDirect(hTriPoly->allocation);
}

void dds3TrianglePolyLinkOt(sdfGrOt_t *pOt, dds3TrianglePoly_t *hTriPoly) {
    sdfDrawTag_t *list = (sdfDrawTag_t *)sdfGrAllocPacketBuff(0x20);
    sdf3DPrimPolyParm_t state;
    s32 remaining;
    sdfDrawTagInit(list);
    sdf3DPrimLinkResetTm(list, 0);
    remaining = hTriPoly->count * 3;
    memset(&state, 0, sizeof(state));
    state.faceCount = 0x10;
    state.pointCount = 0x30;
    state.attr = 0x4000;
    state.vertex = (sdfVector4_t *)hTriPoly->positions;
    state.vertexColor = hTriPoly->colors;
    state.color = hTriPoly->color;
    while (remaining >= 0x30) {
        remaining -= 0x30;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&state));
    }
    if (remaining > 0) {
        D_005698B0.faceCount = remaining / 3;
        D_005698B0.pointCount = remaining;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(&state));
    }
    ((void (*)(sdfGrOt_t *, sdfDrawTag_t *))pOt->regFunc)(pOt, list);
}

dds3SpPolyOldDesc_t *dds3SpPoly_Ring_Old_Create(dds3SpPolyOldDesc_t *src) {
    s32 tailSize = sizeof(dds3SpPolyOldCellParam_t);
    dds3SpPolyOldDesc_t *desc = sdfChipAllocDirect(src->count * 4 + 0xF0);

    memset(desc, 0, 0xF0);
    memcpy(desc, src, src->headerSize);
    memcpy(&desc->cells, (u8 *)src + src->headerSize, tailSize);
    desc->headerSize = 0xC0;
    desc->cells.cellWords = (u32 *)(desc + 1);
    if (desc->cells.verticesPerCell < 3) {
        desc->cells.verticesPerCell = 3;
    }
    desc->cells.cellSystem = dds3TrackPolyCreate(desc->count, desc->cells.verticesPerCell, 1, TRACK_TOPOLOGY_PAIR);
    dds3TrackPolyVCSet(desc->cells.cellSystem, 0,
                   desc->cells.dispatchArgA,
                   desc->cells.dispatchArgB);
    dds3SpPoly_Ring_Old_BuffInit((dds3SpPolyOld_t *)desc);
    return desc;
}

void dds3SpPoly_Ring_Old_Delete(dds3SpPolyOld_t *hData) {
    dds3TrackPolyDelete(hData->strip);
    sdfChipFreeDirect(hData);
}

void dds3SpPoly_Ring_Old_BuffInit(dds3SpPolyOld_t *hData) {
    u16 count;
    s32 age;
    s32 *ages;
    u16 i;
    dds3TrackPoly_t *system;
    dds3TrackPolyList_t *cells;

    count = hData->entryCount;
    age = POLY_INACTIVE_ENTRY_AGE;
    ages = hData->ages;
    system = hData->strip;
    i = 0;
    if (hData->entryCount != 0) {
        cells = system->list;
        do {
            cells[i].rec = 0;
            cells[i].count = 0;
            cells[i].color = 0x00808080;
            *ages = age;
            ages++;
            age -= hData->spawnDelayStep;
            i++;
        } while (i < count);
    }
}

void dds3SpPoly_Ring_Old_SetScale(f32 hData, dds3SpPolyOld_t *scale) {
    scale->radius = scale->radius * hData;
    scale->pairDisplacement = scale->pairDisplacement * hData;
}

void dds3SpPoly_Ring_Old_Calc(dds3SpPolyOld_t *pRing) {
    u16 index;
    u16 completed;
    u16 count;
    u8 loop;
    s32 duration;
    f32 alphaStep;
    dds3TrackPolyList_t *entry;
    s32 *ages;

    if (pRing->active != 0) {
        duration = pRing->duration;
        completed = 0;
        count = pRing->entryCount;
        entry = pRing->strip->list;
        alphaStep = 128.0f / (f32)duration;
        ages = pRing->ages;
        loop = pRing->loop;
        for (index = 0; index < count; index++) {
            s32 age = *ages;

            if (age == POLY_INACTIVE_ENTRY_AGE) {
                dds3SpPoly_Ring_Old_New(pRing, index);
                *ages = 0;
            } else if (age >= 0) {
                entry->color = ((u32)(alphaStep * (f32)(duration - age)) << 24) | 0x808080;
                dds3SpPoly_Ring_Old_Move(pRing, index);
            }
            if (age >= duration) {
                entry->color = 0x808080;
                if (loop != 0) {
                    *ages = POLY_INACTIVE_ENTRY_AGE;
                } else {
                    completed++;
                    if (completed >= count) {
                        entry->color = 0;
                        pRing->active = 0;
                    }
                }
            } else {
                (*ages)++;
            }
            ages++;
            entry++;
        }
    }
}

void dds3SpPoly_Ring_Old_Draw(dds3SpPolyOld_t *hData) {
    dds3SpPoly_Ring_Old_Calc(hData);
    dds3TrackPolyDraw(hData->strip);
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_New);

void dds3SpPoly_Ring_Old_Move(dds3SpPolyOld_t *pRing, s32 number) {
    dds3TrackPoly_t *strip = pRing->strip;
    dds3TrackPolyList_t *entry = &strip->list[number];
    f32 scale[4];
    f32 *p;
    s32 pairs;
    s32 i;

    p = (f32 *)entry->pVertex;
    pairs = strip->face / 2;
    scale[0] = scale[1] = scale[2] = pRing->pairDisplacement;
    SDF_VU0_LOADVECTOR_C(scale);
    for (i = 0; i < pairs; i++) {
        SDF_VU0_LOADVECTOR(p + 4);
        SDF_VU0_LOADVECTOR_B(p);
        SDF_VU0_SUBVECTOR();
        SDF_VU0_NORMALIZE();
        SDF_VU0_COPYVECTOR_C_TO_B();
        SDF_VU0_MULVECTOR();
        SDF_VU0_COPYVECTOR_A_TO_B();
        SDF_VU0_LOADVECTOR(p);
        SDF_VU0_ADDVECTOR();
        SDF_VU0_STOREVECTOR(p);
        SDF_VU0_LOADVECTOR(p + 4);
        SDF_VU0_ADDVECTOR();
        SDF_VU0_STOREVECTOR(p + 4);
        p += 8;
    }
}

u32 dds3SpPoly_Color(u32 frame, u32 frame_end, u32 abgr) {
    f32 ratio = 1.0f;
    s32 tint[4];
    u8 unused[16]; /* SMT3 and DDS1 twin both reserve 0x20 bytes,
                      * but neither touches the spare 0x10-byte local. */
    u32 packed;

    if (frame < frame_end) {
        ratio = (f32)frame / (f32)frame_end;
    }
    SDF_VU0_LOADVECTOR(D_00317660);
    SDF_VU0_LOADVECTOR_B(D_00317650);
    SDF_VU0_SCALEVECTOR_BITS(1.0f - ratio);
    SDF_VU0_SCALEVECTOR_B_BITS(ratio);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_B();
    tint[0] = abgr;
    MMI_RGBA_UNPACK(tint, 1.0f / 128.0f);
    SDF_VU0_MULVECTOR();
    MMI_RGBA_PACK_UNIT(packed, 128.0f);
    return packed;
}

dds3SpPolyRing_t *dds3SpPoly_RingCreate(dds3SpPolyHead_t *templateHead) {
    s32 tailSize = sizeof(dds3SpPolyRing_t) - sizeof(dds3SpPolyHead_t);
    u32 size = templateHead->entryCount * sizeof(dds3SpPolyRingRecord_t) + sizeof(dds3SpPolyRing_t);
    sdfMemHandle_t *allocation = sdfMemAllocDirect(size);
    dds3SpPolyRing_t *band = (dds3SpPolyRing_t *)sdfMemLock(allocation);

    memset(band, 0, size);
    memcpy(band, templateHead, templateHead->templateSize);
    memcpy(&band->spawnDelayStep, (u8 *)templateHead + templateHead->templateSize, tailSize);
    band->head.templateSize = sizeof(dds3SpPolyHead_t);
    band->records = (dds3SpPolyRingRecord_t *)(band + 1);
    band->allocation = allocation;
    if (((dds3SpPolyRingUnion_t *)band)->band.segments < 3) {
        ((dds3SpPolyRingUnion_t *)band)->band.segments = 3;
    }
    band->strip = dds3TrackPolyCreate(band->head.entryCount, band->segments, 1, 0);
    dds3TrackPolyVCSet(band->strip, 0, band->dispatchArgA, band->dispatchArgB);
    dds3SpPoly_RingAllBuffInit(band);
    SDF_VU0_LOADMATRIX(band->head.matrix);
    SDF_VU0_STOREMATRIX(band->head.matrixCopy);
    dds3TrackPolySetMode(band->strip, band->head.drawBucket);
    return band;
}

void dds3SpPoly_RingDelete(dds3SpPolyRing_t *hData) {
    dds3TrackPolyDelete(hData->strip);
    sdfMemFreeDirect(hData->allocation);
}

void dds3SpPoly_RingAllBuffInit(dds3SpPolyRing_t *band) {
    u32 count = band->head.entryCount;
    dds3SpPolyRingRecord_t *record = band->records;
    u32 delayStep = band->spawnDelayStep;
    s32 age = POLY_INACTIVE_ENTRY_AGE;
    u32 index;

    index = 0;
    if (count != 0) {
        do {
            index++;
            record->age = age;
            age -= delayStep;
            record++;
        } while (index < count);
    }
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingBuffInit);

void dds3SpPoly_RingMove(dds3SpPolyRing_t *pRing, s32 number) {
    dds3TrackPoly_t *strip = pRing->strip;
    dds3SpPolyRingRecord_t *rec = &pRing->records[number];
    dds3TrackPolyList_t *entry = &strip->list[number];
    f32 radialDirection[4];
    f32 radiusStepVector[4];
    f32 liftVector[4];
    f32 angleStep;
    f32 angle;
    f32 *pointCursor;
    f32 *firstPointPair;
    s32 pairs;
    s32 i;

    entry->count = strip->face;
    pointCursor = (f32 *)entry->pVertex;
    pairs = strip->face >> 1;
    radiusStepVector[2] = radiusStepVector[1] = radiusStepVector[0] = rec->radiusStep;
    liftVector[1] = pRing->liftStep;
    liftVector[2] = liftVector[0] = 0;
    angleStep = 3.14159265f * 2.0f / (f32)pRing->segments;
    SDF_VU0_LOADMATRIX(pRing->head.matrix);
    angle = 0.0f;
    for (i = 0; i < pairs - 1; i++) {
        radialDirection[0] = sdfCos(angle);
        radialDirection[1] = 0;
        radialDirection[2] = sdfSin(angle);
        SDF_VU0_LOADVECTOR(radialDirection);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_LOADVECTOR_B(radiusStepVector);
        SDF_VU0_MULVECTOR();
        SDF_VU0_LOADVECTOR_B(liftVector);
        SDF_VU0_APPLYMATRIX_B();
        SDF_VU0_ADDVECTOR();
        SDF_VU0_COPYVECTOR_A_TO_C();
        SDF_VU0_LOADVECTOR_B(pointCursor + 4);
        SDF_VU0_ADDVECTOR();
        SDF_VU0_STOREVECTOR(pointCursor + 4);
        SDF_VU0_COPYVECTOR_C_TO_A();
        SDF_VU0_LOADVECTOR_B(pointCursor);
        SDF_VU0_ADDVECTOR();
        SDF_VU0_STOREVECTOR(pointCursor);
        pointCursor += 8;
        angle += angleStep;
    }
    firstPointPair = (f32 *)entry->pVertex;
    SDF_VU0_COPYVECTOR(pointCursor, firstPointPair);
    SDF_VU0_COPYVECTOR(pointCursor + 4, firstPointPair + 4);
}

void dds3SpPoly_RingSetScale(f32 hData, dds3SpPolyRing_t *scale) {
    scale->radialWidth = scale->radialWidth * hData;
    scale->liftStep = scale->liftStep * hData;
    scale->initialRadius = scale->initialRadius * hData;
    scale->targetRadius = scale->targetRadius * hData;
}

void dds3SpPoly_RingDraw(dds3SpPolyRing_t *hData) {
    s32 recordIndex = 0;
    u32 completed = 0;
    u8 loop;
    dds3TrackPoly_t *strip;
    s32 count;
    dds3SpPolyRingRecord_t *record;
    s32 duration;
    u32 color;
    dds3TrackPolyList_t *entry;

    strip = hData->strip;
    count = hData->head.entryCount;
    record = hData->records;
    duration = hData->head.duration;
    loop = hData->head.loop;
    color = hData->head.color;
    entry = strip->list;

    if (count > 0) {
        do {
            s32 age = record->age;

            if (age == POLY_INACTIVE_ENTRY_AGE) {
                dds3SpPoly_RingBuffInit(hData, recordIndex);
                age = record->age;
            }
            if (age < 0) {
                age++;
            } else {
                if ((u32)age < hData->head.startColorRampFrames) {
                    entry->color = dds3SpPoly_Color(age, hData->head.startColorRampFrames, color);
                } else if (age <= duration &&
                           (u32)age >= duration - hData->head.endColorRampFrames) {
                    entry->color = dds3SpPoly_Color(duration - age, hData->head.endColorRampFrames, color);
                }

                if (entry->color & 0xFF000000) {
                    dds3SpPoly_RingMove(hData, recordIndex);
                    age++;
                } else {
                    entry->count = 0;
                    age++;
                }
            }

            if (age >= duration) {
                if (loop != 0) {
                    age = POLY_INACTIVE_ENTRY_AGE;
                } else {
                    completed++;
                    if (completed >= (u32)count) {
                        hData->head.active = 0;
                    }
                }
                entry->count = 0;
            }

            record->age = age;
            recordIndex++;
            entry++;
            record++;
        } while (recordIndex < count);
    }
    dds3TrackPolyDraw(hData->strip);
}

dds3SpPolySphereRing_t *dds3SpPoly_SphereRingCreate(dds3SpPolySphereRing_t *source) {
    s32 tailBytes = sizeof(dds3SpPolySphereRing_t) - sizeof(dds3SpPolyHead_t);
    s32 size = source->head.entryCount * sizeof(dds3SpPolySphereRecord_t) + sizeof(dds3SpPolyHead_t) + tailBytes;
    sdfMemHandle_t *allocation = sdfMemAllocDirect(size);
    dds3SpPolySphereRing_t *clone = (dds3SpPolySphereRing_t *)sdfMemLock(allocation);

    memset(clone, 0, size);
    memcpy(clone, source, source->head.templateSize);
    memcpy(&clone->spawnDelayStep, (u8 *)source + source->head.templateSize, tailBytes);
    clone->head.templateSize = 0xC0;
    clone->records = (dds3SpPolySphereRecord_t *)(clone + 1);
    clone->allocation = allocation;
    if (((dds3SpPolyRingUnion_t *)clone)->arc.segments < 3) {
        ((dds3SpPolyRingUnion_t *)clone)->arc.segments = 3;
    }
    clone->strip = dds3TrackPolyCreate(clone->head.entryCount, clone->segments, 1, 0);
    dds3TrackPolyVCSet(clone->strip, 0, clone->dispatchArgA, clone->dispatchArgB);
    dds3SpPoly_SphereRingAllBuffInit(clone);
    SDF_VU0_LOADMATRIX(clone->head.matrix);
    SDF_VU0_STOREMATRIX(clone->head.matrixCopy);
    dds3TrackPolySetMode(clone->strip, clone->head.drawBucket);
    return clone;
}

void dds3SpPoly_SphereRingDelete(dds3SpPolySphereRing_t *hData) {
    dds3TrackPolyDelete(hData->strip);
    sdfMemFreeDirect(hData->allocation);
}

void dds3SpPoly_SphereRingAllBuffInit(dds3SpPolySphereRing_t *arc) {
    u32 count = arc->head.entryCount;
    dds3SpPolySphereRecord_t *record = arc->records;
    u32 delayStep = arc->spawnDelayStep;
    s32 age = POLY_INACTIVE_ENTRY_AGE;
    u32 index;

    index = 0;
    if (count != 0) {
        do {
            index++;
            record->age = age;
            age -= delayStep;
            record++;
        } while (index < count);
    }
}

void dds3SpPoly_SphereRingBuffInit(dds3SpPolySphereRing_t *pSphereRing, s32 number) {
    dds3SpPolySphereRecord_t *record = pSphereRing->records;
    f32 spread = pSphereRing->radiusJitter;

    record += number;
    record->age = 0;
    record->radius = pSphereRing->radius * (dds3GetRandFloatA(dds3EffectBase) * spread + (1.0f - spread));
}

void dds3SpPoly_SphereRingMove(dds3SpPolySphereRing_t *pSphereRing, s32 number) {
    dds3TrackPoly_t *strip = pSphereRing->strip;
    dds3SpPolySphereRecord_t *record = &pSphereRing->records[number];
    dds3TrackPolyList_t *stripEntry = &strip->list[number];
    f32 innerPointVector[4];
    f32 outerPointVector[4];
    f32 recordRadius;
    f32 projectedInnerRadius;
    f32 verticalOffset;
    f32 radialWidth;
    f32 angleStep;
    f32 angle;
    f32 *pointCursor;
    f32 *firstPointPair;
    s32 pairs;
    s32 i;

    stripEntry->count = strip->face;
    pointCursor = (f32 *)stripEntry->pVertex;
    pairs = strip->face >> 1;
    recordRadius = record->radius;
    angle = (f32)record->age / (f32)pSphereRing->head.duration * 3.14159265f;
    projectedInnerRadius = recordRadius * sdfSin(angle);
    verticalOffset = recordRadius * sdfCos(angle) - recordRadius;
    angleStep = 3.14159265f * 2.0f / (f32)pSphereRing->segments;
    SDF_VU0_LOADMATRIX(pSphereRing->head.matrix);
    radialWidth = pSphereRing->radialWidth;
    angle = 0.0f;
    SDF_VU0_LOADVECTOR_C(pSphereRing->head.origin);
    for (i = 0; i < pairs - 1; i++) {
        innerPointVector[0] = outerPointVector[0] = sdfCos(angle);
        innerPointVector[2] = sdfSin(angle);
        outerPointVector[1] = verticalOffset;
        outerPointVector[2] = innerPointVector[2] * (projectedInnerRadius + radialWidth);
        outerPointVector[0] *= projectedInnerRadius + radialWidth;
        SDF_VU0_LOADVECTOR(outerPointVector);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(outerPointVector);
        innerPointVector[1] = verticalOffset;
        innerPointVector[0] *= projectedInnerRadius;
        innerPointVector[2] *= projectedInnerRadius;
        SDF_VU0_LOADVECTOR(innerPointVector);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_ADDVECTOR_A_C();
        SDF_VU0_STOREVECTOR(pointCursor + 4);
        SDF_VU0_LOADVECTOR(outerPointVector);
        SDF_VU0_ADDVECTOR_A_C();
        SDF_VU0_STOREVECTOR(pointCursor);
        pointCursor += 8;
        angle += angleStep;
    }
    firstPointPair = (f32 *)stripEntry->pVertex;
    SDF_VU0_COPYVECTOR(pointCursor, firstPointPair);
    SDF_VU0_COPYVECTOR(pointCursor + 4, firstPointPair + 4);
}

void dds3SpPoly_SphereRingSetScale(f32 hData, dds3SpPolySphereRing_t *scale) {
    scale->radius = scale->radius * hData;
}

void dds3SpPoly_SphereRingDraw(dds3SpPolySphereRing_t *hData) {
    s32 index = 0;
    u32 completed = 0;
    u8 loop;
    dds3TrackPoly_t *strip;
    s32 count;
    dds3SpPolySphereRecord_t *record;
    s32 duration;
    u32 color;
    dds3TrackPolyList_t *entry;

    strip = hData->strip;
    count = hData->head.entryCount;
    record = hData->records;
    duration = hData->head.duration;
    loop = hData->head.loop;
    color = hData->head.color;
    entry = strip->list;

    if (count > 0) {
        do {
            s32 age = record->age;

            if (age == POLY_INACTIVE_ENTRY_AGE) {
                dds3SpPoly_SphereRingBuffInit(hData, index);
                age = record->age;
            }
            if (age < 0) {
                age++;
            } else {
                if ((u32)age < hData->head.startColorRampFrames) {
                    entry->color = dds3SpPoly_Color(age, hData->head.startColorRampFrames, color);
                } else if (age <= duration &&
                           (u32)age >= duration - hData->head.endColorRampFrames) {
                    entry->color = dds3SpPoly_Color(duration - age, hData->head.endColorRampFrames, color);
                }

                if (entry->color & 0xFF000000) {
                    dds3SpPoly_SphereRingMove(hData, index);
                    age++;
                } else {
                    entry->count = 0;
                    age++;
                }
            }

            if (age >= duration) {
                if (loop != 0) {
                    age = POLY_INACTIVE_ENTRY_AGE;
                } else {
                    completed++;
                    if (completed >= (u32)count) {
                        hData->head.active = 0;
                    }
                }
                entry->count = 0;
            }

            record->age = age;
            index++;
            entry++;
            record++;
        } while (index < count);
    }
    dds3TrackPolyDraw(hData->strip);
}

dds3SpPolyRollRing_t *dds3SpPoly_RollRingCreate(dds3SpPolyHead_t *templateHead) {
    s32 tailSize = sizeof(dds3SpPolyRollRing_t) - sizeof(dds3SpPolyHead_t);
    u32 size = templateHead->entryCount * sizeof(dds3SpPolyRollRecord_t) + sizeof(dds3SpPolyRollRing_t);
    sdfMemHandle_t *allocation = sdfMemAllocDirect(size);
    dds3SpPolyRollRing_t *band = (dds3SpPolyRollRing_t *)sdfMemLock(allocation);

    memset(band, 0, size);
    memcpy(band, templateHead, templateHead->templateSize);
    memcpy(&band->spawnDelayStep, (u8 *)templateHead + templateHead->templateSize, tailSize);
    band->head.templateSize = sizeof(dds3SpPolyHead_t);
    band->records = (dds3SpPolyRollRecord_t *)(band + 1);
    band->allocation = allocation;
    if (((dds3SpPolyRingUnion_t *)band)->rotating.segments < 3) {
        ((dds3SpPolyRingUnion_t *)band)->rotating.segments = 3;
    }
    band->strip = dds3TrackPolyCreate(band->head.entryCount, band->segments, 1, 0);
    dds3TrackPolyVCSet(band->strip, 0, band->dispatchArgA, band->dispatchArgB);
    dds3SpPoly_RollRingAllBuffInit(band);
    SDF_VU0_LOADMATRIX(band->head.matrix);
    SDF_VU0_STOREMATRIX(band->head.matrixCopy);
    dds3TrackPolySetMode(band->strip, band->head.drawBucket);
    return band;
}

void dds3SpPoly_RollRingDelete(dds3SpPolyRollRing_t *hData) {
    dds3TrackPolyDelete(hData->strip);
    sdfMemFreeDirect(hData->allocation);
}

void dds3SpPoly_RollRingAllBuffInit(dds3SpPolyRollRing_t *rotatingBand) {
    u32 count = rotatingBand->head.entryCount;
    dds3SpPolyRollRecord_t *record = rotatingBand->records;
    u32 delayStep = rotatingBand->spawnDelayStep;
    s32 age = POLY_INACTIVE_ENTRY_AGE;
    u32 index;

    index = 0;
    if (count != 0) {
        do {
            record->age = age;
            record++;
            if ((index + 1) % rotatingBand->spawnDelayGroupSize == 0) {
                age -= delayStep;
            }
            index++;
        } while (index < count);
    }
}

void dds3SpPoly_RollRingBuffInit(dds3SpPolyRollRing_t *pRollRing, u32 number) {
    dds3SpPolyRollRecord_t *record = pRollRing->records;
    f32 spread;

    record += number;
    spread = pRollRing->initialRadiusJitter;
    record->age = 0;
    record->radius = pRollRing->initialRadius * (dds3GetRandFloatA(dds3EffectBase) * spread + (1.0f - spread));
    spread = pRollRing->targetRadiusJitter;
    record->radiusStep = (pRollRing->targetRadius * (dds3GetRandFloatA(dds3EffectBase) * spread + (1.0f - spread)) - record->radius) / (f32)pRollRing->head.duration;
    record->rotationXRadians = pRollRing->rotationXDegrees * 0.017453292f;
    record->rotationYRadians = 6.2831852f / (f32)pRollRing->spawnDelayGroupSize * (f32)(number % pRollRing->spawnDelayGroupSize);
}

void dds3SpPoly_RollRingMove(dds3SpPolyRollRing_t *pRollRing, s32 number)
{
    dds3TrackPoly_t *strip = pRollRing->strip;
    dds3SpPolyRollRecord_t *rec = &pRollRing->records[number];
    dds3TrackPolyList_t *entry = &strip->list[number];
    f32 dir[4];
    f32 baseRadiusVector[4];
    f32 offsetRadiusVector[4];
    f32 step;
    f32 radius;
    f32 angle;
    f32 *out;
    f32 *first;
    s32 pairs;
    s32 i;

    entry->count = strip->face;
    out = (f32 *)entry->pVertex;
    pairs = strip->face >> 1;
    SetRotateMatrixX(rec->rotationXRadians);
    SetRotateMatrixY_B(rec->rotationYRadians);
    MultMatrixRevNoArg();
    rec->rotationYRadians += pRollRing->rotationStepDegrees * (3.14159265f / 180.0f);
    radius = rec->radius;
    rec->radius = radius + rec->radiusStep;
    step = 3.14159265f * 2.0f / (f32)pRollRing->segments;
    SDF_VU0_LOADMATRIX_B(pRollRing->head.matrix);
    MultMatrixRevNoArg();
    baseRadiusVector[0] = radius;
    baseRadiusVector[1] = radius;
    baseRadiusVector[2] = radius;
    offsetRadiusVector[0] = radius + pRollRing->radialWidth;
    offsetRadiusVector[1] = radius + pRollRing->radialWidth;
    offsetRadiusVector[2] = radius + pRollRing->radialWidth;
    angle = 0.0f;
    SDF_VU0_LOADVECTOR_C(pRollRing->head.origin);
    for (i = 0; i < pairs - 1; i++) {
        dir[0] = sdfCos(angle);
        dir[1] = 0;
        dir[2] = sdfSin(angle);
        SDF_VU0_LOADVECTOR(dir);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(dir);
        SDF_VU0_LOADVECTOR_B(baseRadiusVector);
        SDF_VU0_MULVECTOR();
        SDF_VU0_ADDVECTOR_A_C();
        SDF_VU0_STOREVECTOR(out + 4);
        SDF_VU0_LOADVECTOR(dir);
        SDF_VU0_LOADVECTOR_B(offsetRadiusVector);
        SDF_VU0_MULVECTOR();
        SDF_VU0_ADDVECTOR_A_C();
        SDF_VU0_STOREVECTOR(out);
        out += 8;
        angle += step;
    }
    first = (f32 *)entry->pVertex;
    SDF_VU0_COPYVECTOR(out, first);
    SDF_VU0_COPYVECTOR(out + 4, first + 4);
}

void dds3SpPoly_RollRingSetScale(f32 hData, dds3SpPolyRollRing_t *scale) {
    scale->initialRadius = scale->initialRadius * hData;
    scale->targetRadius = scale->targetRadius * hData;
}

void dds3SpPoly_RollRingDraw(dds3SpPolyRollRing_t *hData) {
    s32 recordIndex = 0;
    u32 completed = 0;
    u8 loop;
    dds3TrackPoly_t *strip;
    s32 count;
    dds3SpPolyRollRecord_t *record;
    s32 duration;
    u32 color;
    dds3TrackPolyList_t *entry;

    strip = hData->strip;
    count = hData->head.entryCount;
    record = hData->records;
    duration = hData->head.duration;
    loop = hData->head.loop;
    color = hData->head.color;
    entry = strip->list;

    if (count > 0) {
        do {
            s32 age = record->age;

            if (age == POLY_INACTIVE_ENTRY_AGE) {
                dds3SpPoly_RollRingBuffInit(hData, recordIndex);
                age = record->age;
            }
            if (age < 0) {
                age++;
            } else {
                if ((u32)age < hData->head.startColorRampFrames) {
                    entry->color = dds3SpPoly_Color(age, hData->head.startColorRampFrames, color);
                } else if (age <= duration &&
                           (u32)age >= duration - hData->head.endColorRampFrames) {
                    entry->color = dds3SpPoly_Color(duration - age, hData->head.endColorRampFrames, color);
                } else {
                    entry->color = color;
                }

                if (entry->color & 0xFF000000) {
                    dds3SpPoly_RollRingMove(hData, recordIndex);
                    age++;
                } else {
                    entry->count = 0;
                    age++;
                }
            }

            if (age >= duration) {
                if (loop != 0) {
                    age = POLY_INACTIVE_ENTRY_AGE;
                } else {
                    completed++;
                    if (completed >= (u32)count) {
                        hData->head.active = 0;
                    }
                }
                entry->count = 0;
            }

            record->age = age;
            recordIndex++;
            entry++;
            record++;
        } while (recordIndex < count);
    }
    dds3TrackPolyDraw(hData->strip);
}


void dds3SpPoly_RollRingSetEndless(dds3SpPolyRollRing_t *pRollRing) {
    u32 count;
    dds3SpPolyRollRecord_t *entry;
    u32 i;

    count = pRollRing->head.entryCount;
    i = 0;
    pRollRing->head.duration = POLY_RESET_DURATION;
    pRollRing->head.endColorRampFrames = 0;
    pRollRing->head.startColorRampFrames = 0;
    entry = pRollRing->records;
    if (count != 0) {
        do {
            if (entry->age != POLY_INACTIVE_ENTRY_AGE) {
                entry->age = POLY_RESET_ENTRY_AGE;
            }
            i++;
            entry++;
        } while (i < count);
    }
}

void *dds3SpPolyCreate(s32 index, void *arg) {
    dds3SpPolyHead_t *effect = dds3SpPolyTable[index].func(arg);

    effect->handler = index;
    effect->active = 1;
    return effect;
}

void *dds3SpPolyCopy(dds3SpPolyHead_t *effect) {
    dds3SpPolyHead_t *result = dds3SpPolyTable[effect->handler].func(effect);

    result->handler = effect->handler;
    result->active = 1;
    return result;
}

void dds3SpPolyDelete(dds3SpPolyHead_t *hData) {
    dds3SpPolyTable[hData->handler].destroy(hData);
}

void dds3SpPolyDraw(dds3SpPolyHead_t *hData) {
    dds3SpPolyTable[hData->handler].callback(hData);
}

void dds3SpPolyReset(dds3SpPolyHead_t *hData) {
    D_003176A0[hData->handler](hData);
    hData->active = 1;
}

void dds3SpPolySetScale(dds3SpPolyHead_t *hData, f32 scale) {
    D_003176B0[hData->handler](scale, hData);
    dds3SpPolyReset(hData);
}

u16 dds3SpPolyGetState(dds3SpPolyHead_t *hData) {
    return hData->active;
}

void dds3SpPolySetPosition(void *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(hData, pPos);
}

/* vu0 routine: compose the new rotation with the retained source matrix. */
void dds3SpPolySetRotateMatrix(dds3SpPolyHead_t *hData, void *pMatrix) {
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_LOADMATRIX_B(hData->matrixCopy);
    MultMatrixNoArg();
    SDF_VU0_STOREMATRIX(hData->matrix);
}

void dds3SpPolySetRGBA(dds3SpPolyHead_t *hData, u32 abgr) {
    hData->color = abgr;
}

void dds3SpPolySetLoop(dds3SpPolyLoopWork_t *hData, u8 loop) {
    if (hData->handler == 0) {
        hData->oldLoop = loop;
    }
    hData->loop = loop;
}

u8 dds3SpPolyGetLoop(dds3SpPolyLoopWork_t *hData) {
    return hData->loop;
}

void dds3PolygonSetColor(dds3Polygon_t *hPolygon, u32 color) {
    hPolygon->request.color = color;
}

void dds3PolygonTriangleSet(dds3Polygon_t *hPolygon, const f32 *center) {
    s32 count = hPolygon->drawCount;
    f32 point[4] __attribute__((aligned(16)));
    f32 (*positions)[4];
    u16 flags;

    SDF_VU0_COPYVECTOR(point, center);
    positions = (f32 (*)[4])(((u32)count << 4) + (u32)hPolygon->positions);
    SDF_VU0_COPYVECTOR(positions[0], point);
    point[1] += 5.0f;
    SDF_VU0_COPYVECTOR(positions[1], point);
    point[2] += 5.0f;
    SDF_VU0_COPYVECTOR(positions[2], point);
    flags = hPolygon->flags;
    if (flags & 2) {
        f32 (*normals)[4] = (f32 (*)[4])(((u32)count << 4) + (u32)hPolygon->normals);
        SDF_VU0_COPYVECTOR(normals[0], dds3PolygonTriangleParam);
        SDF_VU0_COPYVECTOR(normals[1], dds3PolygonTriangleParam);
        SDF_VU0_COPYVECTOR(normals[2], dds3PolygonTriangleParam);
    }
    if (flags & 4) {
        u32 *colors = (u32 *)(((u32)count << 2) + (u32)hPolygon->colors);
        colors[0] = 0x80000080;
        colors[1] = 0x80000080;
        colors[2] = 0x80000080;
        count = hPolygon->drawCount;
    }
    hPolygon->drawCount = count + 3;
}

void dds3TrackPolyColorSet(dds3TrackPoly_t *hTrackPoly, s32 number, u32 color) {
    hTrackPoly->list[number].color = color;
}
INCLUDE_SDATA(const s32, "effect/polyManager", hDrawListTop);

INCLUDE_SDATA(const s32, "effect/polyManager", dds3TrackPolyTag);

