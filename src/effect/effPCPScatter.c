#include "common.h"
#include "ps2asm.h"
#include "sdf_resource.h"

struct effDebugPolyParams_t;
struct EffRandState;
struct dds3WhirlwindPolyList_tag;
struct effPCPWhirlwindPoly_tag;
struct effPCPWhirlwind2Poly_tag;
struct effPCPWhirlwind3Poly_tag;
struct PcpScatterParamsA;
struct PcpScatterParamsB;
struct PcpScatterParamsC;
struct PcpScatterParticle;
struct effPCPWwSpherePoly_tag;
struct PcpScatterPlainParams;
struct PcpScatterPlainParticle;
struct dds3ScatterPolyList_tag;
struct PcpScatterRadialParams;
struct PcpScatterRadialParticle;
struct effPCPHamaCardSpiral_tag;
struct PcpScatterRibbonParams;
struct effPCPHamaCardScatterPart_tag;
struct effPCPHamaCardScatter_tag;
struct PcpScatterSpinParams;
struct effPCPWwSpherePolyPart_tag;
struct effPCPHamaCardSphere_tag;
struct sdfMatPacket_t;
union SdfDrawTransform;
struct SdfGsTextureStateValues;
struct sdfGsMemSlice_t;

typedef struct effDebugPolyParams_t effDebugPolyParams_t;
typedef struct sdfTexPacket_t sdfTexPacket_t;
typedef union SdfDrawTransform SdfDrawTransform;
typedef struct SdfGsTextureStateValues SdfGsTextureStateValues;
typedef struct sdfMatPacket_t sdfMatPacket_t;
typedef union sdfMatUvwParm_t sdfMatUvwParm_t;
typedef struct dds3WhirlwindPolyList_tag dds3WhirlwindPolyList_tag;
typedef struct PcpScatterParamsA PcpScatterParamsA;
typedef struct PcpScatterParticle PcpScatterParticle;
typedef struct effPCPWhirlwindPoly_tag effPCPWhirlwindPoly_tag;
typedef struct PcpScatterParamsB PcpScatterParamsB;
typedef struct effPCPWhirlwind2Poly_tag effPCPWhirlwind2Poly_tag;
typedef struct PcpScatterParamsC PcpScatterParamsC;
typedef struct effPCPWhirlwind3Poly_tag effPCPWhirlwind3Poly_tag;
typedef struct PcpScatterPlainParams PcpScatterPlainParams;
typedef struct PcpScatterPlainParticle PcpScatterPlainParticle;
typedef struct effPCPWwSpherePoly_tag effPCPWwSpherePoly_tag;
typedef struct effPartHandle_t effPartHandle_t;
typedef struct PcpScatterRadialParams PcpScatterRadialParams;
typedef struct PcpScatterRadialParticle PcpScatterRadialParticle;
typedef struct effPCPHamaCardSpiral_tag effPCPHamaCardSpiral_tag;
typedef struct effPCPHamaCardScatterPart_tag effPCPHamaCardScatterPart_tag;
typedef struct PcpScatterRibbonParams PcpScatterRibbonParams;
typedef struct effPCPHamaCardScatter_tag effPCPHamaCardScatter_tag;
typedef struct PcpScatterSpinParams PcpScatterSpinParams;
typedef struct effPCPWwSpherePolyPart_tag effPCPWwSpherePolyPart_tag;
typedef struct effPCPHamaCardSphere_tag effPCPHamaCardSphere_tag;
typedef struct sdfGrOt_t sdfGrOt_t;
typedef struct EffRandState EffRandState;

#define EFF_SCATTER_NEUTRAL_COLOR 0x80808080

#define EFF_SCATTER_FULL_TURN 6.2831852f

#define EFF_SCATTER_DEGREES_TO_RADIANS 0.017453292f

#define EFF_SCATTER_RIGHT_ANGLE_DEGREES 90.0f

#define EFF_SCATTER_DIAGONAL_ANGLE_DEGREES 45.0f

#define EFF_SCATTER_TILT_STEP_DEGREES 5.0f

#define EFF_SCATTER_INITIAL_RIBBON_RADIUS 200.0f

#define EFF_SCATTER_RANDOM_MIDPOINT 0.5f

#define EFF_SCATTER_RANDOM_SPAN 2.0f

#define EFF_SCATTER_POOL_SLOTS_PER_GROUP 3

#define EFF_SCATTER_POOL_RECORD_WORDS_PER_SLOT 8

#define EFF_SCATTER_POOL_AUX_WORDS_PER_SLOT 2

#define EFF_SCATTER_WORD_BYTES 4

#define EFF_SCATTER_POOL_CONTROL_BYTES 0x34

#define EFF_SCATTER_RECORD_BYTES 0x60

#define EFF_SCATTER_AUX_RECORD_BYTES 0x18

#define EFF_SCATTER_DRAW_TEMPLATE_BYTES 0x2C


struct effDebugPolyParams_t {
    s16 parameterCount;
    s16 vertexCount;
    u16 primitive;
    u16 mask;
    u32 color;
    const u32 *parameters;
    u128 *positions;
    u128 *normals;
    f32 *texcoords;
    u32 *extraTexcoords;
    u32 *colors;
    void *(*allocate)(s32);
    f32 depth;
};

struct dds3ScatterPolyListTexResource_tag;

struct sdfTexHandle_t;

struct sdfTexPacket_t;

struct sdfTexPacket_t {
    s32 gifTagWord; /* Low GIFtag word; NLOOP occupies bits 0-14. */
    u32 gifTagControl; /* High GIFtag control word; one packed register. */
    u64 gifTagRegisters; /* Packed register list: A+D. */
    u64 samplingState; /* GS TEX1 data, including minification/magnification filters. */
    u64 samplingRegister; /* GS TEX1_1/TEX1_2 register selector. */
    u64 textureState; /* GS TEX0 data. */
    u64 textureRegister; /* GS TEX0_1/TEX0_2 register selector. */
    u64 clampState; /* GS CLAMP data. */
    u64 clampRegister; /* GS CLAMP_1/CLAMP_2 register selector. */
};

/* Per-texture reference count and non-owning clone source (0x8). */


/* Serialized byte consumed by the GS texture-state packet builder. */
typedef u8 SdfTextureClampMode;


/* Shared scatter texture reference, created and released independently of pools. */
typedef struct dds3ScatterPolyListTexResource_tag {
    sdfTexHandle_t *textureHandle;
    s32 refCount;
} dds3ScatterPolyListTexResource_tag; /* 0x08 */

struct sdfMat_t;

/* Draw entries store six transform floats and expose the same bytes as three qwords. */
union SdfDrawTransform {
    f32 m[6];
    u64 words[3];
};

/* The cached GS TEX1, TEX0 and CLAMP values shared by texture draw records. */
struct SdfGsTextureStateValues {
    u64 sampling;
    u64 texture;
    u64 clamp;
};

/* Native 0xA0-byte draw-entry block allocated by SDF_ASSET_DRAW_ENTRY_BYTES. */
struct sdfMatPacket_t {
    u32 pad00;             /* 0x00 */
    u32 ambient;             /* 0x04 */
    u32 diffuse;             /* 0x08 */
    u32 multiTexDiffuse;    /* 0x0C: packed color applied by the secondary draw state */
    u32 specular; /* 0x10: copied from the owning asset, or forced to zero. */
    u32 refmapColor;             /* 0x14 */
    u32 pad18;             /* 0x18 */
    f32 constant;             /* 0x1C */
    u32 alphaState;        /* 0x20: packed GS ALPHA_2 state emitted by the VU transfer builder */
    u32 multiTexMethod;     /* 0x24: secondary packet mode copied from SdfAsset */
    f32 y;                 /* 0x28 */
    f32 x;                 /* 0x2C */
    u8 pad30[8];           /* 0x30 */
    SdfGsTextureStateValues primaryTextureState; /* 0x38 */
    SdfGsTextureStateValues secondaryTextureState; /* 0x50 */
    SdfDrawTransform transforms[2]; /* 0x68 */
    u32 vifItopCommand;    /* 0x98: VIF ITOP with immediate 0xC */
    u32 vifMscalCommand;   /* 0x9C: VIF MSCAL with microprogram index zero */
};

union sdfMatUvwParm_t;

/* Optional packed or five-float state block owned by an SdfAsset. */
union sdfMatUvwParm_t {
    struct {
        u64 unk0;
        u64 unk8;
        u32 unk10;
        u32 unk14;
    } packed;
    struct {
        f32 values[5];
        u32 unk14;
    } scalar;
    /* Named view of scalar.values; the last word remains opaque. */
    struct {
        f32 centerOffsetX;
        f32 centerOffsetY;
        f32 basisScaleX;
        f32 basisScaleY;
        f32 rotationInput;
        u32 unk14;
    } transformInputs;
};


struct sdfMemHandle_t;

/* Embedded command work at actor +0x20. Neither record is the whole owning
 * actor. */
struct sdfMemHandle_t;


/* The drawable owns its geometry allocation and asset independently of the
 * effect instance that supplies its transform and scale. */
struct dds3WhirlwindPolyList_tag {
    f32 origin[4];             // 0x00
    f32 matrix[16];            // 0x10
    u32 packetQueueIndex;     // 0x50
    u32 color;                // 0x54
    u32 particleCount;        // 0x58
    s32 vectorsPerParticle;   // 0x5C: two coordinate vectors per vertex pair
    f32 scale;                // 0x60
    f32 *points;              // 0x64
    f32 *uv;                  // 0x68
    u32 *vertexColors;        // 0x6C
    u32 *colors;              // 0x70
    struct sdfMat_t *asset;    // 0x74
    struct sdfMemHandle_t *allocation; // 0x78
    struct dds3ScatterPolyListTexResource_tag *sharedResource; // 0x7C
};

/* The first ring variant retains its complete copied parameter head in the
 * runtime owner. Its particle array follows the fixed 0x18C-byte owner. */
struct PcpScatterParamsA {
    f32 origin[4];
    f32 matrix[16];
    u32 packetQueueIndex;
    u8 loop;
    u8 pad55[3];
    s32 duration;
    u32 particleCount;
    u32 segmentsPerParticle;
    u32 randomDelayRange;
    s32 fadeIn;
    s32 fadeRange;
    f32 angleStepBase;
    f32 angleStepJitter;
    f32 riseStep;
    f32 tiltScale;
    f32 heightOffsetBase;
    f32 heightOffsetJitter;
    f32 initialRise;
    f32 riseDecay;
    u8 pad90[4];
    f32 initialTiltSpeed;
    f32 tiltDamping;
    f32 radiusBase;
    f32 radiusJitter;
    f32 targetRadius;
    f32 targetRadiusJitter;
    s32 baseColor;
    u32 uSpan; /* Horizontal UV extent. */
    u32 vSpan; /* Vertical UV extent. */
    u8 padB8[0x80];
};

/* Shared particle record used by B/C and the related copied-parameter ring. */
struct PcpScatterParticle {
    f32 orientationAngle;
    f32 tiltAngle;
    s32 age;
    f32 rise;
    f32 angle;
    f32 tiltSpeed;
    f32 angleStep;
    f32 radius;
    f32 radiusStep;
    f32 heightOffset;
};

struct effPCPWhirlwindPoly_tag {
    f32 matrix[16];
    PcpScatterParamsA param;
    PcpScatterParticle *pPart;
    f32 scale;
    u32 color;
    dds3WhirlwindPolyList_tag *hPoly;
    sdfMemHandle_t *allocationHandle;
};

/* B's copied parameter block and runtime owner. Its particle pointer follows
 * the copied parameters, and the variable particle array follows the owner. */
struct PcpScatterParamsB {
    f32 origin[4];
    f32 matrix[16];
    u32 packetQueueIndex;
    u8 loop;
    u8 pad55[3];
    s32 duration;
    u32 particleCount;
    u32 segmentsPerParticle;
    u32 randomDelayRange;
    s32 fadeIn;
    s32 fadeRange;
    f32 angleStepBase;
    f32 angleStepJitter;
    f32 riseStep;
    f32 tiltScale;
    f32 heightOffsetBase;
    f32 heightOffsetJitter;
    f32 initialRise;
    f32 riseDecay;
    u8 pad90[4];
    f32 initialTiltSpeed;
    f32 tiltDamping;
    f32 radiusBase;
    f32 radiusJitter;
    f32 radiusStepBase;
    f32 radiusStepJitter;
    f32 radiusDamping;
    s32 baseColor;
    u32 uSpan; /* Horizontal UV extent. */
    u32 vSpan; /* Vertical UV extent. */
    u8 padBC[0x80];
};

struct effPCPWhirlwind2Poly_tag {
    f32 matrix[16];
    PcpScatterParamsB param;
    PcpScatterParticle *pPart;
    f32 scale;
    u32 abgr;
    s32 age;
    dds3WhirlwindPolyList_tag *hPoly;
    sdfMemHandle_t *allocationHandle;
};

/* C adds staggered ring motion and two color keys to the copied parameters. */
struct PcpScatterParamsC {
    f32 origin[4];
    f32 matrix[16];
    u32 packetQueueIndex;
    u8 loop;
    u8 pad55[3];
    s32 duration;
    u32 particleCount;
    u32 segmentsPerParticle;
    u32 randomDelayRange;
    s32 fadeIn;
    s32 fadeRange;
    f32 angleStepBase;
    f32 angleStepJitter;
    f32 tiltScale;
    f32 riseRange;
    f32 radiusRamp;
    f32 heightOffsetBase;
    f32 heightOffsetJitter;
    f32 initialRise;
    f32 riseDecay;
    u8 pad94[4];
    f32 initialTiltSpeed;
    f32 tiltDamping;
    f32 radiusBase;
    f32 radiusJitter;
    f32 radiusStepBase;
    f32 radiusStepJitter;
    f32 radiusDamping;
    s32 startColor;
    s32 endColor;
    u32 uSpan; /* Horizontal UV extent; this variant reverses the legacy names. */
    u32 vSpan;
    u8 padC4[0x80];
};

