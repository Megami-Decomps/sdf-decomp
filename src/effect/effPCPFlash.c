#include "common.h"
#include "ps2asm.h"

struct EffPacketParams;
struct EffRandState;
struct EffRecordPool;
struct EffRingColorSlot;
struct EffRingParticle;
struct EffRingWork;
struct EffThunderDrawParams;
struct PcpFlashAccumulatingParticle;
struct PcpFlashAccumulatingWork;
struct PcpFlashColorSlot;
struct PcpFlashFadingOrbitWork;
struct PcpFlashMotionParticle;
struct PcpFlashOffsetRadialParams;
struct PcpFlashOffsetRadialWork;
struct PcpFlashOrbitArcWork;
struct PcpFlashOrbitParticle;
struct PcpFlashPulseParticle;
struct PcpFlashQuadColorSlot;
struct PcpFlashRadialStripParticle;
struct PcpFlashRadialStripWork;
struct PcpFlashRadialTriangleParams;
struct PcpFlashRadialTriangleWork;
struct PcpFlashRotatingParticle;
struct PcpFlashRotatingQuadParticle;
struct PcpFlashRotatingQuadWork;
struct PcpFlashScalingOrbitWork;
struct PcpFlashStreakParams;
struct PcpFlashStreakWork;
struct PcpFlashTrianglePulseParams;
struct PcpFlashTrianglePulseWork;
struct sdfVector3_t;
struct sdfMatPacket_t;
union SdfDrawTransform;
struct SdfGsTextureStateValues;
struct sdfGsMemSlice_t;

typedef struct EffRandState EffRandState;
typedef union SdfDrawTransform SdfDrawTransform;
typedef struct SdfGsTextureStateValues SdfGsTextureStateValues;
typedef struct sdfMatPacket_t sdfMatPacket_t;
typedef union sdfMatUvwParm_t sdfMatUvwParm_t;
typedef struct sdfTexPacket_t sdfTexPacket_t;
typedef struct sdfGsMemSlice_t sdfGsMemSlice_t;
typedef struct sdfTexHandle_t sdfTexHandle_t;
typedef struct sdfMat_t sdfMat_t;
typedef struct sdfMemHandle_t sdfMemHandle_t;
typedef struct EffRingColorSlot EffRingColorSlot;
typedef struct EffRingParticle EffRingParticle;
typedef struct EffRingWork EffRingWork;
typedef struct PcpFlashColorSlot PcpFlashColorSlot;
typedef struct PcpFlashOffsetRadialParams PcpFlashOffsetRadialParams;
typedef struct PcpFlashPulseParticle PcpFlashPulseParticle;
typedef struct PcpFlashQuadColorSlot PcpFlashQuadColorSlot;
typedef struct PcpFlashRadialTriangleParams PcpFlashRadialTriangleParams;
typedef struct PcpFlashStreakParams PcpFlashStreakParams;
typedef struct PcpFlashTrianglePulseParams PcpFlashTrianglePulseParams;
typedef struct PcpFlashTrianglePulseWork PcpFlashTrianglePulseWork;
typedef struct sdfVector3_t sdfVector3_t;
typedef struct sdfDrawTag_t sdfDrawTag_t;
typedef struct sdfGrOt_t sdfGrOt_t;
typedef struct EffThunderDrawParams EffThunderDrawParams;
typedef struct EffPacketParams EffPacketParams;

#define EFF_POOL_HEADER_BYTES 0x70

#define EFF_PACKET_PARAMS_BYTES 0x2C

#define EFF_PACKET_LIST_BYTES 0x20

#define EFF_STATE_PACKET_BYTES 0x30

#define EFF_MATRIX_WORD_COUNT 16

#define EFF_FAN_VERTEX_COUNT 5

#define EFF_FAN_BATCH_VERTICES 0x19

#define EFF_FAN_BATCH_PARAMS 0xF

#define EFF_TRIANGLE_VERTEX_COUNT 3

#define EFF_TRIANGLE_BATCH_VERTICES 0x30

#define EFF_QUAD_VERTEX_COUNT 4

#define EFF_QUAD_BATCH_VERTICES 0x20

#define EFF_TRIANGLE_POSITION_BYTES 0x30

#define EFF_TRIANGLE_COLOR_BYTES 0xc

#define EFF_QUAD_POSITION_BYTES 0x40

#define EFF_QUAD_COLOR_BYTES 0x10

#define EFF_FAN_POSITION_BYTES 0x50

#define EFF_FAN_COLOR_BYTES 0x14

#define EFF_NEUTRAL_COLOR 0x80808080

#define EFF_RGB_MASK 0xFFFFFF

#define EFF_DIRECT_SURFACE_COUNT 4

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

/* Four ordered 32-bit words in the shared effect random-number state. */
struct EffRandState {
    u32 words[4];
};

typedef struct EffRecordPool EffRecordPool;

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

/* Native VRAM range descriptor (0x1C), shared by textures, graph buffers and streams. */
struct sdfGsMemSlice_t {
    struct sdfGsMemSlice_t *prev; /* 0x00 */
    struct sdfGsMemSlice_t *next; /* 0x04 */
    u32 allocationMode;         /* 0x08: known categories above; unknown values remain valid */
    u32 word;                   /* 0x0C: VRAM offset in 32-bit words */
    s32 size;                   /* 0x10: range length in 32-bit words */
    s16 width;                  /* 0x14 */
    s16 height;                 /* 0x16 */
    s32 format;                 /* 0x18: GS pixel-storage mode */
};

/* Serialized byte consumed by the GS texture-state packet builder. */
typedef u8 SdfTextureClampMode;

struct sdfTexHandle_t {
    struct sdfTexHandle_t *prev;
    struct sdfTexHandle_t *next;
    struct sdfTexHandle_t *origin;
    s16 width;
    s16 height;
    sdfGsMemSlice_t *primaryResource;
    sdfGsMemSlice_t *secondaryResource;
    u8 paletteCount;
    u8 clutFormat;
    u8 pixelFormat;
    u8 maxMipLevel;
    u16 lodParameters; /* Packed GS TEX1 L/K parameters. */
    u8 unk1E;
    SdfTextureClampMode clampMode; /* 0x1F: 0xFF leaves GS CLAMP state zero; otherwise low nibble is copied. */
    s32 resourceKey;
    s32 battleTextureSlot;
    sdfTexPacket_t *primaryBuffer;
    sdfTexPacket_t *secondaryBuffer;
    u8 *paletteData; /* 0x30: retained copy of the texture's palette colors. */
    s32 paletteDataSize; /* 0x34: palette byte count from sdfTexGetClutDataSize. */
    s32 unk38;
    u8 *intensityMap; /* 0x3C: lazily generated weighted-RGB intensity bytes. */
};

struct sdfMat_t {
    u32 unk00; /* Word from the serialized asset-parameter header. */
    u16 unk04; /* Halfword from the serialized asset-parameter header. */
    u8 dirtyFlags; /* 0x06: pending state-group changes for the two draw entries */
    u8 pad07;
    struct sdfMatPacket_t *drawEntries[2]; /* 0x08: owned 0xA0-byte draw entries */
    u32 primaryStateWordFirst; /* 0x10: first word set by primary-state updates */
    u32 primaryStateWordSecond; /* 0x14: second word set by primary-state updates */
    u32 secondaryColor; /* 0x18 */
    f32 primaryStateFloat; /* 0x1C: interpolated primary-state float */
    u32 primaryStateWordThird; /* 0x20 */
    u32 unk24;
    u32 primaryStateWordFourth; /* 0x28: retained as raw bits */
    sdfTexHandle_t *texture;
    sdfTexHandle_t *secondaryTexture; /* 0x30 */
    u32 secondaryMode;       /* 0x34: secondary packet and palette selector */
    sdfMatUvwParm_t *primarySubParam; /* 0x38 */
    sdfMatUvwParm_t *secondarySubParam; /* 0x3C */
    f32 scalarPairFirst; /* 0x40 */
    f32 scalarPairSecond; /* 0x44 */
};

