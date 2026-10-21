#include "common.h"
#include "ps2asm.h"
#include "sdf_resource.h"
#include "sdf_types.h"

struct effDebugPolyParams_t;
struct EffRandState;
struct dds3FlashPolyList_t;
struct EffRingColorSlot;
struct EffRingParticle;
struct EffRingWork;
struct effBezierDrawParam_t;
struct effPCPFlashPolyPipePart_tag;
struct PcpFlashAccumulatingWork;
struct PcpFlashColorSlot;
struct PcpFlashFadingOrbitWork;
struct PcpFlashMotionParticle;
struct effPCPFlashPolyCircleParam_t;
struct PcpFlashOffsetRadialWork;
struct PcpFlashOrbitArcWork;
struct PcpFlashOrbitParticle;
struct PcpFlashPulseParticle;
struct PcpFlashQuadColorSlot;
struct PcpFlashRadialStripParticle;
struct PcpFlashRadialStripWork;
struct effPCPFlashPolySphere2ExpParam_t;
struct PcpFlashRadialTriangleWork;
struct PcpFlashRotatingParticle;
struct PcpFlashRotatingQuadParticle;
struct PcpFlashRotatingQuadWork;
struct PcpFlashScalingOrbitWork;
struct PcpFlashStreakParams;
struct PcpFlashStreakWork;
struct effPCPFlashPolySphere2Param_t;
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
typedef struct EffRingColorSlot EffRingColorSlot;
typedef struct EffRingParticle EffRingParticle;
typedef struct EffRingWork EffRingWork;
typedef struct PcpFlashColorSlot PcpFlashColorSlot;
typedef struct effPCPFlashPolyCircleParam_t effPCPFlashPolyCircleParam_t;
typedef struct PcpFlashPulseParticle PcpFlashPulseParticle;
typedef struct PcpFlashQuadColorSlot PcpFlashQuadColorSlot;
typedef struct effPCPFlashPolySphere2ExpParam_t effPCPFlashPolySphere2ExpParam_t;
typedef struct PcpFlashStreakParams PcpFlashStreakParams;
typedef struct effPCPFlashPolySphere2Param_t effPCPFlashPolySphere2Param_t;
typedef struct PcpFlashTrianglePulseWork PcpFlashTrianglePulseWork;
typedef struct sdfGrOt_t sdfGrOt_t;
typedef struct effBezierDrawParam_t effBezierDrawParam_t;
typedef struct effDebugPolyParams_t effDebugPolyParams_t;

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


/* Four ordered 32-bit words in the shared effect random-number state. */
struct EffRandState {
    u32 words[4];
};

typedef struct dds3FlashPolyList_t dds3FlashPolyList_t;

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



struct sdfMemHandle_t;

/* Embedded command work at actor +0x20. Neither record is the whole owning
 * actor. */
struct sdfMemHandle_t;


/* Position/color arrays precede this render pool; constructors clear the full allocation. */
typedef struct dds3FlashPolyList_t {
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
} dds3FlashPolyList_t; /* 0x70 */

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
    dds3FlashPolyList_t *recordPool;
};

typedef struct effPCPFlashPolyPipePart_tag effPCPFlashPolyPipePart_tag;

struct effPCPFlashPolyPipePart_tag {
    u32 abgr;
    s32 frame;
    f32 length;
    f32 width_in;
    f32 width_out;
    f32 up;
    f32 rotate;
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
    effPCPFlashPolyPipePart_tag *parts;
    u32 updateCount;
    u32 tintColor;
    f32 renderScale;
    sdfMemHandle_t *allocationHandle;
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
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

struct effPCPFlashPolyCircleParam_t {
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
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
};

struct effPCPFlashPolySphere2ExpParam_t {
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
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
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
    dds3FlashPolyList_t *resourceHandle;
};

/* The factory copies this 0x30-byte parameter prefix into its larger work
 * allocation before appending particle and resource state. */
struct effPCPFlashPolySphere2Param_t {
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
    dds3FlashPolyList_t *resourceHandle;
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

/*
 * Fill the x, y and z components of a float vector with one value (plain C;
 * the w component is left alone). Used by the PCP particle and strip
 * builders to set up per-axis scales before the VU0 multiply.
 */
#define VEC3_SPLAT(v, x) ((v)[0] = (x), (v)[1] = (x), (v)[2] = (x))


/* Vertex-draw descriptor used by the thunder history and end-cap packets.
 * Both title providers clear the first 0x2C bytes and keep these globals in
 * 0x30-byte slots; the final four bytes remain opaque. */
struct effBezierDrawParam_t {
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

extern void *effPkt3DPrimMakePoly(effBezierDrawParam_t *);

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

extern effDebugPolyParams_t D_0056ACF8[];

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

/* Parameter prefixes retained by the flash particle constructors. */
typedef struct effFlashScalingOrbitParam_t {
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
    f32 orbitRadius;
    f32 maxScale;
    f32 tilt;
    f32 angularStep;
    u32 drawMode;
} effFlashScalingOrbitParam_t;

typedef struct effFlashAccumulatingParam_t {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
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
} effFlashAccumulatingParam_t;

typedef struct effFlashOrbitArcParam_t {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
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
} effFlashOrbitArcParam_t;

typedef struct effFlashFadingOrbitParam_t {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
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
} effFlashFadingOrbitParam_t;

typedef struct effFlashRotatingQuadParam_t {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
    s32 lifetime;
    s32 scaleRampTime;
    u32 randomRange;
    u8 pad24[4];
    s32 fadeInTime;
    s32 fadeOutTime;
    u32 centerColor;
    u32 edgeColor;
    f32 upSpan;
    f32 acrossSpan;
    f32 maxScale;
    f32 angularStepRange;
    u32 drawMode;
} effFlashRotatingQuadParam_t;

typedef struct effFlashRadialStripParam_t {
    f32 origin[3];
    u8 pad0C[4];
    s32 particleCount;
    u8 restartRandomly;
    u8 pad15[3];
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
} effFlashRadialStripParam_t;

typedef struct effRingBlock_t {
    EffRingWork header;
    EffRingParticle vertices[1];
} effRingBlock_t;

#define EFF_RING_PARAM_BYTES 0x58
#define EFF_RING_HEADER_BYTES 0x80
#define EFF_RING_VERTEX_BYTES 16
#define EFFECT_RING_START_ANGLE (-1.5707963f)
#define EFFECT_RING_FULL_TURN (6.2831853f)
extern void *dds3EffectPCPDataGetData(void *table, s32 index);
extern u32 effColorBlend(u32 firstColor, u32 secondColor, f32 blend);
extern void sdfMemFreeDirect(sdfMemHandle_t *allocation);
extern void sdfMatFreeDelayed(sdfMat_t *material);
extern f32 aViewEye[], aViewTarget[];
extern f32 effPCPFlashPolySphere2Vertex[], effPCPFlashPolySphereVertex[];
extern f32 effPCPFlashPolyRingVertex[], effPCPFlashPolySphereExpVertex[];
extern f32 def_dir[], effPCPFlashPolyRingExp2Vertex[], effPCPFlashPolyCircleVertex[];

typedef struct effFlashOrbitArcBlock_t {
    PcpFlashOrbitArcWork header;
    PcpFlashMotionParticle parts[1];
} effFlashOrbitArcBlock_t;

dds3FlashPolyList_t *dds3FlashPolyListCreate(u32 count);
void dds3FlashPolyListDraw(dds3FlashPolyList_t *work);
void *dds3FlashPolyListGetVertex(dds3FlashPolyList_t *group, s32 groupIndex);
void *dds3FlashPolyListGetVC(dds3FlashPolyList_t *group, s32 groupIndex);
dds3FlashPolyList_t *dds3FlashPolyList2Create(s32 triangleCount);
void dds3FlashPolyList2Draw(dds3FlashPolyList_t *pool);
void *dds3FlashPolyList2GetVertex(dds3FlashPolyList_t *group, s32 groupIndex);
void *dds3FlashPolyList2GetVC(dds3FlashPolyList_t *group, s32 groupIndex);
dds3FlashPolyList_t *dds3FlashPolyList3Create(s32 quadCount);
void dds3FlashPolyList3Draw(dds3FlashPolyList_t *pool);
void *dds3FlashPolyList3GetVertex(dds3FlashPolyList_t *group, s32 groupIndex);
void *dds3FlashPolyList3GetVC(dds3FlashPolyList_t *group, s32 groupIndex);
dds3FlashPolyList_t *dds3FlashPolyListHCreate(u32 fanCount);
void dds3FlashPolyListHDraw(dds3FlashPolyList_t *work);
void *dds3FlashPolyListHGetVertex(dds3FlashPolyList_t *group, s32 groupIndex);
u32 *dds3FlashPolyListHGetVC(dds3FlashPolyList_t *group, s32 groupIndex);
void dds3FlashPolyListDelete(dds3FlashPolyList_t *group);
void dds3FlashPolyListSetBlend(dds3FlashPolyList_t *pool, u32 mode);
void dds3FlashPolyListSetABGR(dds3FlashPolyList_t *group, u32 color);
void dds3FlashPolyListSetScale(dds3FlashPolyList_t *work, f32 scale);
void dds3FlashPolyList2Delete(dds3FlashPolyList_t *group);
void dds3FlashPolyList3Delete(dds3FlashPolyList_t *group);
void dds3FlashPolyListHDelete(dds3FlashPolyList_t *pool);

PcpFlashTrianglePulseWork *effPCPFlashPolySphere2Create(effPCPFlashPolySphere2Param_t *params)
{
    sdfMemHandle_t *allocation = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashPulseParticle) + sizeof(PcpFlashTrianglePulseWork));
    PcpFlashTrianglePulseWork *work = (PcpFlashTrianglePulseWork *)sdfMemLock(allocation);
    dds3FlashPolyList_t *record;
    u32 i;