struct effPCPWhirlwind3Poly_tag {
    f32 matrix[16];
    PcpScatterParamsC param;
    PcpScatterParticle *pPart;
    f32 scale;
    u32 color;
    s32 age;
    dds3WhirlwindPolyList_tag *hPoly;
    sdfMemHandle_t *allocationHandle;
};

/* Plain is a distinct 0xE8-byte parameter and 0x28-byte particle variant. */
struct PcpScatterPlainParams {
    f32 origin[4];
    u32 packetQueueIndex;
    u8 loop;
    u8 pad15[3];
    s32 duration;
    u32 particleCount;
    u32 segmentsPerParticle;
    u32 randomDelayRange;
    s32 fadeIn;
    s32 fadeRange;
    f32 angleStepBase;
    f32 angleStepJitter;
    f32 heightBase;
    f32 heightJitter;
    f32 angularSpeed;
    f32 angularDamping;
    f32 radiusBase;
    f32 radiusJitter;
    f32 radialSpeed;
    f32 radialDamping;
    s32 radialDecayStart;
    s32 baseColor;
    u32 uSpan; /* Horizontal UV extent. */
    u32 vSpan; /* Vertical UV extent. */
    u8 pad68[0x80];
};

/* The flat particle has three rotation angles before its signed age; it is
 * distinct from the other variants' particle even though both records are 0x28. */
struct PcpScatterPlainParticle {
    f32 rot[3];
    s32 age;
    f32 angle;
    f32 angularSpeed;
    f32 angleStep;
    f32 radius;
    f32 radialSpeed;
    f32 height;
};

struct effPCPWwSpherePoly_tag {
    f32 matrix[16];
    PcpScatterPlainParams param;
    PcpScatterPlainParticle *pPart;
    f32 scale;
    u32 abgr;
    dds3WhirlwindPolyList_tag *hPoly;
    sdfMemHandle_t *allocationHandle;
};

/* Appended after the two record arrays; resource helpers own this same control block. */
typedef struct dds3ScatterPolyList_tag {
    f32 origin[4];
    u32 poolMode; /* 0x10: selects the surface node used when drawing the pool */
    u32 color;
    s32 secondWordCount;
    f32 unk1C;
    u8 *recordBase;
    u8 *auxRecordBase;
    sdfMat_t *drawAsset;
    sdfMemHandle_t *allocation;
    dds3ScatterPolyListTexResource_tag *sharedResource;
} dds3ScatterPolyList_tag; /* 0x34 */

struct effPartHandle_t;

/* One randomized channel record. */
struct effPartHandle_t;

/* Compact effect owner: its halfword kind selects the opaque payload's operations. */
struct effPartHandle_t {
    u16 kind;
    u8 pad02[2];
    void *payload;
};

/* Serialized radial scatter parameters copied into each runtime work block. */
struct PcpScatterRadialParams {
    f32 origin[4];
    f32 width;
    f32 height;
    u32 poolMode;
    u8 respawn;
    u8 pad1D[3];
    u32 particleCount;
    u32 radialSegments;
    s32 delaySpread;
    u32 fadeIn;
    u32 fadeOut;
    s32 duration;
    s32 fadeDuration;
    f32 initialHeightStep;
    f32 heightDamping;
    f32 initialAngleStep;
    f32 angleDamping;
    f32 startRadius;
    f32 endRadius;
    f32 radiusJitter;
    f32 targetRadiusJitter;
    f32 speedJitter;
    u8 pulseEnabled;
    u8 pad61[3];
    f32 pulseAngle;
    u8 duplicateParticles;
    u8 pad69[3];
    s32 duplicateStartAge;
    u32 particlesPerGroup;
};

/* Per-particle age and radial/vertical motion state. */
struct PcpScatterRadialParticle {
    s32 age;
    f32 radiusStep;
    f32 angleStep;
    f32 heightStep;
    f32 radius;
    f32 angle;
    f32 height;
};

/* Runtime owner followed inline by its variable-length particle array. */
struct effPCPHamaCardSpiral_tag {
    PcpScatterRadialParams params;
    PcpScatterRadialParticle *particles;
    f32 scale;
    u32 color;
    dds3ScatterPolyList_tag *childWork;
    sdfMemHandle_t *allocation;
    u32 duplicateGroupCount;
    effPartHandle_t **duplicatedHandles;
    sdfMemHandle_t *duplicateAllocation;
};

struct effPCPHamaCardScatterPart_tag {
    s32 frame;
    f32 height;
    f32 height_s;
    f32 radian;
    f32 radian_w;
    f32 radian_s;
    f32 radius;
    f32 roll;
    f32 unk20;
    f32 unk24;
};

/* Serialized parameters copied into each ribbon-scatter work owner. */
struct PcpScatterRibbonParams {
    f32 origin[4];
    f32 width;
    f32 height;
    u32 poolMode;
    u8 respawn;
    u8 pad1D[3];
    u32 particleCount;
    s32 delaySpread;
    u32 fadeIn;
    u32 fadeOut;
    s32 duration;
    s32 fadeDuration;
    f32 heightStep;
    u8 pad3C[4];
    u8 duplicateParticles;
    u8 pad41[3];
    s32 duplicateStartAge;
    u32 particlesPerGroup;
};

/* Runtime owner followed inline by its variable-length particle array. */
struct effPCPHamaCardScatter_tag {
    PcpScatterRibbonParams param;
    effPCPHamaCardScatterPart_tag *pPart;
    f32 scale;
    u32 color;
    dds3ScatterPolyList_tag *hPoly;
    sdfMemHandle_t *allocation;
    u32 duplicateGroupCount;
    effPartHandle_t **pAttach;
    sdfMemHandle_t *duplicateAllocation;
};

/* Serialized parameters copied into each spin-scatter work owner. */
struct PcpScatterSpinParams {
    f32 origin[4];
    f32 width;
    f32 height;
    u32 poolMode;
    u8 respawn;
    u8 pad1D[3];
    u32 particleCount;
    s32 delaySpread;
    u32 fadeIn;
    u32 fadeOut;
    s32 duration;
    s32 fadeDuration;
    f32 angleStep;
    f32 angleDamping;
    f32 startRadius;
    f32 endRadius;
    f32 startRadiusJitter;
    f32 endRadiusJitter;
    u8 adjustAngle;
    u8 pad51[3];
    f32 endAngleStep;
    u8 duplicateParticles;
    u8 pad59[3];
    s32 duplicateStartAge;
    u32 particlesPerGroup;
};

struct effPCPWwSpherePolyPart_tag {
    s32 rot;
    f32 frame;
    f32 radian;
    f32 radian_s;
    f32 radian_add;
    f32 radius;
    f32 radius_s;
    f32 width;
};

/* Runtime owner followed inline by its variable-length particle array. */
struct effPCPHamaCardSphere_tag {
    PcpScatterSpinParams params;
    effPCPWwSpherePolyPart_tag *particles;
    f32 scale;
    u32 abgr;
    dds3ScatterPolyList_tag *childWork;
    sdfMemHandle_t *allocation;
    u32 duplicateGroupCount;
    effPartHandle_t **duplicatedHandles;
    sdfMemHandle_t *duplicateAllocation;
};

struct sdfDrawTag_t;


struct sdfGrOt_t;

/* Draw-surface pool entry (0x20); the SDK initializes and flushes this owner. */
struct sdfGrOt_t {
    struct sdfGrOt_t *next; /* 0x00 */
    sdfDrawTag_t *tagHead;     /* 0x04 */
    sdfDrawTag_t *tagTail;     /* 0x08 */
    u32 noSortTag;
    void (*regFunc)(struct sdfGrOt_t *, sdfDrawTag_t *);          /* 0x10 */
    s32 (*callbackFunc)(struct sdfGrOt_t *, s32, sdfDrawTag_t *); /* 0x14 */
    u32 data;
    u32 pad;
};

struct TMX_HEADER_t;



extern void *effPkt3DPrimMakePoly(effDebugPolyParams_t *aParm);

/* Four ordered 32-bit words in the shared effect random-number state. */
struct EffRandState {
    u32 words[4];
};

extern EffRandState dds3EffectBase[];

void dds3EffectPartDelete(effPartHandle_t *hEffPart);

extern u32 dds3WhirlwindPolyListGetABGR(dds3WhirlwindPolyList_tag *hPoly, s32 index);

/* Returns 24 random bits scaled to [0, 1); NULL selects the shared state. */
f32 dds3GetRandFloatA(EffRandState *name);

/* Constructors copy one serialized parameter variant into its runtime owner. */
effPCPWhirlwindPoly_tag *effPCPWhirlwindPolyCreate(
    PcpScatterParamsA *pParam, u32 pTex);

effPCPWwSpherePoly_tag *effPCPWwSpherePolyCreate(
    PcpScatterPlainParams *pParam, u32 pTex);

extern dds3ScatterPolyListTexResource_tag *dds3ScatterPolyListTexResourceCopy(dds3ScatterPolyListTexResource_tag *hSplTexResSrc);

effPCPWhirlwind2Poly_tag *effPCPWhirlwind2PolyCreate(
    PcpScatterParamsB *pParam, u32 pTex);

effPCPWhirlwind3Poly_tag *effPCPWhirlwind3PolyCreate(
    PcpScatterParamsC *pParam, u32 pTex);

extern effDebugPolyParams_t D_0056AD58;

extern sdfGrOt_t *D_0031F440[];

extern effDebugPolyParams_t D_0056AD28[];

extern sdfGrOt_t *D_0031F3D0[];

extern u32 D_0031F310[];

extern u32 D_0031F2C0[];

extern u8 D_0031F3E0[];

extern void *memset(void *a, s32 b, u32 c);

struct sdfMemHandle_t *sdfMemAllocDirect(s32 aSize);

/* Reserve size bytes from the active packet buffer, rounded up to 16 bytes.
 * Returns the prior cursor as a 32-bit address; the buffer owns this storage. */
s32 sdfGrAllocPacketBuff(s32 aSize);

void *sdfChipAllocDirect(s32 aSize);

void sdfDrawTagLink(struct sdfDrawTag_t *aTag, u32 aPacket);

/* Apply the primary VU0 matrix bank vf28-vf31 to basis vectors vf24-vf27,
 * then replace vf28-vf31 with the four transformed vectors. */
void MultMatrixNoArg(void);

void sdf3DPrimLinkMat(sdfDrawTag_t *aTag, sdfMat_t *aMat,
                              s32 (*aAllocator)(s32));

void sdf3DPrimLinkTm(sdfDrawTag_t *aTag,
                           s32 (*aAllocator)(s32));

struct sdfMat_t *sdfMatNew(void);

f32 sdfCos(f32 aRadius);

void sdfDrawTagInit(struct sdfDrawTag_t *a);

/* Set vf28-vf31 to vf28-vf31 * vf24-vf27 in place. */
void MultMatrixRevNoArg(void);

/* Queue an asset owner for the deferred SdfAsset release callback. */
void sdfMatFreeDelayed(struct sdfMat_t *aMat);

void sdfChipFreeDirect(void *aPtr);

void sdfMemFreeDirect(struct sdfMemHandle_t *aHandle);

u32 sdfMemLock(struct sdfMemHandle_t *aHandle);

/* Store the primary or secondary texture owner and mark its draw state dirty. */
void sdfMatSetTex(struct sdfMat_t *aMat, struct sdfTexHandle_t *aTex);

void sdfMatSetConstant(struct sdfMat_t *aMat, f32 aRatio);

f32 sdfSin(f32 aRadius);

/* Acquire a texture from a serialized texture-file header. */
struct sdfTexHandle_t *sdfTexSet(const TMX_HEADER_t *aTex);

void sdfTexFreeDelayed(sdfTexHandle_t *aHandle);

/* Load the X-axis rotation matrix into the primary VU0 matrix registers. */
void SetRotateMatrixX(f32 aAngle);

/* Load the secondary Y-axis rotation matrix into the primary VU0 matrix registers. */
void SetRotateMatrixY_B(f32 aAngle);

extern void SetRotateMatrixXYZ(f32 *aAngles);


