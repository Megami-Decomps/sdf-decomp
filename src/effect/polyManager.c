#include "common.h"
#include "draw3d.h"

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
    s32 unk2c;
} dds3TrackPoly_t;

extern u32 hDrawListTop;
extern u8 D_005698A0[];
extern sdf3DPrimPolyParm_t D_005698B0;

extern void sdfRandomizeByTime(void *state);

void dds3TrackPolyInit(void);

void dds3PolygonManagerInit(void)
{
    hDrawListTop = 0;
    dds3TrackPolyInit();
    sdfRandomizeByTime(D_005698A0);
}

void dds3PolygonManagerExit(void) {
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonLinkOt);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonSetTex);

void dds3TrackPolyInit(void)
{
    memset(&D_005698B0, 0, sizeof(D_005698B0));
    D_005698B0.attr = 0x4000;
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyDelete);

void dds3TrackPolyListInit(dds3TrackPoly_t *hTrackPoly, s32 number)
{
    dds3TrackPolyList_t *list = (dds3TrackPolyList_t *)(number * 20 + (u32)hTrackPoly->list);

    list->color = 0x80808080;
    list->rec = 0;
    list->count = 0;
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolySet);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyMove);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolySet2);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyMove2);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCRefresh_ParType1);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCRefresh_ParType2);

void dds3TrackPolyVCSet_Normal(dds3TrackPoly_t *hTrackPoly, u32 firstColor, u32 secondColor)
{
    s32 perCell = hTrackPoly->face >> 1;
    s32 count = hTrackPoly->number;
    s32 i;

    for (i = 0; i < count; i++) {
        u32 *vc = hTrackPoly->list[i].pVC;
        s32 j;

        for (j = 0; j < perCell; j++) {
            vc[0] = firstColor;
            vc[1] = secondColor;
            vc += 2;
        }
    }
}

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet_Acute);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet_AcuteW);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet2_Normal);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet2_Acute);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet2_AcuteW);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC3Set_Normal);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC3Set_Acute);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC3Set_AcuteW);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet4_Normal);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet4_Acute);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet4_AcuteW);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC5Set_Normal);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC5Set_Acute);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC5Set_AcuteR);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVC5Set_AcuteW);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolySetMode);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyVCSet);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyLinkOt);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrianglePolyCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrianglePolyDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrianglePolyLinkOt);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_Create);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_Delete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_BuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_SetScale);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_Calc);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_Draw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_New);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Ring_Old_Move);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_Color);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingAllBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingMove);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingSetScale);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RingDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingAllBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingMove);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingSetScale);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_SphereRingDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingAllBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingBuffInit);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingMove);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingSetScale);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPoly_RollRingSetEndless);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyCreate);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyCopy);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyDelete);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyDraw);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyReset);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolySetScale);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyGetState);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolySetPosition);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolySetRotateMatrix);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolySetRGBA);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolySetLoop);

INCLUDE_ASM(const s32, "effect/polyManager", dds3SpPolyGetLoop);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonSetColor);

INCLUDE_ASM(const s32, "effect/polyManager", dds3PolygonTriangleSet);

INCLUDE_ASM(const s32, "effect/polyManager", dds3TrackPolyColorSet);
INCLUDE_SDATA(const s32, "effect/polyManager", hDrawListTop);

INCLUDE_SDATA(const s32, "effect/polyManager", dds3TrackPolyTag);

