#include "common.h"
#include "ps2asm.h"


/* One needle instance: position plus the colour the draw loop stamps. */
typedef struct effNeedleEntry_t
{
    f32 position[3];
    u8 unk0C[4];
    u32 color;
} effNeedleEntry_t;

typedef struct effNeedleWork_t
{
    f32 matrix[16];
    effNeedleEntry_t *entries;
    u32 entryCount;
    u32 mode;
    f32 scale[3];
    u32 vertexCount;
    f32 (*positions)[4];
    f32 (*normals)[4];
    u32 *colors;
    void *streamAllocation;
    void *drawAsset;
    void *backingAllocation;
} effNeedleWork_t;

/* Argument block effPkt3DPrimMakePoly reads. */
typedef struct effPrimParam_t
{
    u16 primCount;
    u16 vertexCount;
    u16 primitive;
    u16 mask;
    u32 color;
    void *parameters;
    f32 (*positions)[4];
    f32 (*normals)[4];
    u8 unk18[8];
    u32 *colors;
    u8 unk24[8];
} effPrimParam_t;

typedef struct effDrawSurface_t
{
    u8 unk00[0x10];
    void (*append)(struct effDrawSurface_t *surface, void *packet);
} effDrawSurface_t;

extern effPrimParam_t D_0056AD88;   /* effPkt3DPrimMakePoly parameter block */
extern f32 D_0031F460[][4];         /* default triangle points */
extern u32 D_0031F520[];            /* default triangle colours */
extern effDrawSurface_t *D_0031F550[];

extern void *sdfGrAllocPacketBuff(s32 aSize);
extern void sdfDrawTagInit(void *a);
extern void sdfDrawTagLink(void *aTag, u32 aPacket);
extern void sdf3DPrimLinkMat(void *aTag, void *aMat, s32 aAllocator);
extern void sdf3DPrimLinkTm(void *aTag, s32 aAllocator);
extern void *effPkt3DPrimMakePoly(effPrimParam_t *aParm);
extern void MultMatrixNoArg(void);
extern void sdfMatFreeDelayed(void *aMat);
extern void sdfMemFreeDirect(void *aHandle);
extern void dds3NeedleModelVNCListDelete(effNeedleWork_t *hNModel);


/* The copied 0x5C-byte needle parameters precede runtime ownership state. */
typedef struct effNeedleSpiralParams_t {
    f32 position[4];
    u32 count;
    s32 duration;
    u32 mode;
    s32 fadeIn;
    s32 fadeOut;
    u32 randomDelayRange;
    f32 speed;
    f32 acceleration;
    f32 radiusBase;
    f32 radiusJitter;
    f32 angleStep;
    u32 fanSegments;
    f32 radiusScale;
    f32 viewOffset;
    u32 centerColor;
    u32 outerColor;
    f32 width;
    s32 historyLength;
    u32 trailColor;
} effNeedleSpiralParams_t;

typedef struct effNeedleSpiralSlot_t {
    f32 direction[4];
    u8 unk10[0x10];
    s32 age;
    f32 radius;
    f32 angle;
} effNeedleSpiralSlot_t; /* 0x2C */

typedef struct effAllocation_t effAllocation_t;
typedef struct trackPolySystem_t trackPolySystem_t;
typedef struct effRandState_t { u32 words[4]; } effRandState_t;
typedef struct effAxis_t { f32 x, y, z; } effAxis_t;
struct BtlUnit;

typedef struct effNeedleSpiralWork_t {
    effNeedleSpiralParams_t params;
    effNeedleSpiralSlot_t *slots;
    u32 color;
    u32 count;
    trackPolySystem_t *system;
    effNeedleWork_t *resource;
    effAllocation_t *allocationHandle;
} effNeedleSpiralWork_t; /* 0x74, followed by count slots */