struct sdfMemHandle_t;

/* Embedded command work at actor +0x20. Neither record is the whole owning
 * actor. */
struct sdfMemHandle_t;

/* General-heap allocation descriptor (0x10); links bound the represented span. */
struct sdfMemHandle_t {
    struct sdfMemHandle_t *prev; /* 0x00 */
    struct sdfMemHandle_t *next; /* 0x04 */
    s32 address;              /* 0x08: start of represented span */
    u16 state;                /* 0x0C: free, used, or end sentinel */
    s16 referenceCount;       /* 0x0E: -1 for sentinels */
};

/* Position/color arrays precede this render pool; constructors clear the full allocation. */
typedef struct EffRecordPool {
    f32 matrix[16];
    f32 origin[3];
    u8 pad4C[4];
    u32 drawMode; /* 0x50: packet submission surface */
    u32 color;
    s32 vertexCount; /* Positions/color words, not group count. */
    f32 scale;
    u8 *recordBase;
    u8 *auxRecordBase;
    sdfMat_t *resource;
    sdfMemHandle_t *buffer;
} EffRecordPool; /* 0x70 */

struct EffRingColorSlot {
    s32 colors[5];
};

struct EffRingParticle {
    u32 color;
    s32 age;
    f32 angle;
    f32 basisFactor;
};

/* The ring allocator copies 0x58 bytes of parameters into this 0x80-byte
 * owner. Its update passes this same work to the phase, color and fan helpers. */
struct EffRingWork {
    f32 origin[4];
    u32 count;
    u8 respawn;
    u8 pad15[3];
    s32 duration;
    s32 spread;
    u32 fadeIn;
    u32 fadeOut;
    u32 firstColor;
    u32 secondColor;
    f32 param30;
    f32 param34;
    f32 param38;
    f32 param3C;
    f32 param40;
    f32 param44;
    f32 param48;
    f32 param4C;
    f32 increment;
    u32 drawMode;
    EffRingParticle *vertices;
    u32 updateCount;
    u32 color;
    f32 scale;
    f32 currentRadius;
    f32 currentAxisOffset;
    f32 currentSideOffsetB;
    f32 currentSideOffsetA;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *recordPool;
};

typedef struct PcpFlashAccumulatingParticle PcpFlashAccumulatingParticle;

struct PcpFlashAccumulatingParticle {
    u32 color;
    s32 age;
    f32 radialHalfThickness;
    f32 span;
    f32 unk10;
    f32 radius;
    f32 orbitAngle;
};

typedef struct PcpFlashAccumulatingWork PcpFlashAccumulatingWork;

struct PcpFlashAccumulatingWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    u32 randomRange;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 spanBase;
    f32 unk34;
    f32 orbitRadius;
    f32 radialHalfThicknessBase;
    f32 tilt;
    f32 increment;
    f32 radialStep;
    u32 drawMode;
    PcpFlashAccumulatingParticle *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

/* Paired Flash runtime owners and their variable particle records.
 * Factory allocations place each particle array immediately after its owner. */
/* Three consecutive packed colors returned by dds3FlashPolyList2GetVC. */
struct PcpFlashColorSlot {
    s32 first;
    s32 second;
    s32 third;
};

typedef struct PcpFlashFadingOrbitWork PcpFlashFadingOrbitWork;

typedef struct PcpFlashOrbitParticle PcpFlashOrbitParticle;

struct PcpFlashOrbitParticle {
    u32 color;
    s32 age;
    f32 scale;
    f32 initialScale;
    f32 angle;
};

struct PcpFlashFadingOrbitWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 orbitRadius;
    f32 maxScale;
    f32 tilt;
    f32 angularStep;
    u32 drawMode;
    PcpFlashOrbitParticle *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

/* Shared 0x10-byte motion state: accumulator is an angle for orbit arcs and
   a radius for radial triangles; stepSpeed is radial speed or arc height. */
typedef struct PcpFlashMotionParticle PcpFlashMotionParticle;

struct PcpFlashMotionParticle {
    u32 color;
    s32 age;
    f32 accumulator;
    f32 stepSpeed; /* 0x0C: multiplied by decay each step, then added to
                       * accumulator; also read as the particle's height */
};

struct PcpFlashOffsetRadialParams {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 initialRadius;
    f32 initialRadialSpeed;
    f32 radialDamping;
    f32 originOffset;
    u32 drawMode;
};

typedef struct PcpFlashOffsetRadialWork PcpFlashOffsetRadialWork;

struct PcpFlashOffsetRadialWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 initialRadius;
    f32 initialRadialSpeed;
    f32 radialDamping;
    f32 originOffset;
    u32 drawMode;
    PcpFlashMotionParticle *parts;
    s32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

typedef struct PcpFlashOrbitArcWork PcpFlashOrbitArcWork;

/* The orbit constructor and renderer share this 0x80-byte work record.
   The 0x58-byte copied parameters are followed by the motion-particle array
   pointer and draw/resource state. */
struct PcpFlashOrbitArcWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    u32 randomRange;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpanStart;
    f32 acrossSpanStart;
    f32 orbitRadiusStart;
    f32 orbitRadiusEnd;
    f32 normalSpanStart;
    f32 normalSpanEnd;
    f32 heightStart;
    f32 heightEnd;
    f32 angularStep;
    u32 drawMode;
    PcpFlashMotionParticle *parts;
    u32 shapeProgressFrame;
    u32 tintColor;
    f32 renderScale;
    f32 orbitRadius;
    f32 normalSpan;
    f32 upSpan;
    f32 acrossSpan;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

struct PcpFlashPulseParticle {
    u32 color;
    s32 age;
    f32 scale;
    u8 pad0C[4];
};

struct PcpFlashQuadColorSlot {
    s32 color[5];
};

typedef struct PcpFlashRadialStripParticle PcpFlashRadialStripParticle;

struct PcpFlashRadialStripParticle {
    u32 color;
    s32 age;
    f32 angularStep;
    f32 thickness;
    f32 radius;
    f32 radialSpeed;
    f32 angle;
    f32 span;
};

typedef struct PcpFlashRadialStripWork PcpFlashRadialStripWork;

struct PcpFlashRadialStripWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    u32 randomRange;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 spanBase;
    f32 spanVariation;
    f32 maxThickness;
    f32 angularStepRange;
    f32 initialRadius;
    f32 initialRadialSpeed;
    f32 radialDamping;
    u32 drawMode;
    u8 pad50[0x80];
    PcpFlashRadialStripParticle *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

struct PcpFlashRadialTriangleParams {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 initialRadius;
    f32 initialRadialSpeed;
    f32 radialDamping;
    u32 drawMode;
};

typedef struct PcpFlashRadialTriangleWork PcpFlashRadialTriangleWork;

struct PcpFlashRadialTriangleWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 initialRadius;
    f32 initialRadialSpeed;
    f32 radialDamping;
    u32 drawMode;
    PcpFlashMotionParticle *parts;
    s32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

typedef struct PcpFlashRotatingParticle PcpFlashRotatingParticle;

struct PcpFlashRotatingParticle {
    u32 color;
    s32 age;
    f32 rotationStep;
    f32 scale;
    f32 position[3];
    f32 unk1C;
    f32 upSpan;
    f32 acrossSpan;
    f32 initialScale;
};

typedef struct PcpFlashRotatingQuadParticle PcpFlashRotatingQuadParticle;

struct PcpFlashRotatingQuadParticle {
    u32 color;
    s32 age;
    f32 angularStep;
    f32 scale;
    f32 angle;
    f32 upSpan;
    f32 acrossSpan;
    f32 initialScale;
};

typedef struct PcpFlashRotatingQuadWork PcpFlashRotatingQuadWork;

struct PcpFlashRotatingQuadWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    u8 pad24[0x04];
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 maxScale;
    f32 angularStepRange;
    u32 drawMode;
    PcpFlashRotatingQuadParticle *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