/* Dependencies for the restored DDS effect implementations. */
extern void *dds3EffectPCPDataGetData(void *table, s32 index);
extern effPartHandle_t *dds3EffectPartCreate(s32 type, void *pData);
extern effPartHandle_t *dds3EffectPartCopy(effPartHandle_t *hEffPartSrc);
extern void dds3EffectPartDraw(effPartHandle_t *hEffPart);
extern void dds3EffectPartSetPosition(effPartHandle_t *hEffPart, f32 *pPos);
extern void dds3EffectPartSetColor(effPartHandle_t *hEffPart, u32 abgr);
extern u32 dds3GetRandA(EffRandState *name);
extern u32 effColorBlend(u32 abgr0, u32 abgr1, f32 rate);
extern s32 effMagaCylinderCalcColor(s32 color, s32 tint);
extern f32 sdfAtan(f32 aX);
extern void SetRotateMatrixAxis(f32 aAngle, const void *aAxis);
extern void *memcpy(void *a, const void *b, u32 c);
effPCPHamaCardSpiral_tag *effPCPHamaCardSpiralCreate(const PcpScatterRadialParams *pParam, u32 pTex, void *pData);
effPCPHamaCardSphere_tag *effPCPHamaCardSphereCreate(const PcpScatterSpinParams *pParam, u32 pTex, void *pData);
effPCPHamaCardScatter_tag *effPCPHamaCardScatterCreate(const PcpScatterRibbonParams *pParam, u32 pTex, void *pData);
dds3WhirlwindPolyList_tag *dds3WhirlwindPolyListCreate(u32 number, u32 face);
void dds3WhirlwindPolyListSetRotateMatrix(dds3WhirlwindPolyList_tag *hPoly, const void *pMatrix);
dds3ScatterPolyList_tag *dds3ScatterPolyListCreate(s32 face);
void dds3ScatterPolyListDelete(dds3ScatterPolyList_tag *hPoly);
void dds3ScatterPolyListDraw(dds3ScatterPolyList_tag *hPoly);
void dds3ScatterPolyListSetTex(dds3ScatterPolyList_tag *hPoly, u32 pTex);
void dds3ScatterPolyListCopyTex(dds3ScatterPolyList_tag *hPoly, dds3ScatterPolyList_tag *hPolySrc);
u128 *dds3ScatterPolyListGetVertex(dds3ScatterPolyList_tag *hPoly, s32 index);
u32 *dds3ScatterPolyListGetVC(dds3ScatterPolyList_tag *hPoly, s32 index);
dds3ScatterPolyListTexResource_tag *dds3ScatterPolyListTexResourceCreate(u32 pTex);
void dds3ScatterPolyListTexResourceDelete(dds3ScatterPolyListTexResource_tag *hSplTexRes);
void dds3WhirlwindPolyListDelete(dds3WhirlwindPolyList_tag *hPoly);
void dds3WhirlwindPolyListDraw(dds3WhirlwindPolyList_tag *object);
void dds3WhirlwindPolyListSetTex(dds3WhirlwindPolyList_tag *hPoly, u32 pTex);
void dds3WhirlwindPolyListCopyTex(dds3WhirlwindPolyList_tag *hPoly, dds3WhirlwindPolyList_tag *hPolySrc);
f32 *dds3WhirlwindPolyListGetVertex(dds3WhirlwindPolyList_tag *hPoly, s32 index);
f32 *dds3WhirlwindPolyListGetUV(dds3WhirlwindPolyList_tag *hPoly, s32 index);
#define EFF_SCATTER_PARAM_BLOCK 0
#define EFF_SCATTER_RESOURCE_BLOCK 1
#define EFF_SCATTER_CHILD_BLOCK 2
#define EFF_SCATTER_RGB_MASK 0xFFFFFF
#define EFF_SCATTER_HALF_TURN 3.14159265f
#define EFF_SCATTER_HALF_SEGMENT 0.5f
#define EFF_PARAM_WORK_KIND_EXTENDED_WORK_WITH_MATRIX_CALLBACK 6

#define VEC3_SPLAT(v, x) ((v)[0] = (x), (v)[1] = (x), (v)[2] = (x))

effPCPHamaCardSpiral_tag *effPCPHamaCardSpiralCreate(params, resource, particleParams)
    const PcpScatterRadialParams *params;
    u32 resource;
    void *particleParams;
{
    effPCPHamaCardSpiral_tag *work;
    PcpScatterRadialParticle *particle;
    sdfMemHandle_t *handle;
    effPartHandle_t **handles;
    u32 count;
    u32 i;
    s32 delaySpread;
    s32 ageOffset;
    u32 segments;

    handle = sdfMemAllocDirect(sizeof(effPCPHamaCardSpiral_tag) + params->particleCount * sizeof(PcpScatterRadialParticle));
    work = (effPCPHamaCardSpiral_tag *)sdfMemLock(handle);
    work->particles = (PcpScatterRadialParticle *)(work + 1);
    work->params = *params;
    work->color = EFF_SCATTER_NEUTRAL_COLOR;
    work->allocation = handle;
    work->scale = 1.0f;
    work->duplicatedHandles = NULL;
    work->duplicateAllocation = NULL;
    work->childWork = dds3ScatterPolyListCreate(params->particleCount);
    work->childWork->poolMode = params->poolMode;
    if (resource != 0) {
        dds3ScatterPolyListSetTex(work->childWork, resource);
    }
    if (particleParams != NULL && work->params.duplicateParticles != 0) {
        if (work->params.particlesPerGroup == 0) {
            work->params.particlesPerGroup = 1;
        }
        work->duplicateGroupCount = work->params.particleCount / work->params.particlesPerGroup;
        if (work->params.particleCount % work->params.particlesPerGroup != 0) {
            work->duplicateGroupCount++;
        }
        count = work->duplicateGroupCount;
        handle = sdfMemAllocDirect(count * sizeof(u32));
        handles = (effPartHandle_t **)sdfMemLock(handle);
        work->duplicateAllocation = handle;
        work->duplicatedHandles = handles;
        work->duplicatedHandles[0] = dds3EffectPartCreate(EFF_PARAM_WORK_KIND_EXTENDED_WORK_WITH_MATRIX_CALLBACK, particleParams);
        for (i = 1; i < count; i++) {
            work->duplicatedHandles[i] = dds3EffectPartCopy(work->duplicatedHandles[0]);
        }
    }
    delaySpread = work->params.delaySpread;
    ageOffset = 0;
    count = work->params.particleCount;
    particle = work->particles;
    if (delaySpread <= 0) {
        delaySpread = 1;
    }
    segments = work->params.radialSegments;
    for (i = 0; i < count; i++, particle++) {
        particle->age = ageOffset - dds3GetRandA((EffRandState *)dds3EffectBase) % delaySpread;
        if ((i + 1) % segments == 0) {
            ageOffset -= delaySpread;
        }
    }
    return work;
}

void effPCPHamaCardSpiralCreatePCF(void *parameterTable)
{
    effPCPHamaCardSpiralCreate(
        dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK),
        (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK),
        dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_CHILD_BLOCK));
}

effPCPHamaCardSpiral_tag *effPCPHamaCardSpiralCopy(source)
    effPCPHamaCardSpiral_tag *source;
{
    effPCPHamaCardSpiral_tag *work;
    u32 count;
    sdfMemHandle_t *allocation;
    effPartHandle_t **handles;
    u32 i;

    work = effPCPHamaCardSpiralCreate(&source->params, 0, 0);
    dds3ScatterPolyListCopyTex(work->childWork, source->childWork);
    if (work->params.duplicateParticles != 0) {
        work->duplicateGroupCount = work->params.particleCount / work->params.particlesPerGroup;
        if (work->params.particleCount % work->params.particlesPerGroup != 0) {
            work->duplicateGroupCount = work->duplicateGroupCount + 1;
        }
        count = work->duplicateGroupCount;
        allocation = sdfMemAllocDirect(count * EFF_SCATTER_WORD_BYTES);
        handles = (effPartHandle_t **)sdfMemLock(allocation);
        work->duplicateAllocation = allocation;
        work->duplicatedHandles = handles;
        for (i = 0; i < count; i++) {
            work->duplicatedHandles[i] = dds3EffectPartCopy(*source->duplicatedHandles);
        }
    }
    return work;
}

void effPCPHamaCardSpiralDelete(effPCPHamaCardSpiral_tag *hData)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (hData->duplicateAllocation != 0) {
        count = hData->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(hData->duplicatedHandles[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(hData->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(hData->childWork);
    sdfMemFreeDirect(hData->allocation);
}

void effPCPHamaCardSpiralPartInit(effPCPHamaCardSpiral_tag *pHCardSpiral, u32 number) {
    u32 segments = pHCardSpiral->params.radialSegments;
    PcpScatterRadialParticle *particle = &pHCardSpiral->particles[number];
    f32 angleStep;
    f32 jitter;

    if (segments == 0) {
        segments = 1;
    }
    angleStep = EFF_SCATTER_FULL_TURN / segments;
    particle->angle = angleStep * (number % segments) + angleStep * EFF_SCATTER_HALF_SEGMENT * dds3GetRandFloatA(dds3EffectBase);
    jitter = pHCardSpiral->params.radiusJitter;
    particle->radius = pHCardSpiral->params.startRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    particle->height = 0.0f;
    jitter = pHCardSpiral->params.targetRadiusJitter;
    particle->radiusStep = (pHCardSpiral->params.endRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) - particle->radius) / pHCardSpiral->params.duration;
    jitter = pHCardSpiral->params.speedJitter;
    particle->heightStep = pHCardSpiral->params.initialHeightStep * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    particle->age = 0;
    particle->angleStep = pHCardSpiral->params.initialAngleStep;
}

void effPCPHamaCardSpiralDraw(effPCPHamaCardSpiral_tag *hData) {
    f32 next[4];
    f32 position[4];
    f32 duplicatePosition[4];
    f32 direction[4];
    f32 radial[4];
    f32 halfWidth[4];
    f32 halfHeight[4];
    f32 side[4];
    s32 respawn;
    s32 pulse;
    s32 duplicates;
    u32 i;
    u32 count;
    u32 fadeIn;
    u32 fadeOut;
    u32 color;
    u32 baseColor;
    u32 perGroup;
    s32 duration;
    s32 endAge;
    s32 duplicateStart;
    PcpScatterRadialParticle *particle;
    f32 pulseAngle;
    f32 heightDamping;
    f32 angleDamping;

    i = 0;
    duration = hData->params.duration;
    endAge = hData->params.fadeDuration + duration;
    duplicates = hData->duplicatedHandles != NULL;
    count = hData->params.particleCount;
    VEC3_SPLAT(halfWidth, hData->params.width * 0.5f);
    VEC3_SPLAT(halfHeight, hData->params.height * 0.5f);
    particle = hData->particles;
    respawn = hData->params.respawn;
    pulse = hData->params.pulseEnabled;
    pulseAngle = hData->params.pulseAngle;
    fadeIn = hData->params.fadeIn;
    fadeOut = hData->params.fadeOut;
    heightDamping = hData->params.heightDamping;
    angleDamping = hData->params.angleDamping;
    duplicateStart = hData->params.duplicateStartAge;
    perGroup = hData->params.particlesPerGroup;
    baseColor = hData->color;

    for (; i < count; i++, particle++) {
        s32 age = particle->age;
        u128 *vertices = dds3ScatterPolyListGetVertex(hData->childWork, i);
        u32 *colors = dds3ScatterPolyListGetVC(hData->childWork, i);
        if (age == 0) {
            effPCPHamaCardSpiralPartInit(hData, i);
        } else if (age > 0) {
            f32 factor = 0.0f;
            s32 moving;
            f32 angle;
            f32 radius;
            f32 height;
            f32 angleStep;
            f32 heightStep;
            u32 j;

            if (age <= endAge) {
                if (age < fadeIn && fadeIn != 0) {
                    factor = (f32)age / fadeIn;
                } else if (endAge - age <= fadeOut && fadeOut != 0) {
                    factor = (f32)(endAge - age) / fadeOut;
                } else {
                    factor = 1.0f;
                }
            }
            moving = age < duration;
            color = effColorBlend(baseColor & 0xFFFFFF, baseColor, factor);
            angle = particle->angle;
            radius = particle->radius;
            height = particle->height;
            angleStep = particle->angleStep;
            heightStep = particle->heightStep;
            if (!moving && pulse) {
                if (age == duration) {
                    angle += pulseAngle;
                    height += heightStep * pulseAngle / angleStep;
                } else if (age == duration + 1) {
                    angle -= pulseAngle;
                    height -= heightStep * pulseAngle / angleStep;
                }
            }
            position[0] = sdfCos(angle) * radius;
            position[1] = height;
            position[2] = sdfSin(angle) * radius;
            SDF_VU0_LOADVECTOR_C(position);
            for (j = 0; j < 3; j++, vertices++, colors++) {
                height += heightStep;
                colors[0] = color;
                colors[3] = color;
                angle += sdfAtan(halfWidth[0] / radius);
                next[0] = sdfCos(angle) * radius;
                next[1] = height;
                next[2] = sdfSin(angle) * radius;
                SDF_VU0_LOADVECTOR(next);
                SDF_VU0_LOADVECTOR_B(position);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_NORMALIZE();
                SDF_VU0_STOREVECTOR_NOMEM(direction);
                SDF_VU0_LOADVECTOR_B(halfWidth);
                SDF_VU0_MULVECTOR();
                SDF_VU0_ADDVECTOR_A_C();
                SDF_VU0_COPYVECTOR_A_TO_C();
                SDF_VU0_STOREVECTOR_NOMEM(position);
                SDF_VU0_STOREVECTOR_NOMEM(radial);
                radial[1] = 0.0f;
                SDF_VU0_LOADVECTOR(direction);
                SDF_VU0_LOADVECTOR_B(radial);
                SDF_VU0_OUTERPRODUCT();
                SDF_VU0_NORMALIZE();
                SDF_VU0_LOADVECTOR_B(halfHeight);
                SDF_VU0_MULVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(side);
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_LOADVECTOR_B(side);
                SDF_VU0_ADDVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_SUBVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices + 3);
                if (j == 1) {
                    SDF_VU0_COPYVECTOR(duplicatePosition, position);
                }
                SDF_VU0_COPYVECTOR(position, next);
            }
            if (moving) {
                particle->radius += particle->radiusStep;
                particle->angle += angleStep;
                particle->height += heightStep;
                particle->angleStep = angleStep * angleDamping;
                particle->heightStep = heightStep * heightDamping;
            }
        } else {
            colors[0] = 0;
            colors[1] = 0;
            colors[2] = 0;
            colors[3] = 0;
            colors[4] = 0;
            colors[5] = 0;
        }
        if (respawn && age >= endAge) {
            particle->age = -1;
        }
        if (duplicates && age >= duplicateStart && age >= 0 && i % perGroup == 0) {
            u32 group = i / perGroup;
            dds3EffectPartSetColor(hData->duplicatedHandles[group], baseColor);
            SDF_VU0_LOADVECTOR(hData->params.origin);
            SDF_VU0_LOADVECTOR_B(duplicatePosition);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR_NOMEM(duplicatePosition);
            dds3EffectPartSetPosition(hData->duplicatedHandles[group], duplicatePosition);
            dds3EffectPartDraw(hData->duplicatedHandles[group]);
        }
        particle->age++;
    }
    hData->childWork->origin[0] = hData->params.origin[0];
    hData->childWork->origin[1] = hData->params.origin[1];
    hData->childWork->origin[2] = hData->params.origin[2];
    dds3ScatterPolyListDraw(hData->childWork);
}

void effPCPHamaCardSpiralSetPosition(void *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(hData, pPos);
}

void effPCPHamaCardSpiralSetColor(effPCPHamaCardSpiral_tag *hData, u32 abgr)
{
    hData->color = abgr;
}

void effPCPHamaCardSpiralSetScale(f32 hData, effPCPHamaCardSpiral_tag *scale)
{
    scale->params.initialHeightStep *= hData;
    scale->params.startRadius *= hData;
    scale->params.endRadius *= hData;
}

effPCPHamaCardSphere_tag *effPCPHamaCardSphereCreate(params, resource, particleParams)
    const PcpScatterSpinParams *params;
    u32 resource;
    void *particleParams;
{
    effPCPHamaCardSphere_tag *work;
    effPCPWwSpherePolyPart_tag *particle;
    sdfMemHandle_t *allocation;
    effPartHandle_t **handles;
    u32 count;
    u32 i;
    s32 delaySpread;

    allocation = sdfMemAllocDirect(sizeof(effPCPHamaCardSphere_tag) + params->particleCount * sizeof(effPCPWwSpherePolyPart_tag));
    work = (effPCPHamaCardSphere_tag *)sdfMemLock(allocation);
    work->particles = (effPCPWwSpherePolyPart_tag *)(work + 1);
    work->params = *params;
    work->abgr = EFF_SCATTER_NEUTRAL_COLOR;
    work->allocation = allocation;
    work->scale = 1.0f;
    work->duplicatedHandles = NULL;
    work->duplicateAllocation = NULL;
    work->childWork = dds3ScatterPolyListCreate(params->particleCount);
    work->childWork->poolMode = params->poolMode;
    if (resource != 0) {
        dds3ScatterPolyListSetTex(work->childWork, resource);
    }
    if (particleParams != NULL && work->params.duplicateParticles != 0) {
        if (work->params.particlesPerGroup == 0) {
            work->params.particlesPerGroup = 1;
        }
        work->duplicateGroupCount = work->params.particleCount / work->params.particlesPerGroup;
        if (work->params.particleCount % work->params.particlesPerGroup != 0) {
            work->duplicateGroupCount++;
        }
        count = work->duplicateGroupCount;
        allocation = sdfMemAllocDirect(count * sizeof(u32));
        handles = (effPartHandle_t **)sdfMemLock(allocation);
        work->duplicateAllocation = allocation;
        work->duplicatedHandles = handles;
        /* Native setup seeds group zero even when the computed group count is zero. */
        work->duplicatedHandles[0] = dds3EffectPartCreate(EFF_PARAM_WORK_KIND_EXTENDED_WORK_WITH_MATRIX_CALLBACK, particleParams);
        for (i = 1; i < count; i++) {
            work->duplicatedHandles[i] = dds3EffectPartCopy(work->duplicatedHandles[0]);
        }
    }
    delaySpread = work->params.delaySpread;
    count = work->params.particleCount;
    particle = work->particles;
    if (delaySpread <= 0) {
        delaySpread = 1;
    }
    for (i = 0; i < count; i++, particle++) {
        particle->rot = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delaySpread);
    }
    return work;
}