#define PAR_CELL_TOPOLOGY_PAIR 0
#define EFFECT_RING_FULL_TURN 6.2831853f
extern effRandState_t dds3EffectBase[];
extern f32 aViewTarget[4], aViewEye[4], D_0031F450[4];
extern void *sdfMemAllocDirect(u32 aSize);
extern void *sdfMemLock(void *aHandle);
extern void *sdfMatNew(void);
extern void sdfMatSetConstant(void *aMat, f32 aRatio);
extern void *memset(void *a, s32 b, u32 c);
extern void *dds3EffectPCPDataGetData(void *table, s32 block);
extern u32 dds3GetRandA(effRandState_t *name);
extern f32 dds3GetRandFloatA(effRandState_t *name);
extern u32 effBTLSkillCheckRun(void);
extern u32 effBTLSkillGetInvokeFormation(void);
extern void effBTLModelGetSphereCenter(struct BtlUnit *pFormation);
extern void LookAtMatrix(f32 *aTarget, f32 *aEye, f32 *aDown);
extern void ReverseMatrix(void);
extern f32 sdfSin(f32 aRadius);
extern f32 sdfCos(f32 aRadius);
extern void SetRotateMatrixAxis(f32 aAngle, const effAxis_t *aAxis);
extern u32 effColorBlend(u32 abgr0, u32 abgr1, f32 rate);
extern trackPolySystem_t *dds3TrackPolyCreate(s32 number, s32 face, s32 rec, u32 type);
extern void dds3TrackPolyDelete(trackPolySystem_t *hTrackPoly);
extern void dds3TrackPolySetMode(trackPolySystem_t *hTrackPoly, u16 mode);
extern void dds3TrackPolyVCSet(u32 hTrackPoly, s32 type, s32 abgr_in, s32 abgr_out);
extern void dds3TrackPolySet(trackPolySystem_t *hTrackPoly, s32 number, const u128 *pV);
extern void dds3TrackPolyVCRefresh_ParType1(trackPolySystem_t *hTrackPoly, s32 index);
extern void dds3TrackPolyColorSet(u32 hTrackPoly, s32 number, u32 color);
extern void dds3TrackPolyListInit(trackPolySystem_t *hTrackPoly, s32 number);
extern void dds3TrackPolyDraw(trackPolySystem_t *system);
effNeedleSpiralWork_t *effPCPNeedleSpiralCreate(effNeedleSpiralParams_t *pParam);
effNeedleWork_t *dds3NeedleModelCreate(u32 number);
void dds3NeedleModelDelete(effNeedleWork_t *hNModel);
void dds3NeedleModelDraw(effNeedleWork_t *hNModel);
void dds3NeedleModelVNCListCreate(effNeedleWork_t *hNModel, u32 split, u32 radius, u32 length, f32 abgr_in, f32 abgr_out);
void dds3NeedleModelSetPosition(effNeedleWork_t *hNModel, s32 index, f32 *pPos);
void dds3NeedleModelGetPosition(effNeedleWork_t *hNModel, s32 index, f32 *pPos);
void dds3NeedleModelSetColor(effNeedleWork_t *hNModel, s32 index, u32 abgr);
void dds3NeedleModelSetMatrix(effNeedleWork_t *work, const f32 *matrix);


effNeedleSpiralWork_t *effPCPNeedleSpiralCreate(effNeedleSpiralParams_t *pParam) {
    effAllocation_t *allocation = sdfMemAllocDirect(pParam->count * sizeof(effNeedleSpiralSlot_t) + sizeof(effNeedleSpiralWork_t));
    effNeedleSpiralWork_t *work = (effNeedleSpiralWork_t *)sdfMemLock(allocation);
    u32 slotIndex;
    effNeedleWork_t *resource;
    u32 count, delayRange;
    effNeedleSpiralSlot_t *slot;
    f32 radiusBase, radiusJitter;

    work->slots = (effNeedleSpiralSlot_t *)(work + 1);
    work->params = *pParam;
    work->allocationHandle = allocation;
    work->count = pParam->count;
    work->color = 0x80808080;
    work->system = 0;
    resource = dds3NeedleModelCreate(pParam->count);
    resource->mode = pParam->mode;
    work->resource = resource;
    dds3NeedleModelVNCListCreate(resource, pParam->fanSegments, pParam->centerColor,
                            pParam->outerColor, pParam->radiusScale, pParam->viewOffset);
    work->system = dds3TrackPolyCreate(work->count, work->params.historyLength, 0, PAR_CELL_TOPOLOGY_PAIR);
    dds3TrackPolyVCSet((u32)work->system, 1, work->params.trailColor, work->params.trailColor);
    dds3TrackPolySetMode(work->system, (u16)work->params.mode);
    count = work->count;
    delayRange = pParam->randomDelayRange;
    slot = work->slots;
    if (delayRange == 0) delayRange = 1;
    radiusBase = work->params.radiusBase;
    radiusJitter = work->params.radiusJitter;
    for (slotIndex = 0; slotIndex < count; slotIndex++, slot++) {
        slot->angle = dds3GetRandFloatA(dds3EffectBase) * EFFECT_RING_FULL_TURN;
        slot->radius = radiusBase * (dds3GetRandFloatA(dds3EffectBase) * radiusJitter + (1.0f - radiusJitter));
        slot->age = -(dds3GetRandA((effRandState_t *)dds3EffectBase) % delayRange);
        dds3NeedleModelSetColor(work->resource, slotIndex, 0x808080);
    }
    return work;
}