typedef struct PcpFlashScalingOrbitWork PcpFlashScalingOrbitWork;

struct PcpFlashScalingOrbitWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 orbitRadius;
    f32 maxScale;
    f32 tilt;
    f32 angularStep;
    u32 drawMode;
    PcpFlashOrbitParticle *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

/* The rotating-streak factory copies this 0x40-byte parameter prefix into
 * its larger work allocation before appending particle and runtime state. */
struct PcpFlashStreakParams {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 maxScale;
    f32 rotationStepRange;
    u32 drawMode;
};

typedef struct PcpFlashStreakWork PcpFlashStreakWork;

/* Spawn, rotation and draw passes share this 0x58-byte streak work.
   The position-rotation routine uses the same parts pointer at 0x40. */
struct PcpFlashStreakWork {
    f32 origin[3];
    u8 pad0C[0x04];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[0x03];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 maxScale;
    f32 rotationStepRange;
    u32 drawMode;
    PcpFlashRotatingParticle *parts;
    s32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

/* The factory copies this 0x30-byte parameter prefix into its larger work
 * allocation before appending particle and resource state. */
struct PcpFlashTrianglePulseParams {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 scaleRampTime;
    u32 colorA;
    u32 colorB;
    f32 maxScale;
    u32 drawMode;
};

/* effPCPFlashPolySphere2Create copies the 0x30-byte parameter prefix, then places
 * these 0x10-byte particles immediately after the 0x48-byte work. */
struct PcpFlashTrianglePulseWork {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 scaleRampTime;
    u32 colorA;
    u32 colorB;
    f32 maxScale;
    u32 drawMode;
    PcpFlashPulseParticle *parts;
    s32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    EffRecordPool *resourceHandle;
};

/* Three-component vector; the VU0 loaders read it as the xyz of a quadword. */
struct sdfVector3_t {
    f32 x;
    f32 y;
    f32 z;
};

struct sdfDrawTag_t;

struct sdfDrawTag_t {
    struct sdfDrawTag_t *nextList; /* CPU link between lists; first/last are DMA addresses. */
    u32 first;
    u32 last;
    u32 unkC;
    u32 firstReferenceSource; /* Optional first DMA reference prefix. */
    u32 secondReferenceSource; /* Optional second DMA reference prefix. */
    u32 unk18;
    u32 unk1C;
};

struct sdfGrOt_t;

/* Draw-surface pool entry (0x20); the SDK initializes and flushes this owner. */
struct sdfGrOt_t {
    struct sdfGrOt_t *next; /* 0x00 */
    sdfDrawTag_t *first;       /* 0x04 */
    sdfDrawTag_t *last;        /* 0x08 */
    u32 unkC;
    void (*append)(struct sdfGrOt_t *, sdfDrawTag_t *);      /* 0x10 */
    s32 (*prepend)(struct sdfGrOt_t *, s32, sdfDrawTag_t *); /* 0x14 */
    u32 unk18;
    u32 unk1C;
};

/*
 * Fill the x, y and z components of a float vector with one value (plain C;
 * the w component is left alone). Used by the PCP particle and strip
 * builders to set up per-axis scales before the VU0 multiply.
 */
#define VEC3_SPLAT(v, x) ((v)[0] = (x), (v)[1] = (x), (v)[2] = (x))

/*
 * 64-byte (4-quadword) copy through vf28-vf31, as the field/effect matrix
 * setters do. COP2 has no plain-C form; .set noreorder keeps ee-as from
 * moving the sqc2 into a delay slot.
 */
#define VU0_COPY_MATRIX(dst, src) __asm__ volatile ( \
    ".set noreorder\n\t" \
    "lqc2 vf28, 0(%1)\n\t" \
    "lqc2 vf29, 16(%1)\n\t" \
    "lqc2 vf30, 32(%1)\n\t" \
    "lqc2 vf31, 48(%1)\n\t" \
    "sqc2 vf28, 0(%0)\n\t" \
    "sqc2 vf29, 16(%0)\n\t" \
    "sqc2 vf30, 32(%0)\n\t" \
    "sqc2 vf31, 48(%0)\n\t" \
    ".set reorder" \
    : : "r" (dst), "r" (src) : "memory")

/* vf10 += vf12: the vf12 spare the flash builders park a vector in. */
#define VU0_ADD_VF10_VF12() __asm__ volatile ( \
    ".set noreorder\n\tvadd.xyzw vf10, vf10, vf12\n\t.set reorder")

/* Vertex-draw descriptor used by the thunder history and end-cap packets.
 * Both title providers clear the first 0x2C bytes and keep these globals in
 * 0x30-byte slots; the final four bytes remain opaque. */
struct EffThunderDrawParams {
    s16 primitiveCount; /* 0x00 */
    s16 pointCount;     /* 0x02 */
    u16 flags;          /* 0x04 */
    u8 opaque_06[2];
    u32 color;          /* 0x08 */
    const u32 *primitiveIndices; /* 0x0C */
    u128 *points;       /* 0x10 */
    u8 opaque_14[0xC];
    u32 *colors;        /* 0x20 */
    u8 opaque_24[0xC];
};

extern void *effPkt3DPrimMakePoly(EffThunderDrawParams *);

extern EffRandState dds3EffectBase[];

extern u32 D_0031F1E0[];

extern f32 effPCPFlashPolyPipeVertex[];

extern f32 effPCPFlashPolyRingExpVertex[];

/* Advance caller-owned state, returning its next 32-bit value. NULL selects
 * the shared effect RNG state. */
u32 dds3GetRandA(EffRandState *state);

/* Returns 24 random bits scaled to [0, 1); NULL selects the shared state. */
f32 dds3GetRandFloatA(EffRandState *state);

extern u32 effMagaCylinderCalcColor(u32 color, u32 colorB);

extern sdfGrOt_t *D_0031F298[];

extern u32 D_0031F250[];

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

extern EffPacketParams D_0056ACF8[];

extern f32 effPCPFlashPolyRingExpHVertex[];

extern sdfGrOt_t *D_0031F228[];

extern sdfGrOt_t *D_0031F2A8[];

extern sdfGrOt_t *D_0031F238[];

extern void *memcpy(void *dst, const void *src, u32 n);

extern void *memset(void *, s32, u32);

struct sdfMemHandle_t *sdfMemAllocDirect(s32 requestedBytes);

/* Reserve size bytes from the active packet buffer, rounded up to 16 bytes.
 * Returns the prior cursor as a 32-bit address; the buffer owns this storage. */
s32 sdfGrAllocPacketBuff(s32 size);

void sdfDrawTagLink(struct sdfDrawTag_t *list, u32 packetAddress);

/* Load axis-angle rotation into VU0 vf28-vf31. */
void SetRotateMatrixAxis(f32 angle, const sdfVector3_t *axis);

void sdf3DPrimLinkMat(sdfDrawTag_t *packetList, sdfMat_t *asset,
                              s32 (*allocatePacket)(s32));

void sdf3DPrimLinkTm(sdfDrawTag_t *packetList,
                           s32 (*allocatePacket)(s32));

struct sdfMat_t *sdfMatNew(void);

f32 sdfCos(f32 angle);

void sdfDrawTagInit(struct sdfDrawTag_t *list);

u32 sdfMemLock(struct sdfMemHandle_t *allocation);

void sdfMatSetConstant(struct sdfMat_t *asset, f32 value);

f32 sdfSin(f32 angle);

extern sdfGrOt_t sdfGrOtTable[85];

#define effFlashOtNode ((sdfGrOt_t *)&sdfGrOtTable[45])