void effPCPHamaCardSphereCreatePCF(void *parameterTable)
{
    effPCPHamaCardSphereCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK), dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_CHILD_BLOCK));
}

effPCPHamaCardSphere_tag *effPCPHamaCardSphereCopy(source)
    effPCPHamaCardSphere_tag *source;
{
    effPCPHamaCardSphere_tag *work;
    u32 count;
    sdfMemHandle_t *allocation;
    effPartHandle_t **handles;
    u32 i;

    work = effPCPHamaCardSphereCreate(&source->params, 0, 0);
    dds3ScatterPolyListCopyTex(work->childWork, source->childWork);
    if (work->params.duplicateParticles != 0) {
        work->duplicateGroupCount = work->params.particleCount / work->params.particlesPerGroup;
        if (work->params.particleCount % work->params.particlesPerGroup != 0) {
            work->duplicateGroupCount = work->duplicateGroupCount + 1;
        }
        count = work->duplicateGroupCount;
        allocation = sdfMemAllocDirect(count * EFF_SCATTER_WORD_BYTES);
        handles = (effPartHandle_t **)sdfMemLock(allocation);
        work->duplicateAllocation = allocation;
        work->duplicatedHandles = handles;
        for (i = 0; i < count; i++) {
            work->duplicatedHandles[i] = dds3EffectPartCopy(*source->duplicatedHandles);
        }
    }
    return work;
}

void effPCPHamaCardSphereDelete(effPCPHamaCardSphere_tag *hData)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (hData->duplicateAllocation != 0) {
        count = hData->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(hData->duplicatedHandles[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(hData->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(hData->childWork);
    sdfMemFreeDirect(hData->allocation);
}

void effPCPHamaCardSpherePartInit(effPCPHamaCardSphere_tag *pHCardSphere, s32 number)
{
    effPCPWwSpherePolyPart_tag *sprite = &pHCardSphere->particles[number];
    f32 direction[4];
    f32 jitter;

    sprite->radian_add = dds3GetRandFloatA(dds3EffectBase) * (EFF_SCATTER_HALF_TURN * 2.0f);
    jitter = pHCardSphere->params.startRadiusJitter;
    sprite->radian_s = pHCardSphere->params.startRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pHCardSphere->params.endRadiusJitter;
    sprite->frame = (pHCardSphere->params.endRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) - sprite->radian_s) / (f32)pHCardSphere->params.duration;
    sprite->radian = pHCardSphere->params.angleStep;
    direction[0] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    direction[1] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    direction[2] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    SDF_VU0_LOADVECTOR(direction);
    SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(direction);
    sprite->rot = 0;
    sprite->radius = direction[0];
    sprite->radius_s = direction[1];
    sprite->width = direction[2];
}

void effPCPHamaCardSphereDraw(effPCPHamaCardSphere_tag *hData) {
    f32 axis[4];
    f32 next[4];
    f32 position[4];
    f32 duplicatePosition[4];
    f32 direction[4];
    f32 radial[4];
    f32 halfWidth[4];
    f32 halfHeight[4];
    f32 side[4];
    s32 respawn;
    s32 pulse;
    s32 duplicates;
    u32 i;
    u32 count;
    u32 fadeIn;
    u32 fadeOut;
    u32 color;
    u32 baseColor;
    u32 perGroup;
    s32 duration;
    s32 endAge;
    s32 duplicateStart;
    effPCPWwSpherePolyPart_tag *particle;
    f32 pulseAngle;
    f32 angleDamping;

    i = 0;
    duration = hData->params.duration;
    endAge = hData->params.fadeDuration + duration;
    duplicates = hData->duplicatedHandles != NULL;
    count = hData->params.particleCount;
    VEC3_SPLAT(halfWidth, hData->params.width * 0.5f);
    VEC3_SPLAT(halfHeight, hData->params.height * 0.5f);
    particle = hData->particles;
    respawn = hData->params.respawn;
    pulse = hData->params.adjustAngle;
    pulseAngle = hData->params.endAngleStep;
    fadeIn = hData->params.fadeIn;
    fadeOut = hData->params.fadeOut;
    angleDamping = hData->params.angleDamping;
    duplicateStart = hData->params.duplicateStartAge;
    perGroup = hData->params.particlesPerGroup;
    baseColor = hData->abgr;

    for (; i < count; i++, particle++) {
        s32 age = particle->rot;
        u128 *vertices = dds3ScatterPolyListGetVertex(hData->childWork, i);
        u32 *colors = dds3ScatterPolyListGetVC(hData->childWork, i);
        if (age == 0) {
            effPCPHamaCardSpherePartInit(hData, i);
        } else if (age > 0) {
            f32 factor = 0.0f;
            s32 moving;
            f32 angle;
            f32 radius;
            f32 angleStep;
            u32 j;

            if (age <= endAge) {
                if (age < fadeIn && fadeIn != 0) {
                    factor = (f32)age / fadeIn;
                } else if (endAge - age <= fadeOut && fadeOut != 0) {
                    factor = (f32)(endAge - age) / fadeOut;
                } else {
                    factor = 1.0f;
                }
            }
            moving = age < duration;
            color = effColorBlend(baseColor & 0xFFFFFF, baseColor, factor);
            angle = particle->radian_add;
            radius = particle->radian_s;
            angleStep = particle->radian;
            if (!moving && pulse) {
                if (age == duration) {
                    angle += pulseAngle;
                } else if (age == duration + 1) {
                    angle -= pulseAngle;
                }
            }
            axis[0] = particle->radius;
            axis[1] = particle->radius_s;
            axis[2] = particle->width;
            SetRotateMatrixAxis(angle, (const void *)axis);
            position[0] = radius;
            position[1] = 0.0f;
            position[2] = 0.0f;
            SDF_VU0_LOADVECTOR(position);
            SDF_VU0_APPLYMATRIX();
            SDF_VU0_STOREVECTOR_NOMEM(position);
            SDF_VU0_COPYVECTOR_A_TO_C();
            for (j = 0; j < 3; j++, vertices++, colors++) {
                colors[0] = color;
                colors[3] = color;
                angle += sdfAtan(halfWidth[0] / radius);
                SetRotateMatrixAxis(angle, (const void *)axis);
                next[0] = radius;
                next[1] = 0.0f;
                next[2] = 0.0f;
                SDF_VU0_LOADVECTOR(next);
                SDF_VU0_APPLYMATRIX();
                SDF_VU0_STOREVECTOR_NOMEM(next);
                SDF_VU0_LOADVECTOR(next);
                SDF_VU0_LOADVECTOR_B(position);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_NORMALIZE();
                SDF_VU0_STOREVECTOR_NOMEM(direction);
                SDF_VU0_LOADVECTOR_B(halfWidth);
                SDF_VU0_MULVECTOR();
                SDF_VU0_ADDVECTOR_A_C();
                SDF_VU0_COPYVECTOR_A_TO_C();
                SDF_VU0_STOREVECTOR_NOMEM(position);
                SDF_VU0_STOREVECTOR_NOMEM(radial);
                radial[1] = 0.0f;
                SDF_VU0_LOADVECTOR(direction);
                SDF_VU0_LOADVECTOR_B(radial);
                SDF_VU0_OUTERPRODUCT();
                SDF_VU0_NORMALIZE();
                SDF_VU0_LOADVECTOR_B(halfHeight);
                SDF_VU0_MULVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(side);
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_LOADVECTOR_B(side);
                SDF_VU0_ADDVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_SUBVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices + 3);
                if (j == 1) {
                    SDF_VU0_COPYVECTOR(duplicatePosition, position);
                }
                SDF_VU0_COPYVECTOR(position, next);
            }
            if (moving) {
                particle->radian_s += particle->frame;
                particle->radian_add += angleStep;
                particle->radian = angleStep * angleDamping;
            }
        } else {
            colors[0] = 0;
            colors[1] = 0;
            colors[2] = 0;
            colors[3] = 0;
            colors[4] = 0;
            colors[5] = 0;
        }
        if (respawn && age >= endAge) {
            particle->rot = -1;
        }
        if (duplicates && age >= duplicateStart && age >= 0 && i % perGroup == 0) {
            u32 group = i / perGroup;
            dds3EffectPartSetColor(hData->duplicatedHandles[group], baseColor);
            SDF_VU0_LOADVECTOR(hData->params.origin);
            SDF_VU0_LOADVECTOR_B(duplicatePosition);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR_NOMEM(duplicatePosition);
            dds3EffectPartSetPosition(hData->duplicatedHandles[group], duplicatePosition);
            dds3EffectPartDraw(hData->duplicatedHandles[group]);
        }
        particle->rot++;
    }
    hData->childWork->origin[0] = hData->params.origin[0];
    hData->childWork->origin[1] = hData->params.origin[1];
    hData->childWork->origin[2] = hData->params.origin[2];
    dds3ScatterPolyListDraw(hData->childWork);
}

void effPCPHamaCardSphereSetPosition(void *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(hData, pPos);
}

void effPCPHamaCardSphereSetColor(effPCPHamaCardSphere_tag *hData, u32 abgr)
{
    hData->abgr = abgr;
}

void effPCPHamaCardSphereSetScale(f32 hData, effPCPHamaCardSphere_tag *scale)
{
    scale->params.startRadius *= hData;
    scale->params.endRadius *= hData;
}

effPCPHamaCardScatter_tag *effPCPHamaCardScatterCreate(params, resource, particleParams)
    const PcpScatterRibbonParams *params;
    u32 resource;
    void *particleParams;
{
    effPCPHamaCardScatter_tag *work;
    effPCPHamaCardScatterPart_tag *particle;
    sdfMemHandle_t *allocation;
    effPartHandle_t **handles;
    u32 count;
    u32 i;
    s32 delaySpread;

    allocation = sdfMemAllocDirect(sizeof(effPCPHamaCardScatter_tag) + params->particleCount * sizeof(effPCPHamaCardScatterPart_tag));
    work = (effPCPHamaCardScatter_tag *)sdfMemLock(allocation);
    work->pPart = (effPCPHamaCardScatterPart_tag *)(work + 1);
    work->param = *params;
    work->color = EFF_SCATTER_NEUTRAL_COLOR;
    work->allocation = allocation;
    work->scale = 1.0f;
    work->pAttach = NULL;
    work->duplicateAllocation = NULL;
    work->hPoly = dds3ScatterPolyListCreate(params->particleCount);
    work->hPoly->poolMode = params->poolMode;
    if (resource != 0) {
        dds3ScatterPolyListSetTex(work->hPoly, resource);
    }
    if (particleParams != NULL && work->param.duplicateParticles != 0) {
        if (work->param.particlesPerGroup == 0) {
            work->param.particlesPerGroup = 1;
        }
        work->duplicateGroupCount = work->param.particleCount / work->param.particlesPerGroup;
        if (work->param.particleCount % work->param.particlesPerGroup != 0) {
            work->duplicateGroupCount++;
        }
        count = work->duplicateGroupCount;
        allocation = sdfMemAllocDirect(count * sizeof(u32));
        handles = (effPartHandle_t **)sdfMemLock(allocation);
        work->duplicateAllocation = allocation;
        work->pAttach = handles;
        /* Native setup seeds group zero even when the computed group count is zero. */
        work->pAttach[0] = dds3EffectPartCreate(EFF_PARAM_WORK_KIND_EXTENDED_WORK_WITH_MATRIX_CALLBACK, particleParams);
        for (i = 1; i < count; i++) {
            work->pAttach[i] = dds3EffectPartCopy(work->pAttach[0]);
        }
    }
    delaySpread = work->param.delaySpread;
    count = work->param.particleCount;
    particle = work->pPart;
    if (delaySpread <= 0) {
        delaySpread = 1;
    }
    for (i = 0; i < count; i++, particle++) {
        particle->frame = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delaySpread);
    }
    return work;
}