void effPCPNeedleSpiralDelete(effNeedleSpiralWork_t *hData) {
    dds3TrackPolyDelete(hData->system);
    dds3NeedleModelDelete(hData->resource);
    sdfMemFreeDirect(hData->allocationHandle);
}

void effPCPNeedleSpiralCreatePCF(void *data) {
    effNeedleSpiralParams_t *work;

    work = dds3EffectPCPDataGetData(data, 0);
    effPCPNeedleSpiralCreate(work);
}

void effPCPNeedleSpiralCopy(effNeedleSpiralParams_t *work) {
    effPCPNeedleSpiralCreate(work);
}

void effPCPNeedleSpiralDraw(effNeedleSpiralWork_t *hData) {
    f32 position[4];
    f32 axis[4];
    f32 vertices[2][4];
    f32 resourcePosition[4];
    f32 viewDirection[4];
    f32 width[4];
    f32 viewOffset[4];
    effNeedleSpiralSlot_t *slot;
    s32 fadeIn;
    u32 count;
    u32 i;
    s32 age;
    u32 baseColor;
    u32 color;
    f32 t;
    f32 speed;
    f32 acceleration;
    f32 angleStep;
    f32 value;
    u32 radius;
    s32 duration;
    s32 fadeOut;

    value = hData->params.width;
    width[0] = value;
    width[1] = value;
    width[2] = value;
    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR_NOMEM(viewDirection);
    value = -hData->params.viewOffset * 0.75f;
    viewOffset[0] = value;
    viewOffset[1] = value;
    viewOffset[2] = value;

    if (effBTLSkillCheckRun() != 0) {
        effBTLModelGetSphereCenter((struct BtlUnit *)effBTLSkillGetInvokeFormation());
        SDF_VU0_STOREVECTOR_NOMEM(position);
        LookAtMatrix(hData->params.position, position, D_0031F450);
        ReverseMatrix();
        SDF_VU0_STOREMATRIX_NOMEM(hData->resource->matrix);
    } else {
        SDF_VU0_LOADMATRIX(hData->resource->matrix);
    }

    axis[0] = 0.0f;
    axis[2] = 1.0f;
    axis[1] = 0.0f;
    SDF_VU0_LOADVECTOR(axis);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_STOREVECTOR_NOMEM(axis);
    SDF_VU0_LOADVECTOR_B(viewOffset);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR_NOMEM(viewOffset);

    count = hData->count;
    i = 0;
    slot = hData->slots;
    speed = hData->params.speed;
    acceleration = hData->params.acceleration;
    duration = hData->params.duration;
    fadeIn = hData->params.fadeIn;
    fadeOut = hData->params.fadeOut;
    angleStep = hData->params.angleStep;
    baseColor = hData->color;
    if (count != 0) {
        do {
            age = slot->age;
            if (age == 0) {
                SDF_VU0_LOADMATRIX(hData->resource->matrix);
                radius = (u32)slot->radius;
                position[0] = sdfCos(slot->angle) * radius;
                position[1] = sdfSin(slot->angle) * radius;
                position[2] = 0.0f;
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_APPLYMATRIX_XYZ();
                SDF_VU0_STOREVECTOR_NOMEM(position);
                slot->direction[0] = position[0];
                slot->direction[1] = position[1];
                slot->direction[2] = position[2];
                dds3NeedleModelSetPosition(hData->resource, i, position);
            }

            if (age >= 0 && age <= duration) {
                dds3NeedleModelGetPosition(hData->resource, i, resourcePosition);
                SDF_VU0_LOADVECTOR(viewOffset);
                SDF_VU0_LOADVECTOR_B(resourcePosition);
                SDF_VU0_ADDVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(resourcePosition);

                position[0] = slot->direction[0];
                position[1] = slot->direction[1];
                position[2] = slot->direction[2];
                SetRotateMatrixAxis(slot->angle, (const effAxis_t *)axis);
                slot->angle += angleStep;
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_APPLYMATRIX_XYZ();
                SDF_VU0_STOREVECTOR_NOMEM(position);

                {
                    f32 elapsed = (f32)slot->age;
                    f32 halfAcceleration = acceleration * 0.5f;
                    f32 distance = speed * elapsed + halfAcceleration * elapsed * elapsed;

                    elapsed = (f32)duration;
                    distance -= speed * elapsed + halfAcceleration * elapsed * elapsed;
                    position[0] += axis[0] * distance;
                    position[1] += axis[1] * distance;
                    position[2] += axis[2] * distance;
                }
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_LOADVECTOR_B(hData->params.position);
                SDF_VU0_ADDVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(position);
                dds3NeedleModelSetPosition(hData->resource, i, position);

                if (age < fadeIn && fadeIn != 0) {
                    t = (f32)age / (f32)fadeIn;
                } else if (duration == age && fadeOut != 0) {
                    t = 0.0f;
                } else {
                    t = fadeOut < duration - age || fadeOut == 0
                        ? 1.0f : (f32)(duration - age) / (f32)fadeOut;
                }
                color = effColorBlend(baseColor & 0xFFFFFF, baseColor, t);
                dds3NeedleModelSetColor(hData->resource, i, color);

                if (age > 0) {
                    SDF_VU0_LOADVECTOR(viewOffset);
                    SDF_VU0_LOADVECTOR_B(position);
                    SDF_VU0_ADDVECTOR();
                    SDF_VU0_COPYVECTOR_A_TO_C();
                    SDF_VU0_LOADVECTOR(resourcePosition);
                    SDF_VU0_COPYVECTOR_C_TO_B();
                    SDF_VU0_SUBVECTOR();
                    SDF_VU0_LOADVECTOR_B(viewDirection);
                    SDF_VU0_OUTERPRODUCT();
                    SDF_VU0_NORMALIZE();
                    SDF_VU0_LOADVECTOR_B(width);
                    SDF_VU0_MULVECTOR();
                    SDF_VU0_SWAPVECTOR_A_C();
                    SDF_VU0_COPYVECTOR_A_TO_B();
                    SDF_VU0_ADDVECTOR_A_C();
                    SDF_VU0_STOREVECTOR_NOMEM(vertices[0]);
                    SDF_VU0_COPYVECTOR_C_TO_A();
                    SDF_VU0_SUBVECTOR_B_A();
                    SDF_VU0_STOREVECTOR_B_NOMEM(vertices[1]);
                    dds3TrackPolySet(hData->system, i, (const u128 *)vertices);
                    dds3TrackPolyVCRefresh_ParType1(hData->system, i);
                    dds3TrackPolyColorSet((u32)hData->system, i, (color & 0xFF000000) | 0x808080);
                }
            } else {
                dds3NeedleModelSetColor(hData->resource, i, 0);
                dds3TrackPolyColorSet((u32)hData->system, i, 0);
                dds3TrackPolyListInit(hData->system, i);
            }
            slot->age++;
            i++;
            slot++;
        } while (i < count);
    }
    dds3TrackPolyDraw(hData->system);
    dds3NeedleModelDraw(hData->resource);
}