PcpFlashTrianglePulseWork *effPCPFlashPolySphere2Create(PcpFlashTrianglePulseParams *params)
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashPulseParticle) + sizeof(PcpFlashTrianglePulseWork));
    PcpFlashTrianglePulseWork *work = (PcpFlashTrianglePulseWork *)sdfMemLock(allocation);
    EffRecordPool *record;
    u32 i;

    memcpy(work, params, 0x30);
    work->parts = (PcpFlashPulseParticle *)(work + 1);
    work->tintColor = 0x80808080;
    work->allocationHandle = allocation;
    work->renderScale = 1.0f;
    work->updateCount = 0;
    record = dds3FlashPolyList2Create(work->particleCount);
    work->resourceHandle = record;
    record->drawMode = work->drawMode;
    for (i = 0; i < work->particleCount; i++) {
        work->parts[i].age = 0;
    }
    return work;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2CreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2Copy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2Delete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2SetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2SetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2SetScale);

void effPCPFlashPolySphere2PartSetVC(PcpFlashTrianglePulseWork *work, s32 index, s32 color)
{
    PcpFlashColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashColorSlot *)dds3FlashPolyList2GetVC(work->resourceHandle, index);
    rgb1 = work->colorA & 0xFFFFFF;
    rgb2 = work->colorB & 0xFFFFFF;
    slot->first = effMagaCylinderCalcColor(rgb2, color);
    slot->second = effMagaCylinderCalcColor(rgb2, color);
    slot->third = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2PartSetVertex);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2Draw);

PcpFlashStreakWork *effPCPFlashPolySphereCreate(PcpFlashStreakParams *src) {
    sdfMemHandle_t *handle = sdfMemAllocDirect(src->particleCount * sizeof(PcpFlashRotatingParticle) + sizeof(PcpFlashStreakWork));
    PcpFlashStreakWork *work = (PcpFlashStreakWork *)sdfMemLock(handle);
    EffRecordPool *record;
    u32 range;
    u32 particleIndex;

    memcpy(work, src, 0x40);
    work->parts = (PcpFlashRotatingParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = 0x80808080;
    work->renderScale = 1.0f;
    work->updateCount = 0;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    record = dds3FlashPolyListCreate(work->particleCount);
    work->resourceHandle = record;
    record->drawMode = work->drawMode;
    range = work->randomRange;
    for (particleIndex = 0; particleIndex < work->particleCount; particleIndex++) {
        work->parts[particleIndex].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
    }
    return work;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereSetScale);

void effPCPFlashPolySpherePartSetVC(PcpFlashStreakWork *work, s32 index, s32 color)
{
    PcpFlashQuadColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashQuadColorSlot *)dds3FlashPolyListGetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->color[0] = effMagaCylinderCalcColor(rgb2, color);
    slot->color[1] = effMagaCylinderCalcColor(rgb2, color);
    if (index & 1) {
        slot->color[2] = effMagaCylinderCalcColor(0x80000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0x80000000, color);
    } else {
        slot->color[2] = effMagaCylinderCalcColor(0xFF000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0x40000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0xFF000000, color);
    }
}

void effPCPFlashPolySpherePartInit(PcpFlashStreakWork *work, s32 index, void *orientation) {
    PcpFlashRotatingParticle *part = work->parts + index;
    f32 direction[4];
    f32 factor;
    f32 scale;

    direction[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    direction[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    direction[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    /* Two plain quadword loads, no memory clobber: retail keeps `direction`
     * and `orientation` CSE'd across them. */
    VU0_LOAD_VF10(direction);
    VU0_LOAD_VF11(orientation);
    VU0_CROSS_VF10_VF11();
        VU0_NORMALIZE_VF10();;
    VU0_STORE_VF10(direction);
    part->position[0] = direction[0];
    part->position[1] = direction[1];
    part->position[2] = direction[2];
    factor = dds3GetRandFloatA(dds3EffectBase) * 0.3f + 0.7f;
    scale = work->maxScale * factor;
    part->initialScale = scale;
    part->scale = scale;
    factor = (dds3GetRandFloatA(dds3EffectBase) * 0.5f + 0.5f) * 0.5f;
    part->upSpan = work->upSpan * factor;
    part->acrossSpan = work->acrossSpan * factor;
    part->rotationStep = work->rotationStepRange * ((dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySpherePartSetVertex);

void effPCPFlashPolySpherePartRotate(PcpFlashStreakWork *work, s32 index, void *orientation)
{
    PcpFlashRotatingParticle *part = &work->parts[index];
    f32 position[4];

    position[0] = part->position[0];
    position[1] = part->position[1];
    position[2] = part->position[2];
    SetRotateMatrixAxis(part->rotationStep, (const sdfVector3_t *)orientation);
    VU0_LOAD_VF10(position);
    VU0_ROTATE_VF10();
    VU0_STORE_VF10(position);
    part->position[0] = position[0];
    part->position[1] = position[1];
    part->position[2] = position[2];
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingCreate);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingSetScale);

void effPCPFlashPolyRingPartSetVC(PcpFlashScalingOrbitWork *work, s32 index, s32 color)
{
    PcpFlashQuadColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashQuadColorSlot *)dds3FlashPolyListGetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->color[0] = effMagaCylinderCalcColor(rgb2, color);
    slot->color[1] = effMagaCylinderCalcColor(rgb2, color);
    if (index & 1) {
        slot->color[2] = effMagaCylinderCalcColor(0x80000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0x80000000, color);
    } else {
        slot->color[2] = effMagaCylinderCalcColor(0xFF000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0x40000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0xFF000000, color);
    }
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingPartSetVertex);

void effPCPFlashPolyRingPartRotate(PcpFlashScalingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part;

    part = &work->parts[index];
    part->angle += work->angularStep;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeCreate);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeSetScale);

void effPCPFlashPolyPipePartSetVC(PcpFlashAccumulatingWork *work, s32 index, u32 colorMultiplier) {
    u32 *colors;
    u32 colorA;
    u32 colorB;
    u32 *mirror;

    colors = (u32 *)dds3FlashPolyList3GetVC(work->resourceHandle, index * 2);
    colorA = work->centerColor & 0xFFFFFF;
    colorB = work->edgeColor & 0xFFFFFF;
    colors[0] = effMagaCylinderCalcColor(colorB | 0x80000000, colorMultiplier);
    colors[1] = effMagaCylinderCalcColor(colorB, colorMultiplier);
    colors[2] = effMagaCylinderCalcColor(colorA | 0x80000000, colorMultiplier);
    colors[3] = effMagaCylinderCalcColor(colorB, colorMultiplier);
    mirror = (u32 *)dds3FlashPolyList3GetVC(work->resourceHandle, index * 2 + 1);
    mirror[0] = colors[0];
    mirror[1] = colors[1];
    mirror[2] = colors[2];
    mirror[3] = colors[3];
}

/* vu0 routine: the orbit offset plus a tilted disc of corner offsets for an
   accumulating particle, whose radius grows by the work record's radial step */
void effPCPFlashPolyPipePartSetVertex(PcpFlashAccumulatingWork *work, s32 index)
{
    PcpFlashAccumulatingParticle *part = &work->parts[index];
    f32 *quad = dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2);
    f32 offset[4];
    f32 unit[4];
    f32 middle[4];
    f32 outer[4];
    f32 inner[4];
    f32 size[4];
    f32 center;
    f32 outerEdge;
    f32 innerEdge;
    f32 span;
    f32 sinv;
    f32 height;
    f32 *mirror;

    center = part->radius;
    VEC3_SPLAT(middle, center);
    outerEdge = center + part->radialHalfThickness;
    VEC3_SPLAT(outer, outerEdge);
    innerEdge = center - part->radialHalfThickness;
    VEC3_SPLAT(inner, innerEdge);
    part->radius = part->radius + work->radialStep;
    span = part->span;
    VEC3_SPLAT(size, span);
    unit[0] = sdfCos(part->orbitAngle);
    unit[1] = 0;
    sinv = sdfSin(part->orbitAngle);
    unit[2] = sinv;
    offset[0] = unit[0] * work->orbitRadius;
    offset[1] = 0;
    offset[2] = sinv * work->orbitRadius;
    height = work->tilt;
    effPCPFlashPolyPipeVertex[0] = unit[0] * height;
    effPCPFlashPolyPipeVertex[1] = height + -1.0f;
    effPCPFlashPolyPipeVertex[2] = sinv * height;
    VU0_LOAD_VF10(effPCPFlashPolyPipeVertex);
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(unit);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_LOAD_VF10(size);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(size);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(outer);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(outer);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(inner);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(inner);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(middle);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(offset);
    VU0_ADD_VF10_VF12();
    VU0_LOAD_VF11(size);
    VU0_STORE_VF10(quad + 8);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(quad + 4);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(quad + 12);
    VU0_LOAD_VF10(offset);
    VU0_LOAD_VF11(outer);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(quad);
    mirror = dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2 + 1);
    PS2_COPY_QUAD(mirror + 8, quad + 8);
    PS2_COPY_QUAD(mirror + 4, quad + 4);
    PS2_COPY_QUAD(mirror + 12, quad + 12);
    VU0_LOAD_VF10(offset);
    VU0_LOAD_VF11(inner);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(mirror);
}