void effPCPHamaCardScatterCreatePCF(void *parameterTable)
{
    effPCPHamaCardScatterCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK), dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_CHILD_BLOCK));
}

effPCPHamaCardScatter_tag *effPCPHamaCardScatterCopy(source)
    effPCPHamaCardScatter_tag *source;
{
    effPCPHamaCardScatter_tag *work;
    u32 count;
    sdfMemHandle_t *allocation;
    effPartHandle_t **handles;
    u32 i;

    work = effPCPHamaCardScatterCreate(&source->param, 0, 0);
    dds3ScatterPolyListCopyTex(work->hPoly, source->hPoly);
    if (work->param.duplicateParticles != 0) {
        if (work->param.particlesPerGroup == 0) {
            work->param.particlesPerGroup = 1;
        }
        work->duplicateGroupCount = work->param.particleCount / work->param.particlesPerGroup;
        if (work->param.particleCount % work->param.particlesPerGroup != 0) {
            work->duplicateGroupCount = work->duplicateGroupCount + 1;
        }
        count = work->duplicateGroupCount;
        allocation = sdfMemAllocDirect(count * EFF_SCATTER_WORD_BYTES);
        handles = (effPartHandle_t **)sdfMemLock(allocation);
        work->duplicateAllocation = allocation;
        work->pAttach = handles;
        for (i = 0; i < count; i++) {
            work->pAttach[i] = dds3EffectPartCopy(*source->pAttach);
        }
    }
    return work;
}

void effPCPHamaCardScatterDelete(effPCPHamaCardScatter_tag *hData)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (hData->duplicateAllocation != 0) {
        count = hData->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(hData->pAttach[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(hData->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(hData->hPoly);
    sdfMemFreeDirect(hData->allocation);
}

void effPCPHamaCardScatterPartInit(effPCPHamaCardScatter_tag *pHCardScatter, s32 number)
{
    f32 rightAngle = EFF_SCATTER_RIGHT_ANGLE_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    f32 diagonalAngle = EFF_SCATTER_DIAGONAL_ANGLE_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    f32 tiltStep = EFF_SCATTER_TILT_STEP_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    effPCPHamaCardScatterPart_tag *particle = &pHCardScatter->pPart[number];
    f32 direction[4];

    particle->height = 0.0f;
    particle->height_s = pHCardScatter->param.heightStep;
    particle->radian_w = rightAngle;
    particle->radian = diagonalAngle;
    particle->radian_s = tiltStep;
    particle->roll = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_FULL_TURN;
    particle->radius = EFF_SCATTER_INITIAL_RIBBON_RADIUS;

    direction[0] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    direction[1] = 0.0f;
    direction[2] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    VU0_NORMALIZE_PACKED_VECTOR(direction);

    particle->unk20 = direction[0];
    particle->frame = 0;
    particle->unk24 = direction[2];
}

/* vu0 routine: each particle sweeps three ribbon quads around its tilt angle,
 * and the tilt reverses once it passes half its stored span. */
void effPCPHamaCardScatterDraw(effPCPHamaCardScatter_tag *hData) {
    f32 next[4];
    f32 position[4];
    f32 duplicatePosition[4];
    f32 direction[4];
    f32 axis[4];
    f32 halfWidth[4];
    f32 halfHeight[4];
    f32 side[4];
    s32 respawn;
    s32 duplicates;
    u32 i;
    u32 count;
    u32 fadeIn;
    u32 fadeOut;
    u32 color;
    u32 baseColor;
    u32 perGroup;
    s32 duration;
    s32 endAge;
    s32 duplicateStart;
    effPCPHamaCardScatterPart_tag *particle;

    i = 0;
    duration = hData->param.duration;
    endAge = hData->param.fadeDuration + duration;
    duplicates = hData->pAttach != NULL;
    count = hData->param.particleCount;
    VEC3_SPLAT(halfWidth, hData->param.width * 0.5f);
    VEC3_SPLAT(halfHeight, hData->param.height * 0.5f);
    particle = hData->pPart;
    respawn = hData->param.respawn;
    fadeIn = hData->param.fadeIn;
    fadeOut = hData->param.fadeOut;
    duplicateStart = hData->param.duplicateStartAge;
    perGroup = hData->param.particlesPerGroup;
    baseColor = hData->color;

    for (; i < count; i++, particle++) {
        s32 age = particle->frame;
        u128 *vertices = dds3ScatterPolyListGetVertex(hData->hPoly, i);
        u32 *colors = dds3ScatterPolyListGetVC(hData->hPoly, i);
        if (age == 0) {
            effPCPHamaCardScatterPartInit(hData, i);
        } else if (age > 0) {
            f32 factor = 0.0f;
            f32 angle;
            f32 radius;
            f32 height;
            f32 heightStep;
            f32 tiltStep;
            u32 j;

            if (age <= endAge) {
                if (age < fadeIn && fadeIn != 0) {
                    factor = (f32)age / fadeIn;
                } else if (endAge - age <= fadeOut && fadeOut != 0) {
                    factor = (f32)(endAge - age) / fadeOut;
                } else {
                    factor = 1.0f;
                }
            }
            color = effColorBlend(baseColor & 0xFFFFFF, baseColor, factor);
            radius = particle->radius;
            height = particle->height;
            heightStep = particle->height_s;
            angle = particle->radian - sdfAtan(halfWidth[0] / radius);
            tiltStep = particle->radian_s;
            axis[0] = 0.0f;
            axis[1] = 0.0f;
            axis[2] = 1.0f;
            position[0] = sdfSin(angle) * radius;
            position[1] = sdfCos(angle) * radius + height;
            position[2] = 0.0f;
            SDF_VU0_LOADVECTOR(position);
            SDF_VU0_COPYVECTOR_A_TO_C();
            for (j = 0; j < 3; j++, vertices++, colors++) {
                colors[0] = color;
                colors[3] = color;
                height += heightStep;
                angle += sdfAtan(halfWidth[0] / radius);
                next[0] = sdfSin(angle) * radius;
                next[1] = sdfCos(angle) * radius + height;
                next[2] = 0.0f;
                SDF_VU0_LOADVECTOR(next);
                SDF_VU0_LOADVECTOR_B(position);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_NORMALIZE();
                SDF_VU0_STOREVECTOR_NOMEM(direction);
                SDF_VU0_LOADVECTOR_B(halfWidth);
                SDF_VU0_MULVECTOR();
                SDF_VU0_ADDVECTOR_A_C();
                SDF_VU0_COPYVECTOR_A_TO_C();
                SDF_VU0_STOREVECTOR_NOMEM(position);
                SDF_VU0_LOADVECTOR(axis);
                SDF_VU0_LOADVECTOR_B(halfHeight);
                SDF_VU0_MULVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(side);
                SDF_VU0_LOADVECTOR(position);
                SDF_VU0_LOADVECTOR_B(side);
                SDF_VU0_ADDVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices);
                SDF_VU0_SUBVECTOR();
                SDF_VU0_SUBVECTOR();
                SDF_VU0_STOREVECTOR_NOMEM(vertices + 3);
                if (j == 1) {
                    SDF_VU0_COPYVECTOR(duplicatePosition, position);
                }
                SDF_VU0_COPYVECTOR(position, next);
            }
            if (age < duration) {
                f32 limit = particle->radian_w * 0.5f;
                f32 tilt = particle->radian + tiltStep;
                f32 magnitude;

                particle->radian = tilt;
                particle->height = particle->height + heightStep;
                magnitude = sdfFabs(tilt);
                if (limit < magnitude) {
                    particle->radian = tilt - tiltStep;
                    particle->radian_s = -particle->radian_s;
                }
            }
        } else {
            colors[0] = 0;
            colors[1] = 0;
            colors[2] = 0;
            colors[3] = 0;
            colors[4] = 0;
            colors[5] = 0;
        }
        if (respawn && age >= endAge) {
            particle->frame = -1;
        }
        if (duplicates && age >= duplicateStart && age >= 0 && i % perGroup == 0) {
            u32 group = i / perGroup;
            dds3EffectPartSetColor(hData->pAttach[group], baseColor);
            SDF_VU0_LOADVECTOR(hData->param.origin);
            SDF_VU0_LOADVECTOR_B(duplicatePosition);
            SDF_VU0_ADDVECTOR();
            SDF_VU0_STOREVECTOR_NOMEM(duplicatePosition);
            dds3EffectPartSetPosition(hData->pAttach[group], duplicatePosition);
            dds3EffectPartDraw(hData->pAttach[group]);
        }
        particle->frame++;
    }
    hData->hPoly->origin[0] = hData->param.origin[0];
    hData->hPoly->origin[1] = hData->param.origin[1];
    hData->hPoly->origin[2] = hData->param.origin[2];
    dds3ScatterPolyListDraw(hData->hPoly);
}

void effPCPHamaCardScatterSetPosition(void *pHCardScatter, void *pPos) {
    SDF_VU0_COPYVECTOR(pHCardScatter, pPos);
}

void effPCPHamaCardScatterSetColor(effPCPHamaCardScatter_tag *pHCardScatter, u32 abgr)
{
    pHCardScatter->color = abgr;
}

void effPCPHamaCardScatterSetScale(void) {
}

dds3ScatterPolyList_tag *dds3ScatterPolyListCreate(s32 face) {
    dds3ScatterPolyList_tag *pool;
    sdfMemHandle_t *allocation;
    u32 *recordBlock;
    s32 slotCount;
    s32 recordWords;
    s32 auxWords;
    u32 allocationBytes;

    slotCount = face * EFF_SCATTER_POOL_SLOTS_PER_GROUP;
    recordWords = slotCount * EFF_SCATTER_POOL_RECORD_WORDS_PER_SLOT;
    auxWords = slotCount * EFF_SCATTER_POOL_AUX_WORDS_PER_SLOT;
    allocationBytes = (recordWords + auxWords) * EFF_SCATTER_WORD_BYTES + EFF_SCATTER_POOL_CONTROL_BYTES;
    allocation = sdfMemAllocDirect(allocationBytes);
    recordBlock = (u32 *)sdfMemLock(allocation);
    memset(recordBlock, 0, allocationBytes);
    pool = (dds3ScatterPolyList_tag *)(recordBlock + (recordWords + auxWords));
    pool->recordBase = (u8 *)recordBlock;
    pool->poolMode = 1;
    pool->auxRecordBase = (u8 *)(recordBlock + recordWords);
    pool->secondWordCount = auxWords;
    pool->allocation = allocation;
    pool->unk1C = 1.0f;
    pool->color = EFF_SCATTER_NEUTRAL_COLOR;
    pool->sharedResource = 0;
    pool->drawAsset = sdfMatNew();
    sdfMatSetConstant(pool->drawAsset, 1.0f);
    memset(D_0056AD28, 0, EFF_SCATTER_DRAW_TEMPLATE_BYTES);
    D_0056AD28->primitive = 0x4000;
    return pool;
}

void dds3ScatterPolyListDelete(dds3ScatterPolyList_tag *hPoly)
{
    if (hPoly->sharedResource != NULL) {
        dds3ScatterPolyListTexResourceDelete(hPoly->sharedResource);
    }
    sdfMatFreeDelayed(hPoly->drawAsset);
    sdfMemFreeDirect(hPoly->allocation);
}

void dds3ScatterPolyListDraw(dds3ScatterPolyList_tag *hPoly) {
    f32 matrix[16];
    sdfDrawTag_t *packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(0x20);
    s32 remainingVertices;
    sdfGrOt_t *surface;

    sdfDrawTagInit(packet);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[12] = hPoly->origin[0];
    matrix[13] = hPoly->origin[1];
    matrix[14] = hPoly->origin[2];
    SDF_VU0_LOADMATRIX(matrix);
    sdf3DPrimLinkTm(packet, 0);
    if (hPoly->sharedResource != NULL) {
        sdfMatSetTex(hPoly->drawAsset, hPoly->sharedResource->textureHandle);
        D_0056AD28->texcoords = (f32 *)D_0031F310;
    } else {
        D_0056AD28->texcoords = NULL;
    }
    sdf3DPrimLinkMat(packet, hPoly->drawAsset, 0);
    remainingVertices = hPoly->secondWordCount;
    D_0056AD28->colors = (u32 *)hPoly->auxRecordBase;
    D_0056AD28->positions = (u128 *)hPoly->recordBase;
    D_0056AD28->color = hPoly->color;
    D_0056AD28->parameterCount = 0x10;
    D_0056AD28->vertexCount = 24;
    D_0056AD28->parameters = D_0031F2C0;
    while (remainingVertices >= 24) {
        remainingVertices -= 24;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056AD28));
        D_0056AD28->positions += 24;
        D_0056AD28->colors += 24;
    }
    if (remainingVertices >= 6) {
        D_0056AD28->parameterCount = remainingVertices / 6 * 4;
        D_0056AD28->vertexCount = remainingVertices;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056AD28));
    }
    surface = D_0031F3D0[hPoly->poolMode];
    surface->regFunc(surface, packet);
}

void dds3ScatterPolyListSetTex(dds3ScatterPolyList_tag *hPoly, u32 pTex)
{
    dds3ScatterPolyListTexResource_tag *res;

    res = dds3ScatterPolyListTexResourceCreate(pTex);
    hPoly->sharedResource = res;
}