void effPCPNeedleSpiralSetPosition(void *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(hData, pPos);
}

void effPCPNeedleSpiralSetRotateMatrix(effNeedleSpiralWork_t *hData, void *pMatrix) {
    dds3NeedleModelSetMatrix(hData->resource, pMatrix);
}

void effPCPNeedleSpiralSetColor(effNeedleSpiralWork_t *hData, u32 abgr) {
    hData->color = abgr;
}

effNeedleWork_t *dds3NeedleModelCreate(u32 number)
{
    struct effAllocation_t *allocation = sdfMemAllocDirect(number * sizeof(effNeedleEntry_t) + sizeof(effNeedleWork_t));
    effNeedleWork_t *work = (void *)sdfMemLock(allocation);
    effNeedleEntry_t *entry;
    u32 entryIndex;

    work->mode = 2;
    work->entries = (effNeedleEntry_t *)(work + 1);
    work->backingAllocation = allocation;
    work->entryCount = number;
    work->scale[0] = 1.0f;
    work->scale[1] = 1.0f;
    work->scale[2] = 1.0f;
    work->streamAllocation = NULL;
    SDF_VU0_STOREIDENTITY(work->matrix);
    work->drawAsset = sdfMatNew();
    sdfMatSetConstant(work->drawAsset, 1.0f);
    entry = work->entries;
    for (entryIndex = 0; entryIndex < number; entryIndex++, entry++) {
        entry->color = 0x80808080;
    }
    memset(&D_0056AD88, 0, sizeof(D_0056AD88));
    D_0056AD88.primitive = 0x4000;
    return work;
}

