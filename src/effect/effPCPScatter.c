#include "common.h"
#include "ps2asm.h"

struct EffPacketParams;
struct EffRandState;
struct PcpScatterDraw;
struct PcpScatterInstanceA;
struct PcpScatterInstanceB;
struct PcpScatterInstanceC;
struct PcpScatterParamsA;
struct PcpScatterParamsB;
struct PcpScatterParamsC;
struct PcpScatterParticle;
struct PcpScatterPlainInstance;
struct PcpScatterPlainParams;
struct PcpScatterPlainParticle;
struct PcpScatterPool;
struct PcpScatterRadialParams;
struct PcpScatterRadialParticle;
struct PcpScatterRadialWork;
struct PcpScatterRibbonParams;
struct PcpScatterRibbonParticle;
struct PcpScatterRibbonWork;
struct PcpScatterSpinParams;
struct PcpScatterSpinParticle;
struct PcpScatterSpinWork;
struct SdfAssetEntry;
union SdfDrawTransform;
struct SdfGsTextureStateValues;
struct SdfTexRef;
struct SdfTexResource;

typedef struct EffPacketParams EffPacketParams;
typedef struct SdfTexBuf SdfTexBuf;
typedef struct SdfTexRef SdfTexRef;
typedef struct SdfTexResource SdfTexResource;
typedef struct SdfTex SdfTex;
typedef union SdfDrawTransform SdfDrawTransform;
typedef struct SdfGsTextureStateValues SdfGsTextureStateValues;
typedef struct SdfAssetEntry SdfAssetEntry;
typedef union SdfSubParam SdfSubParam;
typedef struct SdfAsset SdfAsset;
typedef struct SdfMemBlock SdfMemBlock;
typedef struct PcpScatterDraw PcpScatterDraw;
typedef struct PcpScatterParamsA PcpScatterParamsA;
typedef struct PcpScatterParticle PcpScatterParticle;
typedef struct PcpScatterInstanceA PcpScatterInstanceA;
typedef struct PcpScatterParamsB PcpScatterParamsB;
typedef struct PcpScatterInstanceB PcpScatterInstanceB;
typedef struct PcpScatterParamsC PcpScatterParamsC;
typedef struct PcpScatterInstanceC PcpScatterInstanceC;
typedef struct PcpScatterPlainParams PcpScatterPlainParams;
typedef struct PcpScatterPlainParticle PcpScatterPlainParticle;
typedef struct PcpScatterPlainInstance PcpScatterPlainInstance;
typedef struct EffParamWork EffParamWork;
typedef struct PcpScatterRadialParams PcpScatterRadialParams;
typedef struct PcpScatterRadialParticle PcpScatterRadialParticle;
typedef struct PcpScatterRadialWork PcpScatterRadialWork;
typedef struct PcpScatterRibbonParticle PcpScatterRibbonParticle;
typedef struct PcpScatterRibbonParams PcpScatterRibbonParams;
typedef struct PcpScatterRibbonWork PcpScatterRibbonWork;
typedef struct PcpScatterSpinParams PcpScatterSpinParams;
typedef struct PcpScatterSpinParticle PcpScatterSpinParticle;
typedef struct PcpScatterSpinWork PcpScatterSpinWork;
typedef struct SdfListHead SdfListHead;
typedef struct SdfPoolNode SdfPoolNode;
typedef struct SdfTextureFileHeader SdfTextureFileHeader;
typedef struct EffRandState EffRandState;

#define EFF_SCATTER_NEUTRAL_COLOR 0x80808080

#define EFF_SCATTER_RIBBON_TURN 6.2831850051879883f

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

/*
 * sceVu0UnitMatrix as libvu0 expands it (scratch registers hard-coded by the
 * SDK):
 *     qmfc2.ni $5, vf0            ; (0,0,0,1)
 *     pextuw   $4, $0, $5         ; (0,0,1,0) row 2
 *     pextuw   $2, $0, $4         ; (1,0,0,0) row 0
 *     pextuw   $3, $4, $0         ; (0,1,0,0) row 1
 *     sq $2,0(dst); sq $3,0x10(dst); sq $4,0x20(dst); sq $5,0x30(dst)
 * The four `sq` are part of the idiom: retail stores the identity rows from
 * the registers the shuffle left them in. Tried: sixteen u32 stores of 0/1
 * (gcc 2.96 emits sw of $0 and a constant, never qmfc2/pextuw or sq).
 */
#define EE_MMI_UNIT_MATRIX(dst) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "qmfc2.ni $5, $vf0\n\t" \
    "pextuw $4, $0, $5\n\t" \
    "pextuw $2, $0, $4\n\t" \
    "pextuw $3, $4, $0\n\t" \
    "sq $2, 0(%0)\n\t" \
    "sq $3, 0x10(%0)\n\t" \
    "sq $4, 0x20(%0)\n\t" \
    "sq $5, 0x30(%0)\n\t" \
    ".set reorder" \
    : : "r" (dst) : "$2", "$3", "$4", "$5", "memory")