void dds3ScatterPolyListCopyTex(dds3ScatterPolyList_tag *hPoly, dds3ScatterPolyList_tag *hPolySrc)
{
    dds3ScatterPolyListTexResource_tag *res;

    res = dds3ScatterPolyListTexResourceCopy(hPolySrc->sharedResource);
    hPoly->sharedResource = res;
}

u128 *dds3ScatterPolyListGetVertex(dds3ScatterPolyList_tag *hPoly, s32 index)
{
    return (u128 *)(hPoly->recordBase + index * EFF_SCATTER_RECORD_BYTES);
}

u32 *dds3ScatterPolyListGetVC(dds3ScatterPolyList_tag *hPoly, s32 index)
{
    return (u32 *)(hPoly->auxRecordBase + index * EFF_SCATTER_AUX_RECORD_BYTES);
}

dds3ScatterPolyListTexResource_tag *dds3ScatterPolyListTexResourceCreate(u32 pTex)
{
    dds3ScatterPolyListTexResource_tag *res;

    res = sdfChipAllocDirect(sizeof(dds3ScatterPolyListTexResource_tag));
    res->textureHandle = sdfTexSet((TMX_HEADER_t *)(pTex));
    res->refCount = 1;
    return res;
}

void dds3ScatterPolyListTexResourceDelete(dds3ScatterPolyListTexResource_tag *hSplTexRes)
{
    if (--hSplTexRes->refCount == 0) {
        sdfTexFreeDelayed(hSplTexRes->textureHandle);
        sdfChipFreeDirect(hSplTexRes);
    }
}

dds3ScatterPolyListTexResource_tag *dds3ScatterPolyListTexResourceCopy(dds3ScatterPolyListTexResource_tag *hSplTexResSrc)
{
    hSplTexResSrc->refCount++;
    return hSplTexResSrc;
}

effPCPWhirlwindPoly_tag *effPCPWhirlwindPolyCreate(src, resource)
    PcpScatterParamsA *src;
    u32 resource;
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(src->particleCount * sizeof(PcpScatterParticle) + sizeof(effPCPWhirlwindPoly_tag));
    effPCPWhirlwindPoly_tag *inst = (effPCPWhirlwindPoly_tag *)sdfMemLock(allocation);
    PcpScatterParticle *particle;
    u32 delayModulus;
    u32 count;
    u32 i;
    dds3WhirlwindPolyList_tag *object;
    u32 drawWord;

    particle = (PcpScatterParticle *)(inst + 1);
    /* Preserve the complete serialized parameter block, including padding. */
    memcpy(&inst->param, src, sizeof(inst->param));
    inst->color = EFF_SCATTER_NEUTRAL_COLOR;
    inst->scale = 1.0f;
    inst->allocationHandle = allocation;
    inst->pPart = particle;
    VU0_COPY_MATRIX(inst->matrix, src->matrix);
    object = dds3WhirlwindPolyListCreate(src->particleCount, src->segmentsPerParticle);
    drawWord = src->packetQueueIndex;
    inst->hPoly = object;
    object->packetQueueIndex = drawWord;
    if (resource != 0) {
        dds3WhirlwindPolyListSetTex(object, resource);
    }
    delayModulus = inst->param.randomDelayRange;
    count = inst->param.particleCount;
    if ((s32)delayModulus <= 0) {
        delayModulus = 1;
    }
    for (i = 0; i < count; i++) {
        particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
        particle++;
    }
    return inst;
}

void effPCPWhirlwindPolyCreatePCF(void *parameterTable)
{
    effPCPWhirlwindPolyCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK));
}

effPCPWhirlwindPoly_tag *effPCPWhirlwindPolyCopy(effPCPWhirlwindPoly_tag *work) {
    effPCPWhirlwindPoly_tag *child;

    child = effPCPWhirlwindPolyCreate(&work->param, NULL);
    dds3WhirlwindPolyListCopyTex(child->hPoly, work->hPoly);
    return child;
}

void effPCPWhirlwindPolyDelete(effPCPWhirlwindPoly_tag *hData)
{
    dds3WhirlwindPolyListDelete(hData->hPoly);
    sdfMemFreeDirect(hData->allocationHandle);
}

void effPCPWhirlwindPolyPartInit(effPCPWhirlwindPoly_tag *pWhirlwindPoly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwindPoly->hPoly, index);
    f32 *uv = dds3WhirlwindPolyListGetUV(pWhirlwindPoly->hPoly, index);
    PcpScatterParticle *ring;
    f32 angle;
    f32 angleStep;
    f32 radius;
    f32 height;
    f32 rise;
    f32 riseStep;
    f32 u;
    f32 du;
    f32 v;
    f32 jitter;
    f32 s;
    u32 count;
    u32 i;

    ring = &pWhirlwindPoly->pPart[index];
    count = pWhirlwindPoly->hPoly->vectorsPerParticle >> 1;
    angle = dds3GetRandFloatA(dds3EffectBase) * (EFF_SCATTER_HALF_TURN * 2.0f);
    jitter = pWhirlwindPoly->param.angleStepJitter;
    angleStep = pWhirlwindPoly->param.angleStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) / (f32)count;
    riseStep = pWhirlwindPoly->param.riseStep;
    jitter = pWhirlwindPoly->param.radiusJitter;
    radius = pWhirlwindPoly->param.radiusBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwindPoly->param.heightOffsetJitter;
    height = pWhirlwindPoly->param.heightOffsetBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwindPoly->param.targetRadiusJitter;
    rise = 0.0f;
    ring->radiusStep = (pWhirlwindPoly->param.targetRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) - radius) / (f32)pWhirlwindPoly->param.duration;
    ring->orientationAngle = pWhirlwindPoly->param.tiltScale * dds3GetRandFloatA(dds3EffectBase);
    ring->tiltAngle = rise;
    ring->angle = angle;
    ring->radius = radius;
    ring->heightOffset = height;
    ring->angleStep = angleStep;
    ring->tiltSpeed = pWhirlwindPoly->param.initialTiltSpeed;
    ring->rise = pWhirlwindPoly->param.initialRise;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    v = (f32)pWhirlwindPoly->param.vSpan;
    u = 0.0f;
    du = (f32)pWhirlwindPoly->param.uSpan / (f32)count;
    for (i = 0; i < count; i++) {
        vertex[0] = sdfCos(angle) * radius;
        vertex[1] = rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] = vertex[1] - height;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        uv[0] = u;
        uv[2] = u;
        uv[1] = 0;
        uv[3] = v;
        uv += 4;
        rise += riseStep;
        u += du;
        angle += angleStep;
        vertex += 8;
    }
}

void effPCPWhirlwindPolyPartMove(effPCPWhirlwindPoly_tag *pWhirlwindPoly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwindPoly->hPoly, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(pWhirlwindPoly->hPoly, index);
    ring = &pWhirlwindPoly->pPart[index];
    radius = ring->radius + ring->radiusStep;
    count = pWhirlwindPoly->hPoly->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= pWhirlwindPoly->param.tiltDamping;
    ring->radius = radius;
    ring->rise = rise * pWhirlwindPoly->param.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

void effPCPWhirlwindPolyDraw(effPCPWhirlwindPoly_tag *hData) {
    s32 loop;
    u32 i;
    u32 count = hData->param.particleCount;
    dds3WhirlwindPolyList_tag *draw = hData->hPoly;
    PcpScatterParticle *particle = hData->pPart;
    s32 duration = hData->param.duration;
    s32 fadeIn;
    s32 fadeRange;
    s32 color;
    loop = hData->param.loop;
    fadeIn = hData->param.fadeIn;
    fadeRange = hData->param.fadeRange;
    color = effMagaCylinderCalcColor(hData->color, hData->param.baseColor);

    for (i = 0; i < count; i++) {
        s32 age = particle->age;
        if (duration < age) {
            draw->colors[i] = 0;
        } else {
            if (age == 0) {
                effPCPWhirlwindPolyPartInit(hData, i);
            } else if (age > 0) {
                s32 remaining = duration - age;
                f32 factor;
                if (age < fadeIn && fadeIn != 0) {
                    factor = (f32)age / (f32)fadeIn;
                } else if (fadeRange >= remaining && fadeRange != 0) {
                    factor = (f32)remaining / (f32)fadeRange;
                } else {
                    factor = 1.0f;
                }
                draw->colors[i] = effColorBlend(color & EFF_SCATTER_RGB_MASK, color, factor);
                effPCPWhirlwindPolyPartMove(hData, i);
            }
            if (loop != 0 && age >= duration) {
                particle->age = 0;
            } else {
                particle->age++;
            }
        }
        particle++;
    }
    draw->scale = hData->scale;
    SDF_VU0_COPYVECTOR(draw->origin, hData->param.origin);
    dds3WhirlwindPolyListSetRotateMatrix(draw, hData);
    dds3WhirlwindPolyListDraw(draw);
}

void effPCPWhirlwindPolySetPosition(effPCPWhirlwindPoly_tag *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(&hData->param, pPos);
}

void effPCPWhirlwindPolySetScale(effPCPWhirlwindPoly_tag *hData, f32 scale)
{
    hData->scale = scale;
}

void effPCPWhirlwindPolySetColor(effPCPWhirlwindPoly_tag *hData, u32 abgr)
{
    hData->color = abgr;
}

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwindPolySetRotateMatrix(effPCPWhirlwindPoly_tag *hData, void *pMatrix)
{
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_LOADMATRIX_B(hData->param.matrix);
    MultMatrixNoArg();
    SDF_VU0_STOREMATRIX(hData);
}

effPCPWhirlwind2Poly_tag *effPCPWhirlwind2PolyCreate(src, resource)
    PcpScatterParamsB *src;
    u32 resource;
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(src->particleCount * sizeof(PcpScatterParticle) + sizeof(effPCPWhirlwind2Poly_tag));
    effPCPWhirlwind2Poly_tag *inst = (effPCPWhirlwind2Poly_tag *)sdfMemLock(allocation);
    PcpScatterParticle *particle;
    u32 delayModulus;
    u32 count;
    u32 i;
    dds3WhirlwindPolyList_tag *object;
    u32 drawWord;
    s32 delayLimit;

    particle = (PcpScatterParticle *)(inst + 1);
    /* Retain every byte of the serialized parameter block, including padding. */
    memcpy(&inst->param, src, sizeof(inst->param));
    inst->abgr = EFF_SCATTER_NEUTRAL_COLOR;
    inst->scale = 1.0f;
    inst->allocationHandle = allocation;
    inst->pPart = particle;
    inst->age = 0;
    VU0_COPY_MATRIX(inst->matrix, src->matrix);
    object = dds3WhirlwindPolyListCreate(src->particleCount, src->segmentsPerParticle);
    drawWord = src->packetQueueIndex;
    inst->hPoly = object;
    object->packetQueueIndex = drawWord;
    if (resource != 0) {
        dds3WhirlwindPolyListSetTex(object, resource);
    }
    delayLimit = inst->param.randomDelayRange;
    if (delayLimit <= 0) {
        inst->param.randomDelayRange = 1;
        delayLimit = 1;
    }
    delayModulus = delayLimit;
    count = inst->param.particleCount;
    for (i = 0; i < count; i++) {
        particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
        particle++;
    }
    return inst;
}

void effPCPWhirlwind2PolyCreatePCF(void *parameterTable)
{
    effPCPWhirlwind2PolyCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK));
}

effPCPWhirlwind2Poly_tag *effPCPWhirlwind2PolyCopy(effPCPWhirlwind2Poly_tag *work)
{
    effPCPWhirlwind2Poly_tag *child;

    child = effPCPWhirlwind2PolyCreate(&work->param, NULL);
    dds3WhirlwindPolyListCopyTex(child->hPoly, work->hPoly);
    return child;
}

void effPCPWhirlwind2PolyDelete(effPCPWhirlwind2Poly_tag *hData)
{
    dds3WhirlwindPolyListDelete(hData->hPoly);
    sdfMemFreeDirect(hData->allocationHandle);
}

void effPCPWhirlwind2PolyPartInit(effPCPWhirlwind2Poly_tag *pWhirlwind2Poly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwind2Poly->hPoly, index);
    f32 *uv = dds3WhirlwindPolyListGetUV(pWhirlwind2Poly->hPoly, index);
    PcpScatterParticle *ring;
    f32 angle;
    f32 angleStep;
    f32 radius;
    f32 height;
    f32 rise;
    f32 riseStep;
    f32 u;
    f32 du;
    f32 v;
    f32 jitter;
    f32 s;
    u32 count;
    u32 i;

    ring = &pWhirlwind2Poly->pPart[index];
    count = pWhirlwind2Poly->hPoly->vectorsPerParticle >> 1;
    angle = dds3GetRandFloatA(dds3EffectBase) * (EFF_SCATTER_HALF_TURN * 2.0f);
    jitter = pWhirlwind2Poly->param.angleStepJitter;
    angleStep = pWhirlwind2Poly->param.angleStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) / (f32)count;
    riseStep = pWhirlwind2Poly->param.riseStep;
    jitter = pWhirlwind2Poly->param.radiusJitter;
    radius = pWhirlwind2Poly->param.radiusBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwind2Poly->param.heightOffsetJitter;
    height = pWhirlwind2Poly->param.heightOffsetBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwind2Poly->param.radiusStepJitter;
    rise = 0.0f;
    ring->radiusStep = pWhirlwind2Poly->param.radiusStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    ring->orientationAngle = pWhirlwind2Poly->param.tiltScale * dds3GetRandFloatA(dds3EffectBase);
    ring->tiltAngle = rise;
    ring->angle = angle;
    ring->radius = radius;
    ring->heightOffset = height;
    ring->angleStep = angleStep;
    ring->tiltSpeed = pWhirlwind2Poly->param.initialTiltSpeed;
    ring->rise = pWhirlwind2Poly->param.initialRise;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    v = (f32)pWhirlwind2Poly->param.vSpan;
    u = 0.0f;
    du = (f32)pWhirlwind2Poly->param.uSpan / (f32)count;
    for (i = 0; i < count; i++) {
        vertex[0] = sdfCos(angle) * radius;
        vertex[1] = rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] = vertex[1] - height;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        uv[0] = u;
        uv[2] = u;
        uv[1] = 0;
        uv[3] = v;
        uv += 4;
        rise += riseStep;
        u += du;
        angle += angleStep;
        vertex += 8;
    }
}