void effPCPFlashPolyPipePartRotate(PcpFlashAccumulatingWork *work, s32 index)
{
    PcpFlashAccumulatingParticle *part;

    part = &work->parts[index];
    part->orbitAngle += work->increment;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyPipeDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpCreate);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpSetScale);

void effPCPFlashPolyRingExpPartSetVC(PcpFlashOrbitArcWork *work, s32 index, s32 color)
{
    PcpFlashQuadColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashQuadColorSlot *)dds3FlashPolyListGetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->color[0] = effMagaCylinderCalcColor(rgb2, color);
    slot->color[1] = effMagaCylinderCalcColor(rgb2, color);
    if (index & 1) {
        slot->color[2] = effMagaCylinderCalcColor(0x80000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0x80000000, color);
    } else {
        slot->color[2] = effMagaCylinderCalcColor(0xFF000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0x40000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0xFF000000, color);
    }
}

/* vu0 routine: billboard corner offsets for a particle on an arc, built from a normalised direction and its perpendicular */
void effPCPFlashPolyRingExpPartSetVertex(PcpFlashOrbitArcWork *work, s32 index)
{
    PcpFlashMotionParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyListGetVertex(work->resourceHandle, index);
    f32 offset[4];
    f32 unit[4];
    f32 scaleA[4];
    f32 scaleB[4];
    f32 scaleC[4];
    f32 sinv;
    f32 height;

    VEC3_SPLAT(scaleA, work->normalSpan);
    VEC3_SPLAT(scaleB, work->acrossSpan);
    VEC3_SPLAT(scaleC, work->upSpan);
    unit[0] = sdfCos(part->accumulator);
    unit[1] = 0;
    sinv = sdfSin(part->accumulator);
    unit[2] = sinv;
    offset[0] = unit[0] * work->orbitRadius;
    offset[1] = 0;
    offset[2] = sinv * work->orbitRadius;
    height = part->stepSpeed;
    effPCPFlashPolyRingExpVertex[0] = unit[0] * height;
    effPCPFlashPolyRingExpVertex[1] = height + -1.0f;
    effPCPFlashPolyRingExpVertex[2] = sinv * height;
    VU0_LOAD_VF10(effPCPFlashPolyRingExpVertex);
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(unit);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_LOAD_VF10(scaleB);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(scaleB);
    VU0_LOAD_VF10(scaleC);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(scaleC);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(scaleA);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(offset);
    VU0_LOAD_VF11(scaleC);
    VU0_STORE_VF10(quad + 12);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(quad + 8);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(quad + 16);
    VU0_LOAD_VF10(offset);
    VU0_LOAD_VF11(scaleB);
    VU0_ADD_VF10_VF12();
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(quad);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(quad + 4);
}

void effPCPFlashPolyRingExpPartRotate(PcpFlashOrbitArcWork *work, s32 index)
{
    PcpFlashMotionParticle *part;

    part = &work->parts[index];
    part->accumulator += work->angularStep;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpCreate);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpSetScale);

void effPCPFlashPolySphereExpPartSetVC(PcpFlashRotatingQuadWork *work, s32 index, s32 color)
{
    PcpFlashQuadColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashQuadColorSlot *)dds3FlashPolyListGetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->color[0] = effMagaCylinderCalcColor(rgb2, color);
    slot->color[1] = effMagaCylinderCalcColor(rgb2, color);
    if (index & 1) {
        slot->color[2] = effMagaCylinderCalcColor(0x80000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0x80000000, color);
    } else {
        slot->color[2] = effMagaCylinderCalcColor(0xFF000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0x40000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0xFF000000, color);
    }
}

void effPCPFlashPolySphereExpPartInit(PcpFlashRotatingQuadWork *work, s32 index, void *orientation) {
    PcpFlashRotatingQuadParticle *part = work->parts + index;
    f32 factor;
    f32 scale;

    part->angle = dds3GetRandFloatA(dds3EffectBase) * 6.2831853f;
    factor = dds3GetRandFloatA(dds3EffectBase) * 0.3f + 0.7f;
    scale = work->maxScale * factor;
    part->initialScale = scale;
    part->scale = scale;
    factor = (dds3GetRandFloatA(dds3EffectBase) * 0.5f + 0.5f) * 0.5f;
    part->upSpan = work->upSpan * factor;
    part->acrossSpan = work->acrossSpan * factor;
    part->angularStep = work->angularStepRange * ((dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpPartSetVertex);

void effPCPFlashPolySphereExpPartRotate(PcpFlashRotatingQuadWork *work, s32 index, void *orientation)
{
    PcpFlashRotatingQuadParticle *part;

    part = &work->parts[index];
    part->angle += part->angularStep;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphereExpDraw);

PcpFlashRadialTriangleWork *effPCPFlashPolySphere2ExpCreate(PcpFlashRadialTriangleParams *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashMotionParticle) + sizeof(PcpFlashRadialTriangleWork));
    PcpFlashRadialTriangleWork *work = (PcpFlashRadialTriangleWork *)sdfMemLock(handle);
    EffRecordPool *record;
    u32 i;

    memcpy(work, params, 0x3C);
    work->parts = (PcpFlashMotionParticle *)(work + 1);
    work->tintColor = 0x80808080;
    work->allocationHandle = handle;
    work->renderScale = 1.0f;
    work->updateCount = 0;
    record = dds3FlashPolyList2Create(work->particleCount);
    work->resourceHandle = record;
    record->drawMode = work->drawMode;
    for (i = 0; i < work->particleCount; i++) {
        work->parts[i].age = 0;
    }
    return work;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpSetScale);

void effPCPFlashPolySphere2ExpPartSetVC(PcpFlashRadialTriangleWork *work, s32 index, s32 color)
{
    PcpFlashColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashColorSlot *)dds3FlashPolyList2GetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->first = effMagaCylinderCalcColor(rgb2, color);
    slot->second = effMagaCylinderCalcColor(rgb2, color);
    slot->third = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpPartSetVertex);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere2ExpDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3Create);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3CreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3Copy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3Delete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3SetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3SetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3SetScale);

void effPCPFlashPolySphere3PartSetVC(PcpFlashRadialStripWork *work, s32 index, u32 colorMultiplier) {
    u32 *colors;
    u32 colorA;
    u32 colorB;
    u32 *mirror;

    colors = (u32 *)dds3FlashPolyList3GetVC(work->resourceHandle, index * 2);
    colorA = work->centerColor & 0xFFFFFF;
    colorB = work->edgeColor & 0xFFFFFF;
    colors[0] = effMagaCylinderCalcColor(colorB | 0x80000000, colorMultiplier);
    colors[1] = effMagaCylinderCalcColor(colorB, colorMultiplier);
    colors[2] = effMagaCylinderCalcColor(colorA | 0x80000000, colorMultiplier);
    colors[3] = effMagaCylinderCalcColor(colorB, colorMultiplier);
    mirror = (u32 *)dds3FlashPolyList3GetVC(work->resourceHandle, index * 2 + 1);
    mirror[0] = colors[0];
    mirror[1] = colors[1];
    mirror[2] = colors[2];
    mirror[3] = colors[3];
}