struct EffPacketParams {
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

struct PcpScatterRes;

struct SdfTex;

struct SdfTexBuf;

struct SdfTexBuf {
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
struct SdfTexRef {
    struct SdfTex *cloneSource; /* Borrowed source pointer; marks the shared-primary clone path. */
    s32 refCount;
};

/* Native VRAM range descriptor (0x1C), shared by textures, graph buffers and streams. */
struct SdfTexResource {
    struct SdfTexResource *next; /* 0x00 */
    struct SdfTexResource *prev; /* 0x04 */
    u32 allocationMode;         /* 0x08: known categories above; unknown values remain valid */
    u32 word;                   /* 0x0C: VRAM offset in 32-bit words */
    s32 size;                   /* 0x10: range length in 32-bit words */
    s16 width;                  /* 0x14 */
    s16 height;                 /* 0x16 */
    s32 format;                 /* 0x18: GS pixel-storage mode */
};

/* Serialized byte consumed by the GS texture-state packet builder. */
typedef u8 SdfTextureClampMode;

struct SdfTex {
    struct SdfTex *next;
    struct SdfTex *prev;
    SdfTexRef *reference;
    s16 width;
    s16 height;
    SdfTexResource *primaryResource;
    SdfTexResource *secondaryResource;
    u8 paletteCount;
    u8 clutFormat;
    u8 pixelFormat;
    u8 maxMipLevel;
    u16 lodParameters; /* Packed GS TEX1 L/K parameters. */
    u8 unk1E;
    SdfTextureClampMode clampMode; /* 0x1F: 0xFF leaves GS CLAMP state zero; otherwise low nibble is copied. */
    s32 resourceKey;
    s32 battleTextureSlot;
    SdfTexBuf *primaryBuffer;
    SdfTexBuf *secondaryBuffer;
    u8 *paletteData; /* 0x30: retained copy of the texture's palette colors. */
    s32 paletteDataSize; /* 0x34: palette byte count from sdfTexGetClutDataSize. */
    s32 unk38;
    u8 *intensityMap; /* 0x3C: lazily generated weighted-RGB intensity bytes. */
};

/* Shared scatter texture reference, created and released independently of pools. */
typedef struct PcpScatterRes {
    SdfTex *textureHandle;
    s32 refCount;
} PcpScatterRes; /* 0x08 */

struct SdfAsset;

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
struct SdfAssetEntry {
    u32 pad00;             /* 0x00 */
    u32 unk04;             /* 0x04 */
    u32 unk08;             /* 0x08 */
    u32 secondaryColor;    /* 0x0C: packed color applied by the secondary draw state */
    u32 primaryStateWordThird; /* 0x10: copied from the owning asset, or forced to zero. */
    u32 unk14;             /* 0x14 */
    u32 pad18;             /* 0x18 */
    f32 unk1C;             /* 0x1C */
    u32 alphaState;        /* 0x20: packed GS ALPHA_2 state emitted by the VU transfer builder */
    u32 secondaryMode;     /* 0x24: secondary packet mode copied from SdfAsset */
    f32 y;                 /* 0x28 */
    f32 x;                 /* 0x2C */
    u8 pad30[8];           /* 0x30 */
    SdfGsTextureStateValues primaryTextureState; /* 0x38 */
    SdfGsTextureStateValues secondaryTextureState; /* 0x50 */
    SdfDrawTransform transforms[2]; /* 0x68 */
    u32 vifItopCommand;    /* 0x98: VIF ITOP with immediate 0xC */
    u32 vifMscalCommand;   /* 0x9C: VIF MSCAL with microprogram index zero */
};

union SdfSubParam;

/* Optional packed or five-float state block owned by an SdfAsset. */
union SdfSubParam {
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

struct SdfAsset {
    u32 unk00; /* Word from the serialized asset-parameter header. */
    u16 unk04; /* Halfword from the serialized asset-parameter header. */
    u8 dirtyFlags; /* 0x06: pending state-group changes for the two draw entries */
    u8 pad07;
    struct SdfAssetEntry *drawEntries[2]; /* 0x08: owned 0xA0-byte draw entries */
    u32 primaryStateWordFirst; /* 0x10: first word set by primary-state updates */
    u32 primaryStateWordSecond; /* 0x14: second word set by primary-state updates */
    u32 secondaryColor; /* 0x18 */
    f32 primaryStateFloat; /* 0x1C: interpolated primary-state float */
    u32 primaryStateWordThird; /* 0x20 */
    u32 unk24;
    u32 primaryStateWordFourth; /* 0x28: retained as raw bits */
    SdfTex *texture;
    SdfTex *secondaryTexture; /* 0x30 */
    u32 secondaryMode;       /* 0x34: secondary packet and palette selector */
    SdfSubParam *primarySubParam; /* 0x38 */
    SdfSubParam *secondarySubParam; /* 0x3C */
    f32 scalarPairFirst; /* 0x40 */
    f32 scalarPairSecond; /* 0x44 */
};

struct SdfMemBlock;

/* Embedded command work at actor +0x20. Neither record is the whole owning
 * actor. */
struct SdfMemBlock;

/* General-heap allocation descriptor (0x10); links bound the represented span. */
struct SdfMemBlock {
    struct SdfMemBlock *prev; /* 0x00 */
    struct SdfMemBlock *next; /* 0x04 */
    s32 address;              /* 0x08: start of represented span */
    u16 state;                /* 0x0C: free, used, or end sentinel */
    s16 referenceCount;       /* 0x0E: -1 for sentinels */
};

/* The drawable owns its geometry allocation and asset independently of the
 * effect instance that supplies its transform and scale. */
struct PcpScatterDraw {
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
    struct SdfAsset *asset;    // 0x74
    struct SdfMemBlock *allocation; // 0x78
    struct PcpScatterRes *sharedResource; // 0x7C
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

struct PcpScatterInstanceA {
    f32 matrix[16];
    PcpScatterParamsA params;
    PcpScatterParticle *particles;
    f32 scale;
    u32 color;
    PcpScatterDraw *scatterObject;
    SdfMemBlock *allocationHandle;
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

struct PcpScatterInstanceB {
    f32 matrix[16];
    PcpScatterParamsB params;
    PcpScatterParticle *particles;
    f32 scale;
    u32 color;
    s32 age;
    PcpScatterDraw *scatterObject;
    SdfMemBlock *allocationHandle;
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

struct PcpScatterInstanceC {
    f32 matrix[16];
    PcpScatterParamsC params;
    PcpScatterParticle *particles;
    f32 scale;
    u32 color;
    s32 age;
    PcpScatterDraw *scatterObject;
    SdfMemBlock *allocationHandle;
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

struct PcpScatterPlainInstance {
    f32 matrix[16];
    PcpScatterPlainParams params;
    PcpScatterPlainParticle *particles;
    f32 scale;
    u32 color;
    PcpScatterDraw *scatterObject;
    SdfMemBlock *allocationHandle;
};

/* Appended after the two record arrays; resource helpers own this same control block. */
typedef struct PcpScatterPool {
    f32 origin[4];
    u32 poolMode; /* 0x10: selects the surface node used when drawing the pool */
    u32 color;
    s32 secondWordCount;
    f32 unk1C;
    u8 *recordBase;
    u8 *auxRecordBase;
    SdfAsset *drawAsset;
    SdfMemBlock *allocation;
    PcpScatterRes *sharedResource;
} PcpScatterPool; /* 0x34 */

struct EffParamWork;

/* One randomized channel record. */
struct EffParamWork;

/* Compact effect owner: its halfword kind selects the opaque payload's operations. */
struct EffParamWork {
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
struct PcpScatterRadialWork {
    PcpScatterRadialParams params;
    PcpScatterRadialParticle *particles;
    f32 scale;
    u32 color;
    PcpScatterPool *childWork;
    SdfMemBlock *allocation;
    u32 duplicateGroupCount;
    EffParamWork **duplicatedHandles;
    SdfMemBlock *duplicateAllocation;
};

struct PcpScatterRibbonParticle {
    s32 age;
    f32 height;
    f32 heightStep;
    f32 tiltAngle;
    f32 tiltHalfAngle;
    f32 tiltStep;
    f32 radius;
    f32 unk1C;
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
struct PcpScatterRibbonWork {
    PcpScatterRibbonParams params;
    PcpScatterRibbonParticle *particles;
    f32 scale;
    u32 color;
    PcpScatterPool *childWork;
    SdfMemBlock *allocation;
    u32 duplicateGroupCount;
    EffParamWork **duplicatedHandles;
    SdfMemBlock *duplicateAllocation;
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

struct PcpScatterSpinParticle {
    s32 age;
    f32 radiusStep;
    f32 angleStep;
    f32 radius;
    f32 angle;
    f32 dirX;
    f32 dirY;
    f32 dirZ;
};

/* Runtime owner followed inline by its variable-length particle array. */
struct PcpScatterSpinWork {
    PcpScatterSpinParams params;
    PcpScatterSpinParticle *particles;
    f32 scale;
    u32 color;
    PcpScatterPool *childWork;
    SdfMemBlock *allocation;
    u32 duplicateGroupCount;
    EffParamWork **duplicatedHandles;
    SdfMemBlock *duplicateAllocation;
};

struct SdfListHead;

struct SdfListHead {
    struct SdfListHead *nextList; /* CPU link between lists; first/last are DMA addresses. */
    u32 first;
    u32 last;
    u32 unkC;
    u32 firstReferenceSource; /* Optional first DMA reference prefix. */
    u32 secondReferenceSource; /* Optional second DMA reference prefix. */
    u32 unk18;
    u32 unk1C;
};

struct SdfPoolNode;

/* Draw-surface pool entry (0x20); the SDK initializes and flushes this owner. */
struct SdfPoolNode {
    struct SdfPoolNode *next; /* 0x00 */
    SdfListHead *first;       /* 0x04 */
    SdfListHead *last;        /* 0x08 */
    u32 unkC;
    void (*append)(struct SdfPoolNode *, SdfListHead *);      /* 0x10 */
    s32 (*prepend)(struct SdfPoolNode *, s32, SdfListHead *); /* 0x14 */
    u32 unk18;
    u32 unk1C;
};

struct SdfTextureFileHeader;

/* Serialized texture-resource header; payload begins after this 0x40-byte prefix. */
struct SdfTextureFileHeader {
    u8 unk00;
    u8 payloadOffsetFlags; /* 0x01: high nibble adds payload offset bytes; low bits stay opaque. */
    u8 pad02[2];       /* 0x02 */
    u32 byteCount;     /* 0x04 */
    u32 magic;         /* 0x08 */
    u8 pad0C[4];       /* 0x0C */
    u8 paletteCount;   /* 0x10 */
    u8 clutFormat;     /* 0x11 */
    s16 width;         /* 0x12 */
    s16 height;        /* 0x14 */
    u8 pixelFormat;    /* 0x16 */
    u8 maxMipLevel;    /* 0x17 */
    u16 lodParameters; /* 0x18 */
    u8 unk1A;          /* 0x1A */
    SdfTextureClampMode clampMode; /* 0x1B: serialized GS clamp mode */
    s32 resourceKey;   /* 0x1C */
    s32 battleTextureSlot; /* 0x20: selects one of two battle-model texture headers */
    u8 pad24[0x1C];    /* 0x24 */
};

/* Load four quadwords into the second matrix bank vf24-vf27 (the right-hand
 * operand of the sdf matrix-multiply routines). */
#define VU0_LOAD_MATRIX_B(src) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "lqc2 vf24, 0(%0)\n\t" \
    "lqc2 vf25, 16(%0)\n\t" \
    "lqc2 vf26, 32(%0)\n\t" \
    "lqc2 vf27, 48(%0)\n\t" \
    ".set reorder" \
    : : "r" (src) : "memory")

/* Load-normalize-store of one packed vector in a single block ("+m" operand
 * with a memory clobber). */
#define VU0_NORMALIZE_PACKED_VECTOR(v) __asm__ volatile ( \
    ".set noreorder\n\tlqc2 vf10, %0\n\tvmul.xyz vf2, vf10, vf10\n\tvmulax.w ACC, vf0, vf2x\n\t" \
    "vmadday.w ACC, vf0, vf2y\n\tvmaddz.w vf2, vf0, vf2z\n\tvrsqrt Q, vf0w, vf2w\n\t" \
    "vwaitq\n\tvmulq.xyz vf10, vf10, Q\n\tsqc2 vf10, %0\n\t.set reorder" \
    : "+m" (v) :: "memory")

extern void *effPkt3DPrimMakePoly(EffPacketParams *params);

/* Four ordered 32-bit words in the shared effect random-number state. */
struct EffRandState {
    u32 words[4];
};

extern EffRandState dds3EffectBase[];

void dds3EffectPartDelete(EffParamWork *work);

extern u32 dds3WhirlwindPolyListGetABGR(PcpScatterDraw *object, s32 particleIndex);

/* Returns 24 random bits scaled to [0, 1); NULL selects the shared state. */
f32 dds3GetRandFloatA(EffRandState *state);

/* Constructors copy one serialized parameter variant into its runtime owner. */
PcpScatterInstanceA *effPCPWhirlwindPolyCreate(
    PcpScatterParamsA *params, u32 resource);

PcpScatterPlainInstance *effPCPWwSpherePolyCreate(
    PcpScatterPlainParams *params, u32 resource);

extern PcpScatterRes *dds3ScatterPolyListTexResourceCopy(PcpScatterRes *res);

PcpScatterInstanceB *effPCPWhirlwind2PolyCreate(
    PcpScatterParamsB *params, u32 resource);

PcpScatterInstanceC *effPCPWhirlwind3PolyCreate(
    PcpScatterParamsC *params, u32 resource);

extern EffPacketParams D_0056AD58;

extern SdfPoolNode *D_0031F440[];

extern EffPacketParams D_0056AD28[];

extern SdfPoolNode *D_0031F3D0[];

extern u32 D_0031F310[];

extern u32 D_0031F2C0[];

extern u8 D_0031F3E0[];

extern void *memset(void *dst, s32 value, u32 size);

struct SdfMemBlock *sdfMemAllocDirect(s32 requestedBytes);

/* Reserve size bytes from the active packet buffer, rounded up to 16 bytes.
 * Returns the prior cursor as a 32-bit address; the buffer owns this storage. */
s32 sdfGrAllocPacketBuff(s32 size);

void *sdfChipAllocDirect(s32 size);

void sdfDrawTagLink(struct SdfListHead *list, u32 packetAddress);

/* Apply the primary VU0 matrix bank vf28-vf31 to basis vectors vf24-vf27,
 * then replace vf28-vf31 with the four transformed vectors. */
void MultMatrixNoArg(void);

void sdf3DPrimLinkMat(SdfListHead *packetList, SdfAsset *asset,
                              s32 (*allocatePacket)(s32));

void sdf3DPrimLinkTm(SdfListHead *packetList,
                           s32 (*allocatePacket)(s32));

struct SdfAsset *sdfMatNew(void);

f32 sdfCos(f32 angle);

void sdfDrawTagInit(struct SdfListHead *list);

/* Set vf28-vf31 to vf28-vf31 * vf24-vf27 in place. */
void MultMatrixRevNoArg(void);

/* Queue an asset owner for the deferred SdfAsset release callback. */
void sdfMatFreeDelayed(struct SdfAsset *asset);

void sdfChipFreeDirect(void *memory);

void sdfMemFreeDirect(struct SdfMemBlock *allocation);

u32 sdfMemLock(struct SdfMemBlock *allocation);

/* Store the primary or secondary texture owner and mark its draw state dirty. */
void sdfMatSetTex(struct SdfAsset *asset, struct SdfTex *texture);

void sdfMatSetConstant(struct SdfAsset *asset, f32 value);

f32 sdfSin(f32 angle);

/* Acquire a texture from a serialized texture-file header. */
struct SdfTex *sdfTexSet(const SdfTextureFileHeader *header);

void sdfTexFreeDelayed(SdfTex *texture);

/* Load the X-axis rotation matrix into the primary VU0 matrix registers. */
void SetRotateMatrixX(f32 angle);

/* Load the secondary Y-axis rotation matrix into the primary VU0 matrix registers. */
void SetRotateMatrixY_B(f32 angle);

extern void SetRotateMatrixXYZ(f32 *rot);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralCopy);

void effPCPHamaCardSpiralDelete(PcpScatterRadialWork *work)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (work->duplicateAllocation != 0) {
        count = work->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(work->duplicatedHandles[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(work->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(work->childWork);
    sdfMemFreeDirect(work->allocation);
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralPartInit);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralSetColor);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpiralSetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereCopy);

void effPCPHamaCardSphereDelete(PcpScatterSpinWork *work)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (work->duplicateAllocation != 0) {
        count = work->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(work->duplicatedHandles[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(work->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(work->childWork);
    sdfMemFreeDirect(work->allocation);
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSpherePartInit);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereSetColor);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardSphereSetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterCopy);

void effPCPHamaCardScatterDelete(PcpScatterRibbonWork *work)
{
    u32 duplicatedHandleIndex;
    u32 count;

    if (work->duplicateAllocation != 0) {
        count = work->duplicateGroupCount;
        duplicatedHandleIndex = 0;
        if (count != 0) {
            do {
                dds3EffectPartDelete(work->duplicatedHandles[duplicatedHandleIndex]);
                duplicatedHandleIndex++;
            } while (duplicatedHandleIndex < count);
        }
        sdfMemFreeDirect(work->duplicateAllocation);
    }
    dds3ScatterPolyListDelete(work->childWork);
    sdfMemFreeDirect(work->allocation);
}

void effPCPHamaCardScatterPartInit(PcpScatterRibbonWork *work, s32 index)
{
    f32 rightAngle = EFF_SCATTER_RIGHT_ANGLE_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    f32 diagonalAngle = EFF_SCATTER_DIAGONAL_ANGLE_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    f32 tiltStep = EFF_SCATTER_TILT_STEP_DEGREES * EFF_SCATTER_DEGREES_TO_RADIANS;
    PcpScatterRibbonParticle *particle = &work->particles[index];
    f32 direction[4];

    particle->height = 0.0f;
    particle->heightStep = work->params.heightStep;
    particle->tiltHalfAngle = rightAngle;
    particle->tiltAngle = diagonalAngle;
    particle->tiltStep = tiltStep;
    particle->unk1C = dds3GetRandFloatA(dds3EffectBase) * EFF_SCATTER_RIBBON_TURN;
    particle->radius = EFF_SCATTER_INITIAL_RIBBON_RADIUS;

    direction[0] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    direction[1] = 0.0f;
    direction[2] = (dds3GetRandFloatA(dds3EffectBase) - EFF_SCATTER_RANDOM_MIDPOINT) * EFF_SCATTER_RANDOM_SPAN;
    VU0_NORMALIZE_PACKED_VECTOR(direction);

    particle->unk20 = direction[0];
    particle->age = 0;
    particle->unk24 = direction[2];
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPHamaCardScatterSetColor);

void effPCPHamaCardScatterSetScale(void) {
}

PcpScatterPool *dds3ScatterPolyListCreate(s32 groups) {
    PcpScatterPool *pool;
    SdfMemBlock *allocation;
    u32 *recordBlock;
    s32 slotCount;
    s32 recordWords;
    s32 auxWords;
    u32 allocationBytes;

    slotCount = groups * EFF_SCATTER_POOL_SLOTS_PER_GROUP;
    recordWords = slotCount * EFF_SCATTER_POOL_RECORD_WORDS_PER_SLOT;
    auxWords = slotCount * EFF_SCATTER_POOL_AUX_WORDS_PER_SLOT;
    allocationBytes = (recordWords + auxWords) * EFF_SCATTER_WORD_BYTES + EFF_SCATTER_POOL_CONTROL_BYTES;
    allocation = sdfMemAllocDirect(allocationBytes);
    recordBlock = (u32 *)sdfMemLock(allocation);
    memset(recordBlock, 0, allocationBytes);
    pool = (PcpScatterPool *)(recordBlock + (recordWords + auxWords));
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

void dds3ScatterPolyListDelete(PcpScatterPool *work)
{
    if (work->sharedResource != NULL) {
        dds3ScatterPolyListTexResourceDelete(work->sharedResource);
    }
    sdfMatFreeDelayed(work->drawAsset);
    sdfMemFreeDirect(work->allocation);
}

void dds3ScatterPolyListDraw(PcpScatterPool *pool) {
    f32 matrix[16];
    SdfListHead *packet = (SdfListHead *)sdfGrAllocPacketBuff(0x20);
    s32 remainingVertices;
    SdfPoolNode *surface;

    sdfDrawTagInit(packet);
    EE_MMI_UNIT_MATRIX(matrix);
    matrix[12] = pool->origin[0];
    matrix[13] = pool->origin[1];
    matrix[14] = pool->origin[2];
    VU0_LOAD_MATRIX(matrix);
    sdf3DPrimLinkTm(packet, 0);
    if (pool->sharedResource != NULL) {
        sdfMatSetTex(pool->drawAsset, pool->sharedResource->textureHandle);
        D_0056AD28->texcoords = (f32 *)D_0031F310;
    } else {
        D_0056AD28->texcoords = NULL;
    }
    sdf3DPrimLinkMat(packet, pool->drawAsset, 0);
    remainingVertices = pool->secondWordCount;
    D_0056AD28->colors = (u32 *)pool->auxRecordBase;
    D_0056AD28->positions = (u128 *)pool->recordBase;
    D_0056AD28->color = pool->color;
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
    surface = D_0031F3D0[pool->poolMode];
    surface->append(surface, packet);
}

void dds3ScatterPolyListSetTex(PcpScatterPool *work, u32 resId)
{
    PcpScatterRes *res;

    res = dds3ScatterPolyListTexResourceCreate(resId);
    work->sharedResource = res;
}

void dds3ScatterPolyListCopyTex(PcpScatterPool *work, PcpScatterPool *src)
{
    PcpScatterRes *res;

    res = dds3ScatterPolyListTexResourceCopy(src->sharedResource);
    work->sharedResource = res;
}

u128 *dds3ScatterPolyListGetVertex(PcpScatterPool *work, s32 index)
{
    return (u128 *)(work->recordBase + index * EFF_SCATTER_RECORD_BYTES);
}

u32 *dds3ScatterPolyListGetVC(PcpScatterPool *work, s32 index)
{
    return (u32 *)(work->auxRecordBase + index * EFF_SCATTER_AUX_RECORD_BYTES);
}

PcpScatterRes *dds3ScatterPolyListTexResourceCreate(u32 resId)
{
    PcpScatterRes *res;

    res = sdfChipAllocDirect(sizeof(PcpScatterRes));
    res->textureHandle = sdfTexSet((SdfTextureFileHeader *)(resId));
    res->refCount = 1;
    return res;
}

void dds3ScatterPolyListTexResourceDelete(PcpScatterRes *res)
{
    if (--res->refCount == 0) {
        sdfTexFreeDelayed(res->textureHandle);
        sdfChipFreeDirect(res);
    }
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", dds3ScatterPolyListTexResourceCopy);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolyCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolyCreatePCF);

PcpScatterInstanceA *effPCPWhirlwindPolyCopy(PcpScatterInstanceA *work) {
    PcpScatterInstanceA *child;

    child = effPCPWhirlwindPolyCreate(&work->params, NULL);
    dds3WhirlwindPolyListCopyTex(child->scatterObject, work->scatterObject);
    return child;
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolyDelete);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolyPartInit);

void effPCPWhirlwindPolyPartMove(PcpScatterInstanceA *work, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(work->scatterObject, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(work->scatterObject, index);
    ring = &work->particles[index];
    radius = ring->radius + ring->radiusStep;
    count = work->scatterObject->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= work->params.tiltDamping;
    ring->radius = radius;
    ring->rise = rise * work->params.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        VU0_LOAD_VF10(vertex);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex);
        VU0_LOAD_VF10(vertex + 4);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolyDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolySetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolySetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwindPolySetColor);

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwindPolySetRotateMatrix(PcpScatterInstanceA *work, void *source)
{
    VU0_LOAD_MATRIX(source);
    VU0_LOAD_MATRIX_B(work->params.matrix);
    MultMatrixNoArg();
    VU0_STORE_MATRIX(work);
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolyCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolyCreatePCF);

PcpScatterInstanceB *effPCPWhirlwind2PolyCopy(PcpScatterInstanceB *work)
{
    PcpScatterInstanceB *child;

    child = effPCPWhirlwind2PolyCreate(&work->params, NULL);
    dds3WhirlwindPolyListCopyTex(child->scatterObject, work->scatterObject);
    return child;
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolyDelete);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolyPartInit);

void effPCPWhirlwind2PolyPartMove(PcpScatterInstanceB *work, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(work->scatterObject, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(work->scatterObject, index);
    ring = &work->particles[index];
    radius = ring->radius + ring->radiusStep;
    count = work->scatterObject->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= work->params.tiltDamping;
    ring->radius = radius;
    ring->radiusStep *= work->params.radiusDamping;
    ring->rise = rise * work->params.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        VU0_LOAD_VF10(vertex);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex);
        VU0_LOAD_VF10(vertex + 4);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolyDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolySetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolySetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind2PolySetColor);

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwind2PolySetRotateMatrix(PcpScatterInstanceB *work, void *source)
{
    VU0_LOAD_MATRIX(source);
    VU0_LOAD_MATRIX_B(work->params.matrix);
    MultMatrixNoArg();
    VU0_STORE_MATRIX(work);
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolyCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolyCreatePCF);

PcpScatterInstanceC *effPCPWhirlwind3PolyCopy(PcpScatterInstanceC *work)
{
    PcpScatterInstanceC *child;

    child = effPCPWhirlwind3PolyCreate(&work->params, NULL);
    dds3WhirlwindPolyListCopyTex(child->scatterObject, work->scatterObject);
    return child;
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolyDelete);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolyPartInit);

void effPCPWhirlwind3PolyPartMove(PcpScatterInstanceC *work, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(work->scatterObject, index);
    PcpScatterParticle *ring;
    f32 radius;
    f32 rise;
    f32 angle;
    f32 step;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(work->scatterObject, index);
    ring = &work->particles[index];
    radius = ring->radius + ring->radiusStep;
    count = work->scatterObject->vectorsPerParticle >> 1;
    rise = ring->rise;
    angle = ring->angle;
    step = ring->angleStep;
    SetRotateMatrixX(ring->orientationAngle);
    SetRotateMatrixY_B(ring->tiltAngle);
    MultMatrixRevNoArg();
    ring->tiltAngle += ring->tiltSpeed;
    ring->tiltSpeed *= work->params.tiltDamping;
    ring->radius = radius;
    ring->radiusStep *= work->params.radiusDamping;
    ring->rise = rise * work->params.riseDecay;
    for (i = 0; i < count; i++) {
        f32 s;

        vertex[0] = sdfCos(angle) * radius;
        vertex[1] += rise;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] += rise;
        vertex[6] = vertex[2] = s * radius;
        VU0_LOAD_VF10(vertex);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex);
        VU0_LOAD_VF10(vertex + 4);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolyDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolySetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolySetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWhirlwind3PolySetColor);

/* vu0 routine: compose params.matrix and source via the native product into work->matrix. */
void effPCPWhirlwind3PolySetRotateMatrix(PcpScatterInstanceC *work, void *source)
{
    VU0_LOAD_MATRIX(source);
    VU0_LOAD_MATRIX_B(work->params.matrix);
    MultMatrixNoArg();
    VU0_STORE_MATRIX(work);
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolyCreate);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolyCreatePCF);

PcpScatterPlainInstance *effPCPWwSpherePolyCopy(PcpScatterPlainInstance *work)
{
    PcpScatterPlainInstance *child;

    child = effPCPWwSpherePolyCreate(&work->params, NULL);
    dds3WhirlwindPolyListCopyTex(child->scatterObject, work->scatterObject);
    return child;
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolyDelete);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolyPartInit);

void effPCPWwSpherePolyPartMove(PcpScatterPlainInstance *work, s32 index)
{
    f32 *vertex = dds3WhirlwindPolyListGetVertex(work->scatterObject, index);
    PcpScatterPlainParticle *ring;
    f32 angle;
    f32 radius;
    f32 step;
    f32 height;
    f32 s;
    u32 count;
    u32 i;

    dds3WhirlwindPolyListGetUV(work->scatterObject, index);
    ring = &work->particles[index];
    count = work->scatterObject->vectorsPerParticle >> 1;
    angle = ring->angle + ring->angularSpeed;
    radius = ring->radius;
    step = ring->angleStep;
    radius += ring->radialSpeed;
    if (ring->age >= work->params.radialDecayStart) {
        ring->radialSpeed *= work->params.radialDamping;
    }
    SetRotateMatrixXYZ(ring->rot);
    ring->radius = radius;
    ring->angle = angle;
    ring->angularSpeed *= work->params.angularDamping;
    height = ring->height;
    for (i = 0; i < count; i++) {
        vertex[0] = sdfCos(angle) * radius;
        vertex[1] = 0;
        s = sdfSin(angle);
        vertex[4] = vertex[0];
        vertex[5] = -height;
        vertex[6] = vertex[2] = s * radius;
        VU0_LOAD_VF10(vertex);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex);
        VU0_LOAD_VF10(vertex + 4);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(vertex + 4);
        angle += step;
        vertex += 8;
    }
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolyDraw);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolySetPosition);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolySetScale);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolySetColor);

INCLUDE_ASM(const s32, "effect/effPCPScatter", effPCPWwSpherePolySetRotateMatrix);

INCLUDE_ASM(const s32, "effect/effPCPScatter", dds3WhirlwindPolyListCreate);

void dds3WhirlwindPolyListDelete(PcpScatterDraw *object) {
    PcpScatterDraw *current;

    current = object;
    if (current->sharedResource != 0) {
        dds3ScatterPolyListTexResourceDelete(current->sharedResource);
    }
    sdfMatFreeDelayed(current->asset);
    sdfMemFreeDirect(current->allocation);
    sdfChipFreeDirect(object);
}

void dds3WhirlwindPolyListDraw(PcpScatterDraw *object) {
    f32 matrix[16] __attribute__((aligned(16)));
    SdfListHead *packet;
    EffPacketParams *draw;
    s32 count;
    s32 remaining;
    s32 particleIndex;
    SdfPoolNode *surface;

    packet = (SdfListHead *)sdfGrAllocPacketBuff(0x20);
    sdfDrawTagInit(packet);
    EE_MMI_UNIT_MATRIX(matrix);
    matrix[10] = matrix[5] = matrix[0] = object->scale;
    object->matrix[12] = object->origin[0];
    object->matrix[13] = object->origin[1];
    object->matrix[14] = object->origin[2];
    VU0_LOAD_MATRIX(object->matrix);
    VU0_LOAD_MATRIX_B(matrix);
    MultMatrixNoArg();
    sdf3DPrimLinkTm(packet, 0);
    if (object->sharedResource != NULL) {
        sdfMatSetTex((struct SdfAsset *)object->asset,
                      object->sharedResource->textureHandle);
    }
    sdf3DPrimLinkMat(packet, (SdfAsset *)object->asset, 0);
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
    surface->append(surface, packet);
}

void dds3WhirlwindPolyListSetTex(PcpScatterDraw *object, u32 resource) {
    PcpScatterRes *shared;

    shared = dds3ScatterPolyListTexResourceCreate(resource);
    object->sharedResource = shared;
}

void dds3WhirlwindPolyListCopyTex(PcpScatterDraw *object, PcpScatterDraw *source) {
    PcpScatterRes *shared;

    shared = dds3ScatterPolyListTexResourceCopy(source->sharedResource);
    object->sharedResource = shared;
}

f32 *dds3WhirlwindPolyListGetVertex(PcpScatterDraw *object, s32 particleIndex) {
    return object->points + particleIndex * object->vectorsPerParticle * 4;
}

f32 *dds3WhirlwindPolyListGetUV(PcpScatterDraw *object, s32 particleIndex) {
    return object->uv + particleIndex * object->vectorsPerParticle * 2;
}

INCLUDE_ASM(const s32, "effect/effPCPScatter", dds3WhirlwindPolyListGetABGR);

INCLUDE_ASM(const s32, "effect/effPCPScatter", dds3WhirlwindPolyListSetRotateMatrix);