void dds3NeedleModelDelete(effNeedleWork_t *hNModel)
{
    sdfMatFreeDelayed(hNModel->drawAsset);
    dds3NeedleModelVNCListDelete(hNModel);
    sdfMemFreeDirect(hNModel->backingAllocation);
}

void dds3NeedleModelDraw(effNeedleWork_t *hNModel)
{
    f32 matrix[16] __attribute__((aligned(16)));
    void *packet;
    effNeedleEntry_t *entry;
    u32 entryIndex;
    u32 remaining;
    u32 perPrim;

    packet = sdfGrAllocPacketBuff(0x20);
    sdfDrawTagInit(packet);
    sdf3DPrimLinkMat(packet, hNModel->drawAsset, 0);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[0] = hNModel->scale[0];
    matrix[5] = hNModel->scale[1];
    matrix[10] = hNModel->scale[2];
    SDF_VU0_LOADMATRIX(matrix);
    SDF_VU0_LOADMATRIX_B(hNModel->matrix);
    MultMatrixNoArg();
    SDF_VU0_STOREMATRIX(matrix);
    D_0056AD88.parameters = NULL;
    entry = hNModel->entries;
    for (entryIndex = 0; entryIndex < hNModel->entryCount; entryIndex++, entry++) {
        perPrim = 3;
        matrix[12] = entry->position[0];
        matrix[13] = entry->position[1];
        matrix[14] = entry->position[2];
        SDF_VU0_LOADMATRIX(matrix);
        sdf3DPrimLinkTm(packet, 0);
        if (hNModel->streamAllocation == NULL) {
            remaining = 12;
            D_0056AD88.colors = D_0031F520;
            D_0056AD88.positions = D_0031F460;
            D_0056AD88.normals = NULL;
        } else {
            D_0056AD88.positions = hNModel->positions;
            D_0056AD88.colors = hNModel->colors;
            D_0056AD88.normals = hNModel->normals;
            remaining = hNModel->vertexCount;
        }
        D_0056AD88.color = entry->color;
        D_0056AD88.primCount = 16;
        D_0056AD88.vertexCount = 48;
        while (remaining >= 48) {
            sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(&D_0056AD88));
            if (hNModel->streamAllocation == NULL) {
                D_0056AD88.colors += 48;
                D_0056AD88.positions += 48;
            } else {
                D_0056AD88.colors += 48;
                D_0056AD88.positions += 48;
                D_0056AD88.normals += 48;
            }
            remaining -= 48;
        }
        if (remaining >= 3) {
            D_0056AD88.primCount = remaining / perPrim;
            D_0056AD88.vertexCount = remaining;
            sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(&D_0056AD88));
        }
    }
    D_0031F550[hNModel->mode]->append(D_0031F550[hNModel->mode], packet);
}