void effPCPFlashPolySphere3PartInit(PcpFlashRadialStripWork *work, s32 index, void *orientation) {
    PcpFlashRadialStripParticle *part = work->parts + index;
    f32 factor;

    part->angle = dds3GetRandFloatA(dds3EffectBase) * 6.2831853f;
    factor = dds3GetRandFloatA(dds3EffectBase) * 0.3f + 0.7f;
    part->thickness = work->maxThickness * factor;
    factor = work->spanVariation;
    part->span = work->spanBase * (dds3GetRandFloatA(dds3EffectBase) * factor + (1.0f - factor));
    part->angularStep = work->angularStepRange * ((dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f);
}

/* vu0 routine: two quads of corner offsets for a flash particle (a strip and its mirror), turned around the view axis by the particle's angle */
void effPCPFlashPolySphere3PartSetVertex(PcpFlashRadialStripWork *work, s32 index, void *view)
{
    PcpFlashRadialStripParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2);
    f32 base[4];
    f32 size[4];
    f32 spare[4];
    f32 middle[4];
    f32 outer[4];
    f32 inner[4];
    f32 center;
    f32 outerEdge;
    f32 innerEdge;
    f32 span;
    f32 *mirror;

    center = part->radius;
    VEC3_SPLAT(middle, center);
    outerEdge = center + part->thickness;
    VEC3_SPLAT(outer, outerEdge);
    innerEdge = center - part->thickness;
    VEC3_SPLAT(inner, innerEdge);
    span = part->span;
    VEC3_SPLAT(size, span);
    SetRotateMatrixAxis(part->angle, (const sdfVector3_t *)view);
    base[0] = 0;
    base[1] = 1.0f;
    base[2] = 0;
    VU0_LOAD_VF10(base);
    VU0_LOAD_VF11(view);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_ROTATE_VF10();
    VU0_COPY_VF10_TO_VF12();
    /* retail multiplies the (never written) `spare` slot here and stores it back */
    VU0_LOAD_VF11(spare);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(spare);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(view);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_LOAD_VF10(size);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(size);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(outer);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(outer);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(inner);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(inner);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(middle);
    VU0_MUL_VF10_VF11();
    VU0_LOAD_VF11(size);
    VU0_STORE_VF10(quad + 8);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(quad + 4);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(quad + 12);
    VU0_LOAD_VF10(outer);
    VU0_STORE_VF10(quad);
    mirror = (f32 *)dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2 + 1);
    PS2_COPY_QUAD(mirror + 8, quad + 8);
    PS2_COPY_QUAD(mirror + 4, quad + 4);
    PS2_COPY_QUAD(mirror + 12, quad + 12);
    VU0_LOAD_VF10(inner);
    VU0_STORE_VF10(mirror);
}

void effPCPFlashPolySphere3PartRotate(PcpFlashRadialStripWork *work, s32 index, void *orientation)
{
    PcpFlashRadialStripParticle *part;

    part = &work->parts[index];
    part->angle += part->angularStep;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolySphere3Draw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2Create);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2CreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2Copy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2Delete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2SetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2SetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2SetScale);

void effPCPFlashPolyRingExp2PartSetVC(PcpFlashFadingOrbitWork *work, s32 index, s32 color)
{
    PcpFlashQuadColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashQuadColorSlot *)dds3FlashPolyListGetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->color[0] = effMagaCylinderCalcColor(rgb2, color);
    slot->color[1] = effMagaCylinderCalcColor(rgb2, color);
    if (index & 1) {
        slot->color[2] = effMagaCylinderCalcColor(0x80000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0x80000000, color);
    } else {
        slot->color[2] = effMagaCylinderCalcColor(0xFF000000, color);
        slot->color[3] = effMagaCylinderCalcColor(rgb1 | 0x40000000, color);
        slot->color[4] = effMagaCylinderCalcColor(0xFF000000, color);
    }
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2PartSetVertex);

void effPCPFlashPolyRingExp2PartRotate(PcpFlashFadingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part;

    part = &work->parts[index];
    part->angle += work->angularStep;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExp2Draw);

PcpFlashOffsetRadialWork *effPCPFlashPolyCircleCreate(PcpFlashOffsetRadialParams *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashMotionParticle) + sizeof(PcpFlashOffsetRadialWork));
    PcpFlashOffsetRadialWork *work = (PcpFlashOffsetRadialWork *)sdfMemLock(handle);
    EffRecordPool *record;
    u32 i;

    memcpy(work, params, 0x40);
    work->parts = (PcpFlashMotionParticle *)(work + 1);
    work->tintColor = 0x80808080;
    work->allocationHandle = handle;
    work->renderScale = 1.0f;
    work->updateCount = 0;
    record = dds3FlashPolyList2Create(work->particleCount);
    work->resourceHandle = record;
    record->drawMode = work->drawMode;
    for (i = 0; i < work->particleCount; i++) {
        work->parts[i].age = 0;
    }
    return work;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleSetScale);

void effPCPFlashPolyCirclePartSetVC(PcpFlashOffsetRadialWork *work, s32 index, s32 color)
{
    PcpFlashColorSlot *slot;
    s32 rgb1;
    s32 rgb2;

    slot = (PcpFlashColorSlot *)dds3FlashPolyList2GetVC(work->resourceHandle, index);
    rgb1 = work->centerColor & 0xFFFFFF;
    rgb2 = work->edgeColor & 0xFFFFFF;
    slot->first = effMagaCylinderCalcColor(rgb2, color);
    slot->second = effMagaCylinderCalcColor(rgb2, color);
    slot->third = effMagaCylinderCalcColor(rgb1 | 0xFF000000, color);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCirclePartSetVertex);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyCircleDraw);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHCreate);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHCreatePCF);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHCopy);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHDelete);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHSetPosition);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHSetColor);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHSetScale);

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHSetRotateMatrix);

void effPCPFlashPolyRingExpHPartSetVC(EffRingWork *work, s32 index, s32 modulationColor) {
    EffRingColorSlot *slot;
    s32 firstRgb;
    s32 secondRgb;

    slot = (EffRingColorSlot *)dds3FlashPolyListHGetVC(work->recordPool, index);
    firstRgb = work->firstColor & EFF_RGB_MASK;
    secondRgb = work->secondColor & EFF_RGB_MASK;
    slot->colors[0] = effMagaCylinderCalcColor(secondRgb, modulationColor);
    slot->colors[1] = effMagaCylinderCalcColor(secondRgb, modulationColor);
    if (index & 1) {
        slot->colors[2] = effMagaCylinderCalcColor(0x80000000, modulationColor);
        slot->colors[3] = effMagaCylinderCalcColor(firstRgb | 0xFF000000, modulationColor);
        slot->colors[4] = effMagaCylinderCalcColor(0x80000000, modulationColor);
    } else {
        slot->colors[2] = effMagaCylinderCalcColor(0xFF000000, modulationColor);
        slot->colors[3] = effMagaCylinderCalcColor(firstRgb | 0x40000000, modulationColor);
        slot->colors[4] = effMagaCylinderCalcColor(0xFF000000, modulationColor);
    }
}

/* vu0 routine: build five orbiting fan positions from a phase-derived basis.
 * No camera matrix is read. Quadword loads retain the native scratch W values. */