void effPCPWhirlwind2PolyPartMove(effPCPWhirlwind2Poly_tag *pWhirlwind2Poly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwind2Poly->hPoly, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(pWhirlwind2Poly->hPoly, index);
    ring = &pWhirlwind2Poly->pPart[index];
    radius = ring->radius + ring->radiusStep;
    count = pWhirlwind2Poly->hPoly->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= pWhirlwind2Poly->param.tiltDamping;
    ring->radius = radius;
    ring->radiusStep *= pWhirlwind2Poly->param.radiusDamping;
    ring->rise = rise * pWhirlwind2Poly->param.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

void effPCPWhirlwind2PolyDraw(effPCPWhirlwind2Poly_tag *hData) {
    s32 loop;
    s32 duration = hData->param.duration;
    dds3WhirlwindPolyList_tag *draw = hData->hPoly;
    PcpScatterParticle *particle = hData->pPart;
    u32 count = hData->param.particleCount;
    s32 fadeIn;
    s32 fadeRange;
    u32 delayModulus;
    s32 instanceAge;
    s32 color;
    s32 remaining;
    f32 instanceFade;
    f32 particleFade;
    u32 i;

    loop = hData->param.loop;
    fadeIn = hData->param.fadeIn;
    fadeRange = hData->param.fadeRange;
    delayModulus = hData->param.randomDelayRange;
    color = effMagaCylinderCalcColor(hData->abgr, hData->param.baseColor);
    instanceAge = hData->age;
    if (duration < instanceAge) {
        return;
    }
    remaining = duration - instanceAge;
    if (fadeRange >= remaining && fadeRange != 0) {
        instanceFade = (f32)remaining / (f32)fadeRange;
    } else {
        instanceFade = 1.0f;
    }
    for (i = 0; i < count; i++) {
        s32 particleAge = particle->age;

        if (duration < particleAge) {
            draw->colors[i] = 0;
        } else {
            if (particleAge == 0) {
                effPCPWhirlwind2PolyPartInit(hData, i);
            } else if (particleAge > 0) {
                if (particleAge < fadeIn && fadeIn != 0) {
                    particleFade = (f32)particleAge / (f32)fadeIn;
                } else {
                    particleFade = 1.0f;
                }
                draw->colors[i] = effColorBlend(color & EFF_SCATTER_RGB_MASK, color, particleFade * instanceFade);
                effPCPWhirlwind2PolyPartMove(hData, i);
            }
            if (loop != 0 && !(instanceAge < duration)) {
                particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
            } else {
                particle->age++;
            }
        }
        particle++;
    }
    if (loop != 0 && instanceAge >= duration) {
        hData->age = 0;
    } else {
        hData->age++;
    }
    draw->scale = hData->scale;
    SDF_VU0_COPYVECTOR(draw->origin, hData->param.origin);
    dds3WhirlwindPolyListSetRotateMatrix(draw, hData);
    dds3WhirlwindPolyListDraw(draw);
}

void effPCPWhirlwind2PolySetPosition(effPCPWhirlwind2Poly_tag *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(&hData->param, pPos);
}

void effPCPWhirlwind2PolySetScale(effPCPWhirlwind2Poly_tag *hData, f32 scale)
{
    hData->scale = scale;
}

void effPCPWhirlwind2PolySetColor(effPCPWhirlwind2Poly_tag *hData, u32 abgr)
{
    hData->abgr = abgr;
}

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwind2PolySetRotateMatrix(effPCPWhirlwind2Poly_tag *hData, void *pMatrix)
{
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_LOADMATRIX_B(hData->param.matrix);
    MultMatrixNoArg();
    SDF_VU0_STOREMATRIX(hData);
}

effPCPWhirlwind3Poly_tag *effPCPWhirlwind3PolyCreate(src, resource)
    PcpScatterParamsC *src;
    u32 resource;
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(src->particleCount * sizeof(PcpScatterParticle) + sizeof(effPCPWhirlwind3Poly_tag));
    effPCPWhirlwind3Poly_tag *inst = (effPCPWhirlwind3Poly_tag *)sdfMemLock(allocation);
    PcpScatterParticle *particle;
    u32 delayModulus;
    u32 count;
    u32 i;
    dds3WhirlwindPolyList_tag *object;
    u32 drawWord;
    s32 delayLimit;

    particle = (PcpScatterParticle *)(inst + 1);
    /* Copy the complete serialized parameter block, including padding. */
    memcpy(&inst->param, src, sizeof(inst->param));
    inst->color = EFF_SCATTER_NEUTRAL_COLOR;
    inst->scale = 1.0f;
    inst->allocationHandle = allocation;
    inst->pPart = particle;
    inst->age = 0;
    VU0_COPY_MATRIX(inst->matrix, src->matrix);
    object = dds3WhirlwindPolyListCreate(src->particleCount, src->segmentsPerParticle);
    drawWord = src->packetQueueIndex;
    inst->hPoly = object;
    object->packetQueueIndex = drawWord;
    if (resource != 0) {
        dds3WhirlwindPolyListSetTex(object, resource);
    }
    delayLimit = inst->param.randomDelayRange;
    if (delayLimit <= 0) {
        inst->param.randomDelayRange = 1;
        delayLimit = 1;
    }
    delayModulus = delayLimit;
    count = inst->param.particleCount;
    for (i = 0; i < count; i++) {
        particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
        particle++;
    }
    return inst;
}

void effPCPWhirlwind3PolyCreatePCF(void *parameterTable)
{
    effPCPWhirlwind3PolyCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK));
}

effPCPWhirlwind3Poly_tag *effPCPWhirlwind3PolyCopy(effPCPWhirlwind3Poly_tag *work)
{
    effPCPWhirlwind3Poly_tag *child;

    child = effPCPWhirlwind3PolyCreate(&work->param, NULL);
    dds3WhirlwindPolyListCopyTex(child->hPoly, work->hPoly);
    return child;
}

void effPCPWhirlwind3PolyDelete(effPCPWhirlwind3Poly_tag *hData)
{
    dds3WhirlwindPolyListDelete(hData->hPoly);
    sdfMemFreeDirect(hData->allocationHandle);
}

void effPCPWhirlwind3PolyPartInit(effPCPWhirlwind3Poly_tag *pWhirlwind3Poly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwind3Poly->hPoly, index);
    f32 *uv = dds3WhirlwindPolyListGetUV(pWhirlwind3Poly->hPoly, index);
    PcpScatterParticle *ring;
    f32 angle;
    f32 angleStep;
    f32 radius;
    f32 height;
    f32 rise;
    f32 u;
    f32 du;
    f32 v;
    f32 jitter;
    f32 s;
    u32 count;
    u32 i;

    ring = &pWhirlwind3Poly->pPart[index];
    count = pWhirlwind3Poly->hPoly->vectorsPerParticle >> 1;
    angle = dds3GetRandFloatA(dds3EffectBase) * (EFF_SCATTER_HALF_TURN * 2.0f);
    jitter = pWhirlwind3Poly->param.angleStepJitter;
    angleStep = pWhirlwind3Poly->param.angleStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) / (f32)count;
    jitter = pWhirlwind3Poly->param.radiusJitter;
    radius = pWhirlwind3Poly->param.radiusBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwind3Poly->param.heightOffsetJitter;
    height = pWhirlwind3Poly->param.heightOffsetBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWhirlwind3Poly->param.radiusStepJitter;
    ring->radiusStep = pWhirlwind3Poly->param.radiusStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    ring->orientationAngle = pWhirlwind3Poly->param.tiltScale * dds3GetRandFloatA(dds3EffectBase);
    ring->tiltAngle = 0;
    rise = pWhirlwind3Poly->param.riseRange / (f32)pWhirlwind3Poly->param.particleCount * (f32)index;
    radius = radius + pWhirlwind3Poly->param.radiusRamp * (f32)(pWhirlwind3Poly->param.particleCount - index);
    ring->angle = angle;
    ring->heightOffset = height;
    ring->angleStep = angleStep;
    ring->radius = radius;
    ring->tiltSpeed = pWhirlwind3Poly->param.initialTiltSpeed;
    ring->rise = pWhirlwind3Poly->param.initialRise;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    v = (f32)pWhirlwind3Poly->param.vSpan;
    u = 0.0f;
    du = (f32)pWhirlwind3Poly->param.uSpan / (f32)count;
    for (i = 0; i < count; i++) {
        vertex[0] = sdfCos(angle) * radius;
        vertex[1] = rise;
        s = sdfSin(angle) * radius;
        vertex[4] = vertex[0];
        vertex[6] = vertex[2] = s;
        vertex[5] = vertex[1] - height;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        uv[0] = u;
        uv[2] = u;
        uv[1] = 0;
        uv[3] = v;
        uv += 4;
        angle += angleStep;
        u += du;
        vertex += 8;
    }
}

void effPCPWhirlwind3PolyPartMove(effPCPWhirlwind3Poly_tag *pWhirlwind3Poly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWhirlwind3Poly->hPoly, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(pWhirlwind3Poly->hPoly, index);
    ring = &pWhirlwind3Poly->pPart[index];
    radius = ring->radius + ring->radiusStep;
    count = pWhirlwind3Poly->hPoly->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= pWhirlwind3Poly->param.tiltDamping;
    ring->radius = radius;
    ring->radiusStep *= pWhirlwind3Poly->param.radiusDamping;
    ring->rise = rise * pWhirlwind3Poly->param.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

void effPCPWhirlwind3PolyDraw(effPCPWhirlwind3Poly_tag *hData) {
    s32 loop;
    s32 duration = hData->param.duration;
    dds3WhirlwindPolyList_tag *draw = hData->hPoly;
    PcpScatterParticle *particle = hData->pPart;
    u32 count = hData->param.particleCount;
    s32 fadeIn;
    s32 fadeRange;
    u32 delayModulus;
    s32 instanceAge;
    s32 remaining;
    f32 instanceFade;
    f32 particleFade;
    f32 particleAgeFloat;
    s32 tintColor;
    s32 startColor;
    s32 endColor;
    s32 color;
    u32 i;

    instanceAge = hData->age;
    loop = hData->param.loop;
    fadeIn = hData->param.fadeIn;
    fadeRange = hData->param.fadeRange;
    delayModulus = hData->param.randomDelayRange;
    startColor = hData->param.startColor;
    endColor = hData->param.endColor;
    tintColor = hData->color;
    if (duration < instanceAge) {
        return;
    }
    remaining = duration - instanceAge;
    if (fadeRange >= remaining && fadeRange != 0) {
        instanceFade = (f32)remaining / (f32)fadeRange;
    } else {
        instanceFade = 1.0f;
    }
    for (i = 0; i < count; i++) {
        s32 particleAge = particle->age;

        if (duration < particleAge) {
            draw->colors[i] = 0;
        } else {
            if (particleAge == 0) {
                effPCPWhirlwind3PolyPartInit(hData, i);
            } else if (particleAge > 0) {
                particleAgeFloat = (f32)particleAge;
                color = effColorBlend(startColor, endColor, particleAgeFloat / (f32)duration);
                if (particleAge < fadeIn && fadeIn != 0) {
                    particleFade = particleAgeFloat / (f32)fadeIn;
                } else {
                    particleFade = 1.0f;
                }
                color = effMagaCylinderCalcColor(tintColor, color);
                draw->colors[i] = effColorBlend(color & EFF_SCATTER_RGB_MASK, color, particleFade * instanceFade);
                effPCPWhirlwind3PolyPartMove(hData, i);
            }
            if (loop != 0 && !(instanceAge < duration)) {
                particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
            } else {
                particle->age++;
            }
        }
        particle++;
    }
    if (loop != 0 && instanceAge >= duration) {
        hData->age = 0;
    } else {
        hData->age++;
    }
    draw->scale = hData->scale;
    SDF_VU0_COPYVECTOR(draw->origin, hData->param.origin);
    dds3WhirlwindPolyListSetRotateMatrix(draw, hData);
    dds3WhirlwindPolyListDraw(draw);
}

void effPCPWhirlwind3PolySetPosition(effPCPWhirlwind3Poly_tag *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(&hData->param, pPos);
}

void effPCPWhirlwind3PolySetScale(effPCPWhirlwind3Poly_tag *hData, f32 scale)
{
    hData->scale = scale;
}

void effPCPWhirlwind3PolySetColor(effPCPWhirlwind3Poly_tag *hData, u32 abgr)
{
    hData->color = abgr;
}

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwind3PolySetRotateMatrix(effPCPWhirlwind3Poly_tag *hData, void *pMatrix)
{
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_LOADMATRIX_B(hData->param.matrix);
    MultMatrixNoArg();
    SDF_VU0_STOREMATRIX(hData);
}

effPCPWwSpherePoly_tag *effPCPWwSpherePolyCreate(src, resource)
    PcpScatterPlainParams *src;
    u32 resource;
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(src->particleCount * sizeof(PcpScatterPlainParticle) + sizeof(effPCPWwSpherePoly_tag));
    effPCPWwSpherePoly_tag *inst = (effPCPWwSpherePoly_tag *)sdfMemLock(allocation);
    PcpScatterPlainParticle *particle;
    u32 delayModulus;
    u32 count;
    u32 i;
    dds3WhirlwindPolyList_tag *object;
    u32 drawWord;

    particle = (PcpScatterPlainParticle *)(inst + 1);
    /* Preserve the complete serialized parameter block, including padding. */
    memcpy(&inst->param, src, sizeof(inst->param));
    inst->abgr = EFF_SCATTER_NEUTRAL_COLOR;
    inst->scale = 1.0f;
    inst->allocationHandle = allocation;
    inst->pPart = particle;
    SDF_VU0_STOREIDENTITY(inst->matrix);
    object = dds3WhirlwindPolyListCreate(src->particleCount, src->segmentsPerParticle);
    drawWord = src->packetQueueIndex;
    inst->hPoly = object;
    object->packetQueueIndex = drawWord;
    if (resource != 0) {
        dds3WhirlwindPolyListSetTex(object, resource);
    }
    delayModulus = inst->param.randomDelayRange;
    count = inst->param.particleCount;
    if ((s32)delayModulus <= 0) {
        delayModulus = 1;
    }
    for (i = 0; i < count; i++) {
        particle->age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % delayModulus);
        particle++;
    }
    return inst;
}