extern s32 printf(const char *format, ...);

/* Build a closed triangle fan: one outer vertex, the axis centre and the next
 * outer vertex per segment. */
void dds3NeedleModelVNCListCreate(effNeedleWork_t *hNModel, u32 split, u32 radius, u32 length, f32 abgr_in, f32 abgr_out)
{
    void *allocation;
    f32 (*position)[4];
    f32 (*normal)[4];
    u32 *color;
    u32 vertexCount;
    u32 i;
    f32 angle;
    f32 step;
    f32 mid;

    if (split < 3) {
        split = 3;
    }
    if (hNModel->streamAllocation != NULL) {
        dds3NeedleModelVNCListDelete(hNModel);
    }
    angle = 0.0f;
    vertexCount = split * 3;
    allocation = sdfMemAllocDirect(vertexCount * 36);
    hNModel->streamAllocation = allocation;
    position = (f32 (*)[4])sdfMemLock(allocation);
    normal = position + vertexCount;
    color = (u32 *)(normal + vertexCount);
    hNModel->vertexCount = vertexCount;
    hNModel->positions = position;
    hNModel->normals = normal;
    hNModel->colors = color;
    step = EFFECT_RING_FULL_TURN / (f32)split;
    for (i = 0; i < split; i++) {
        mid = step * 0.5f;
        normal[0][0] = sdfCos(angle);
        normal[0][1] = sdfSin(angle);
        normal[0][2] = 0.0f;
        mid = angle + step * 0.5f;
        position[0][0] = normal[0][0] * abgr_in;
        angle += step;
        position[0][1] = normal[0][1] * abgr_in;
        position[0][2] = -abgr_out;
        normal++;
        position++;
        normal[0][0] = sdfCos(mid);
        normal[0][1] = sdfSin(mid);
        normal[0][2] = 0.0f;
        position[0][0] = 0.0f;
        position[0][1] = 0.0f;
        position[0][2] = 0.0f;
        normal++;
        position++;
        normal[0][0] = sdfCos(angle);
        normal[0][1] = sdfSin(angle);
        normal[0][2] = 0.0f;
        color[0] = length;
        color[1] = radius;
        color[2] = length;
        color += 3;
        position[0][0] = normal[0][0] * abgr_in;
        position[0][1] = normal[0][1] * abgr_in;
        position[0][2] = -abgr_out;
        normal++;
        position++;
    }
    printf("dds3NeedleModelVNCListCreate[%p] ****\n", hNModel->streamAllocation);
}

void dds3NeedleModelVNCListDelete(effNeedleWork_t *hNModel)
{
    if (hNModel->streamAllocation != NULL) {
        printf("dds3NeedleModelVNCListDelete[%p] ****\n", hNModel->streamAllocation);
        sdfMemFreeDirect(hNModel->streamAllocation);
    }
}

void dds3NeedleModelSetPosition(effNeedleWork_t *hNModel, s32 index, f32 *pPos)
{
    f32 *dst = (f32 *)(index * sizeof(effNeedleEntry_t) + (s32)hNModel->entries);

    dst[0] = pPos[0];
    dst[1] = pPos[1];
    dst[2] = pPos[2];
}

void dds3NeedleModelGetPosition(effNeedleWork_t *hNModel, s32 index, f32 *pPos)
{
    f32 *src = (f32 *)(index * sizeof(effNeedleEntry_t) + (s32)hNModel->entries);

    pPos[0] = src[0];
    pPos[1] = src[1];
    pPos[2] = src[2];
}

void dds3NeedleModelSetColor(effNeedleWork_t *hNModel, s32 index, u32 abgr)
{
    hNModel->entries[index].color = abgr;
}

/* vu0 routine: copy the resource transform through the matrix bank. */
void dds3NeedleModelSetMatrix(effNeedleWork_t *work, const f32 *matrix) {
    VU0_COPY_MATRIX(work->matrix, matrix);
}