void effPCPFlashPolyRingExpHPartSetVertex(EffRingWork *work, s32 index) {
    EffRingParticle *part = &work->vertices[index];
    f32 *positions = (f32 *)dds3FlashPolyListHGetVertex(work->recordPool, index);
    f32 orbitOffset[4];
    f32 radialDirection[4];
    f32 axisOffset[4];
    f32 sideOffsetA[4];
    f32 sideOffsetB[4];
    f32 sine;
    f32 basisFactor;

    VEC3_SPLAT(axisOffset, work->currentAxisOffset);
    VEC3_SPLAT(sideOffsetA, work->currentSideOffsetA);
    VEC3_SPLAT(sideOffsetB, work->currentSideOffsetB);
    radialDirection[0] = sdfCos(part->angle);
    radialDirection[1] = 0;
    sine = sdfSin(part->angle);
    radialDirection[2] = sine;
    orbitOffset[0] = radialDirection[0] * work->currentRadius;
    orbitOffset[1] = 0;
    orbitOffset[2] = sine * work->currentRadius;
    basisFactor = part->basisFactor;
    effPCPFlashPolyRingExpHVertex[0] = radialDirection[0] * basisFactor;
    effPCPFlashPolyRingExpHVertex[1] = basisFactor + -1.0f;
    effPCPFlashPolyRingExpHVertex[2] = sine * basisFactor;
    VU0_LOAD_VF10(effPCPFlashPolyRingExpHVertex);
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(radialDirection);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_COPY_VF10_TO_VF11();
    VU0_LOAD_VF10(sideOffsetA);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(sideOffsetA);
    VU0_LOAD_VF10(sideOffsetB);
    VU0_MUL_VF10_VF11();
    VU0_STORE_VF10(sideOffsetB);
    VU0_COPY_VF12_TO_VF10();
    VU0_LOAD_VF11(axisOffset);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF12();
    VU0_LOAD_VF10(orbitOffset);
    VU0_LOAD_VF11(sideOffsetB);
    VU0_STORE_VF10(positions + 12);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(positions + 8);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(positions + 16);
    VU0_LOAD_VF10(orbitOffset);
    VU0_LOAD_VF11(sideOffsetA);
    VU0_ADD_VF10_VF12();
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(positions);
    VU0_SUB_VF10_VF11();
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(positions + 4);
}

void effPCPFlashPolyRingExpHPartRotate(EffRingWork *work, s32 index) {
    EffRingParticle *particle;

    particle = &work->vertices[index];
    particle->angle = particle->angle + work->increment;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", effPCPFlashPolyRingExpHDraw);

EffRecordPool *dds3FlashPolyListCreate(u32 count) {
    EffRecordPool *pool;
    sdfMemHandle_t *handle;
    u8 *block;
    s32 vertexCount;
    s32 recordBytes;
    s32 size;
    s32 quads;

    vertexCount = count * 5;
    recordBytes = vertexCount * 5 * 4;
    quads = recordBytes >> 4;
    recordBytes = ((recordBytes & 0xF) != 0 ? quads + 1 : quads) << 4;
    size = recordBytes + EFF_POOL_HEADER_BYTES;
    handle = sdfMemAllocDirect(size);
    block = (u8 *)sdfMemLock(handle);
    memset(block, 0, size);
    pool = (EffRecordPool *)(block + recordBytes);
    pool->recordBase = (u8 *)block;
    pool->auxRecordBase = (u8 *)(block + vertexCount * 16);
    pool->color = EFF_NEUTRAL_COLOR;
    pool->drawMode = 2;
    pool->vertexCount = vertexCount;
    pool->buffer = handle;
    pool->scale = 1.0f;
    pool->resource = sdfMatNew();
    sdfMatSetConstant(pool->resource, 1.0f);
    memset(D_0056ACF8, 0, EFF_PACKET_PARAMS_BYTES);
    D_0056ACF8->primitive = 0x4000;
    return pool;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListDelete);

void dds3FlashPolyListDraw(EffRecordPool *work)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT] __attribute__((aligned(16)));
    sdfDrawTag_t *list;
    s32 remaining;
    s32 fanCount;
    u64 *packet;
    sdfDrawTag_t *stateList;

    list = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    sdfDrawTagInit(list);
    EE_MMI_UNIT_MATRIX(matrix);
    matrix[0] = work->scale;
    matrix[5] = work->scale;
    matrix[10] = work->scale;
    matrix[12] = work->origin[0];
    matrix[13] = work->origin[1];
    matrix[14] = work->origin[2];
    VU0_LOAD_MATRIX(matrix);
    sdf3DPrimLinkTm(list, 0);
    sdf3DPrimLinkMat(list, work->resource, 0);
    remaining = work->vertexCount;
    D_0056ACF8->colors = (u32 *)work->auxRecordBase;
    D_0056ACF8->positions = (u128 *)work->recordBase;
    D_0056ACF8->color = work->color;
    D_0056ACF8->parameterCount = EFF_FAN_BATCH_PARAMS;
    D_0056ACF8->vertexCount = EFF_FAN_BATCH_VERTICES;
    D_0056ACF8->parameters = D_0031F1E0;
    while (remaining >= EFF_FAN_BATCH_VERTICES) {
        remaining -= EFF_FAN_BATCH_VERTICES;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
        D_0056ACF8->positions += EFF_FAN_BATCH_VERTICES;
        D_0056ACF8->colors += EFF_FAN_BATCH_VERTICES;
    }
    if (remaining >= EFF_FAN_VERTEX_COUNT) {
        fanCount = remaining / EFF_FAN_VERTEX_COUNT;
        D_0056ACF8->vertexCount = remaining;
        D_0056ACF8->parameterCount = fanCount * 3;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
    }
    if (work->drawMode < EFF_DIRECT_SURFACE_COUNT) {
        D_0031F228[work->drawMode]->append(D_0031F228[work->drawMode], list);
    } else {
        stateList = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
        sdfDrawTagInit(stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[4] = 6;
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[5] = 0x42;
        sdfDrawTagLink(stateList, (u32)packet);
        (*effFlashOtNode).append(&(*effFlashOtNode), stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x42;
        packet[5] = 0x42;
        sdfDrawTagLink(list, (u32)packet);
        (*effFlashOtNode).append(&(*effFlashOtNode), list);
    }
}

void *dds3FlashPolyListGetVertex(EffRecordPool *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_FAN_POSITION_BYTES;
}

void *dds3FlashPolyListGetVC(EffRecordPool *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_FAN_COLOR_BYTES;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListSetBlend);

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListSetABGR);

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListSetScale);

EffRecordPool *dds3FlashPolyList2Create(s32 triangleCount) {
    EffRecordPool *pool;
    sdfMemHandle_t *handle;
    u32 *block;
    s32 vertexCount;
    s32 positionWordCount;
    s32 colorWordCount;
    u32 size;

    vertexCount = triangleCount * EFF_TRIANGLE_VERTEX_COUNT;
    positionWordCount = vertexCount * 4;
    colorWordCount = vertexCount;
    size = (positionWordCount + colorWordCount) * 4 + EFF_POOL_HEADER_BYTES;
    handle = sdfMemAllocDirect(size);
    block = (u32 *)sdfMemLock(handle);
    memset(block, 0, size);
    pool = (EffRecordPool *)(block + (positionWordCount + colorWordCount));
    pool->recordBase = (u8 *)block;
    pool->auxRecordBase = (u8 *)(block + positionWordCount);
    pool->color = EFF_NEUTRAL_COLOR;
    pool->drawMode = 2;
    pool->vertexCount = colorWordCount;
    pool->buffer = handle;
    pool->scale = 1.0f;
    pool->resource = sdfMatNew();
    sdfMatSetConstant(pool->resource, 1.0f);
    memset(D_0056ACF8, 0, EFF_PACKET_PARAMS_BYTES);
    D_0056ACF8->primitive = 0x4000;
    return pool;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyList2Delete);