void effPCPWwSpherePolyCreatePCF(void *parameterTable)
{
    effPCPWwSpherePolyCreate(dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_PARAM_BLOCK), (u32)dds3EffectPCPDataGetData(parameterTable, EFF_SCATTER_RESOURCE_BLOCK));
}

effPCPWwSpherePoly_tag *effPCPWwSpherePolyCopy(effPCPWwSpherePoly_tag *work)
{
    effPCPWwSpherePoly_tag *child;

    child = effPCPWwSpherePolyCreate(&work->param, NULL);
    dds3WhirlwindPolyListCopyTex(child->hPoly, work->hPoly);
    return child;
}

void effPCPWwSpherePolyDelete(effPCPWwSpherePoly_tag *hData)
{
    dds3WhirlwindPolyListDelete(hData->hPoly);
    sdfMemFreeDirect(hData->allocationHandle);
}

void effPCPWwSpherePolyPartInit(effPCPWwSpherePoly_tag *pWwSpherePoly, s32 index) {
    f32 *uv = dds3WhirlwindPolyListGetUV(pWwSpherePoly->hPoly, index);
    PcpScatterPlainParticle *ring;
    f32 angle;
    f32 angleStep;
    f32 radius;
    f32 height;
    f32 u;
    f32 du;
    f32 v;
    f32 jitter;
    u32 count;
    u32 i;

    ring = &pWwSpherePoly->pPart[index];
    count = pWwSpherePoly->hPoly->vectorsPerParticle >> 1;
    angle = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_FULL_TURN;
    jitter = pWwSpherePoly->param.angleStepJitter;
    angleStep = pWwSpherePoly->param.angleStepBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) / (f32)count;
    jitter = pWwSpherePoly->param.radiusJitter;
    radius = pWwSpherePoly->param.radiusBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = pWwSpherePoly->param.heightJitter;
    height = pWwSpherePoly->param.heightBase * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    /* Euler samples use the retail constant's lower-rounded full turn. */
    ring->rot[0] = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_FULL_TURN;
    ring->rot[1] = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_FULL_TURN;
    ring->rot[2] = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_FULL_TURN;
    ring->angle = angle;
    ring->radius = radius;
    ring->height = height;
    ring->angleStep = angleStep;
    ring->angularSpeed = pWwSpherePoly->param.angularSpeed;
    ring->radialSpeed = pWwSpherePoly->param.radialSpeed;
    v = (f32)pWwSpherePoly->param.vSpan;
    u = 0.0f;
    du = (f32)pWwSpherePoly->param.uSpan / (f32)count;
    for (i = 0; i < count; i++) {
        uv[0] = u;
        uv[2] = u;
        uv[1] = 0;
        uv[3] = v;
        uv += 4;
        u += du;
    }
}

void effPCPWwSpherePolyPartMove(effPCPWwSpherePoly_tag *pWwSpherePoly, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(pWwSpherePoly->hPoly, index);
    PcpScatterPlainParticle *ring;
    f32 angle;
    f32 radius;
    f32 step;
    f32 height;
    f32 s;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(pWwSpherePoly->hPoly, index);
    ring = &pWwSpherePoly->pPart[index];
    count = pWwSpherePoly->hPoly->vectorsPerParticle >> 1;
    angle = ring->angle + ring->angularSpeed;
    radius = ring->radius;
    step = ring->angleStep;
    radius += ring->radialSpeed;
    if (ring->age >= pWwSpherePoly->param.radialDecayStart) {
        ring->radialSpeed *= pWwSpherePoly->param.radialDamping;
    }
    SetRotateMatrixXYZ(ring->rot);
    ring->radius = radius;
    ring->angle = angle;
    ring->angularSpeed *= pWwSpherePoly->param.angularDamping;
    height = ring->height;
    for (i = 0; i < count; i++) {
        vertex[0] = sdfCos(angle) * radius;
        vertex[1] = 0;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] = -height;
        vertex[6] = vertex[2] = s * radius;
        SDF_VU0_LOADVECTOR(vertex);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex);
        SDF_VU0_LOADVECTOR(vertex + 4);
        SDF_VU0_APPLYMATRIX();
        SDF_VU0_STOREVECTOR(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

void effPCPWwSpherePolyDraw(effPCPWwSpherePoly_tag *hData) {
    s32 loop;
    u32 i;
    u32 count = hData->param.particleCount;
    dds3WhirlwindPolyList_tag *draw = hData->hPoly;
    PcpScatterPlainParticle *particle = hData->pPart;
    s32 duration = hData->param.duration;
    s32 fadeIn;
    s32 fadeRange;
    s32 color;
    loop = hData->param.loop;
    fadeIn = hData->param.fadeIn;
    fadeRange = hData->param.fadeRange;
    color = effMagaCylinderCalcColor(hData->abgr, hData->param.baseColor);

    for (i = 0; i < count; i++) {
        s32 age = particle->age;
        if (duration < age) {
            draw->colors[i] = 0;
        } else {
            if (age == 0) {
                effPCPWwSpherePolyPartInit(hData, i);
            } else if (age > 0) {
                s32 remaining = duration - age;
                f32 factor;
                if (age < fadeIn && fadeIn != 0) {
                    factor = (f32)age / (f32)fadeIn;
                } else if (fadeRange >= remaining && fadeRange != 0) {
                    factor = (f32)remaining / (f32)fadeRange;
                } else {
                    factor = 1.0f;
                }
                draw->colors[i] = effColorBlend(color & EFF_SCATTER_RGB_MASK, color, factor);
                effPCPWwSpherePolyPartMove(hData, i);
            }
            if (loop != 0 && age >= duration) {
                particle->age = 0;
            } else {
                particle->age++;
            }
        }
        particle++;
    }
    draw->scale = hData->scale;
    SDF_VU0_COPYVECTOR(draw->origin, hData->param.origin);
    dds3WhirlwindPolyListSetRotateMatrix(draw, hData);
    dds3WhirlwindPolyListDraw(draw);
}

void effPCPWwSpherePolySetPosition(effPCPWwSpherePoly_tag *hData, void *pPos) {
    SDF_VU0_COPYVECTOR(hData->param.origin, pPos);
}

void effPCPWwSpherePolySetScale(effPCPWwSpherePoly_tag *hData, f32 scale) {
    hData->scale = scale;
}

void effPCPWwSpherePolySetColor(effPCPWwSpherePoly_tag *hData, u32 abgr) {
    hData->abgr = abgr;
}

/* vu0 routine: copy four transform rows through the matrix bank. */
void effPCPWwSpherePolySetRotateMatrix(effPCPWwSpherePoly_tag *hData, void *pMatrix) {
    VU0_COPY_MATRIX(hData->matrix, pMatrix);
}

dds3WhirlwindPolyList_tag *dds3WhirlwindPolyListCreate(u32 number, u32 face) {
    s32 vectors = face * 2 + 2;
    s32 totalVectors = vectors * number;
    s32 bytes = (totalVectors * 6 + vectors + number) * 4;
    dds3WhirlwindPolyList_tag *object;
    struct sdfMemHandle_t *allocation;
    u8 *cursor;
    u32 quarter;
    u32 rampIndex;
    u32 *left;
    u32 *right;
    f32 alpha;
    f32 step;
    u32 color;

    object = sdfChipAllocDirect(sizeof(dds3WhirlwindPolyList_tag));
    allocation = sdfMemAllocDirect(bytes);
    cursor = (u8 *)sdfMemLock(allocation);
    memset(cursor, 0, bytes);
    object->points = (f32 *)cursor;
    cursor += totalVectors * 16;
    object->uv = (f32 *)cursor;
    cursor += totalVectors * 8;
    object->vertexColors = (u32 *)cursor;
    cursor += vectors * 4;
    object->colors = (u32 *)cursor;
    object->color = EFF_SCATTER_NEUTRAL_COLOR;
    object->vectorsPerParticle = vectors;
    object->packetQueueIndex = 1;
    object->particleCount = number;
    object->allocation = allocation;
    object->scale = 1.0f;
    object->sharedResource = NULL;
    object->asset = sdfMatNew();
    sdfMatSetConstant(object->asset, 1.0f);
    memset(object->vertexColors, 0x80, vectors * 4);
    quarter = vectors >> 2;
    if (quarter >= 2) {
        left = object->vertexColors;
        right = left + vectors - 2;
        alpha = 0.0f;
        step = 1.0f / quarter;
        for (rampIndex = 0; rampIndex < quarter; rampIndex++) {
            color = ((u32)(alpha * 128.0f) << 24) | 0x808080;
            alpha += step;
            left[0] = color;
            left[1] = left[0];
            right[0] = left[0];
            right[1] = left[0];
            left += 2;
            right -= 2;
        }
    }
    memset(&D_0056AD58, 0, sizeof(effDebugPolyParams_t));
    D_0056AD58.primitive = 0x4000;
    return object;
}

void dds3WhirlwindPolyListDelete(dds3WhirlwindPolyList_tag *hPoly) {
    dds3WhirlwindPolyList_tag *current;

    current = hPoly;
    if (current->sharedResource != 0) {
        dds3ScatterPolyListTexResourceDelete(current->sharedResource);
    }
    sdfMatFreeDelayed(current->asset);
    sdfMemFreeDirect(current->allocation);
    sdfChipFreeDirect(hPoly);
}

void dds3WhirlwindPolyListDraw(dds3WhirlwindPolyList_tag *object) {
    f32 matrix[16] __attribute__((aligned(16)));
    sdfDrawTag_t *packet;
    effDebugPolyParams_t *draw;
    s32 count;
    s32 remaining;
    s32 particleIndex;
    sdfGrOt_t *surface;

    packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(0x20);
    sdfDrawTagInit(packet);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[10] = matrix[5] = matrix[0] = object->scale;
    object->matrix[12] = object->origin[0];
    object->matrix[13] = object->origin[1];
    object->matrix[14] = object->origin[2];
    SDF_VU0_LOADMATRIX(object->matrix);
    SDF_VU0_LOADMATRIX_B(matrix);
    MultMatrixNoArg();
    sdf3DPrimLinkTm(packet, 0);
    if (object->sharedResource != NULL) {
        sdfMatSetTex((struct sdfMat_t *)object->asset,
                      object->sharedResource->textureHandle);
    }
    sdf3DPrimLinkMat(packet, (sdfMat_t *)object->asset, 0);
    count = (s32)object->particleCount;
    D_0056AD58.parameters = (const u32 *)D_0031F3E0;
    for (particleIndex = 0; particleIndex < count; particleIndex++) {
        draw = &D_0056AD58;
        remaining = object->vectorsPerParticle;
        draw->positions = (u128 *)dds3WhirlwindPolyListGetVertex(object, particleIndex);
        draw->texcoords = dds3WhirlwindPolyListGetUV(object, particleIndex);
        draw->colors = object->vertexColors;
        draw->color = dds3WhirlwindPolyListGetABGR(object, particleIndex);
        draw->parameterCount = 16;
        draw->vertexCount = 18;
        if ((draw->color & 0xFF000000) != 0) {
            while (remaining >= 18) {
                remaining -= 16;
                sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(&D_0056AD58));
                D_0056AD58.positions += 16;
                D_0056AD58.colors += 16;
                D_0056AD58.texcoords += 32;
            }
            if (remaining >= 4) {
                draw->parameterCount = remaining - 2;
                draw->vertexCount = remaining;
                sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(draw));
            }
        }
    }
    surface = D_0031F440[object->packetQueueIndex];
    surface->regFunc(surface, packet);
}

void dds3WhirlwindPolyListSetTex(dds3WhirlwindPolyList_tag *hPoly, u32 pTex) {
    dds3ScatterPolyListTexResource_tag *shared;

    shared = dds3ScatterPolyListTexResourceCreate(pTex);
    hPoly->sharedResource = shared;
}

void dds3WhirlwindPolyListCopyTex(dds3WhirlwindPolyList_tag *hPoly, dds3WhirlwindPolyList_tag *hPolySrc) {
    dds3ScatterPolyListTexResource_tag *shared;

    shared = dds3ScatterPolyListTexResourceCopy(hPolySrc->sharedResource);
    hPoly->sharedResource = shared;
}

f32 *dds3WhirlwindPolyListGetVertex(dds3WhirlwindPolyList_tag *hPoly, s32 index) {
    return hPoly->points + index * hPoly->vectorsPerParticle * 4;
}

f32 *dds3WhirlwindPolyListGetUV(dds3WhirlwindPolyList_tag *hPoly, s32 index) {
    return hPoly->uv + index * hPoly->vectorsPerParticle * 2;
}

u32 dds3WhirlwindPolyListGetABGR(dds3WhirlwindPolyList_tag *hPoly, s32 index) {
    return hPoly->colors[index];
}

/* vu0 routine: copy four transform rows through the matrix bank. */
void dds3WhirlwindPolyListSetRotateMatrix(dds3WhirlwindPolyList_tag *hPoly, const void *pMatrix) {
    SDF_VU0_LOADMATRIX(pMatrix);
    SDF_VU0_STOREMATRIX(hPoly->matrix);
}