    memcpy(work, params, 0x30);
    work->parts = (PcpFlashPulseParticle *)(work + 1);
    work->tintColor = EFF_NEUTRAL_COLOR;
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

void effPCPFlashPolySphere2CreatePCF(void *table)
{
    effPCPFlashPolySphere2Create((effPCPFlashPolySphere2Param_t *)dds3EffectPCPDataGetData(table, 0));
}

PcpFlashTrianglePulseWork *effPCPFlashPolySphere2Copy(effPCPFlashPolySphere2Param_t *params)
{
    return effPCPFlashPolySphere2Create(params);
}

void effPCPFlashPolySphere2Delete(PcpFlashTrianglePulseWork *work)
{
    dds3FlashPolyList2Delete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolySphere2SetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolySphere2SetColor(PcpFlashTrianglePulseWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolySphere2SetScale(PcpFlashTrianglePulseWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolySphere2PartSetVertex(PcpFlashTrianglePulseWork *work, s32 index, void *view)
{
    PcpFlashPulseParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyList2GetVertex(work->resourceHandle, index);
    f32 base[4];
    f32 size[4];
    f32 step;
    f32 angle;
    f32 radius;

    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    radius = part->scale;
    VEC3_SPLAT(size, radius);
    angle = step * (f32)index;
    base[0] = 0;
    base[1] = 1.0f;
    base[2] = 0;
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_STOREVECTOR(base);
    SetRotateMatrixAxis(angle, (const sdfVector3_t *)view);
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(effPCPFlashPolySphere2Vertex);
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SetRotateMatrixAxis(angle + step, (const sdfVector3_t *)view);
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_LOADVECTOR_B(effPCPFlashPolySphere2Vertex);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolySphere2Draw(PcpFlashTrianglePulseWork *work) {
    f32 axis[4];
    s32 particleIndex;
    s32 lifetime;
    s32 particleCount;
    s32 half;
    s32 ramp;
    f32 maxScale;
    s32 restart;
    s32 tintColor;
    PcpFlashPulseParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
        SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(axis);
    lifetime = work->lifetime;
    particleCount = work->particleCount;
    part = work->parts;
    half = lifetime >> 1;
    ramp = work->scaleRampTime;
    maxScale = work->maxScale;
    restart = work->restartRandomly;
    tintColor = work->tintColor;
    for (particleIndex = 0; particleIndex < particleCount; particleIndex++, part++) {
        s32 age = part->age;
        s32 color;
        f32 blend;

        if (part->age == 0) {
            effPCPFlashPolySphere2PartSetVertex(work, particleIndex, axis);
            effPCPFlashPolySphere2PartSetVC(work, particleIndex, 0);
            if (ramp == 0) {
                part->scale = maxScale;
            } else {
                part->scale = 0.0f;
            }
            part->color = EFF_NEUTRAL_COLOR;
        } else {
            if (part->age >= lifetime) {
                if (restart != 0) {
                    part->age = 0;
                }
                color = 0;
                effPCPFlashPolySphere2PartSetVC(work, particleIndex, color);
            } else if (part->age > 0) {
                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = (maxScale * (f32)part->age) / (f32)ramp;
                    if (maxScale < part->scale) {
                        part->scale = maxScale;
                    }
                }
                effPCPFlashPolySphere2PartSetVertex(work, particleIndex, axis);
                if (part->age < half) {
                    blend = (f32)age / (f32)half;
                } else {
                    blend = (f32)(lifetime - age) / (f32)half;
                }
                color = effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor);
                effPCPFlashPolySphere2PartSetVC(work, particleIndex, color);
            }
        }
        part->age = part->age + 1;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyList2Draw(handle);
}

PcpFlashStreakWork *effPCPFlashPolySphereCreate(PcpFlashStreakParams *src) {
    sdfMemHandle_t *handle = sdfMemAllocDirect(src->particleCount * sizeof(PcpFlashRotatingParticle) + sizeof(PcpFlashStreakWork));
    PcpFlashStreakWork *work = (PcpFlashStreakWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    u32 range;
    u32 particleIndex;

    memcpy(work, src, 0x40);
    work->parts = (PcpFlashRotatingParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
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

void effPCPFlashPolySphereCreatePCF(void *data)
{
    effPCPFlashPolySphereCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashStreakWork *effPCPFlashPolySphereCopy(PcpFlashStreakParams *src)
{
    return effPCPFlashPolySphereCreate(src);
}

void effPCPFlashPolySphereDelete(PcpFlashStreakWork *work)
{
    dds3FlashPolyListDelete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolySphereSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolySphereSetColor(PcpFlashStreakWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolySphereSetScale(PcpFlashStreakWork *work, f32 value)
{
    work->renderScale = value;
}

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
    SDF_VU0_LOADVECTOR(direction);
    SDF_VU0_LOADVECTOR_B(orientation);
    SDF_VU0_OUTERPRODUCT();
        SDF_VU0_NORMALIZE();;
    SDF_VU0_STOREVECTOR(direction);
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

void effPCPFlashPolySpherePartSetVertex(PcpFlashStreakWork *work, s32 index, void *view)
{
    PcpFlashRotatingParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyListGetVertex(work->resourceHandle, index);
    f32 center[4];
    f32 scale[4];
    f32 across[4];
    f32 up[4];
    f32 ratio;
    f32 size;
    f32 acrossLen;
    f32 upLen;

    size = part->scale;
    ratio = size / part->initialScale;
    VEC3_SPLAT(scale, size);
    acrossLen = part->acrossSpan * ratio;
    VEC3_SPLAT(across, acrossLen);
    upLen = part->upSpan * ratio;
    VEC3_SPLAT(up, upLen);
    center[0] = part->position[0];
    center[1] = part->position[1];
    center[2] = part->position[2];
    SDF_VU0_LOADVECTOR(center);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(across);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(across);
    SDF_VU0_LOADVECTOR(up);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(up);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(scale);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(effPCPFlashPolySphereVertex);
    SDF_VU0_LOADVECTOR_B(up);
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 16);
    SDF_VU0_LOADVECTOR(effPCPFlashPolySphereVertex);
    SDF_VU0_LOADVECTOR_B(across);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolySpherePartRotate(PcpFlashStreakWork *work, s32 index, void *orientation)
{
    PcpFlashRotatingParticle *part = &work->parts[index];
    f32 position[4];

    position[0] = part->position[0];
    position[1] = part->position[1];
    position[2] = part->position[2];
    SetRotateMatrixAxis(part->rotationStep, (const sdfVector3_t *)orientation);
    SDF_VU0_LOADVECTOR(position);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_STOREVECTOR(position);
    part->position[0] = position[0];
    part->position[1] = position[1];
    part->position[2] = position[2];
}

void effPCPFlashPolySphereDraw(PcpFlashStreakWork *work) {
    s128 axis;
    s32 index;
    s32 lifetime;
    s32 count;
    s32 half;
    s32 ramp;
    f32 maxScale;
    s32 restart;
    s32 tintColor;
    u32 range;
    PcpFlashRotatingParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(&axis);
    lifetime = work->lifetime;
    count = work->particleCount;
    part = work->parts;
    half = lifetime >> 1;
    ramp = work->scaleRampTime;
    maxScale = work->maxScale;
    restart = work->restartRandomly;
    range = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;
        s32 color;
        f32 blend;

        if (part->age == 0) {
            effPCPFlashPolySpherePartInit(work, index, &axis);
            effPCPFlashPolySpherePartSetVertex(work, index, &axis);
            effPCPFlashPolySpherePartSetVC(work, index, 0);
            if (ramp == 0) {
                part->scale = maxScale;
            } else {
                part->scale = 0.0f;
            }
            part->color = EFF_NEUTRAL_COLOR;
        } else {
            if (part->age >= lifetime) {
                if (restart != 0) {
                    part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
                }
                color = 0;
                effPCPFlashPolySpherePartSetVC(work, index, color);
            } else if (part->age > 0) {
                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = (maxScale * (f32)part->age) / (f32)ramp;
                    if (maxScale < part->scale) {
                        part->scale = maxScale;
                    }
                }
                effPCPFlashPolySpherePartRotate(work, index, &axis);
                effPCPFlashPolySpherePartSetVertex(work, index, &axis);
                if (part->age < half) {
                    blend = (f32)age / (f32)half;
                } else {
                    blend = (f32)(lifetime - age) / (f32)half;
                }
                color = effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor);
                effPCPFlashPolySpherePartSetVC(work, index, color);
            }
        }
        part->age = part->age + 1;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyListDraw(handle);
}

PcpFlashScalingOrbitWork *effPCPFlashPolyRingCreate(effFlashScalingOrbitParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashOrbitParticle) + sizeof(PcpFlashScalingOrbitWork));
    PcpFlashScalingOrbitWork *work = (PcpFlashScalingOrbitWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    f32 angle;
    f32 step;
    u32 range;
    u32 i;

    memcpy(work, params, 0x48);
    work->parts = (PcpFlashOrbitParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
    work->updateCount = 0;
    work->renderScale = 1.0f;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    angle = -3.14159265f / 2.0f;
    record = dds3FlashPolyListCreate(work->particleCount);
    record->scale = 1.0f;
    record->drawMode = work->drawMode;
    work->resourceHandle = record;
    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    range = work->randomRange;
    for (i = 0; i < (u32)work->particleCount; i++) {
        work->parts[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
        work->parts[i].angle = angle;
        angle += step;
    }
    return work;
}

void effPCPFlashPolyRingCreatePCF(void *data)
{
    effPCPFlashPolyRingCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashScalingOrbitWork *effPCPFlashPolyRingCopy(effFlashScalingOrbitParam_t *params)
{
    return effPCPFlashPolyRingCreate(params);
}

void effPCPFlashPolyRingDelete(PcpFlashScalingOrbitWork *work)
{
    dds3FlashPolyListDelete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyRingSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyRingSetColor(PcpFlashScalingOrbitWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolyRingSetScale(PcpFlashScalingOrbitWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolyRingPartSetVertex(PcpFlashScalingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyListGetVertex(work->resourceHandle, index);
    f32 offset[4];
    f32 unit[4];
    f32 scaleA[4];
    f32 scaleB[4];
    f32 scaleC[4];
    f32 size;
    f32 ratio;
    f32 sinv;
    f32 height;
    f32 widthB;
    f32 widthC;

    size = part->scale;
    ratio = size / part->initialScale;
    VEC3_SPLAT(scaleA, size);
    widthB = work->acrossSpan * ratio;
    widthC = work->upSpan * ratio;
    VEC3_SPLAT(scaleB, widthB);
    VEC3_SPLAT(scaleC, widthC);
    unit[0] = sdfCos(part->angle);
    unit[1] = 0;
    sinv = sdfSin(part->angle);
    unit[2] = sinv;
    offset[0] = unit[0] * work->orbitRadius;
    offset[1] = 0;
    offset[2] = sinv * work->orbitRadius;
    height = work->tilt;
    effPCPFlashPolyRingVertex[0] = unit[0] * height;
    effPCPFlashPolyRingVertex[1] = height + -1.0f;
    effPCPFlashPolyRingVertex[2] = sinv * height;
    SDF_VU0_LOADVECTOR(effPCPFlashPolyRingVertex);
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(unit);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(scaleB);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleB);
    SDF_VU0_LOADVECTOR(scaleC);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleC);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(scaleA);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleC);
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 16);
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleB);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolyRingPartRotate(PcpFlashScalingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part;

    part = &work->parts[index];
    part->angle += work->angularStep;
}

void effPCPFlashPolyRingDraw(PcpFlashScalingOrbitWork *work) {
    s32 index;
    s32 lifetime;
    s32 count;
    s32 half;
    s32 ramp;
    f32 maxScale;
    s32 restart;
    s32 tintColor;
    u32 range;
    PcpFlashOrbitParticle *part;
    dds3FlashPolyList_t *handle;

    lifetime = work->lifetime;
    count = work->particleCount;
    part = work->parts;
    half = lifetime >> 1;
    ramp = work->scaleRampTime;
    maxScale = work->maxScale;
    restart = work->restartRandomly;
    range = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;
        s32 color;
        f32 blend;

        if (part->age == 0) {
            part->initialScale = maxScale;
            if (ramp == 0) {
                part->scale = maxScale;
            } else {
                part->scale = 0.0f;
            }
            effPCPFlashPolyRingPartSetVertex(work, index);
            effPCPFlashPolyRingPartSetVC(work, index, 0);
            part->color = EFF_NEUTRAL_COLOR;
        } else {
            if (part->age >= lifetime) {
                if (restart != 0) {
                    part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
                }
                color = 0;
                effPCPFlashPolyRingPartSetVC(work, index, color);
            } else if (part->age > 0) {
                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = (maxScale * (f32)part->age) / (f32)ramp;
                    if (maxScale < part->scale) {
                        part->scale = maxScale;
                    }
                }
                effPCPFlashPolyRingPartRotate(work, index);
                effPCPFlashPolyRingPartSetVertex(work, index);
                if (part->age < half) {
                    blend = (f32)age / (f32)half;
                } else {
                    blend = (f32)(lifetime - age) / (f32)half;
                }
                color = effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor);
                effPCPFlashPolyRingPartSetVC(work, index, color);
            }
        }
        part->age = part->age + 1;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyListDraw(handle);
}

PcpFlashAccumulatingWork *effPCPFlashPolyPipeCreate(effFlashAccumulatingParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(effPCPFlashPolyPipePart_tag) + sizeof(PcpFlashAccumulatingWork));
    PcpFlashAccumulatingWork *work = (PcpFlashAccumulatingWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    f32 angle;
    f32 step;
    u32 range;
    u32 i;

    memcpy(work, params, 0x50);
    work->parts = (effPCPFlashPolyPipePart_tag *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
    work->updateCount = 0;
    work->renderScale = 1.0f;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    angle = -3.14159265f / 2.0f;
    record = dds3FlashPolyList3Create(work->particleCount * 2);
    record->scale = 1.0f;
    record->drawMode = work->drawMode;
    work->resourceHandle = record;
    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    range = work->randomRange;
    for (i = 0; i < (u32)work->particleCount; i++) {
        work->parts[i].frame = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
        work->parts[i].rotate = angle;
        angle += step;
    }
    return work;
}

void effPCPFlashPolyPipeCreatePCF(void *data)
{
    effPCPFlashPolyPipeCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashAccumulatingWork *effPCPFlashPolyPipeCopy(effFlashAccumulatingParam_t *params)
{
    return effPCPFlashPolyPipeCreate(params);
}

void effPCPFlashPolyPipeDelete(PcpFlashAccumulatingWork *work)
{
    dds3FlashPolyList3Delete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyPipeSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyPipeSetColor(PcpFlashAccumulatingWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolyPipeSetScale(PcpFlashAccumulatingWork *work, f32 value)
{
    work->renderScale = value;
}

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
    effPCPFlashPolyPipePart_tag *part = &work->parts[index];
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

    center = part->up;
    VEC3_SPLAT(middle, center);
    outerEdge = center + part->length;
    VEC3_SPLAT(outer, outerEdge);
    innerEdge = center - part->length;
    VEC3_SPLAT(inner, innerEdge);
    part->up = part->up + work->radialStep;
    span = part->width_in;
    VEC3_SPLAT(size, span);
    unit[0] = sdfCos(part->rotate);
    unit[1] = 0;
    sinv = sdfSin(part->rotate);
    unit[2] = sinv;
    offset[0] = unit[0] * work->orbitRadius;
    offset[1] = 0;
    offset[2] = sinv * work->orbitRadius;
    height = work->tilt;
    effPCPFlashPolyPipeVertex[0] = unit[0] * height;
    effPCPFlashPolyPipeVertex[1] = height + -1.0f;
    effPCPFlashPolyPipeVertex[2] = sinv * height;
    SDF_VU0_LOADVECTOR(effPCPFlashPolyPipeVertex);
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(unit);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(size);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(outer);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(outer);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(inner);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(inner);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(middle);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(outer);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    mirror = dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2 + 1);
    SDF_VU0_COPYVECTOR(mirror + 8, quad + 8);
    SDF_VU0_COPYVECTOR(mirror + 4, quad + 4);
    SDF_VU0_COPYVECTOR(mirror + 12, quad + 12);
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(inner);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(mirror);
}

void effPCPFlashPolyPipePartRotate(PcpFlashAccumulatingWork *work, s32 index)
{
    effPCPFlashPolyPipePart_tag *part;

    part = &work->parts[index];
    part->rotate += work->increment;
}

void effPCPFlashPolyPipeDraw(PcpFlashAccumulatingWork *work)
{
    s32 index;
    s32 count;
    effPCPFlashPolyPipePart_tag *part;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    u32 randomRange;
    s32 restart;
    s32 tintColor;
    dds3FlashPolyList_t *handle;

    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    restart = work->restartRandomly;
    randomRange = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->frame;
        s32 color;
        f32 factor;

        if (age == 0) {
            effPCPFlashPolyPipePartSetVertex(work, index);
            effPCPFlashPolyPipePartSetVC(work, index, 0);
            part->abgr = EFF_NEUTRAL_COLOR;
            factor = dds3GetRandFloatA(dds3EffectBase) * 0.5f + 0.5f;
            part->length = work->radialHalfThicknessBase * factor;
            factor = dds3GetRandFloatA(dds3EffectBase) * 0.7f + 0.3f;
            part->width_in = work->spanBase * factor;
            part->width_out = work->unk34 * factor;
            part->up = 0;
        } else if (age >= lifetime) {
            if (restart != 0) {
                part->frame = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % randomRange);
            }
            color = 0;
            effPCPFlashPolyPipePartSetVC(work, index, color);
        } else if (age > 0) {
            effPCPFlashPolyPipePartRotate(work, index);
            effPCPFlashPolyPipePartSetVertex(work, index);
            if (part->frame < fadeIn && fadeIn != 0) {
                factor = (f32)part->frame / (f32)fadeIn;
            } else if (fadeOut >= lifetime - part->frame && fadeOut != 0) {
                factor = (f32)(lifetime - part->frame) / (f32)fadeOut;
            } else {
                factor = 1.0f;
            }
            color = effMagaCylinderCalcColor(effColorBlend(0, part->abgr, factor), tintColor);
            effPCPFlashPolyPipePartSetVC(work, index, color);
        }
        part->frame = part->frame + 1;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyList3Draw(handle);
}

PcpFlashOrbitArcWork *effPCPFlashPolyRingExpCreate(effFlashOrbitArcParam_t *params)
{
    sdfMemHandle_t *handle;
    effFlashOrbitArcBlock_t *block;
    PcpFlashOrbitArcWork *ring;
    dds3FlashPolyList_t *record;
    f32 angle;
    f32 step;
    u32 spread;
    u32 i;

    handle = sdfMemAllocDirect((u32)params->particleCount * 16 + 0x80);
    block = (effFlashOrbitArcBlock_t *)sdfMemLock(handle);
    ring = &block->header;
    memcpy(ring, params, 0x58);
    ring->parts = block->parts;
    ring->allocationHandle = handle;
    ring->tintColor = EFF_NEUTRAL_COLOR;
    ring->orbitRadius = ring->orbitRadiusStart;
    ring->upSpan = ring->upSpanStart;
    ring->acrossSpan = ring->acrossSpanStart;
    ring->shapeProgressFrame = 0;
    ring->renderScale = 1.0f;
    if (ring->randomRange == 0) {
        ring->randomRange = 1;
    }
    angle = EFFECT_RING_START_ANGLE;
    record = dds3FlashPolyListCreate(ring->particleCount);
    record->scale = 1.0f;
    record->drawMode = ring->drawMode;
    ring->resourceHandle = record;
    step = EFFECT_RING_FULL_TURN / (u32)ring->particleCount;
    spread = ring->randomRange;
    for (i = 0; i < (u32)ring->particleCount; i++) {
        ring->parts[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % spread);
        ring->parts[i].accumulator = angle;
        angle += step;
    }
    return ring;
}

void effPCPFlashPolyRingExpCreatePCF(void *data)
{
    effPCPFlashPolyRingExpCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashOrbitArcWork *effPCPFlashPolyRingExpCopy(effFlashOrbitArcParam_t *params)
{
    return effPCPFlashPolyRingExpCreate(params);
}

void effPCPFlashPolyRingExpDelete(PcpFlashOrbitArcWork *work)
{
    dds3FlashPolyListDelete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyRingExpSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyRingExpSetColor(PcpFlashOrbitArcWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolyRingExpSetScale(PcpFlashOrbitArcWork *work, f32 value)
{
    work->renderScale = value;
}

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
    SDF_VU0_LOADVECTOR(effPCPFlashPolyRingExpVertex);
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(unit);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(scaleB);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleB);
    SDF_VU0_LOADVECTOR(scaleC);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleC);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(scaleA);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleC);
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 16);
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleB);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolyRingExpPartRotate(PcpFlashOrbitArcWork *work, s32 index)
{
    PcpFlashMotionParticle *part;

    part = &work->parts[index];
    part->accumulator += work->angularStep;
}

void effPCPFlashPolyRingExpDraw(PcpFlashOrbitArcWork *work)
{
    s32 index;
    s32 count;
    PcpFlashMotionParticle *part;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    u32 randomRange;
    s32 restart;
    s32 tintColor;
    dds3FlashPolyList_t *handle;
    f32 heightDelta;
    f32 delta;
    f32 progress;
    f32 factor;
    f32 start;
    f32 radiusEnd;
    f32 upStart;
    f32 acrossStart;
    f32 upEnd;
    f32 acrossEnd;
    f32 normalEnd;

    part = work->parts;
    count = work->particleCount;
    lifetime = work->lifetime;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    restart = work->restartRandomly;
    randomRange = work->randomRange;
    tintColor = work->tintColor;
    heightDelta = work->heightEnd - work->heightStart;
    progress = 1.0f;
    if (lifetime > 0) {
        progress = (f32)work->shapeProgressFrame / (f32)lifetime;
    }
    start = work->orbitRadiusStart;
    radiusEnd = work->orbitRadiusEnd;
    delta = radiusEnd - start;
    work->orbitRadius = start + delta * progress;
    if (start > 0.0f) {
        factor = radiusEnd / start;
    } else {
        factor = radiusEnd;
    }
    upStart = work->upSpanStart;
    acrossStart = work->acrossSpanStart;
    upEnd = upStart * factor;
    acrossEnd = acrossStart * factor;
    start = work->normalSpanStart;
    normalEnd = work->normalSpanEnd;
    delta = upEnd - upStart;
    work->upSpan = upStart + delta * progress;
    delta = acrossEnd - acrossStart;
    work->acrossSpan = acrossStart + delta * progress;
    delta = normalEnd - start;
    work->normalSpan = start + delta * progress;

    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;
        s32 color;

        if (age == 0) {
            part->stepSpeed = work->heightStart;
            effPCPFlashPolyRingExpPartSetVertex(work, index);
            effPCPFlashPolyRingExpPartSetVC(work, index, 0);
            part->color = EFF_NEUTRAL_COLOR;
        } else if (age >= lifetime) {
            if (restart != 0) {
                part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % randomRange);
            }
            color = 0;
            effPCPFlashPolyRingExpPartSetVC(work, index, color);
        } else if (age > 0) {
            progress = (f32)age / (f32)lifetime;
            part->stepSpeed = heightDelta * progress + work->heightStart;
            effPCPFlashPolyRingExpPartRotate(work, index);
            effPCPFlashPolyRingExpPartSetVertex(work, index);
            if (part->age < fadeIn && fadeIn != 0) {
                factor = (f32)part->age / (f32)fadeIn;
            } else if (fadeOut >= lifetime - part->age && fadeOut != 0) {
                factor = (f32)(lifetime - part->age) / (f32)fadeOut;
            } else {
                factor = 1.0f;
            }
            color = effMagaCylinderCalcColor(effColorBlend(0, part->color, factor), tintColor);
            effPCPFlashPolyRingExpPartSetVC(work, index, color);
        }
        part->age = part->age + 1;
    }
    if (work->shapeProgressFrame == lifetime) {
        work->shapeProgressFrame = 0;
    } else {
        work->shapeProgressFrame++;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyListDraw(handle);
}

PcpFlashRotatingQuadWork *effPCPFlashPolySphereExpCreate(effFlashRotatingQuadParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashRotatingQuadParticle) + sizeof(PcpFlashRotatingQuadWork));
    PcpFlashRotatingQuadWork *work = (PcpFlashRotatingQuadWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    u32 range;
    u32 i;

    memcpy(work, params, 0x4C);
    work->parts = (PcpFlashRotatingQuadParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
    work->renderScale = 1.0f;
    work->updateCount = 0;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    record = dds3FlashPolyListCreate(work->particleCount);
    work->resourceHandle = record;
    record->drawMode = work->drawMode;
    range = work->randomRange;
    for (i = 0; i < work->particleCount; i++) {
        work->parts[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
    }
    return work;
}

void effPCPFlashPolySphereExpCreatePCF(void *data)
{
    effPCPFlashPolySphereExpCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashRotatingQuadWork *effPCPFlashPolySphereExpCopy(effFlashRotatingQuadParam_t *params)
{
    return effPCPFlashPolySphereExpCreate(params);
}

void effPCPFlashPolySphereExpDelete(PcpFlashRotatingQuadWork *work)
{
    dds3FlashPolyListDelete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolySphereExpSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolySphereExpSetColor(PcpFlashRotatingQuadWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolySphereExpSetScale(PcpFlashRotatingQuadWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolySphereExpPartSetVertex(PcpFlashRotatingQuadWork *work, s32 index, void *view)
{
    PcpFlashRotatingQuadParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyListGetVertex(work->resourceHandle, index);
    f32 base[4];
    f32 scale[4];
    f32 across[4];
    f32 up[4];
    f32 ratio;
    f32 size;
    f32 acrossLen;
    f32 upLen;

    size = part->scale;
    ratio = size / part->initialScale;
    VEC3_SPLAT(scale, size);
    acrossLen = part->acrossSpan * ratio;
    VEC3_SPLAT(across, acrossLen);
    upLen = part->upSpan * ratio;
    VEC3_SPLAT(up, upLen);
    SetRotateMatrixAxis(part->angle, (const sdfVector3_t *)view);
    base[0] = 0;
    base[1] = 1.0f;
    base[2] = 0;
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(across);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(across);
    SDF_VU0_LOADVECTOR(up);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(up);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(scale);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(effPCPFlashPolySphereExpVertex);
    SDF_VU0_LOADVECTOR_B(up);
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 16);
    SDF_VU0_LOADVECTOR(effPCPFlashPolySphereExpVertex);
    SDF_VU0_LOADVECTOR_B(across);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolySphereExpPartRotate(PcpFlashRotatingQuadWork *work, s32 index, void *orientation)
{
    PcpFlashRotatingQuadParticle *part;

    part = &work->parts[index];
    part->angle += part->angularStep;
}

void effPCPFlashPolySphereExpDraw(PcpFlashRotatingQuadWork *work) {
    s128 axis;
    s32 index;
    s32 lifetime;
    s32 count;
    s32 ramp;
    s32 fadeIn;
    s32 fadeOut;
    f32 maxScale;
    s32 restart;
    s32 tintColor;
    u32 range;
    PcpFlashRotatingQuadParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(&axis);
    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    ramp = work->scaleRampTime;
    maxScale = work->maxScale;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    restart = work->restartRandomly;
    range = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;
        s32 color;
        f32 blend;

        if (part->age == 0) {
            effPCPFlashPolySphereExpPartInit(work, index, &axis);
            effPCPFlashPolySphereExpPartSetVertex(work, index, &axis);
            effPCPFlashPolySphereExpPartSetVC(work, index, 0);
            if (ramp == 0) {
                part->scale = maxScale;
            } else {
                part->scale = 0.0f;
            }
            part->color = EFF_NEUTRAL_COLOR;
        } else {
            if (part->age >= lifetime) {
                if (restart != 0) {
                    part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
                }
                color = 0;
                effPCPFlashPolySphereExpPartSetVC(work, index, color);
            } else if (part->age > 0) {
                s32 remain;

                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = (maxScale * (f32)part->age) / (f32)ramp;
                    if (maxScale < part->scale) {
                        part->scale = maxScale;
                    }
                }
                effPCPFlashPolySphereExpPartRotate(work, index, &axis);
                effPCPFlashPolySphereExpPartSetVertex(work, index, &axis);
                age = part->age;
                if (age < fadeIn && fadeIn != 0) {
                    blend = (f32)age / (f32)fadeIn;
                } else {
                    remain = lifetime - age;
                    if (fadeOut >= remain && fadeOut != 0) {
                        blend = (f32)remain / (f32)fadeOut;
                    } else {
                        blend = 1.0f;
                    }
                }
                color = effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor);
                effPCPFlashPolySphereExpPartSetVC(work, index, color);
            }
        }
        part->age = part->age + 1;
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyListDraw(handle);
}

PcpFlashRadialTriangleWork *effPCPFlashPolySphere2ExpCreate(effPCPFlashPolySphere2ExpParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashMotionParticle) + sizeof(PcpFlashRadialTriangleWork));
    PcpFlashRadialTriangleWork *work = (PcpFlashRadialTriangleWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    u32 i;

    memcpy(work, params, 0x3C);
    work->parts = (PcpFlashMotionParticle *)(work + 1);
    work->tintColor = EFF_NEUTRAL_COLOR;
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

void effPCPFlashPolySphere2ExpCreatePCF(void *data)
{
    effPCPFlashPolySphere2ExpCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashRadialTriangleWork *effPCPFlashPolySphere2ExpCopy(effPCPFlashPolySphere2ExpParam_t *params)
{
    return effPCPFlashPolySphere2ExpCreate(params);
}

void effPCPFlashPolySphere2ExpDelete(PcpFlashRadialTriangleWork *work)
{
    dds3FlashPolyList2Delete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolySphere2ExpSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolySphere2ExpSetColor(PcpFlashRadialTriangleWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolySphere2ExpSetScale(PcpFlashRadialTriangleWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolySphere2ExpPartSetVertex(PcpFlashRadialTriangleWork *work, s32 index, void *view)
{
    PcpFlashMotionParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyList2GetVertex(work->resourceHandle, index);
    f32 base[4];
    f32 size[4];
    f32 step;
    f32 angle;
    f32 radius;

    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    radius = part->accumulator;
    VEC3_SPLAT(size, radius);
    angle = step * (f32)index;
    SetRotateMatrixAxis(angle, (const sdfVector3_t *)view);
    base[0] = 0;
    base[1] = 1.0f;
    base[2] = 0;
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(def_dir);
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SetRotateMatrixAxis(angle + step, (const sdfVector3_t *)view);
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_LOADVECTOR_B(def_dir);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolySphere2ExpDraw(PcpFlashRadialTriangleWork *work) {
    s128 axis;
    s32 restart;
    s32 tintColor;
    s32 index;
    s32 active;
    s32 count;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    f32 startA;
    f32 startB;
    f32 decay;
    PcpFlashMotionParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
        SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(&axis);
    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    startA = work->initialRadius;
    startB = work->initialRadialSpeed;
    decay = work->radialDamping;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    restart = work->restartRandomly;
    tintColor = work->tintColor;
    active = 0;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;

        if (lifetime < age) {
            effPCPFlashPolySphere2ExpPartSetVC(work, index, 0);
        } else {
            if (age == 0) {
                effPCPFlashPolySphere2ExpPartSetVertex(work, index, &axis);
                effPCPFlashPolySphere2ExpPartSetVC(work, index, 0);
                part->accumulator = startA;
                part->stepSpeed = startB;
                part->color = EFF_NEUTRAL_COLOR;
            } else if (age > 0) {
                f32 speed = part->stepSpeed;
                s32 remain;
                f32 blend;

                part->stepSpeed = speed * decay;
                part->accumulator = part->accumulator + speed;
                effPCPFlashPolySphere2ExpPartSetVertex(work, index, &axis);
                if (age < fadeIn && fadeIn != 0) {
                    blend = (f32)age / (f32)fadeIn;
                } else {
                    remain = lifetime - age;
                    if (fadeOut >= remain && fadeOut != 0) {
                        blend = (f32)remain / (f32)fadeOut;
                    } else {
                        blend = 1.0f;
                    }
                }
                active++;
                effPCPFlashPolySphere2ExpPartSetVC(work, index, effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor));
            }
            if (age == lifetime && restart != 0) {
                part->age = 0;
            } else {
                part->age = part->age + 1;
            }
        }
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    if (active != 0) {
        dds3FlashPolyList2Draw(handle);
    }
}

PcpFlashRadialStripWork *effPCPFlashPolySphere3Create(effFlashRadialStripParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashRadialStripParticle) + sizeof(PcpFlashRadialStripWork));
    PcpFlashRadialStripWork *work = (PcpFlashRadialStripWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    u32 range;
    u32 i;

    memcpy(work, params, 0xD0);
    work->parts = (PcpFlashRadialStripParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
    work->updateCount = 0;
    work->renderScale = 1.0f;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    record = dds3FlashPolyList3Create(work->particleCount * 2);
    record->scale = 1.0f;
    record->drawMode = work->drawMode;
    work->resourceHandle = record;
    range = work->randomRange;
    for (i = 0; i < work->particleCount; i++) {
        work->parts[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
    }
    return work;
}

void effPCPFlashPolySphere3CreatePCF(void *data)
{
    effPCPFlashPolySphere3Create(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashRadialStripWork *effPCPFlashPolySphere3Copy(effFlashRadialStripParam_t *params)
{
    return effPCPFlashPolySphere3Create(params);
}

void effPCPFlashPolySphere3Delete(PcpFlashRadialStripWork *work)
{
    dds3FlashPolyList3Delete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolySphere3SetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolySphere3SetColor(PcpFlashRadialStripWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolySphere3SetScale(PcpFlashRadialStripWork *work, f32 value)
{
    work->renderScale = value;
}

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
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_COPYVECTOR_A_TO_C();
    /* retail multiplies the (never written) `spare` slot here and stores it back */
    SDF_VU0_LOADVECTOR_B(spare);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(spare);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(size);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(outer);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(outer);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(inner);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(inner);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(middle);
    SDF_VU0_MULVECTOR();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_LOADVECTOR(outer);
    SDF_VU0_STOREVECTOR(quad);
    mirror = (f32 *)dds3FlashPolyList3GetVertex(work->resourceHandle, index * 2 + 1);
    SDF_VU0_COPYVECTOR(mirror + 8, quad + 8);
    SDF_VU0_COPYVECTOR(mirror + 4, quad + 4);
    SDF_VU0_COPYVECTOR(mirror + 12, quad + 12);
    SDF_VU0_LOADVECTOR(inner);
    SDF_VU0_STOREVECTOR(mirror);
}

void effPCPFlashPolySphere3PartRotate(PcpFlashRadialStripWork *work, s32 index, void *orientation)
{
    PcpFlashRadialStripParticle *part;

    part = &work->parts[index];
    part->angle += part->angularStep;
}

void effPCPFlashPolySphere3Draw(PcpFlashRadialStripWork *work) {
    s128 axis;
    s32 index;
    s32 count;
    s32 lifetime;
    f32 maxThickness;
    s32 fadeIn;
    s32 fadeOut;
    f32 startA;
    f32 startB;
    f32 decay;
    s32 restart;
    u32 range;
    s32 tintColor;
    PcpFlashRadialStripParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(&axis);
    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    maxThickness = work->maxThickness;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    startA = work->initialRadius;
    startB = work->initialRadialSpeed;
    decay = work->radialDamping;
    restart = work->restartRandomly;
    range = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;

        if (lifetime < age) {
            effPCPFlashPolySphere3PartSetVC(work, index, 0);
        } else {
            if (age == 0) {
                effPCPFlashPolySphere3PartInit(work, index, &axis);
                effPCPFlashPolySphere3PartSetVertex(work, index, &axis);
                effPCPFlashPolySphere3PartSetVC(work, index, 0);
                part->thickness = maxThickness;
                part->radius = startA;
                part->radialSpeed = startB;
                part->color = EFF_NEUTRAL_COLOR;
            } else if (age > 0) {
                f32 speed = part->radialSpeed;
                s32 remain;
                f32 blend;

                part->radialSpeed = speed * decay;
                part->radius = part->radius + speed;
                effPCPFlashPolySphere3PartRotate(work, index, &axis);
                effPCPFlashPolySphere3PartSetVertex(work, index, &axis);
                age = part->age;
                if (age < fadeIn && fadeIn != 0) {
                    blend = (f32)age / (f32)fadeIn;
                } else {
                    remain = lifetime - age;
                    if (fadeOut >= remain && fadeOut != 0) {
                        blend = (f32)remain / (f32)fadeOut;
                    } else {
                        blend = 1.0f;
                    }
                }
                effPCPFlashPolySphere3PartSetVC(work, index, effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor));
            }
            if (age == lifetime && restart != 0) {
                part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
                effPCPFlashPolySphere3PartSetVC(work, index, 0);
            } else {
                part->age = part->age + 1;
            }
        }
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyList3Draw(handle);
}

PcpFlashFadingOrbitWork *effPCPFlashPolyRingExp2Create(effFlashFadingOrbitParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashOrbitParticle) + sizeof(PcpFlashFadingOrbitWork));
    PcpFlashFadingOrbitWork *work = (PcpFlashFadingOrbitWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    f32 angle;
    f32 step;
    u32 range;
    u32 i;

    memcpy(work, params, 0x50);
    work->parts = (PcpFlashOrbitParticle *)(work + 1);
    work->allocationHandle = handle;
    work->tintColor = EFF_NEUTRAL_COLOR;
    work->updateCount = 0;
    work->renderScale = 1.0f;
    if (work->randomRange == 0) {
        work->randomRange = 1;
    }
    angle = -3.14159265f / 2.0f;
    record = dds3FlashPolyListCreate(work->particleCount);
    record->scale = 1.0f;
    record->drawMode = work->drawMode;
    work->resourceHandle = record;
    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    range = work->randomRange;
    for (i = 0; i < (u32)work->particleCount; i++) {
        work->parts[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
        work->parts[i].angle = angle;
        angle += step;
    }
    return work;
}

void effPCPFlashPolyRingExp2CreatePCF(void *data)
{
    effPCPFlashPolyRingExp2Create(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashFadingOrbitWork *effPCPFlashPolyRingExp2Copy(effFlashFadingOrbitParam_t *params)
{
    return effPCPFlashPolyRingExp2Create(params);
}

void effPCPFlashPolyRingExp2Delete(PcpFlashFadingOrbitWork *work)
{
    dds3FlashPolyListDelete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyRingExp2SetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyRingExp2SetColor(PcpFlashFadingOrbitWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolyRingExp2SetScale(PcpFlashFadingOrbitWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolyRingExp2PartSetVertex(PcpFlashFadingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyListGetVertex(work->resourceHandle, index);
    f32 offset[4];
    f32 unit[4];
    f32 scaleA[4];
    f32 scaleB[4];
    f32 scaleC[4];
    f32 size;
    f32 ratio;
    f32 sinv;
    f32 height;
    f32 widthB;
    f32 widthC;

    size = part->scale;
    ratio = size / part->initialScale;
    VEC3_SPLAT(scaleA, size);
    widthB = work->acrossSpan * ratio;
    widthC = work->upSpan * ratio;
    VEC3_SPLAT(scaleB, widthB);
    VEC3_SPLAT(scaleC, widthC);
    unit[0] = sdfCos(part->angle);
    unit[1] = 0;
    sinv = sdfSin(part->angle);
    unit[2] = sinv;
    offset[0] = unit[0] * work->orbitRadius;
    offset[1] = 0;
    offset[2] = sinv * work->orbitRadius;
    height = work->tilt;
    effPCPFlashPolyRingExp2Vertex[0] = unit[0] * height;
    effPCPFlashPolyRingExp2Vertex[1] = height + -1.0f;
    effPCPFlashPolyRingExp2Vertex[2] = sinv * height;
    SDF_VU0_LOADVECTOR(effPCPFlashPolyRingExp2Vertex);
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(unit);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(scaleB);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleB);
    SDF_VU0_LOADVECTOR(scaleC);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(scaleC);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(scaleA);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleC);
    SDF_VU0_STOREVECTOR(quad + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 16);
    SDF_VU0_LOADVECTOR(offset);
    SDF_VU0_LOADVECTOR_B(scaleB);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolyRingExp2PartRotate(PcpFlashFadingOrbitWork *work, s32 index)
{
    PcpFlashOrbitParticle *part;

    part = &work->parts[index];
    part->angle += work->angularStep;
}

void effPCPFlashPolyRingExp2Draw(PcpFlashFadingOrbitWork *work) {
    s32 restart;
    s32 tintColor;
    s32 count;
    s32 index;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    s32 ramp;
    f32 maxScale;
    u32 range;
    PcpFlashOrbitParticle *part;
    dds3FlashPolyList_t *handle;

    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    ramp = work->scaleRampTime;
    maxScale = work->maxScale;
    restart = work->restartRandomly;
    range = work->randomRange;
    tintColor = work->tintColor;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;

        if (lifetime < age) {
            effPCPFlashPolyRingExp2PartSetVC(work, index, 0);
        } else {
            if (age == 0) {
                part->initialScale = maxScale;
                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = 0.0f;
                }
                effPCPFlashPolyRingExp2PartSetVertex(work, index);
                effPCPFlashPolyRingExp2PartSetVC(work, index, 0);
                part->color = EFF_NEUTRAL_COLOR;
            } else if (age > 0) {
                s32 remain;
                f32 blend;

                if (ramp == 0) {
                    part->scale = maxScale;
                } else {
                    part->scale = (maxScale * (f32)age) / (f32)ramp;
                    if (maxScale < part->scale) {
                        part->scale = maxScale;
                    }
                }
                effPCPFlashPolyRingExp2PartRotate(work, index);
                effPCPFlashPolyRingExp2PartSetVertex(work, index);
                if (age < fadeIn && fadeIn != 0) {
                    blend = (f32)age / (f32)fadeIn;
                } else {
                    remain = lifetime - age;
                    if (fadeOut >= remain && fadeOut != 0) {
                        blend = (f32)remain / (f32)fadeOut;
                    } else {
                        blend = 1.0f;
                    }
                }
                effPCPFlashPolyRingExp2PartSetVC(work, index, effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor));
            }
            if (age == lifetime && restart != 0) {
                part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % range);
                effPCPFlashPolyRingExp2PartSetVC(work, index, 0);
            } else {
                part->age = part->age + 1;
            }
        }
    }
    handle = work->resourceHandle;
    handle->origin[0] = work->origin[0];
    work->updateCount = work->updateCount + 1;
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->renderScale;
    dds3FlashPolyListDraw(handle);
}

PcpFlashOffsetRadialWork *effPCPFlashPolyCircleCreate(effPCPFlashPolyCircleParam_t *params)
{
    sdfMemHandle_t *handle = sdfMemAllocDirect(params->particleCount * sizeof(PcpFlashMotionParticle) + sizeof(PcpFlashOffsetRadialWork));
    PcpFlashOffsetRadialWork *work = (PcpFlashOffsetRadialWork *)sdfMemLock(handle);
    dds3FlashPolyList_t *record;
    u32 i;

    memcpy(work, params, 0x40);
    work->parts = (PcpFlashMotionParticle *)(work + 1);
    work->tintColor = EFF_NEUTRAL_COLOR;
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

void effPCPFlashPolyCircleCreatePCF(void *data)
{
    effPCPFlashPolyCircleCreate(dds3EffectPCPDataGetData(data, 0));
}

PcpFlashOffsetRadialWork *effPCPFlashPolyCircleCopy(effPCPFlashPolyCircleParam_t *params)
{
    return effPCPFlashPolyCircleCreate(params);
}

void effPCPFlashPolyCircleDelete(PcpFlashOffsetRadialWork *work)
{
    dds3FlashPolyList2Delete(work->resourceHandle);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyCircleSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyCircleSetColor(PcpFlashOffsetRadialWork *work, u32 value)
{
    work->tintColor = value;
}

void effPCPFlashPolyCircleSetScale(PcpFlashOffsetRadialWork *work, f32 value)
{
    work->renderScale = value;
}

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

void effPCPFlashPolyCirclePartSetVertex(PcpFlashOffsetRadialWork *work, s32 index, void *view)
{
    PcpFlashMotionParticle *part = &work->parts[index];
    f32 *quad = (f32 *)dds3FlashPolyList2GetVertex(work->resourceHandle, index);
    f32 base[4];
    f32 size[4];
    f32 step;
    f32 angle;
    f32 radius;

    step = 3.14159265f * 2.0f / (f32)(u32)work->particleCount;
    radius = part->accumulator;
    VEC3_SPLAT(size, radius);
    angle = step * (f32)index;
    SetRotateMatrixAxis(angle, (const sdfVector3_t *)view);
    base[0] = 0;
    base[1] = 1.0f;
    base[2] = 0;
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_LOADVECTOR_B(view);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(effPCPFlashPolyCircleVertex);
    SDF_VU0_STOREVECTOR(quad + 8);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad);
    SetRotateMatrixAxis(angle + step, (const sdfVector3_t *)view);
    SDF_VU0_LOADVECTOR(base);
    SDF_VU0_APPLYMATRIX_XYZ();
    SDF_VU0_LOADVECTOR_B(size);
    SDF_VU0_MULVECTOR();
    SDF_VU0_LOADVECTOR_B(effPCPFlashPolyCircleVertex);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(quad + 4);
}

void effPCPFlashPolyCircleDraw(PcpFlashOffsetRadialWork *work) {
    f32 axis[4];
    s32 restart;
    s32 tintColor;
    s32 index;
    s32 active;
    s32 count;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    f32 startA;
    f32 startB;
    f32 decay;
    f32 scale;
    PcpFlashMotionParticle *part;
    dds3FlashPolyList_t *handle;

    SDF_VU0_LOADVECTOR(aViewTarget);
    SDF_VU0_LOADVECTOR_B(aViewEye);
        SDF_VU0_SUBVECTOR();
        SDF_VU0_NORMALIZE();
    SDF_VU0_STOREVECTOR(axis);
    count = work->particleCount;
    part = work->parts;
    lifetime = work->lifetime;
    startA = work->initialRadius;
    startB = work->initialRadialSpeed;
    decay = work->radialDamping;
    fadeIn = work->fadeInTime;
    fadeOut = work->fadeOutTime;
    restart = work->restartRandomly;
    tintColor = work->tintColor;
    active = 0;
    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;

        if (lifetime < age) {
            effPCPFlashPolyCirclePartSetVC(work, index, 0);
        } else {
            if (age == 0) {
                effPCPFlashPolyCirclePartSetVertex(work, index, axis);
                effPCPFlashPolyCirclePartSetVC(work, index, 0);
                part->accumulator = startA;
                part->stepSpeed = startB;
                part->color = EFF_NEUTRAL_COLOR;
            } else if (age > 0) {
                f32 speed = part->stepSpeed;
                s32 remain;
                f32 blend;

                part->stepSpeed = speed * decay;
                part->accumulator = part->accumulator + speed;
                effPCPFlashPolyCirclePartSetVertex(work, index, axis);
                if (age < fadeIn && fadeIn != 0) {
                    blend = (f32)age / (f32)fadeIn;
                } else {
                    remain = lifetime - age;
                    if (fadeOut >= remain && fadeOut != 0) {
                        blend = (f32)remain / (f32)fadeOut;
                    } else {
                        blend = 1.0f;
                    }
                }
                active++;
                effPCPFlashPolyCirclePartSetVC(work, index, effMagaCylinderCalcColor(effColorBlend(0, part->color, blend), tintColor));
            }
            if (age == lifetime && restart != 0) {
                part->age = 0;
            } else {
                part->age = part->age + 1;
            }
        }
    }
    scale = work->originOffset * work->renderScale;
    handle = work->resourceHandle;
    work->updateCount = work->updateCount + 1;
    handle->origin[0] = work->origin[0] + axis[0] * scale;
    handle->origin[1] = work->origin[1] + axis[1] * scale;
    handle->origin[2] = work->origin[2] + axis[2] * scale;
    handle->scale = work->renderScale;
    if (active != 0) {
        dds3FlashPolyList2Draw(handle);
    }
}

EffRingWork *effPCPFlashPolyRingExpHCreate(EffRingWork *source)
{
    sdfMemHandle_t *allocation;
    EffRingWork *ring;
    effRingBlock_t *block;
    f32 angle;
    f32 step;
    u32 spread;
    u32 i;

    allocation = sdfMemAllocDirect(source->count * EFF_RING_VERTEX_BYTES + EFF_RING_HEADER_BYTES);
    block = (effRingBlock_t *)sdfMemLock(allocation);
    ring = &block->header;
    memcpy(ring, source, EFF_RING_PARAM_BYTES);
    ring->vertices = &block->vertices[0];
    ring->allocationHandle = allocation;
    ring->color = EFF_NEUTRAL_COLOR;
    ring->currentRadius = ring->param38;
    ring->currentSideOffsetB = ring->param30;
    ring->currentSideOffsetA = ring->param34;
    ring->updateCount = 0;
    ring->scale = 1.0f;
    if (ring->spread == 0) {
        ring->spread = 1;
    }
    angle = EFFECT_RING_START_ANGLE;
    ring->recordPool = dds3FlashPolyListHCreate(ring->count);
    ring->recordPool->scale = 1.0f;
    ring->recordPool->drawMode = ring->drawMode;
    step = EFFECT_RING_FULL_TURN / ring->count;
    spread = ring->spread;
    for (i = 0; i < ring->count; i++) {
        ring->vertices[i].age = -(dds3GetRandA((EffRandState *)dds3EffectBase) % spread);
        ring->vertices[i].angle = angle;
        angle += step;
    }
    return ring;
}

void effPCPFlashPolyRingExpHCreatePCF(void *table) {
    void *parameterBlock;

    parameterBlock = dds3EffectPCPDataGetData(table, 0);
    effPCPFlashPolyRingExpHCreate(parameterBlock);
}

EffRingWork *effPCPFlashPolyRingExpHCopy(EffRingWork *params)
{
    return effPCPFlashPolyRingExpHCreate(params);
}

void effPCPFlashPolyRingExpHDelete(EffRingWork *work)
{
    dds3FlashPolyListHDelete(work->recordPool);
    sdfMemFreeDirect(work->allocationHandle);
}

void effPCPFlashPolyRingExpHSetPosition(void *dst, void *src) {
    SDF_VU0_COPYVECTOR(dst, src);
}

void effPCPFlashPolyRingExpHSetColor(EffRingWork *ring, u32 color) {
    ring->color = color;
}

void effPCPFlashPolyRingExpHSetScale(EffRingWork *work, f32 scale) {
    work->scale = scale;
}

/* vu0 routine: copy the four matrix rows through vf28-vf31. */
void effPCPFlashPolyRingExpHSetRotateMatrix(EffRingWork *work, void *matrixSource) {
    SDF_VU0_LOADMATRIX(matrixSource);
    SDF_VU0_STOREMATRIX(work->recordPool->matrix);
}

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
    SDF_VU0_LOADVECTOR(effPCPFlashPolyRingExpHVertex);
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(radialDirection);
    SDF_VU0_OUTERPRODUCT();
    SDF_VU0_NORMALIZE();
    SDF_VU0_COPYVECTOR_A_TO_B();
    SDF_VU0_LOADVECTOR(sideOffsetA);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(sideOffsetA);
    SDF_VU0_LOADVECTOR(sideOffsetB);
    SDF_VU0_MULVECTOR();
    SDF_VU0_STOREVECTOR(sideOffsetB);
    SDF_VU0_COPYVECTOR_C_TO_A();
    SDF_VU0_LOADVECTOR_B(axisOffset);
    SDF_VU0_MULVECTOR();
    SDF_VU0_COPYVECTOR_A_TO_C();
    SDF_VU0_LOADVECTOR(orbitOffset);
    SDF_VU0_LOADVECTOR_B(sideOffsetB);
    SDF_VU0_STOREVECTOR(positions + 12);
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(positions + 8);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(positions + 16);
    SDF_VU0_LOADVECTOR(orbitOffset);
    SDF_VU0_LOADVECTOR_B(sideOffsetA);
    SDF_VU0_ADDVECTOR_A_C();
    SDF_VU0_ADDVECTOR();
    SDF_VU0_STOREVECTOR(positions);
    SDF_VU0_SUBVECTOR();
    SDF_VU0_SUBVECTOR();
    SDF_VU0_STOREVECTOR(positions + 4);
}

void effPCPFlashPolyRingExpHPartRotate(EffRingWork *work, s32 index) {
    EffRingParticle *particle;

    particle = &work->vertices[index];
    particle->angle = particle->angle + work->increment;
}

void effPCPFlashPolyRingExpHDraw(EffRingWork *work)
{
    s32 index;
    s32 count;
    EffRingParticle *part;
    s32 lifetime;
    s32 fadeIn;
    s32 fadeOut;
    u32 randomRange;
    s32 restart;
    s32 tintColor;
    dds3FlashPolyList_t *handle;
    f32 heightDelta;
    f32 delta;
    f32 progress;
    f32 factor;
    f32 start;
    f32 radiusEnd;
    f32 upStart;
    f32 acrossStart;
    f32 upEnd;
    f32 acrossEnd;
    f32 normalEnd;

    part = work->vertices;
    count = work->count;
    lifetime = work->duration;
    fadeIn = work->fadeIn;
    fadeOut = work->fadeOut;
    restart = work->respawn;
    randomRange = work->spread;
    tintColor = work->color;
    heightDelta = work->param4C - work->param48;
    progress = 1.0f;
    if (lifetime > 0) {
        progress = (f32)work->updateCount / (f32)lifetime;
    }
    start = work->param38;
    radiusEnd = work->param3C;
    delta = radiusEnd - start;
    work->currentRadius = start + delta * progress;
    if (start > 0.0f) {
        factor = radiusEnd / start;
    } else {
        factor = radiusEnd;
    }
    upStart = work->param30;
    acrossStart = work->param34;
    upEnd = upStart * factor;
    acrossEnd = acrossStart * factor;
    start = work->param40;
    normalEnd = work->param44;
    delta = upEnd - upStart;
    work->currentSideOffsetB = upStart + delta * progress;
    delta = acrossEnd - acrossStart;
    work->currentSideOffsetA = acrossStart + delta * progress;
    delta = normalEnd - start;
    work->currentAxisOffset = start + delta * progress;

    for (index = 0; index < count; index++, part++) {
        s32 age = part->age;
        s32 color;

        if (age == 0) {
            part->basisFactor = work->param48;
            effPCPFlashPolyRingExpHPartSetVertex(work, index);
            effPCPFlashPolyRingExpHPartSetVC(work, index, 0);
            part->color = EFF_NEUTRAL_COLOR;
        } else if (age >= lifetime) {
            if (restart != 0) {
                part->age = ~(dds3GetRandA((EffRandState *)dds3EffectBase) % randomRange);
            }
            color = 0;
            effPCPFlashPolyRingExpHPartSetVC(work, index, color);
        } else if (age > 0) {
            progress = (f32)age / (f32)lifetime;
            part->basisFactor = heightDelta * progress + work->param48;
            effPCPFlashPolyRingExpHPartRotate(work, index);
            effPCPFlashPolyRingExpHPartSetVertex(work, index);
            if (part->age < fadeIn && fadeIn != 0) {
                factor = (f32)part->age / (f32)fadeIn;
            } else if (fadeOut >= lifetime - part->age && fadeOut != 0) {
                factor = (f32)(lifetime - part->age) / (f32)fadeOut;
            } else {
                factor = 1.0f;
            }
            color = effMagaCylinderCalcColor(effColorBlend(0, part->color, factor), tintColor);
            effPCPFlashPolyRingExpHPartSetVC(work, index, color);
        }
        part->age = part->age + 1;
    }
    if (work->updateCount == lifetime) {
        work->updateCount = 0;
    } else {
        work->updateCount++;
    }
    handle = work->recordPool;
    handle->origin[0] = work->origin[0];
    handle->origin[1] = work->origin[1];
    handle->origin[2] = work->origin[2];
    handle->scale = work->scale;
    dds3FlashPolyListHDraw(handle);
}

dds3FlashPolyList_t *dds3FlashPolyListCreate(u32 count) {
    dds3FlashPolyList_t *pool;
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
    pool = (dds3FlashPolyList_t *)(block + recordBytes);
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

void dds3FlashPolyListDelete(dds3FlashPolyList_t *group) {
    sdfMatFreeDelayed(group->resource);
    sdfMemFreeDirect(group->buffer);
}

void dds3FlashPolyListDraw(dds3FlashPolyList_t *work)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT] __attribute__((aligned(16)));
    sdfDrawTag_t *list;
    s32 remaining;
    s32 fanCount;
    u64 *packet;
    sdfDrawTag_t *stateList;

    list = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    sdfDrawTagInit(list);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[0] = work->scale;
    matrix[5] = work->scale;
    matrix[10] = work->scale;
    matrix[12] = work->origin[0];
    matrix[13] = work->origin[1];
    matrix[14] = work->origin[2];
    SDF_VU0_LOADMATRIX(matrix);
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
        D_0031F228[work->drawMode]->regFunc(D_0031F228[work->drawMode], list);
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
        (*effFlashOtNode).regFunc(&(*effFlashOtNode), stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x42;
        packet[5] = 0x42;
        sdfDrawTagLink(list, (u32)packet);
        (*effFlashOtNode).regFunc(&(*effFlashOtNode), list);
    }
}

void *dds3FlashPolyListGetVertex(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_FAN_POSITION_BYTES;
}

void *dds3FlashPolyListGetVC(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_FAN_COLOR_BYTES;
}

void dds3FlashPolyListSetBlend(dds3FlashPolyList_t *pool, u32 mode) {
    pool->drawMode = mode;
}

void dds3FlashPolyListSetABGR(dds3FlashPolyList_t *group, u32 color) {
    group->color = color;
}

void dds3FlashPolyListSetScale(dds3FlashPolyList_t *work, f32 scale) {
    work->scale = scale;
}

dds3FlashPolyList_t *dds3FlashPolyList2Create(s32 triangleCount) {
    dds3FlashPolyList_t *pool;
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
    pool = (dds3FlashPolyList_t *)(block + (positionWordCount + colorWordCount));
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

void dds3FlashPolyList2Delete(dds3FlashPolyList_t *group) {
    sdfMatFreeDelayed(group->resource);
    sdfMemFreeDirect(group->buffer);
}

void dds3FlashPolyList2Draw(dds3FlashPolyList_t *pool)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT];
    sdfDrawTag_t *packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    s32 remainingVertices;
    sdfGrOt_t *surface;

    sdfDrawTagInit(packet);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[0] = pool->scale;
    matrix[5] = pool->scale;
    matrix[10] = pool->scale;
    matrix[12] = pool->origin[0];
    matrix[13] = pool->origin[1];
    matrix[14] = pool->origin[2];
    SDF_VU0_LOADMATRIX(matrix);
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
    surface->regFunc(surface, packet);
}

void *dds3FlashPolyList2GetVertex(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_TRIANGLE_POSITION_BYTES;
}

void *dds3FlashPolyList2GetVC(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_TRIANGLE_COLOR_BYTES;
}

dds3FlashPolyList_t *dds3FlashPolyList3Create(s32 quadCount) {
    dds3FlashPolyList_t *pool;
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
    pool = (dds3FlashPolyList_t *)(block + (positionWordCount + colorWordCount));
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

void dds3FlashPolyList3Delete(dds3FlashPolyList_t *group) {
    sdfMatFreeDelayed(group->resource);
    sdfMemFreeDirect(group->buffer);
}

void dds3FlashPolyList3Draw(dds3FlashPolyList_t *pool)
{
    f32 matrix[EFF_MATRIX_WORD_COUNT];
    sdfDrawTag_t *packet = (sdfDrawTag_t *)sdfGrAllocPacketBuff(EFF_PACKET_LIST_BYTES);
    s32 remainingVertices;
    sdfGrOt_t *surface;

    sdfDrawTagInit(packet);
    SDF_VU0_STOREIDENTITY(matrix);
    matrix[0] = pool->scale;
    matrix[5] = pool->scale;
    matrix[10] = pool->scale;
    matrix[12] = pool->origin[0];
    matrix[13] = pool->origin[1];
    matrix[14] = pool->origin[2];
    SDF_VU0_LOADMATRIX(matrix);
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
    surface->regFunc(surface, packet);
}

void *dds3FlashPolyList3GetVertex(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_QUAD_POSITION_BYTES;
}

void *dds3FlashPolyList3GetVC(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->auxRecordBase + groupIndex * EFF_QUAD_COLOR_BYTES;
}

dds3FlashPolyList_t *dds3FlashPolyListHCreate(u32 fanCount) {
    dds3FlashPolyList_t *pool = dds3FlashPolyListCreate(fanCount);

    SDF_VU0_STOREIDENTITY(pool->matrix);
    return pool;
}

void dds3FlashPolyListHDelete(dds3FlashPolyList_t *pool) {
    dds3FlashPolyListDelete(pool);
}

void dds3FlashPolyListHDraw(dds3FlashPolyList_t *work)
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
    SDF_VU0_LOADMATRIX(matrix);
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
        D_0031F2A8[work->drawMode]->regFunc(D_0031F2A8[work->drawMode], list);
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
        (*effFlashOtNode).regFunc(&(*effFlashOtNode), stateList);
        packet = (u64 *)sdfGrAllocPacketBuff(EFF_STATE_PACKET_BYTES);
        packet[0] = 2;
        packet[1] = 0x5000000210000000ULL;
        packet[2] = 0x1000000000008001ULL;
        packet[3] = 0xE;
        packet[4] = 0x42;
        packet[5] = 0x42;
        sdfDrawTagLink(list, (u32)packet);
        (*effFlashOtNode).regFunc(&(*effFlashOtNode), list);
    }
}

void *dds3FlashPolyListHGetVertex(dds3FlashPolyList_t *group, s32 groupIndex) {
    return group->recordBase + groupIndex * EFF_FAN_POSITION_BYTES;
}

u32 *dds3FlashPolyListHGetVC(dds3FlashPolyList_t *group, s32 groupIndex) {
    return (u32 *)(group->auxRecordBase + groupIndex * EFF_FAN_COLOR_BYTES);
}

void dds3FlashPolyListHSetBlend(dds3FlashPolyList_t *pool, u32 mode) {
    pool->drawMode = mode;
}

void dds3FlashPolyListHSetABGR(dds3FlashPolyList_t *pool, u32 color) {
    pool->color = color;
}

void dds3FlashPolyListHSetScale(dds3FlashPolyList_t *work, f32 scale) {
    work->scale = scale;
}