void dds3FlashPolyList2Draw(EffRecordPool *pool)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT];
    sdfDrawTag_t *packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    s32 remainingVertices;
    sdfGrOt_t *surface;

    sdfDrawTagInit(packet);
    EE_MMI_UNIT_MATRIX(matrix);
    matrix[0] = pool->scale;
    matrix[5] = pool->scale;
    matrix[10] = pool->scale;
    matrix[12] = pool->origin[0];
    matrix[13] = pool->origin[1];
    matrix[14] = pool->origin[2];
    VU0_LOAD_MATRIX(matrix);
    sdf3DPrimLinkTm(packet, 0);
    sdf3DPrimLinkMat(packet, pool->resource, 0);
    remainingVertices = pool->vertexCount;
    D_0056ACF8->colors = (u32 *)pool->auxRecordBase;
    D_0056ACF8->positions = (u128 *)pool->recordBase;
    D_0056ACF8->color = pool->color;
    D_0056ACF8->parameterCount = 0x10;
    D_0056ACF8->vertexCount = EFF_TRIANGLE_BATCH_VERTICES;
    D_0056ACF8->parameters = NULL;
    while (remainingVertices >= EFF_TRIANGLE_BATCH_VERTICES) {
        remainingVertices -= EFF_TRIANGLE_BATCH_VERTICES;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
        D_0056ACF8->positions += EFF_TRIANGLE_BATCH_VERTICES;
        D_0056ACF8->colors += EFF_TRIANGLE_BATCH_VERTICES;
    }
    if (remainingVertices >= EFF_TRIANGLE_VERTEX_COUNT) {
        D_0056ACF8->parameterCount = remainingVertices / EFF_TRIANGLE_VERTEX_COUNT;
        D_0056ACF8->vertexCount = remainingVertices;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
    }
    surface = D_0031F238[pool->drawMode];
    surface->append(surface, packet);
}

void *dds3FlashPolyList2GetVertex(EffRecordPool *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_TRIANGLE_POSITION_BYTES;
}

void *dds3FlashPolyList2GetVC(EffRecordPool *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_TRIANGLE_COLOR_BYTES;
}

EffRecordPool *dds3FlashPolyList3Create(s32 quadCount) {
    EffRecordPool *pool;
    sdfMemHandle_t *handle;
    u32 *block;
    s32 positionWordCount;
    s32 colorWordCount;
    u32 size;

    positionWordCount = quadCount * 16;
    colorWordCount = quadCount * EFF_QUAD_VERTEX_COUNT;
    size = (positionWordCount + colorWordCount) * 4 + EFF_POOL_HEADER_BYTES;
    handle = sdfMemAllocDirect(size);
    block = (u32 *)sdfMemLock(handle);
    memset(block, 0, size);
    pool = (EffRecordPool *)(block + (positionWordCount + colorWordCount));
    pool->recordBase = (u8 *)block;
    pool->drawMode = 2;
    pool->auxRecordBase = (u8 *)(block + positionWordCount);
    pool->vertexCount = colorWordCount;
    pool->buffer = handle;
    pool->scale = 1.0f;
    pool->color = EFF_NEUTRAL_COLOR;
    pool->resource = sdfMatNew();
    sdfMatSetConstant(pool->resource, 1.0f);
    memset(D_0056ACF8, 0, EFF_PACKET_PARAMS_BYTES);
    D_0056ACF8->primitive = 0x4000;
    return pool;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyList3Delete);

void dds3FlashPolyList3Draw(EffRecordPool *pool)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT];
    sdfDrawTag_t *packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    s32 remainingVertices;
    sdfGrOt_t *surface;

    sdfDrawTagInit(packet);
    EE_MMI_UNIT_MATRIX(matrix);
    matrix[0] = pool->scale;
    matrix[5] = pool->scale;
    matrix[10] = pool->scale;
    matrix[12] = pool->origin[0];
    matrix[13] = pool->origin[1];
    matrix[14] = pool->origin[2];
    VU0_LOAD_MATRIX(matrix);
    sdf3DPrimLinkTm(packet, 0);
    sdf3DPrimLinkMat(packet, pool->resource, 0);
    remainingVertices = pool->vertexCount;
    D_0056ACF8->colors = (u32 *)pool->auxRecordBase;
    D_0056ACF8->positions = (u128 *)pool->recordBase;
    D_0056ACF8->color = pool->color;
    D_0056ACF8->parameterCount = 0x10;
    D_0056ACF8->vertexCount = EFF_QUAD_BATCH_VERTICES;
    D_0056ACF8->parameters = D_0031F250;
    while (remainingVertices >= EFF_QUAD_BATCH_VERTICES) {
        remainingVertices -= EFF_QUAD_BATCH_VERTICES;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
        D_0056ACF8->positions += EFF_QUAD_BATCH_VERTICES;
        D_0056ACF8->colors += EFF_QUAD_BATCH_VERTICES;
    }
    if (remainingVertices >= EFF_QUAD_VERTEX_COUNT) {
        D_0056ACF8->parameterCount = remainingVertices / EFF_QUAD_VERTEX_COUNT * 2;
        D_0056ACF8->vertexCount = remainingVertices;
        sdfDrawTagLink(packet, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
    }
    surface = D_0031F298[pool->drawMode];
    surface->append(surface, packet);
}

void *dds3FlashPolyList3GetVertex(EffRecordPool *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_QUAD_POSITION_BYTES;
}

void *dds3FlashPolyList3GetVC(EffRecordPool *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_QUAD_COLOR_BYTES;
}

EffRecordPool *dds3FlashPolyListHCreate(u32 fanCount) {
    EffRecordPool *pool = dds3FlashPolyListCreate(fanCount);

    EE_MMI_UNIT_MATRIX(pool->matrix);
    return pool;
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListHDelete);

void dds3FlashPolyListHDraw(EffRecordPool *work)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT] __attribute__((aligned(16)));
    sdfDrawTag_t *list;
    s32 remaining;
    s32 fanCount;
    u64 *packet;
    sdfDrawTag_t *stateList;

    list = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    sdfDrawTagInit(list);
    VU0_COPY_MATRIX(matrix, work->matrix);
    matrix[12] = work->origin[0];
    matrix[13] = work->origin[1];
    matrix[14] = work->origin[2];
    VU0_LOAD_MATRIX(matrix);
    sdf3DPrimLinkTm(list, 0);
    sdf3DPrimLinkMat(list, work->resource, 0);
    remaining = work->vertexCount;
    D_0056ACF8->colors = (u32 *)work->auxRecordBase;
    D_0056ACF8->positions = (u128 *)work->recordBase;
    D_0056ACF8->color = work->color;
    D_0056ACF8->parameterCount = EFF_FAN_BATCH_PARAMS;
    D_0056ACF8->vertexCount = EFF_FAN_BATCH_VERTICES;
    D_0056ACF8->parameters = D_0031F1E0;
    while (remaining >= EFF_FAN_BATCH_VERTICES) {
        remaining -= EFF_FAN_BATCH_VERTICES;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
        D_0056ACF8->positions += EFF_FAN_BATCH_VERTICES;
        D_0056ACF8->colors += EFF_FAN_BATCH_VERTICES;
    }
    if (remaining >= EFF_FAN_VERTEX_COUNT) {
        fanCount = remaining / EFF_FAN_VERTEX_COUNT;
        D_0056ACF8->vertexCount = remaining;
        D_0056ACF8->parameterCount = fanCount * 3;
        sdfDrawTagLink(list, (u32)effPkt3DPrimMakePoly(D_0056ACF8));
    }
    if (work->drawMode < EFF_DIRECT_SURFACE_COUNT) {
        D_0031F2A8[work->drawMode]->append(D_0031F2A8[work->drawMode], list);
    } else {
        stateList = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
        sdfDrawTagInit(stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[4] = 6;
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[5] = 0x42;
        sdfDrawTagLink(stateList, (u32)packet);
        (*effFlashOtNode).append(&(*effFlashOtNode), stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x42;
        packet[5] = 0x42;
        sdfDrawTagLink(list, (u32)packet);
        (*effFlashOtNode).append(&(*effFlashOtNode), list);
    }
}

void *dds3FlashPolyListHGetVertex(EffRecordPool *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_FAN_POSITION_BYTES;
}

u32 *dds3FlashPolyListHGetVC(EffRecordPool *group, s32 groupIndex) {
    return (u32 *)(group->auxRecordBase + groupIndex * EFF_FAN_COLOR_BYTES);
}

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListHSetBlend);

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListHSetABGR);

INCLUDE_ASM(const s32, "effect/effPCPFlash", dds3FlashPolyListHSetScale);
