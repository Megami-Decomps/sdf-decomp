#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"
#include "effect.h"
#include "billboard.h"

/* Particle emitters, history ribbons and billboard dispatch, reconstructed from
 * the shared DDS engine and checked against SLUS_209.11. All emitter views
 * retain the native 0x150-byte prefix; the kind-specific tail follows it. */

typedef struct parSlot_t parSlot_t;
typedef struct parTable_t parTable_t;
typedef struct parKindState_t parKindState_t;
typedef struct parDispatch_t parDispatch_t;
typedef struct effectBufferRecord_t effectBufferRecord_t;
typedef struct effectBufferTail_t effectBufferTail_t;
typedef struct effEmitterHead_t effEmitterHead_t;
typedef struct effTemplatePacketList_t effTemplatePacketList_t;
typedef struct parColorRamp_t parColorRamp_t;
typedef struct parObj_t parObj_t;
typedef struct parObjClientView_t parObjClientView_t;
typedef struct parKindResource_t parKindResource_t;
typedef struct parHistory_t parHistory_t;
typedef struct parHistoryTable_t parHistoryTable_t;
typedef struct effParticleRecord_t effParticleRecord_t;
typedef struct effParticleOverlay_t effParticleOverlay_t;
typedef struct effTemplateScaleRecord_t effTemplateScaleRecord_t;
typedef struct effLookAtRingEmitter_t effLookAtRingEmitter_t;
typedef struct effRingEmitter_t effRingEmitter_t;
typedef struct effDiscEmitter_t effDiscEmitter_t;
typedef struct effBallisticEmitter_t effBallisticEmitter_t;
typedef struct effBurstEmitter_t effBurstEmitter_t;
typedef struct effSphereEmitter_t effSphereEmitter_t;
typedef struct effExpandingRingEmitter_t effExpandingRingEmitter_t;
typedef struct effConeEmitter_t effConeEmitter_t;
typedef struct effOffsetGravityEmitter_t effOffsetGravityEmitter_t;
typedef struct effDiscAuxEmitter_t effDiscAuxEmitter_t;
typedef struct billChildPayload_t billChildPayload_t;
typedef struct billData_t billData_t;
typedef struct billOut_t billOut_t;
typedef struct effTrackPolyList_t effTrackPolyList_t;


typedef struct {
    struct MdlCtx *model;
    s32 idA;
    s32 idB;
    f32 startFrame;
    f32 endFrame;
    u32 sampleInterval;
    s32 historyLength;
    u32 unk1C;
    u32 kind;
    u32 gradientColors[4];
} effTrackPolyParams_t;


typedef struct parSlot_t {
    u128 *points;
    u16 pointCount;
    u8 pad06[2];
    u32 color;
    f32 billboardScale;
} parSlot_t;

typedef struct parTable_t {
    u16 slotCount;
    u16 pointCapacity;
    parSlot_t *slots;
    billObj_t **billboardRef;
    sdfMemHandle_t *allocation;
} parTable_t;

typedef struct {
    struct parTable_t *table; /* +8: kind 1 history source */
    u8 opaque0C[0xC];       /* No other kind-1 payload words are read here. */
} parKindHistoryPayload_t;

typedef struct {
    f32 scale;                      /* +8: shared by kinds 2..4 */
    void *dispatchParameter;        /* +0xC: kinds 2 and 3 */
    struct parSystem_t *drawSystem;   /* +0x10: kind 2 */
    u8 opaque14[4];                 /* Kind 2's final word is unobserved. */
} parKindCrossStripPayload_t;

typedef struct {
    f32 scale;                      /* +8: shared by kinds 2..4 */
    void *dispatchParameter;        /* +0xC: kinds 2 and 3 */
    u8 opaque10[4];                 /* Kind 3's first system word is unobserved. */
    struct parSystem_t *drawSystem;   /* +0x14: kind 3 */
} parKindCrossTrianglePayload_t;

typedef struct {
    f32 scale;                          /* +8: shared by kinds 2..4 */
    u32 firstGradientColor;             /* +0xC: kind 4 */
    u32 secondGradientColor;            /* +0x10: kind 4 */
    struct effTrackPolyList_t *modelList; /* +0x14: kind 4 */
} parKindTrackPolygonPayload_t;

typedef union {
    parKindHistoryPayload_t history;
    parKindCrossStripPayload_t crossStrip;
    parKindCrossTrianglePayload_t crossTriangle;
    parKindTrackPolygonPayload_t trackPolygon;
    u8 opaque[0x10];
} parKindPayload_t;

typedef struct parKindState_t {
    u16 kind; /* +0: ParKindStateKind tag */
    u8 pad02[2];
    /* The halfword bitfield preserves native int-alias dependencies; see
     * dds-decomp/docs/idioms.md, "Trace-justified alias sets: emitter subrecord count". */
    u32 templateEntryCount : 16; /* 0x04: subrecords per particle in template clones */
    u32 : 16;
    parKindPayload_t payload; /* +8: tag-specific pointer/scalar view */
} parKindState_t;

typedef struct parDispatch_t {
    void *(*func)();
    void (*update)(struct parObj_t *);
    void (*destroy)();
} parDispatch_t;

typedef struct effectBufferRecord_t {
    f32 position[4];
    f32 velocity[4]; /* 0x10 */
    s32 age;        /* 0x20: live frame age; negative values encode delays or initialization states. */
    u32 color;      /* 0x24: packed color passed to billboard and subeffect drawing. */
    f32 scale;      /* 0x28: current billboard scale */
    f32 spin;       /* 0x2C: current billboard rotation */
    f32 motionMagnitude;       /* 0x30 */
    u8 pad34[0x0C];
} effectBufferRecord_t;

typedef struct effectBufferTail_t {
    sdfMemHandle_t *allocation;
    effectBufferRecord_t *records;
} effectBufferTail_t;

typedef struct parColorRamp_t {
    s32 mode;          /* 0x00: 1 = no ramp, 2 = color0-to-color1 only */
    u32 color0;        /* 0x04 */
    u32 color1;        /* 0x08 */
    u32 color2;        /* 0x0C */
    u32 alpha;         /* 0x10 */
    s32 fadeInFrames;  /* 0x14 */
    s32 fadeOutFrames; /* 0x18 */
    s32 frames;        /* 0x1C */
    s32 rampFrames;    /* 0x20 */
    f32 r01;           /* 0x24 */
    f32 r12;           /* 0x28 */
    f32 g01;           /* 0x2C */
    f32 g12;           /* 0x30 */
    f32 b01;           /* 0x34 */
    f32 b12;           /* 0x38 */
    u32 fadeIn;        /* 0x3C */
    u32 fadeOut;       /* 0x40 */
} parColorRamp_t;
typedef struct effEmitterHead_t {
    f32 origin[4];         /* 0x00 */
    f32 scale[3];
    u8 pad1C[4];
    u32 particleCount;
    s32 lifetimeFrames;
    s32 billboardResource;
    s16 drawKind;
    u8 pad2E[2];
    parKindState_t sub;      /* 0x30: kind-tagged shared drawing owner */
    parColorRamp_t fade;   /* 0x48 */
    f32 scaleEnd;          /* 0x8C */
    f32 spinDegrees;       /* 0x90 */
    f32 speedJitter;       /* 0x94 */
    f32 spinJitter;        /* 0x98 */
    u8 pad9C[4];
    s32 templateSize;
    u32 restartStepCount;
    u8 padA8[8];
    f32 matrix[16];        /* 0xB0 */
    u32 colorMask;         /* 0xF0 */
    billObj_t *billboard; /* 0xF4: owned billboard shared with the particle view */
    effectBufferTail_t *buffer; /* 0xF8 */
    u32 pendingRestartSteps;
    f32 backupMatrix[16];
    u8 pad140[2];
    u16 active;            /* 0x142 */
    u8 pad144[0xC];
} effEmitterHead_t;

typedef struct effTemplatePacketList_t {
    effEmitterHead_t head; /* Common runtime prefix, 0x150 bytes. */
    u8 randomSphere; /* 0x150 */
    u8 restart; /* 0x151 */
    u8 pad152[2];
    union {
        s32 decayStep; /* 0x154: subtraction from each later packet tag */
        u32 delayRange; /* 0x154: bounded-radius spawn delay */
    };
    f32 recordScale; /* 0x158 */
    union {
        f32 tailValues[4]; /* 0x15C-0x168: variant-specific scaled values */
        u32 tailWords[4];
        struct {
            f32 targetRadius; /* 0x15C */
            f32 motionMagnitude; /* 0x160 */
            f32 accelerationPct; /* 0x164 */
            f32 unk168;
        };
    };
    parHistory_t *recordList; /* 0x16C: start of the history descriptors */
    s32 recordsPerPacket; /* 0x170 */
    sdfMemHandle_t *listAllocation; /* 0x174: allocation descriptor */
    void *auxiliaryData; /* 0x178: optional 16 bytes per packet */
    sdfMemHandle_t *auxiliaryAllocation; /* 0x17C: allocation descriptor */
} effTemplatePacketList_t;


typedef struct parObj_t {
    f32 origin[4];                /* 0x00 */
    f32 billboardScale;           /* 0x10: radial packet's base scale */
    f32 billboardScaleY;          /* 0x14 */
    u8 pad18[8];
    s32 particleCount;            /* 0x20 */
    s32 lifetimeFrames;           /* 0x24 */
    s32 billboardCloneMarker;     /* 0x28: PAR_BILLBOARD_CLONE_FROM_RESOURCE */
    s16 billboardMode;            /* 0x2C */
    u8 pad2E[2];
    parKindState_t kindState;       /* 0x30 */
    parColorRamp_t colorRamp;       /* 0x48 */
    f32 scaleEnd;                /* 0x8C: rendered scale after interpolation. */
    f32 spinDegrees;             /* 0x90: converted to radians by the client update. */
    f32 billboardScaleJitter;     /* 0x94 */
    f32 initialPhaseJitter;       /* 0x98 */
    u8 alternateSpin;            /* 0x9C: selects the sign of particle rotation. */
    u8 pad9D[7];
    u32 restartStepCount;         /* 0xA4 */
    u8 padA8[8];
    f32 matrix[16];               /* 0xB0 */
    u32 parameterWord;           /* 0xF0: set through dds3ParticleSetRGBA. */
    billObj_t *billboard;           /* 0xF4 */
    effectBufferTail_t *buffer;     /* 0xF8 */
    u32 pendingRestartSteps;      /* 0xFC: count, not a pointer */
    f32 sourceMatrix[16];         /* 0x100 */
    u16 dispatchIndex;            /* 0x140: object dispatch, distinct from kind */
    u16 restartFlag;              /* 0x142 */
    u8 pad144[0xC];
} parObj_t;

typedef struct parObjClientView_t {
    parObj_t owner;
    u8 mode150; /* 0x150 */
    u8 mode151; /* 0x151 */
    u8 opaque152[0x26];
} parObjClientView_t;

typedef struct parKindResource_t {
    s32 type;
    s32 offset;
    u8 opaque08[8];
} parKindResource_t;

typedef struct parHistory_t {
    s32 nextIndex;
    s32 count;
    f32 (*points)[3];
} parHistory_t;

typedef struct parHistoryTable_t {
    parHistory_t *records;
    s32 capacity;
} parHistoryTable_t;

typedef struct {
    parObj_t head;
    u8 axisMode;                 /* 0x150 */
    u8 loop;                     /* 0x151: rearm expired particles */
    u8 pad152[2];
    u32 spawnDelayFrames;        /* 0x154 */
    f32 initialRadius;           /* 0x158 */
    f32 rotationStepDegrees;     /* 0x15C: native updater converts to radians */
    f32 targetRadius;
    f32 initialRadiusJitter;
    f32 targetRadiusJitter;
    parHistoryTable_t history;
    sdfMemHandle_t *childHandle;     /* 0x174: additional owned allocation */
    f32 previousOrigin[3];       /* 0x178: native updater caches origin here */
} parBurstEmitter_t;

typedef struct {
    f32 position[4];
    f32 radialOffset[3];
    u8 pad1C[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 initialPhaseRadians; /* initialized as a full-turn phase; later use is not established */
    f32 rotationAxis[3];
    f32 radiusStep;
} parBurstPacket_t;

typedef struct effParticleRecord_t {
    f32 x;
    f32 y;
    f32 z;
    u8 opaque0C[0x14];
    s32 frame;
    u32 color;
    f32 speed;
    f32 angle;
    u8 opaque30[0x10];
} effParticleRecord_t;

typedef struct effParticleOverlay_t {
    f32 x;
    f32 y;
    f32 z;
    u8 pad0C[4];
    f32 speed;
    u8 pad14[0x10];
    u32 drawEndFrame;
    u8 pad28[0x24];
    u32 color;
    u8 pad50[8];
    u32 alpha;
    u8 pad5C[0x30];
    f32 lastSpeed;
    u8 pad90[4];
    f32 speedJitter;
    f32 angleJitter;
    u8 pad9C[0x54];
    u32 colorMask;
    u8 padF4[4];
    effectBufferTail_t *buffer;
} effParticleOverlay_t;

typedef struct effTemplateScaleRecord_t {
    u8 opaque00[0x20];
    u32 initialTag;
    u8 opaque24[0x10];
    f32 scale;
    u8 opaque38[8];
} effTemplateScaleRecord_t;

typedef struct effLookAtRingEmitter_t {
    effEmitterHead_t head; /* shared emitter prefix, 0x150 bytes */
    u8 mode;             /* 0x150 */
    u8 loop;             /* 0x151 */
    u8 opaque152[10];
    u32 period;          /* 0x15C */
    f32 rotationStepDegrees;            /* 0x160 */
    f32 stepLength;            /* 0x164 */
} effLookAtRingEmitter_t;

/* Each emitter uses a 64-byte packet, with its own four-word motion state. */
typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 phase;
    f32 radialDistance;
    f32 verticalSpeed;
    f32 verticalOffset;
} parSpiralPacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 waveAmplitude;
    f32 wavePhase;
    f32 previousWaveValue;
    f32 verticalVelocity;
} parSmokePacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    u8 unk30[8];
    f32 verticalAcceleration;
    u8 unk3C[4];
} parSparkPacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    u8 unk30[4];
    f32 phaseAngle;
    u8 unk38[8];
} parRadiatePacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 axisX;
    f32 axisY;
    f32 axisZ;
    f32 radiusStep;
} parSphereSpiralPacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 radius;
    u8 unk34[12];
} parSphereRingPacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 motionMagnitude;
    f32 boundaryRadius;
    u8 unk38[8];
} parSphereAtomPacket_t;

typedef struct {
    f32 pos[4];
    f32 vel[4];
    s32 age;
    u32 color;
    f32 billboardScale;
    f32 spin;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 offsetAcceleration;
} parSparkOffsetPacket_t;

typedef struct effRingEmitter_t {
    effEmitterHead_t head;
    u8 flag150;
    u8 loop;
    u8 pad152[6];
    f32 radius;      /* 0x158 */
    u32 period;      /* 0x15C */
    f32 verticalSpeed;
    f32 rotationStepDegrees;
    f32 targetRadius;
    f32 verticalAccelerationPct;
    f32 radiusRange; /* 0x170 */
    f32 targetRadiusJitter;  /* 0x174 */
} effRingEmitter_t;

typedef struct effDiscEmitter_t {
    effEmitterHead_t head;
    u8 loop;
    u8 pad151[3];
    u32 spread;      /* 0x154 */
    f32 radius;      /* 0x158 */
    f32 verticalSpeed;
    f32 phaseStepDegrees;
    f32 waveAmplitude;
    f32 waveGrowth;
    f32 verticalAccelerationPct;
    f32 verticalSpeedJitter;     /* 0x170 */
    f32 waveAmplitudeJitter;     /* 0x174 */
} effDiscEmitter_t;

typedef struct effBallisticEmitter_t {
    effEmitterHead_t head;
    u8 mode;         /* 0x150 */
    u8 loop;         /* 0x151 */
    u8 pad152[2];
    u32 spread;      /* 0x154 */
    u8 randomDir;    /* 0x158 */
    u8 pad159[3];
    f32 radius;      /* 0x15C */
    f32 cone;        /* 0x160 */
    f32 gravity;     /* 0x164 */
    f32 speed;       /* 0x168 */
    f32 decayPct;    /* 0x16C */
    f32 jitter;      /* 0x170 */
} effBallisticEmitter_t;

typedef struct effBurstEmitter_t {
    effEmitterHead_t head;
    u8 mode;         /* 0x150 */
    u8 loop;         /* 0x151 */
    u8 pad152[2];
    u32 spread;      /* 0x154 */
    f32 speed;       /* 0x158 */
    f32 spinRate;    /* 0x15C */
    f32 targetRadius;
    f32 jitterA;     /* 0x164 */
    f32 jitterB;     /* 0x168 */
} effBurstEmitter_t;

typedef struct effSphereEmitter_t {
    effEmitterHead_t head;
    u8 loop;
    u8 pad151[3];
    u32 spread;      /* 0x154 */
    f32 radius;      /* 0x158 */
    f32 verticalSpeed;
    f32 phaseStepDegrees;
    f32 waveAmplitude;
    f32 waveGrowth;
    f32 verticalAccelerationPct;
    f32 verticalSpeedJitter;
    f32 waveAmplitudeJitter;
    f32 radiusJitter;
} effSphereEmitter_t;

typedef struct effExpandingRingEmitter_t {
    effEmitterHead_t head;
    u8 pad150;
    u8 loop;         /* 0x151 */
    u8 pad152[6];
    f32 radius;      /* 0x158 */
    f32 jitter;      /* 0x15C */
    u32 period;      /* 0x160 */
    f32 spinRate;    /* 0x164 */
} effExpandingRingEmitter_t;

typedef struct effConeEmitter_t {
    effEmitterHead_t head;
    u8 mode;         /* 0x150 */
    u8 loop;         /* 0x151 */
    u8 pad152[2];
    u32 spread;      /* 0x154 */
    u8 pad158[4];
    f32 radius;      /* 0x15C */
    f32 cone;        /* 0x160 */
    f32 gravity;     /* 0x164 */
    f32 speed;       /* 0x168 */
    f32 decayPct;    /* 0x16C */
    u8 pad170[4];
    f32 degrees;     /* 0x174 */
} effConeEmitter_t;

typedef struct effOffsetGravityEmitter_t {
    effEmitterHead_t head;
    u8 mode;         /* 0x150 */
    u8 loop;         /* 0x151 */
    u8 pad152[2];
    u32 spread;      /* 0x154: random activation delay bound */
    u8 randomDir;    /* 0x158 */
    u8 pad159[3];
    f32 radius;      /* 0x15C */
    f32 cone;        /* 0x160: horizontal direction weight */
    f32 gravity;     /* 0x164: divided by 100 for acceleration */
    f32 speed;       /* 0x168: launch magnitude */
    f32 decayPct;    /* 0x16C */
    f32 jitter;      /* 0x170: launch magnitude jitter */
} effOffsetGravityEmitter_t;

typedef struct effDiscAuxEmitter_t {
    effEmitterHead_t head;
    u8 loop;
    u8 flag151;
    u8 pad152[2];
    u32 spread;      /* 0x154 */
    f32 radius;      /* 0x158 */
    f32 verticalSpeed;
    f32 phaseStepDegrees;
    f32 waveAmplitude;
    f32 waveGrowth;
    f32 verticalAccelerationPct;
    f32 verticalSpeedJitter;
    f32 waveAmplitudeJitter;
    f32 (*aux)[4];   /* 0x178: 16 bytes per packet */
} effDiscAuxEmitter_t;


#define VU0_COPY_VF24_TO_VF28() __asm__ volatile ( \
    ".set noreorder\n\tvmove.xyzw " "vf28" ", " "vf24" "\n\t.set reorder")

#define VU0_COPY_VF25_TO_VF29() __asm__ volatile ( \
    ".set noreorder\n\tvmove.xyzw " "vf29" ", " "vf25" "\n\t.set reorder")

#define VU0_COPY_VF26_TO_VF30() __asm__ volatile ( \
    ".set noreorder\n\tvmove.xyzw " "vf30" ", " "vf26" "\n\t.set reorder")

#define VU0_COPY_VF27_TO_VF31() __asm__ volatile ( \
    ".set noreorder\n\tvmove.xyzw " "vf31" ", " "vf27" "\n\t.set reorder")

#define VU0_STORE_VF12(dst) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 " "vf12" ", 0(%0)\n\t.set reorder" \
    : : "r" (dst) : "memory")

#define VU0_STORE_VF11_UNCLOBBERED(dst) __asm__ volatile ( \
    ".set noreorder\n\tsqc2 " "vf11" ", 0(%0)\n\t.set reorder" \
    : : "r" (dst))


enum ParKindStateKind {
    PAR_KIND_STATE_HISTORY = 1,
    PAR_KIND_STATE_BILLBOARD_CROSS_STRIP = 2,
    PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE = 3,
    PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES = 4
};

enum { PAR_BILLBOARD_CLONE_FROM_RESOURCE = -1 };

enum {
    PAR_BURST_RANDOM_AXIS = 1
};

#define EFF_TEMPLATE_TAIL_OFFSET 0x150
#define EFF_PACKET_INITIAL_TAG 0xF0000001
#define PAR_PACKET_INITIAL_TAG 0xF0000001

extern f32 parTrack_w[];
extern f32 parLook[];
extern u8 aViewTarget[], aViewEye[];
extern parDispatch_t dds3ParticleFuncTable[];
extern effRandState_t dds3EffectBase[];
extern s32 ParRand[];
extern f32 D_003171A0[4];
extern u8 aViewDown[];
extern void (*dds3ParticleAllBuffInitTable[])();
extern void (*dds3ParticleSetScaleTable[])(parObj_t *, f32);

extern void *memset(void *dst, s32 value, u32 size);
extern void *memcpy(void *dst, const void *src, u32 size);

void dds3ParticleManagerInit(void);
effectBufferTail_t *dds3Particle_PhysicsCreate(s32 recordCount);
void dds3Particle_PhysicsDelete(effectBufferTail_t *buffer);
void dds3Particle_BasicCreate(void *effect);
void dds3Particle_BasicDelete(void *effect);
void dds3Particle_SpiralAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SpiralSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SpiralCreate(effTemplatePacketList_t *source);
void dds3Particle_SpiralDelete(void *effect);
void dds3Particle_SpiralBuffInit(effRingEmitter_t *effect, u32 packetIndex);
void dds3Particle_SpiralDraw(effRingEmitter_t *effect);
void dds3Particle_SmokeAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SmokeSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SmokeCreate(effTemplatePacketList_t *source);
void dds3Particle_SmokeDelete(void *effect);
void dds3Particle_SmokeBuffInit(effDiscEmitter_t *effect, u32 packetIndex);
void dds3Particle_SmokeDraw(effDiscEmitter_t *effect);
void dds3Particle_SparkAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SparkSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SparkCreate(effTemplatePacketList_t *source);
void dds3Particle_SparkDelete(void *effect);
void dds3Particle_SparkBuffInit(effBallisticEmitter_t *effect, s32 index);
void dds3Particle_SparkDraw(effBallisticEmitter_t *effect);
void dds3Particle_RadiateAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_RadiateSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_RadiateCreate(effTemplatePacketList_t *source);
void dds3Particle_RadiateDelete(void *effect);
void dds3Particle_RadiateBuffInit(effLookAtRingEmitter_t *effect, u32 packetIndex);
void dds3Particle_RadiateDraw(effLookAtRingEmitter_t *effect);
void dds3Particle_SphereSpiralAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SphereSpiralSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SphereSpiralCreate(effTemplatePacketList_t *source);
void dds3Particle_SphereSpiralDelete(void *effect);
void dds3Particle_SphereSpiralBuffInit(effBurstEmitter_t *effect, u32 packetIndex);
void dds3Particle_SphereSpiralDraw(effBurstEmitter_t *effect);
void dds3Particle_SphereSmokeAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SphereSmokeSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SphereSmokeCreate(effTemplatePacketList_t *source);
void dds3Particle_SphereSmokeDelete(void *effect);
void dds3Particle_SphereSmokeBuffInit(effSphereEmitter_t *effect, u32 packetIndex);
void dds3Particle_SphereSmokeDraw(effSphereEmitter_t *effect);
void dds3Particle_SphereRingAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SphereRingSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SphereRingCreate(effTemplatePacketList_t *source);
void dds3Particle_SphereRingDelete(void *effect);
void dds3Particle_SphereRingBuffInit(effExpandingRingEmitter_t *effect, u32 packetIndex);
void dds3Particle_SphereRingDraw(effExpandingRingEmitter_t *effect);
void dds3Particle_Spark2AllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_Spark2SetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_Spark2Create(effTemplatePacketList_t *source);
void dds3Particle_Spark2Delete(void *effect);
void dds3Particle_Spark2BuffInit(effConeEmitter_t *effect, s32 packetIndex);
void dds3Particle_Spark2Draw(effConeEmitter_t *effect);
void dds3Particle_SphereAtomAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SphereAtomSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SphereAtomCreate(effTemplatePacketList_t *source);
void dds3Particle_SphereAtomDelete(void *effect);
void dds3Particle_SphereAtomBuffInit(effTemplatePacketList_t *effect, s32 index);
void dds3Particle_SphereAtomDraw(effTemplatePacketList_t *effect);
void dds3Particle_SpiralBlendColor(effParticleOverlay_t *effect);
void dds3Particle_SparkOffsetAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SparkOffsetSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SparkOffsetCreate(effTemplatePacketList_t *source);
void dds3Particle_SparkOffsetDelete(void *effect);
void dds3Particle_SparkOffsetBuffInit(effOffsetGravityEmitter_t *effect, s32 index);
void dds3Particle_SparkOffsetDraw(effOffsetGravityEmitter_t *effect);
void dds3Particle_SmokeOffsetAllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SmokeOffsetSetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SmokeOffsetCreate(effTemplatePacketList_t *source);
void dds3Particle_SmokeOffsetDelete(effTemplatePacketList_t *effect);
void dds3Particle_SmokeOffsetBuffInit(effDiscAuxEmitter_t *effect, u32 packetIndex);
void dds3Particle_SmokeOffsetDraw(effDiscAuxEmitter_t *effect);
void dds3Particle_SphereSpiral2AllBuffInit(effTemplatePacketList_t *effect);
void dds3Particle_SphereSpiral2SetScale(float scale, effTemplatePacketList_t *effect);
effTemplatePacketList_t *dds3Particle_SphereSpiral2Create(effTemplatePacketList_t *source);
void dds3Particle_SphereSpiral2Delete(parBurstEmitter_t *obj);
void dds3Particle_SphereSpiral2BuffInit(parBurstEmitter_t *effect, u32 particleIndex);
void dds3Particle_SphereSpiral2Draw(parBurstEmitter_t *effect);
void dds3ParticleSetColorTable(parColorRamp_t *p, s32 frames);
parTable_t *dds3Particle_BlurCreate(u32 slotCount, u16 pointCapacity, billObj_t **billboardRef);
void dds3Particle_BlurDelete(parTable_t *table);
void dds3Particle_BlurSet(parTable_t *table, s32 slotIndex, u32 color, f32 billboardScale);
void dds3Particle_BlurSetExp(parTable_t *table, s32 slotIndex, const f32 *origin, u32 color,
                   parHistoryTable_t *source, f32 scale);
void dds3Particle_BlurDraw(parTable_t *table);
void dds3Particle_BlurClearItem(parTable_t *table, s32 slotIndex);
void dds3Particle_TrackSetExp(parSystem_t *system, s32 index, const f32 *origin, u32 color, parHistoryTable_t *source);
void dds3Particle_Track2SetExp(parSystem_t *system, s32 index, const f32 *origin, u32 color, parHistoryTable_t *source);
u32 dds3ParticleColorBlend(u32 colorA, u32 colorB);
void dds3ParticleCreate(s32 dispatchIndex, void *creationData);
void dds3ParticleDelete(parObj_t *obj);
void dds3ParticleDraw(parObj_t *effect);
parObj_t *dds3ParticleCopy(parObj_t *obj);
void dds3ParticleReset(parObj_t *obj);
void dds3ParticleSetScale(parObj_t *obj, f32 factor);
u16 dds3ParticleGetState(parObj_t *obj);
void dds3ParticleSetPosition(void *destination, void *source);
void dds3ParticleSetRotateMatrix(parObj_t *effect, void *matrix);
void dds3ParticleSetRGBA(parObj_t *work, u32 parameterWord);
void dds3ParticleSetTypeFlag(parObj_t *object, s32 mode);
u8 dds3ParticleGetLoop(parObj_t *object);
parObj_t *dds3ParticleExpCreate(s32 kind, parKindResource_t *resource);
parObj_t *dds3ParticleExpCopy(parObj_t *source);
void dds3ParticleExpDelete(parObj_t *object);
void dds3ParticleExpDraw(parObj_t *effect);
void dds3ParticleExpReset(parObj_t *obj);
void dds3ParticleExpSetScale(float scale, parObj_t *work);
u16 dds3ParticleExpGetState(parObj_t *obj);
void dds3ParticleExpSetPosition(void *dst, void *src);
void dds3ParticleExpSetRotateMatrix(parObj_t *effect, void *matrix);
void dds3ParticleExpSetRGBA(parObj_t *work, u32 parameterWord);
void dds3ParticleExpSetLoop(parObj_t *work, u8 mode);
u8 dds3ParticleExpGetLoop(parObj_t *work);
void dds3Particle_EffectInit(parKindState_t *obj);
void dds3Particle_EffectSet(parKindState_t *work, s32 index, u32 color, f32 speed);
void dds3Particle_EffectReset(parKindState_t *work, s32 index);
void dds3Particle_TrackSet(parSystem_t *system, s32 index, u32 color);
void dds3Particle_Track2Set(parSystem_t *system, s32 index, u32 color);
void dds3Particle_Track3Set(effTrackPolyList_t *list, s32 index, u32 color);
u32 dds3Particle_TrackPolyGetRecTiming();
void sdfRandomizeByTime(effRandState_t *state);
sdfMemHandle_t *sdfMemAllocDirect(s32 requestedBytes);
void *sdfMemLock(sdfMemHandle_t *allocation);
void sdfMemFreeDirect(sdfMemHandle_t *allocation);
billObj_t *dds3BillboardGeneral(s32 index);
void dds3BillboardSetScale(billObj_t *effect, float x, float y);
void dds3BillboardSetMode(billObj_t *effect, s32 mode);
parSystem_t *dds3TrackPolyCreate(s32 count, s32 perCell, s32 groupDivisor, u32 kind);
void dds3TrackPolyVCSet(void *work, s32 sub, void *dispatchArgA, void *dispatchArgB);
void dds3TrackPolySetMode(parSystem_t *system, u16 bucket);
effTrackPolyList_t *Eff_ModelTrackPolyListCreate(effTrackPolyParams_t *params, u32 count);
void dds3TrackPolyDelete(parSystem_t *system);
void Eff_ModelTrackPolyListDelete(effTrackPolyList_t *list);
void dds3BillboardDelete(struct billObj_t *obj);
void *sdfChipAllocDirect(s32 size);
void sdfChipFreeDirect(void *memory);
f32 dds3GetRandFloatA(effRandState_t *state);
f32 sdfCos(f32 angle);
f32 sdfSin(f32 angle);
u32 dds3ParticleColorAnim(parColorRamp_t *colorRamp, u32 color, s32 age);
u32 dds3GetRandA(effRandState_t *state);
void LookAtMatrix(const void *directionPoint, const void *basisOrigin, const void *up);
void ReverseMatrix(void);
void SetRotateMatrixAxis(f32 angle, const sdfVector3_t *axis);
extern void dds3BillboardDraw(billObj_t *billboard);
void dds3TrackPolyVCRefresh_ParType1(parSystem_t *system, s32 cellIndex);
extern void dds3TrackPolyColorSet(parSystem_t *system, s32 index, u32 color);
void dds3TrackPolyVCRefresh_ParType2(parSystem_t *system, s32 cellIndex);
void dds3TrackPolyDraw(parSystem_t *node);
void Eff_ModelTrackPolyListDrawOnly(effTrackPolyList_t *list);
void MultMatrixNoArg(void);
struct billObj_t *dds3BillboardCreate(s32 kind, u32 data);
void dds3BillboardSetAnimColorAdvMode(billObj_t *billboard);
struct billObj_t *dds3BillboardCopy(const struct billObj_t *source);
void dds3BillboardSetAnimKey(billObj_t *effect, u32 startFrame);
void dds3TrackPolyListInit(parSystem_t *system, s32 cellIndex);
void Eff_ModelTrackPolyListReset(effTrackPolyList_t *list, s32 index);
void dds3TrackPolySet(parSystem_t *system, s32 cellIndex, const u128 *vertices);
void dds3TrackPolySet2(parSystem_t *system, s32 cellIndex, const u128 *vertices);
void Eff_ModelTrackPolyListVertexSet(effTrackPolyList_t *list, s32 index, const u128 *endpoints);
void Eff_ModelTrackPolyListColorSet(effTrackPolyList_t *list, s32 index, u32 color);
void func_00162270();

/* Both draw paths submit the resource owned by the selected particle kind. */
static __inline__ void SubmitKindDrawing(parKindState_t *kind) {
    switch (kind->kind) {
    case PAR_KIND_STATE_HISTORY:
        dds3Particle_BlurDraw(kind->payload.history.table);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
        dds3TrackPolyDraw(kind->payload.crossStrip.drawSystem);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
        dds3TrackPolyDraw(kind->payload.crossTriangle.drawSystem);
        break;
    case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
        Eff_ModelTrackPolyListDrawOnly(kind->payload.trackPolygon.modelList);
        break;
    }
}

/* Seed the generator used for per-particle launch delays. */
void dds3ParticleManagerInit(void) {
    sdfRandomizeByTime((effRandState_t *)&ParRand);
}

void dds3ParticleManagerExit(void) {
}

/* Allocate records followed by their owner tail; clear only two native state words per record. */
effectBufferTail_t *dds3Particle_PhysicsCreate(s32 recordCount) {
    s32 recordBytes = recordCount * sizeof(effectBufferRecord_t);
    sdfMemHandle_t *allocationHandle = sdfMemAllocDirect(recordBytes + sizeof(effectBufferTail_t));
    effectBufferRecord_t *recordCursor = (void *)sdfMemLock(allocationHandle);
    effectBufferTail_t *bufferTail = (effectBufferTail_t *)((u8 *)recordCursor + recordBytes);

    bufferTail->allocation = allocationHandle;
    bufferTail->records = recordCursor;
    if (recordCount > 0) {
        s32 remaining = recordCount;
        do {
            remaining--;
            recordCursor->age = 0;
            recordCursor->color = 0;
            recordCursor++;
        } while (remaining != 0);
    }
    return bufferTail;
}

void dds3Particle_PhysicsDelete(effectBufferTail_t *buffer) {
    sdfMemFreeDirect(buffer->allocation);
}

/* Allocate drawing resources and reset the emitter's common runtime state. */
/* The subrecord bitfield keeps native int-alias dependencies; see
 * dds-decomp/docs/idioms.md, "Trace-justified alias sets: emitter subrecord count". */
void dds3Particle_BasicCreate(void *effect) {
    effEmitterHead_t *owner = effect;
    effTrackPolyParams_t params;

    owner->templateSize = 0x150;
    owner->buffer = dds3Particle_PhysicsCreate(owner->particleCount);
    if (owner->billboardResource != -1) {
        owner->billboard = dds3BillboardGeneral(owner->billboardResource);
        dds3BillboardSetScale(owner->billboard, owner->scale[0], owner->scale[1]);
        dds3BillboardSetMode(owner->billboard, owner->drawKind);
    }
    dds3ParticleSetColorTable(&owner->fade, owner->lifetimeFrames);
    switch (owner->sub.kind) {
    case PAR_KIND_STATE_HISTORY:
        owner->sub.payload.history.table = dds3Particle_BlurCreate(owner->particleCount,
                                             owner->sub.templateEntryCount,
                                             &owner->billboard);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
        owner->sub.payload.crossStrip.drawSystem = dds3TrackPolyCreate(
            owner->particleCount, owner->sub.templateEntryCount,
            dds3Particle_TrackPolyGetRecTiming(owner->sub.templateEntryCount), 0);
        dds3TrackPolyVCSet(owner->sub.payload.crossStrip.drawSystem, 1,
                       owner->sub.payload.crossStrip.dispatchParameter, owner->sub.payload.crossStrip.dispatchParameter);
        dds3TrackPolySetMode(owner->sub.payload.crossStrip.drawSystem, (u16)owner->drawKind);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
        owner->sub.payload.crossTriangle.drawSystem = dds3TrackPolyCreate(
            owner->particleCount, owner->sub.templateEntryCount,
            dds3Particle_TrackPolyGetRecTiming(owner->sub.templateEntryCount), 1);
        dds3TrackPolyVCSet(owner->sub.payload.crossTriangle.drawSystem, 1,
                       owner->sub.payload.crossTriangle.dispatchParameter, owner->sub.payload.crossStrip.drawSystem);
        dds3TrackPolySetMode(owner->sub.payload.crossTriangle.drawSystem, (u16)owner->drawKind);
        break;
    case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
        /* These tracks receive endpoints directly; model sampling is unused. */
        params.sampleInterval = 1;
        params.historyLength = owner->sub.templateEntryCount;
        params.unk1C = 10;
        params.kind = (u16)owner->drawKind;
        params.gradientColors[0] = owner->sub.payload.trackPolygon.firstGradientColor;
        params.gradientColors[1] = owner->sub.payload.trackPolygon.firstGradientColor;
        params.gradientColors[2] = owner->sub.payload.trackPolygon.secondGradientColor;
        params.gradientColors[3] = owner->sub.payload.trackPolygon.secondGradientColor;
        owner->sub.payload.trackPolygon.modelList =
            Eff_ModelTrackPolyListCreate(&params, owner->particleCount);
        break;
    }
    owner->pendingRestartSteps = owner->restartStepCount;
    VU0_LOAD_MATRIX(owner->matrix);
    VU0_STORE_MATRIX(owner->backupMatrix);
    owner->active = 1;
    owner->colorMask = 0x80808080;
}

/* Release the selected sub-effect resource, billboard and packet-buffer
 * allocation described by the common emitter prefix. */
void dds3Particle_BasicDelete(void *effect)
{
    effEmitterHead_t *owner = (effEmitterHead_t *)effect;
    switch (owner->sub.kind) {
    case PAR_KIND_STATE_HISTORY:
        dds3Particle_BlurDelete(owner->sub.payload.history.table);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
        dds3TrackPolyDelete(owner->sub.payload.crossStrip.drawSystem);
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
        dds3TrackPolyDelete(owner->sub.payload.crossTriangle.drawSystem);
        break;
    case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
        Eff_ModelTrackPolyListDelete(owner->sub.payload.trackPolygon.modelList);
        break;
    }
    dds3BillboardDelete(owner->billboard);
    dds3Particle_PhysicsDelete(owner->buffer);
}

/* Stagger native tags in period-sized groups; a nonempty buffer requires a nonzero period. */
void dds3Particle_SpiralAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;
    u32 nextPacketIndex;
    s32 packetTag;

    packetTag = EFF_PACKET_INITIAL_TAG;
    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = packetTag;
            nextPacketIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
            if (nextPacketIndex % effect->tailWords[0] == 0) {
                packetTag = packetTag - effect->decayStep;
            }
            packetIndex = nextPacketIndex;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale the position, record scale, and this variant's two tail floats. */
void dds3Particle_SpiralSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[1] = effect->tailValues[1] * scale;
    effect->tailValues[3] = effect->tailValues[3] * scale;
}

/* Clone the template prefix and appended tail, then allocate its packet records. */
effTemplatePacketList_t *dds3Particle_SpiralCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x180);
    s32 tailBytes = 0x30;

    memset(clone, 0, 0x180);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SpiralAllBuffInit(clone);
    return clone;
}

void dds3Particle_SpiralDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed one ring packet's phase, radius and vertical motion, then its subeffect. */
void dds3Particle_SpiralBuffInit(effRingEmitter_t *effect, u32 packetIndex) {
    parSpiralPacket_t *packet = (parSpiralPacket_t *)effect->head.buffer->records;
    u32 phaseIndex;
    f32 phase;
    f32 jitter;

    packet += packetIndex;
    packet->color = 0;
    packet->age = 0;
    phase = 0.0f;
    phaseIndex = packetIndex % effect->period;
    if (phaseIndex != 0) {
        phase = (3.14159265f * 2.0f) / effect->period * phaseIndex;
    }
    packet->phase = phase;
    jitter = effect->radiusRange;
    packet->radialDistance = effect->radius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->targetRadiusJitter;
    packet->vel[0] = effect->targetRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    packet->verticalSpeed = effect->verticalSpeed;
    packet->pos[0] = sdfCos(phase) * effect->radius;
    packet->pos[1] = 0;
    packet->pos[2] = sdfSin(phase) * effect->radius;
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(packet->pos);
    VU0_APPLY_MATRIX_VF10();
    VU0_STORE_VF10(packet->pos);
    packet->verticalOffset = 0;
    packet->pos[0] += effect->head.origin[0];
    packet->pos[1] += effect->head.origin[1];
    packet->pos[2] += effect->head.origin[2];
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Advance ring motion and ages; repeat by tagging packets for the next update.
 * completedCount counts expired nonrepeating packets in this update only. */
/* vf12 keeps the packet's previous position and vf10 the new one for
 * dds3Particle_EffectSet, which reads them as implicit arguments. */
void dds3Particle_SpiralDraw(effRingEmitter_t *effect) {
    parSpiralPacket_t *packet = (parSpiralPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 origin[4];
    f32 offset[4];
    f32 verticalScale;
    f32 phaseStep;
    s32 completedCount;
    s32 repeatEnabled;
    s32 packetCount;
    s32 lifetimeFrames;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    phaseStep = effect->rotationStepDegrees * (3.14159265f / 180.0f);
    verticalScale = effect->verticalAccelerationPct / 100.0f + 1.0f;
    completedCount = 0;
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    repeatEnabled = effect->loop;
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SpiralBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            f32 theta;
            f32 radius;

            /* dds3Particle_BasicCreate initializes this opaque slot as a parColorRamp_t. */
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->radialDistance += (packet->vel[0] - effect->radius) / lifetimeFrames;
            packet->phase += phaseStep;
            if (packet->radialDistance < 0) {
                packet->radialDistance = 0;
            }
            packet->verticalOffset += packet->verticalSpeed;
            theta = packet->phase;
            radius = packet->radialDistance;
            offset[0] = sdfCos(theta) * radius;
            offset[1] = packet->verticalOffset;
            offset[2] = sdfSin(theta) * radius;
            VU0_LOAD_VF10(offset);
            VU0_APPLY_MATRIX_VF10();
            VU0_STORE_VF10(offset);
            packet->pos[0] = origin[0] + offset[0];
            packet->pos[1] = origin[1] + offset[1];
            packet->pos[2] = origin[2] + offset[2];
            packet->verticalSpeed *= verticalScale;
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                completedCount++;
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SmokeAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale template position, record scale, and this variant's two tail values. */
void dds3Particle_SmokeSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
}

/* Clone the disc prefix/tail, allocate packet records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_SmokeCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x180);
    s32 tailBytes = 0x30;

    memset(clone, 0, 0x180);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SmokeAllBuffInit(clone);
    return clone;
}

void dds3Particle_SmokeDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed a random planar position and direction; radialDistance may be negative. */
void dds3Particle_SmokeBuffInit(effDiscEmitter_t *effect, u32 packetIndex) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    f32 directionVector[4];
    f32 radialDistance;
    f32 jitter;

    packet += packetIndex;
    radialDistance = effect->radius * ((dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f);
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = 0;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(directionVector);
    packet->pos[0] = directionVector[0] * radialDistance;
    packet->pos[1] = 0;
    packet->pos[2] = directionVector[2] * radialDistance;
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(packet->pos);
    VU0_APPLY_MATRIX_VF10();
    VU0_STORE_VF10(packet->pos);
    packet->pos[0] += effect->head.origin[0];
    packet->pos[1] += effect->head.origin[1];
    packet->pos[2] += effect->head.origin[2];
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = 0;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(packet->vel);
    packet->waveAmplitude = effect->waveAmplitude * (dds3GetRandFloatA(dds3EffectBase) * effect->waveAmplitudeJitter + (1.0f - effect->waveAmplitudeJitter));
    packet->wavePhase = 0;
    packet->previousWaveValue = 0;
    packet->verticalVelocity = effect->verticalSpeed * (dds3GetRandFloatA(dds3EffectBase) * effect->verticalSpeedJitter + (1.0f - effect->verticalSpeedJitter));
    packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->spread + 1));
    packet->color = 0;
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Advance sinusoidal planar displacement and scaled vertical steps, then ages. */
void dds3Particle_SmokeDraw(effDiscEmitter_t *effect) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 positionDelta[4];
    f32 verticalScale;
    f32 phaseStep;
    f32 waveRate;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 packetIndex;
    u32 repeatEnabled;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    phaseStep = effect->phaseStepDegrees * (3.14159265f / 180.0f);
    verticalScale = effect->verticalAccelerationPct / 100.0f + 1.0f;
    lifetimeFrames = effect->head.lifetimeFrames;
    packetCount = effect->head.particleCount;
    repeatEnabled = effect->loop;
    waveRate = effect->waveGrowth;
    completedCount = 0;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SmokeBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            f32 verticalStep;
            f32 previousWaveValue;
            f32 factor;

            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->waveAmplitude += waveRate;
            verticalStep = packet->verticalVelocity;
            packet->wavePhase += phaseStep;
            factor = sdfSin(packet->wavePhase);
            previousWaveValue = packet->previousWaveValue;
            packet->previousWaveValue = factor;
            factor = (factor - previousWaveValue) * packet->waveAmplitude;
            positionDelta[0] = packet->vel[0] * factor;
            positionDelta[1] = verticalStep;
            positionDelta[2] = packet->vel[2] * factor;
            VU0_LOAD_VF10(positionDelta);
            VU0_APPLY_MATRIX_VF10();
            VU0_STORE_VF10(positionDelta);
            packet->pos[0] += positionDelta[0];
            packet->pos[1] += positionDelta[1];
            packet->pos[2] += positionDelta[2];
            packet->verticalVelocity = verticalStep * verticalScale;
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SparkAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale the position and three variant-specific tail values. */
void dds3Particle_SparkSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
    effect->tailValues[3] = effect->tailValues[3] * scale;
}

/* Clone the ballistic prefix/tail, allocate packet records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_SparkCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x180);
    s32 tailBytes = 0x30;

    memset(clone, 0, 0x180);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SparkAllBuffInit(clone);
    return clone;
}

void dds3Particle_SparkDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed a ballistic packet; positions are emitter-relative and then offset by the origin. */
INCLUDE_ASM(const s32, "effect/parManager", dds3Particle_SparkBuffInit);

/* Spark launch initialization retains an s4/s5 RNG-address allocation residual. */
/* Scale velocity, add its stored vertical acceleration, and advance position.
 * Only mode zero transforms the displacement through the emitter matrix. */
void dds3Particle_SparkDraw(effBallisticEmitter_t *effect) {
    parSparkPacket_t *packet = (parSparkPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    u32 mode;
    f32 positionDelta[4];
    f32 velocityScale;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 repeatEnabled;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    mode = effect->mode;
    if (mode == 0) {
        VU0_LOAD_MATRIX(effect->head.matrix);
    }
    velocityScale = effect->decayPct / 100.0f + 1.0f;
    completedCount = 0;
    lifetimeFrames = effect->head.lifetimeFrames;
    packetCount = effect->head.particleCount;
    repeatEnabled = effect->loop;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SparkBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->vel[1] = packet->vel[1] * velocityScale + packet->verticalAcceleration;
            packet->vel[0] *= velocityScale;
            packet->vel[2] *= velocityScale;
            VU0_LOAD_VF10(packet->vel);
            if (mode == 0) {
                VU0_APPLY_MATRIX_VF10();
            }
            VU0_STORE_VF10(positionDelta);
            packet->pos[0] += positionDelta[0];
            packet->pos[1] += positionDelta[1];
            packet->pos[2] += positionDelta[2];
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Spawn each packet, then overwrite its tag with a zero-based, period-grouped schedule. */
void dds3Particle_RadiateAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;
    u32 nextPacketIndex;
    s32 packetTag;

    packetTag = 0;
    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            dds3Particle_RadiateBuffInit((effLookAtRingEmitter_t *)effect, packetIndex);
            packetCursor->age = packetTag;
            nextPacketIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
            if (nextPacketIndex % effect->tailWords[0] == 0) {
                packetTag = packetTag - effect->decayStep;
            }
            packetIndex = nextPacketIndex;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale position and the third variant-specific tail value. */
void dds3Particle_RadiateSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
}

/* Clone the look-at ring prefix/tail and spawn its staggered packet schedule. */
effTemplatePacketList_t *dds3Particle_RadiateCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x170);
    s32 tailBytes = 0x20;

    memset(clone, 0, 0x170);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_RadiateAllBuffInit(clone);
    return clone;
}

void dds3Particle_RadiateDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed a view-oriented ring packet and initialize its subeffect. */
void dds3Particle_RadiateBuffInit(effLookAtRingEmitter_t *effect, u32 packetIndex) {
    parRadiatePacket_t *packet = (parRadiatePacket_t *)effect->head.buffer->records;
    u32 period = 0;
    f32 angle;
    f32 scale;
    f32 jitter;

    packet += packetIndex;
    LookAtMatrix(aViewTarget, aViewEye, aViewDown);
    ReverseMatrix();
    if (effect->mode == 0) {
        PS2_COPY_QUAD(packet, effect->head.origin);
        period = effect->period;
        angle = (3.14159265f * 2.0f) / period * (packetIndex % period);
        packet->vel[0] = sdfCos(angle);
        packet->vel[1] = sdfSin(angle);
        packet->vel[2] = 0;
        VU0_LOAD_VF10(packet->vel);
        VU0_ROTATE_VF10();
        VU0_STORE_VF10(packet->vel);
        packet->phaseAngle = angle;
        packet->age = 0;
        packet->color = 0;
    } else {
        f32 lifetimeFrames = effect->head.lifetimeFrames;

        scale = effect->stepLength;
        /* Native quirk: period was assigned only in mode zero and is still zero here. */
        angle = (3.14159265f * 2.0f) / period * (packetIndex % period);
        scale *= lifetimeFrames;
        angle += effect->rotationStepDegrees * (3.14159265f / 180.0f) * lifetimeFrames;
        packet->vel[0] = sdfCos(angle);
        packet->vel[1] = 0;
        packet->vel[2] = sdfSin(angle);
        packet->phaseAngle = angle;
        packet->pos[0] = packet->vel[0] * scale;
        packet->pos[1] = packet->vel[1] * scale;
        packet->pos[2] = packet->vel[2] * scale;
        packet->age = 0;
        packet->color = 0;
    }
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Advance view-oriented motion and ages; repeated packets respawn immediately. */
void dds3Particle_RadiateDraw(effLookAtRingEmitter_t *effect) {
    parRadiatePacket_t *packet = (parRadiatePacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 phaseStep;
    f32 stepScale;
    s32 completedCount = 0;
    s32 repeatEnabled;
    s32 packetCount;
    s32 lifetimeFrames;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    LookAtMatrix(aViewTarget, aViewEye, aViewDown);
    ReverseMatrix();
    lifetimeFrames = effect->head.lifetimeFrames;
    phaseStep = effect->rotationStepDegrees * (3.14159265f / 180.0f);
    packetCount = effect->head.particleCount;
    repeatEnabled = effect->loop;
    stepScale = effect->stepLength;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;
        f32 angle;

        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->pos[0] += packet->vel[0] * stepScale;
            packet->pos[1] += packet->vel[1] * stepScale;
            packet->pos[2] += packet->vel[2] * stepScale;
            angle = packet->phaseAngle;
            packet->vel[0] = sdfCos(angle);
            packet->vel[1] = sdfSin(angle);
            packet->vel[2] = 0;
            VU0_LOAD_VF10(packet->vel);
            VU0_ROTATE_VF10();
            VU0_STORE_VF10(packet->vel);
            packet->phaseAngle += phaseStep;
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                dds3Particle_RadiateBuffInit((effLookAtRingEmitter_t *)effect, packetIndex);
                age = packet->age;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SphereSpiralAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale position, record scale, and the second tail value. */
void dds3Particle_SphereSpiralSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[1] = effect->tailValues[1] * scale;
}

/* Clone the burst prefix/tail, allocate packet records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_SphereSpiralCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x170);
    s32 tailBytes = 0x20;

    memset(clone, 0, 0x170);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SphereSpiralAllBuffInit(clone);
    return clone;
}

void dds3Particle_SphereSpiralDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed burst velocity and an optional random axis; retain the native spawn delay. */
void dds3Particle_SphereSpiralBuffInit(effBurstEmitter_t *effect, u32 packetIndex) {
    parSphereSpiralPacket_t *packet = (parSphereSpiralPacket_t *)effect->head.buffer->records;
    f32 directionVector[4];
    f32 speed;
    f32 jitter;
    f32 velocityLength;

    packet += packetIndex;
    packet->color = 0;
    packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->spread + 1));
    speed = effect->speed;
    jitter = effect->jitterA;
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(directionVector);
    packet->vel[0] = speed * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[0];
    packet->vel[1] = speed * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[1];
    packet->vel[2] = speed * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[2];
    VU0_LOAD_VF10(packet->vel);
    VU0_LENGTH_VF10(velocityLength);
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(packet->vel);
    VU0_ROTATE_VF10();
    VU0_STORE_VF10_UNCLOBBERED(packet->vel);
    packet->pos[0] = packet->vel[0] + effect->head.origin[0];
    packet->pos[1] = packet->vel[1] + effect->head.origin[1];
    packet->pos[2] = packet->vel[2] + effect->head.origin[2];
    if (effect->mode == 1) {
        directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        directionVector[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        VU0_LOAD_VF10(directionVector);
        VU0_NORMALIZE_VF10();
        VU0_STORE_VF10(directionVector);
        packet->axisX = directionVector[0];
        packet->axisY = directionVector[1];
        packet->axisZ = directionVector[2];
    } else {
        packet->axisX = 0;
        packet->axisY = -1.0f;
        packet->axisZ = 0;
    }
    jitter = effect->jitterB;
    packet->radiusStep = (effect->targetRadius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) - velocityLength) / effect->head.lifetimeFrames;
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Adjust velocity length, rotate around the packet axis, and rebuild position. */
void dds3Particle_SphereSpiralDraw(effBurstEmitter_t *effect) {
    parSphereSpiralPacket_t *packet = (parSphereSpiralPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 origin[4];
    f32 rotationAxis[4];
    f32 scaleVector[4];
    f32 velocityLength;
    f32 rotationStep;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 repeatEnabled;
    s32 packetCount;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX_B(effect->head.matrix);
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    completedCount = 0;
    rotationStep = effect->spinRate * (3.14159265f / 180.0f);
    repeatEnabled = effect->loop;
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SphereSpiralBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            VU0_LOAD_VF10(packet->vel);
            VU0_LENGTH_VF10(velocityLength);
            velocityLength += packet->radiusStep;
            scaleVector[0] = velocityLength;
            scaleVector[1] = velocityLength;
            scaleVector[2] = velocityLength;
            VU0_NORMALIZE_VF10();
            VU0_LOAD_VF11(scaleVector);
            VU0_MUL_VF10_VF11();
            VU0_STORE_VF10(packet->vel);
            VU0_COPY_VF24_TO_VF28();
            VU0_COPY_VF25_TO_VF29();
            VU0_COPY_VF26_TO_VF30();
            VU0_COPY_VF27_TO_VF31();
            rotationAxis[0] = packet->axisX;
            rotationAxis[1] = packet->axisY;
            rotationAxis[2] = packet->axisZ;
            rotationAxis[3] = 0;
            VU0_LOAD_VF10(rotationAxis);
            VU0_ROTATE_VF10();
            VU0_STORE_VF10_UNCLOBBERED(rotationAxis);
            SetRotateMatrixAxis(rotationStep, (const sdfVector3_t *)rotationAxis);
            VU0_LOAD_VF10(packet->vel);
            VU0_ROTATE_VF10();
            VU0_LOAD_VF11(origin);
            VU0_STORE_VF10(packet->vel);
            VU0_ADD_VF10_VF11();
            VU0_STORE_VF10(packet);
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SphereSmokeAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale template position, record scale, and this variant's tail values. */
void dds3Particle_SphereSmokeSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
}

/* Clone the sphere prefix/tail, allocate packet records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_SphereSmokeCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x180);
    s32 tailBytes = 0x30;

    memset(clone, 0, 0x180);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SphereSmokeAllBuffInit(clone);
    return clone;
}

void dds3Particle_SphereSmokeDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed a jittered spherical position and planar direction.
 * Native delay uses bitwise complement, not unary negation. */
void dds3Particle_SphereSmokeBuffInit(effSphereEmitter_t *effect, u32 packetIndex) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    f32 directionVector[4];
    f32 radius;
    f32 jitter;

    packet += packetIndex;
    radius = effect->radius;
    jitter = effect->radiusJitter;
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(directionVector);
    packet->pos[0] = radius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[0];
    packet->pos[1] = radius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[1];
    packet->pos[2] = radius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * directionVector[2];
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(packet->pos);
    VU0_APPLY_MATRIX_VF10();
    VU0_STORE_VF10(packet->pos);
    packet->pos[0] += effect->head.origin[0];
    packet->pos[1] += effect->head.origin[1];
    packet->pos[2] += effect->head.origin[2];
    packet->vel[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    packet->vel[1] = 0;
    packet->vel[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(packet->vel);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(packet->vel);
    packet->waveAmplitude = effect->waveAmplitude * (dds3GetRandFloatA(dds3EffectBase) * effect->waveAmplitudeJitter + (1.0f - effect->waveAmplitudeJitter));
    packet->wavePhase = 0;
    packet->previousWaveValue = 0;
    packet->verticalVelocity = effect->verticalSpeed * (dds3GetRandFloatA(dds3EffectBase) * effect->verticalSpeedJitter + (1.0f - effect->verticalSpeedJitter));
    packet->age = ~(dds3GetRandA((effRandState_t *)ParRand) % (effect->spread + 1));
    packet->color = 0;
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Advance sinusoidal planar displacement and scaled vertical steps, then ages. */
void dds3Particle_SphereSmokeDraw(effSphereEmitter_t *effect) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 positionDelta[4];
    f32 verticalScale;
    f32 phaseStep;
    f32 waveRate;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 packetIndex;
    u32 repeatEnabled;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    phaseStep = effect->phaseStepDegrees * (3.14159265f / 180.0f);
    verticalScale = effect->verticalAccelerationPct / 100.0f + 1.0f;
    lifetimeFrames = effect->head.lifetimeFrames;
    packetCount = effect->head.particleCount;
    repeatEnabled = effect->loop;
    waveRate = effect->waveGrowth;
    completedCount = 0;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SphereSmokeBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            f32 verticalStep;
            f32 previousWaveValue;
            f32 factor;

            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->waveAmplitude += waveRate;
            verticalStep = packet->verticalVelocity;
            packet->wavePhase += phaseStep;
            factor = sdfSin(packet->wavePhase);
            previousWaveValue = packet->previousWaveValue;
            packet->previousWaveValue = factor;
            factor = (factor - previousWaveValue) * packet->waveAmplitude;
            positionDelta[0] = packet->vel[0] * factor;
            positionDelta[1] = verticalStep;
            positionDelta[2] = packet->vel[2] * factor;
            VU0_LOAD_VF10(positionDelta);
            VU0_APPLY_MATRIX_VF10();
            VU0_STORE_VF10(positionDelta);
            packet->pos[0] += positionDelta[0];
            packet->pos[1] += positionDelta[1];
            packet->pos[2] += positionDelta[2];
            packet->verticalVelocity = verticalStep * verticalScale;
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Stagger native tags using this variant's second tail word as the group period. */
void dds3Particle_SphereRingAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;
    u32 nextPacketIndex;
    s32 packetTag;

    packetTag = EFF_PACKET_INITIAL_TAG;
    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = packetTag;
            nextPacketIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
            if (nextPacketIndex % effect->tailWords[1] == 0) {
                packetTag = packetTag - effect->decayStep;
            }
            packetIndex = nextPacketIndex;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale the position and record scale of a short template. */
void dds3Particle_SphereRingSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
}

/* Clone the expanding-ring prefix/tail and initialize its period-grouped packet tags. */
effTemplatePacketList_t *dds3Particle_SphereRingCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x170);
    s32 tailBytes = 0x20;

    memset(clone, 0, 0x170);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SphereRingAllBuffInit(clone);
    return clone;
}

void dds3Particle_SphereRingDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed radius and ring direction, placing the packet at its rotated vertical offset. */
void dds3Particle_SphereRingBuffInit(effExpandingRingEmitter_t *effect, u32 packetIndex) {
    parSphereRingPacket_t *packet = (parSphereRingPacket_t *)effect->head.buffer->records;
    f32 offsetVector[4];
    f32 jitter;
    f32 radius;
    f32 angle;

    packet += packetIndex;
    packet->age = -1;
    packet->color = 0;
    jitter = effect->jitter;
    radius = effect->radius * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    packet->radius = radius;
    angle = (3.14159265f * 2.0f) / effect->period * (packetIndex % effect->period);
    packet->vel[0] = sdfCos(angle);
    packet->vel[1] = 0;
    packet->vel[2] = sdfSin(angle);
    VU0_LOAD_MATRIX(effect->head.matrix);
    offsetVector[1] = radius;
    offsetVector[0] = 0;
    offsetVector[2] = 0;
    VU0_LOAD_VF10(offsetVector);
    VU0_ROTATE_VF10();
    VU0_LOAD_VF11(effect->head.origin);
    VU0_ADD_VF10_VF11();
    VU0_STORE_VF10(packet);
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Build age-phase ring positions and rotate the stored X/Z directions.
 * vector is reused for the position offset and old direction components. */
void dds3Particle_SphereRingDraw(effExpandingRingEmitter_t *effect) {
    parSphereRingPacket_t *packet = (parSphereRingPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 origin[4];
    f32 vector[4];
    f32 rotationStep;
    f32 rotationCos;
    f32 rotationSin;
    s32 completedCount;
    s32 repeatEnabled;
    s32 packetCount;
    s32 lifetimeFrames;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    rotationStep = effect->spinRate * (3.14159265f / 180.0f);
    completedCount = 0;
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    repeatEnabled = effect->loop;
    rotationCos = sdfCos(rotationStep);
    rotationSin = sdfSin(rotationStep);
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SphereRingBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            f32 angle;
            f32 radius;
            f32 phaseCos;
            f32 phaseSin;

            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            angle = (f32)age / (f32)lifetimeFrames * 3.14159265f;
            radius = packet->radius;
            phaseCos = sdfCos(angle);
            phaseSin = sdfSin(angle);
            vector[0] = packet->vel[0] * (radius * phaseSin);
            vector[1] = radius * phaseCos;
            vector[2] = packet->vel[2] * (radius * phaseSin);
            VU0_LOAD_VF10(vector);
            VU0_APPLY_MATRIX_VF10();
            VU0_LOAD_VF11(origin);
            VU0_ADD_VF10_VF11();
            VU0_STORE_VF10(packet);
            vector[0] = packet->vel[0];
            vector[2] = packet->vel[2];
            packet->vel[0] = rotationCos * vector[0] + rotationSin * vector[2];
            packet->vel[2] = rotationCos * vector[2] - rotationSin * vector[0];
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_Spark2AllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale template position and three variant-specific tail values. */
void dds3Particle_Spark2SetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
    effect->tailValues[3] = effect->tailValues[3] * scale;
}

/* Clone the cone prefix/tail, allocate packet records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_Spark2Create(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x180);
    s32 tailBytes = 0x30;

    memset(clone, 0, 0x180);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_Spark2AllBuffInit(clone);
    return clone;
}

void dds3Particle_Spark2Delete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed mode-zero cone position/velocity; every mode initializes age and subeffect. */
void dds3Particle_Spark2BuffInit(effConeEmitter_t *effect, s32 packetIndex) {
    parSparkPacket_t *packet = (parSparkPacket_t *)effect->head.buffer->records;
    f32 gravity;
    f32 sweepRadians;
    f32 angle;
    f32 radius;
    f32 cone;
    f32 speed;
    f32 angleCos;
    f32 angleSin;
    f32 jitter;

    packet += packetIndex;
    gravity = effect->gravity / 100.0f;
    sweepRadians = effect->degrees * (3.14159265f / 180.0f);
    radius = effect->radius;
    cone = effect->cone;
    speed = effect->speed;
    if (effect->mode == 0) {
        angle = sweepRadians / (u32)effect->head.particleCount * packetIndex;
        angleCos = sdfCos(angle);
        angleSin = sdfSin(angle);
        packet->pos[0] = angleCos * radius;
        packet->pos[1] = 0;
        packet->pos[2] = angleSin * radius;
        VU0_LOAD_MATRIX(effect->head.matrix);
        VU0_LOAD_VF10(packet->pos);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(packet->pos);
        packet->pos[0] += effect->head.origin[0];
        packet->pos[1] += effect->head.origin[1];
        packet->pos[2] += effect->head.origin[2];
        packet->vel[0] = angleCos * cone * speed;
        packet->vel[1] = -speed * (1.0f - cone);
        packet->vel[2] = angleSin * cone * speed;
        packet->verticalAcceleration = gravity;
    }
    packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->spread + 1));
    packet->color = 0;
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Scale velocity, apply stored gravity, and transform each motion step before aging. */
void dds3Particle_Spark2Draw(effConeEmitter_t *effect) {
    parSparkPacket_t *packet = (parSparkPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 positionDelta[4];
    f32 velocityScale;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 repeatEnabled;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    velocityScale = effect->decayPct / 100.0f + 1.0f;
    completedCount = 0;
    lifetimeFrames = effect->head.lifetimeFrames;
    packetCount = effect->head.particleCount;
    repeatEnabled = effect->loop;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_Spark2BuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->vel[0] *= velocityScale;
            packet->vel[1] = packet->vel[1] * velocityScale + packet->verticalAcceleration;
            packet->vel[2] *= velocityScale;
            VU0_LOAD_VF10(packet->vel);
            VU0_APPLY_MATRIX_VF10();
            VU0_STORE_VF10(positionDelta);
            packet->pos[0] += positionDelta[0];
            packet->pos[1] += positionDelta[1];
            packet->pos[2] += positionDelta[2];
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize each packet's native tag and its template-selected uniform scale. */
void dds3Particle_SphereAtomAllBuffInit(effTemplatePacketList_t *effect) {
    s32 packetIndex = 0;
    effTemplateScaleRecord_t *packetCursor = (effTemplateScaleRecord_t *)effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->initialTag = EFF_PACKET_INITIAL_TAG;
            packetCursor->scale = effect->recordScale;
            packetIndex++;
            packetCursor++;
        } while ((u32)packetIndex < effect->head.particleCount);
    }
}

/* Scale the position, record scale, and first two tail values. */
void dds3Particle_SphereAtomSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[1] = effect->tailValues[1] * scale;
}

/* Clone the prefix/tail and initialize each packet record with the template's uniform scale. */
effTemplatePacketList_t *dds3Particle_SphereAtomCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x170);
    s32 tailBytes = 0x20;

    memset(clone, 0, 0x170);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SphereAtomAllBuffInit(clone);
    return clone;
}

void dds3Particle_SphereAtomDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* vu0 routine: seed radial placement, normalized velocity and activation age. */
void dds3Particle_SphereAtomBuffInit(effTemplatePacketList_t *effect, s32 index) {
    f32 direction[4];
    effectBufferRecord_t *packet = effect->head.buffer->records;
    f32 radius;
    f32 value;

    packet += index;
    if (effect->randomSphere) {
        radius = effect->recordScale;
        direction[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        VU0_LOAD_VF10(direction);
        VU0_NORMALIZE_VF10();
        VU0_STORE_VF10(direction);
        packet->position[0] = radius * dds3GetRandFloatA(dds3EffectBase) * direction[0];
        packet->position[1] = radius * dds3GetRandFloatA(dds3EffectBase) * direction[1];
        packet->position[2] = radius * dds3GetRandFloatA(dds3EffectBase) * direction[2];
        VU0_LOAD_MATRIX(effect->head.matrix);
        VU0_LOAD_VF10(packet->position);
        VU0_APPLY_MATRIX_VF10();
        VU0_STORE_VF10(packet->position);
        packet->position[0] += effect->head.origin[0];
        packet->position[1] += effect->head.origin[1];
        packet->position[2] += effect->head.origin[2];
        direction[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        VU0_LOAD_VF10(direction);
        VU0_NORMALIZE_VF10();
        VU0_STORE_VF10(packet->velocity);
        packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->delayRange + 1));
    } else {
        PS2_COPY_QUAD(packet->position, effect->head.origin);
        value = (3.14159265f * 2.0f) / effect->head.particleCount * index;
        direction[0] = sdfCos(value);
        direction[1] = sdfSin(value);
        direction[2] = 0;
        VU0_LOAD_VF10(direction);
        VU0_NORMALIZE_VF10();
        VU0_STORE_VF10(packet->velocity);
        packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->delayRange + 1));
    }
    packet->motionMagnitude = effect->motionMagnitude;
    packet->color = 0;
    value = effect->head.speedJitter;
    packet->scale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * value + (1.0f - value));
    value = effect->head.spinJitter;
    if (value != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * value + (1.0f - value)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, index);
}

/* Reflect moving packets at the frame's entry radius. The first record's
 * updated radius becomes the boundary on the following frame. */
void dds3Particle_SphereAtomDraw(effTemplatePacketList_t *effect) {
    f32 origin[4];
    f32 previousPosition[4];
    /* Retail leaves the fourth broadcast components uninitialized. The
     * XYZW operations propagate those W lanes into the packet stores. */
    f32 reflectionScale[4];
    f32 normal[4];
    f32 boundaryScale[4];
    s32 completedCount = 0;
    s32 repeatEnabled;
    parSphereAtomPacket_t *packet = (parSphereAtomPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 index;
    f32 radiusStep;
    f32 targetRadius;
    f32 radius;
    f32 velocityScale;

    dds3Particle_EffectInit(&effect->head.sub);
    lifetimeFrames = (s32)effect->head.lifetimeFrames;
    targetRadius = effect->targetRadius;
    radiusStep = (targetRadius - effect->recordScale) / lifetimeFrames;
    velocityScale = effect->accelerationPct / 100.0f + 1.0f;
    packetCount = (s32)effect->head.particleCount;
    repeatEnabled = effect->restart;
    radius = packet->boundaryRadius;
    if (radiusStep >= 0.0f) {
        if (radius < targetRadius) {
            packet->boundaryRadius = radius + radiusStep;
            if (packet->boundaryRadius > targetRadius) {
                packet->boundaryRadius = targetRadius;
            }
        }
    } else {
        if (radius > targetRadius) {
            packet->boundaryRadius = radius + radiusStep;
            if (packet->boundaryRadius < targetRadius) {
                packet->boundaryRadius = targetRadius;
            }
        }
    }
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (index = 0; index < packetCount; index++, packet++) {
        s32 age = packet->age;
        f32 distance;
        f32 dot;
        f32 dx, dy, dz;
        f32 magnitude;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SphereAtomBuffInit(effect, index);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                PS2_COPY_QUAD(previousPosition, packet->pos);
            }
            magnitude = packet->motionMagnitude;
            dx = packet->vel[0] * magnitude;
            dy = packet->vel[1] * magnitude;
            dz = packet->vel[2] * magnitude;
            packet->pos[0] += dx;
            packet->pos[1] += dy;
            packet->pos[2] += dz;
            magnitude *= velocityScale;
            packet->motionMagnitude = magnitude;
            VU0_LOAD_VF10(packet->pos);
            VU0_LOAD_VF11(origin);
            VU0_SUB_VF10_VF11();
            VU0_COPY_VF10_TO_VF12();
            VU0_LENGTH_VF10(distance);
            VU0_COPY_VF12_TO_VF10();
            VU0_NORMALIZE_VF10();
            VU0_STORE_VF10(normal);
            if (distance > radius) {
                VU0_LOAD_VF10(packet->vel);
                VU0_LOAD_VF11(D_003171A0);
                VU0_MUL_VF10_VF11();
                VU0_COPY_VF10_TO_VF12();
                VU0_LOAD_VF10(origin);
                VU0_LOAD_VF11(packet->pos);
                VU0_SUB_VF10_VF11();
                VU0_NORMALIZE_VF10();
                VU0_COPY_VF12_TO_VF11();
                VU0_COPY_VF10_TO_VF12();
                VU0_DOT_VF10_VF11(dot);
                reflectionScale[0] = dot * 2.0f;
                reflectionScale[1] = dot * 2.0f;
                reflectionScale[2] = dot * 2.0f;
                VU0_LOAD_VF10(reflectionScale);
                VU0_COPY_VF12_TO_VF11();
                VU0_MUL_VF10_VF11();
                VU0_LOAD_VF11(packet->vel);
                VU0_ADD_VF10_VF11();
                VU0_NORMALIZE_VF10();
                VU0_STORE_VF10(packet->vel);
                boundaryScale[0] = radius;
                boundaryScale[1] = radius;
                boundaryScale[2] = radius;
                VU0_LOAD_VF10(normal);
                VU0_LOAD_VF11(boundaryScale);
                VU0_MUL_VF10_VF11();
                VU0_LOAD_VF11(origin);
                VU0_ADD_VF10_VF11();
                VU0_STORE_VF10(packet->pos);
            }
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(previousPosition);
                VU0_LOAD_VF10(packet->pos);
                dds3Particle_EffectSet(&effect->head.sub, index, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, index);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

INCLUDE_ASM(const s32, "effect/parManager", dds3Particle_SpiralRandomize);

/* Frame zero reinitializes to -1 before incrementing; refresh position and modulated packed color. */
void dds3Particle_SpiralBlendColor(effParticleOverlay_t *effect) {
    effParticleRecord_t *record = (effParticleRecord_t *)effect->buffer->records;
    s32 frame = record->frame;
    u32 baseColor;

    if (frame == 0) {
        func_00162270(effect);
        frame = record->frame;
    }
    record->frame = frame + 1;
    record->x = effect->x;
    effect->drawEndFrame = frame + 2;
    record->y = effect->y;
    baseColor = effect->color | (effect->alpha << 24);
    record->z = effect->z;
    record->color = baseColor;
    record->color = dds3ParticleColorBlend(baseColor, effect->colorMask);
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SparkOffsetAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale the position and three variant-specific tail values. */
void dds3Particle_SparkOffsetSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
    effect->tailValues[3] = effect->tailValues[3] * scale;
}

/* Clone the offset-gravity prefix/tail, allocate records, and initialize their native age tags. */
effTemplatePacketList_t *dds3Particle_SparkOffsetCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x190);
    s32 tailBytes = 0x40;

    memset(clone, 0, 0x190);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SparkOffsetAllBuffInit(clone);
    return clone;
}

void dds3Particle_SparkOffsetDelete(void *effect) {
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed an offset-gravity packet; preserve each mode's native partial vector setup. */
INCLUDE_ASM(const s32, "effect/parManager", dds3Particle_SparkOffsetBuffInit);

/* Accumulate motion in the stored offset, then add the emitter origin.
 * Only mode zero transforms the per-frame displacement through the matrix. */
void dds3Particle_SparkOffsetDraw(effOffsetGravityEmitter_t *effect) {
    parSparkOffsetPacket_t *packet = (parSparkOffsetPacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    u32 mode;
    f32 positionDelta[4];
    f32 origin[4];
    f32 velocityScale;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 repeatEnabled;
    s32 packetIndex;

    dds3Particle_EffectInit(&effect->head.sub);
    mode = effect->mode;
    if (mode == 0) {
        VU0_LOAD_MATRIX(effect->head.matrix);
    }
    velocityScale = effect->decayPct / 100.0f + 1.0f;
    completedCount = 0;
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    repeatEnabled = effect->loop;
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SparkOffsetBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->vel[1] = packet->vel[1] * velocityScale + packet->offsetAcceleration;
            packet->vel[0] *= velocityScale;
            packet->vel[2] *= velocityScale;
            VU0_LOAD_VF10(packet->vel);
            if (mode == 0) {
                VU0_APPLY_MATRIX_VF10();
            }
            VU0_STORE_VF10(positionDelta);
            packet->offsetX += positionDelta[0];
            packet->offsetY += positionDelta[1];
            packet->offsetZ += positionDelta[2];
            VU0_LOAD_VF10(&packet->offsetX);
            VU0_LOAD_VF11(origin);
            VU0_ADD_VF10_VF11();
            VU0_STORE_VF10(packet);
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SmokeOffsetAllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale position, record scale, and two tail values. */
void dds3Particle_SmokeOffsetSetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[0] = effect->tailValues[0] * scale;
    effect->tailValues[2] = effect->tailValues[2] * scale;
}

/* Clone the disc prefix/tail plus a separate 16-byte-per-packet auxiliary allocation. */
effTemplatePacketList_t *dds3Particle_SmokeOffsetCreate(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x200);
    sdfMemHandle_t *auxAllocationHandle;
    s32 tailBytes = 0xB0;

    memset(clone, 0, 0x200);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + EFF_TEMPLATE_TAIL_OFFSET, (u8 *)source + source->head.templateSize, tailBytes);
    auxAllocationHandle = sdfMemAllocDirect(clone->head.particleCount << 4);
    clone->auxiliaryAllocation = auxAllocationHandle;
    clone->auxiliaryData = (void *)sdfMemLock(auxAllocationHandle);
    dds3Particle_BasicCreate(clone);
    dds3Particle_SmokeOffsetAllBuffInit(clone);
    return clone;
}

void dds3Particle_SmokeOffsetDelete(effTemplatePacketList_t *effect) {
    sdfMemFreeDirect(effect->auxiliaryAllocation);
    dds3Particle_BasicDelete(effect);
    sdfChipFreeDirect(effect);
}

/* Seed the auxiliary position and packet direction separately, then the subeffect. */
void dds3Particle_SmokeOffsetBuffInit(effDiscAuxEmitter_t *effect, u32 packetIndex) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    f32 (*auxiliaryCursor)[4] = effect->aux;
    f32 directionVector[4];
    f32 radialDistance;
    f32 jitter;

    packet += packetIndex;
    auxiliaryCursor += packetIndex;
    radialDistance = effect->radius * ((dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f);
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = 0;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(directionVector);
    (*auxiliaryCursor)[0] = directionVector[0] * radialDistance;
    (*auxiliaryCursor)[1] = 0;
    (*auxiliaryCursor)[2] = directionVector[2] * radialDistance;
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(*auxiliaryCursor);
    VU0_APPLY_MATRIX_VF10();
    VU0_STORE_VF10(*auxiliaryCursor);
    directionVector[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    directionVector[1] = 0;
    directionVector[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(directionVector);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(packet->vel);
    packet->waveAmplitude = effect->waveAmplitude * (dds3GetRandFloatA(dds3EffectBase) * effect->waveAmplitudeJitter + (1.0f - effect->waveAmplitudeJitter));
    packet->wavePhase = 0;
    packet->previousWaveValue = 0;
    packet->verticalVelocity = effect->verticalSpeed * (dds3GetRandFloatA(dds3EffectBase) * effect->verticalSpeedJitter + (1.0f - effect->verticalSpeedJitter));
    packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->spread + 1));
    packet->color = 0;
    PS2_COPY_QUAD(packet, effect->head.origin);
    jitter = effect->head.speedJitter;
    packet->billboardScale = effect->head.scale[0] * (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter));
    jitter = effect->head.spinJitter;
    if (jitter != 0) {
        packet->spin = (dds3GetRandFloatA(dds3EffectBase) * jitter + (1.0f - jitter)) * (3.14159265f * 2.0f);
    } else {
        packet->spin = 0;
    }
    dds3Particle_EffectReset(&effect->head.sub, packetIndex);
}

/* Advance auxiliary positions, then transform/store packet positions with origin.
 * Auxiliary accumulation remains separate from the transformed packet position. */
void dds3Particle_SmokeOffsetDraw(effDiscAuxEmitter_t *effect) {
    parSmokePacket_t *packet = (parSmokePacket_t *)effect->head.buffer->records;
    u32 subeffectKind = effect->head.sub.kind;
    f32 (*auxiliaryCursor)[4] = effect->aux;
    f32 positionVector[4];
    f32 origin[4];
    f32 verticalScale;
    f32 phaseStep;
    f32 waveRate;
    s32 completedCount;
    s32 lifetimeFrames;
    s32 packetCount;
    s32 packetIndex;
    u32 repeatEnabled;

    dds3Particle_EffectInit(&effect->head.sub);
    VU0_LOAD_MATRIX(effect->head.matrix);
    phaseStep = effect->phaseStepDegrees * (3.14159265f / 180.0f);
    verticalScale = effect->verticalAccelerationPct / 100.0f + 1.0f;
    completedCount = 0;
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    repeatEnabled = effect->loop;
    waveRate = effect->waveGrowth;
    PS2_COPY_QUAD(origin, effect->head.origin);
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++, auxiliaryCursor++) {
        s32 age = packet->age;

        if (age == EFF_PACKET_INITIAL_TAG) {
            dds3Particle_SmokeOffsetBuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            f32 verticalStep;
            f32 previousWaveValue;
            f32 factor;

            packet->color = dds3ParticleColorAnim(&effect->head.fade, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.colorMask);
            if (subeffectKind != 0) {
                VU0_LOAD_VF12(packet);
            }
            packet->waveAmplitude += waveRate;
            verticalStep = packet->verticalVelocity;
            packet->wavePhase += phaseStep;
            factor = sdfSin(packet->wavePhase);
            previousWaveValue = packet->previousWaveValue;
            packet->previousWaveValue = factor;
            factor = (factor - previousWaveValue) * packet->waveAmplitude;
            positionVector[0] = (*auxiliaryCursor)[0] + packet->vel[0] * factor;
            positionVector[1] = (*auxiliaryCursor)[1] + verticalStep;
            positionVector[2] = (*auxiliaryCursor)[2] + packet->vel[2] * factor;
            VU0_LOAD_VF10(positionVector);
            VU0_STORE_VF10(*auxiliaryCursor);
            VU0_APPLY_MATRIX_VF10();
            VU0_LOAD_VF11(origin);
            VU0_ADD_VF10_VF11();
            VU0_STORE_VF10(packet);
            packet->verticalVelocity = verticalStep * verticalScale;
            if (subeffectKind != 0) {
                VU0_LOAD_VF10(packet);
                dds3Particle_EffectSet(&effect->head.sub, packetIndex, packet->color, packet->billboardScale);
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = EFF_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.sub, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.active = 0;
                }
            }
        }
        packet->age = age;
    }
}

/* Initialize only the native age/tag word of each packet record. */
void dds3Particle_SphereSpiral2AllBuffInit(effTemplatePacketList_t *effect) {
    effectBufferRecord_t *packetCursor;
    u32 packetIndex;

    packetIndex = 0;
    packetCursor = effect->head.buffer->records;
    if (effect->head.particleCount != 0) {
        do {
            packetCursor->age = EFF_PACKET_INITIAL_TAG;
            packetIndex = packetIndex + 1;
            packetCursor = packetCursor + 1;
        } while (packetIndex < effect->head.particleCount);
    }
}

/* Scale template position, record scale, and the second tail value. */
void dds3Particle_SphereSpiral2SetScale(float scale, effTemplatePacketList_t *effect) {
    effect->head.scale[0] = effect->head.scale[0] * scale;
    effect->head.scale[1] = effect->head.scale[1] * scale;
    effect->head.scale[2] = effect->head.scale[2] * scale;
    effect->recordScale = effect->recordScale * scale;
    effect->tailValues[1] = effect->tailValues[1] * scale;
}

/* Allocate one 12-byte descriptor per packet and its attached 16-byte records. */
effTemplatePacketList_t *dds3Particle_SphereSpiral2Create(effTemplatePacketList_t *source) {
    effTemplatePacketList_t *clone = sdfChipAllocDirect(0x190);
    s32 tailLen = 0x40;
    s32 count;
    s32 perRecord;
    s32 listBytes;
    parHistory_t *history;
    parHistory_t *historyBase;
    s32 step;
    u8 *pointBase;
    s32 particleIndex;

    memset(clone, 0, 0x190);
    memcpy(clone, source, source->head.templateSize);
    memcpy((u8 *)clone + 0x150, (u8 *)source + source->head.templateSize, tailLen);
    dds3Particle_BasicCreate(clone);
    perRecord = 0;
    if (clone->head.sub.kind != 0) {
        count = clone->head.particleCount;
        /* All four supported resource kinds use the same subrecord count. */
        switch (clone->head.sub.kind) {
        case PAR_KIND_STATE_HISTORY:
            perRecord = clone->head.sub.templateEntryCount;
            break;
        case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
            perRecord = clone->head.sub.templateEntryCount;
            break;
        case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
            perRecord = clone->head.sub.templateEntryCount;
            break;
        case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
            perRecord = clone->head.sub.templateEntryCount;
            break;
        }
        listBytes = count * 12;
        clone->recordsPerPacket = perRecord;
        clone->listAllocation = sdfMemAllocDirect(listBytes + perRecord * count * 16);
        historyBase = sdfMemLock(clone->listAllocation);
        particleIndex = 0;
        pointBase = (u8 *)historyBase;
        clone->recordList = historyBase;
        pointBase += listBytes;
        if (count > 0) {
            step = perRecord * 16;
            history = historyBase;
            do {
                particleIndex++;
                history->points = (f32 (*)[3])pointBase;
                history->nextIndex = 0;
                history->count = 0;
                history++;
                pointBase += step;
            } while (particleIndex < count);
        }
    } else {
        clone->listAllocation = 0;
    }
    dds3Particle_SphereSpiral2AllBuffInit(clone);
    return clone;
}

/* Release the radial emitter's extra allocation, shared resources, and block. */
void dds3Particle_SphereSpiral2Delete(parBurstEmitter_t *obj) {
    if (obj->childHandle != 0) {
        sdfMemFreeDirect(obj->childHandle);
    }
    dds3Particle_BasicDelete(obj);
    sdfChipFreeDirect(obj);
}

/* Initialize one 64-byte particle record and its kind-specific state. Negative
   ages delay activation; radial distance changes toward a jittered target over
   lifetimeFrames. RNG calls and the pre-transform length measurement stay in
   their original order. */
void dds3Particle_SphereSpiral2BuffInit(parBurstEmitter_t *effect, u32 particleIndex) {
    parBurstPacket_t *packet = (parBurstPacket_t *)effect->head.buffer->records;
    f32 direction[4];
    f32 initialRadius;
    f32 jitterFactor;
    f32 initialRadiusLength;

    packet += particleIndex;
    packet->color = 0;
    packet->age = -(dds3GetRandA((effRandState_t *)ParRand) % (effect->spawnDelayFrames + 1));
    initialRadius = effect->initialRadius;
    jitterFactor = effect->initialRadiusJitter;
    direction[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    direction[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    direction[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
    VU0_LOAD_VF10(direction);
    VU0_NORMALIZE_VF10();
    VU0_STORE_VF10(direction);
    /* Each component gets a separate jitter sample, not one shared radius draw. */
    packet->radialOffset[0] = initialRadius * (dds3GetRandFloatA(dds3EffectBase) * jitterFactor + (1.0f - jitterFactor)) * direction[0];
    packet->radialOffset[1] = initialRadius * (dds3GetRandFloatA(dds3EffectBase) * jitterFactor + (1.0f - jitterFactor)) * direction[1];
    packet->radialOffset[2] = initialRadius * (dds3GetRandFloatA(dds3EffectBase) * jitterFactor + (1.0f - jitterFactor)) * direction[2];
    VU0_LOAD_VF10(packet->radialOffset);
    VU0_LENGTH_VF10(initialRadiusLength);
    VU0_LOAD_MATRIX(effect->head.matrix);
    VU0_LOAD_VF10(packet->radialOffset);
    VU0_ROTATE_VF10();
    VU0_STORE_VF10_UNCLOBBERED(packet->radialOffset);
    packet->position[0] = packet->radialOffset[0] + effect->head.origin[0];
    packet->position[1] = packet->radialOffset[1] + effect->head.origin[1];
    packet->position[2] = packet->radialOffset[2] + effect->head.origin[2];
    if (effect->axisMode == PAR_BURST_RANDOM_AXIS) {
        direction[0] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[1] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        direction[2] = (dds3GetRandFloatA(dds3EffectBase) - 0.5f) * 2.0f;
        VU0_LOAD_VF10(direction);
        VU0_NORMALIZE_VF10();
        VU0_STORE_VF10(direction);
        packet->rotationAxis[0] = direction[0];
        packet->rotationAxis[1] = direction[1];
        packet->rotationAxis[2] = direction[2];
    } else {
        packet->rotationAxis[0] = 0;
        packet->rotationAxis[1] = -1.0f;
        packet->rotationAxis[2] = 0;
    }
    jitterFactor = effect->targetRadiusJitter;
    packet->radiusStep = (effect->targetRadius * (dds3GetRandFloatA(dds3EffectBase) * jitterFactor +
                                    (1.0f - jitterFactor)) - initialRadiusLength) /
                   effect->head.lifetimeFrames;
    jitterFactor = effect->head.billboardScaleJitter;
    packet->billboardScale = effect->head.billboardScale * (dds3GetRandFloatA(dds3EffectBase) * jitterFactor +
                                         (1.0f - jitterFactor));
    jitterFactor = effect->head.initialPhaseJitter;
    if (jitterFactor != 0) {
        packet->initialPhaseRadians = (dds3GetRandFloatA(dds3EffectBase) * jitterFactor + (1.0f - jitterFactor)) *
                       (3.14159265f * 2.0f);
    } else {
        packet->initialPhaseRadians = 0;
    }
    dds3Particle_EffectReset(&effect->head.kindState, particleIndex);
}

/* Grow each radial offset toward its target radius, spin it about the particle axis,
 * rebuild position from the origin, and feed the kind's point history. */
void dds3Particle_SphereSpiral2Draw(parBurstEmitter_t *effect) {
    parBurstPacket_t *packet = (parBurstPacket_t *)effect->head.buffer->records;
    u32 kind;
    parHistory_t *history = effect->history.records;
    f32 origin[4];
    f32 previousOrigin[4];
    f32 rotationAxis[4];
    f32 scaleVector[4];
    f32 radius;
    f32 rotationStep;
    s32 completedCount;
    s32 lifetimeFrames;
    u32 capacity;
    s32 repeatEnabled;
    s32 packetCount;
    s32 packetIndex;

    capacity = effect->history.capacity;
    kind = effect->head.kindState.kind;
    dds3Particle_EffectInit(&effect->head.kindState);
    VU0_LOAD_MATRIX_B(effect->head.matrix);
    packetCount = effect->head.particleCount;
    lifetimeFrames = effect->head.lifetimeFrames;
    completedCount = 0;
    rotationStep = effect->rotationStepDegrees * (3.14159265f / 180.0f);
    repeatEnabled = effect->loop;
    PS2_COPY_QUAD(origin, effect->head.origin);
    previousOrigin[0] = effect->previousOrigin[0];
    previousOrigin[1] = effect->previousOrigin[1];
    previousOrigin[2] = effect->previousOrigin[2];
    previousOrigin[3] = 0;
    for (packetIndex = 0; packetIndex < packetCount; packetIndex++, packet++, history++) {
        s32 age = packet->age;

        if (age == PAR_PACKET_INITIAL_TAG) {
            dds3Particle_SphereSpiral2BuffInit(effect, packetIndex);
            age = packet->age;
        }
        if (age >= 0) {
            packet->color = dds3ParticleColorAnim(&effect->head.colorRamp, packet->color, age);
            packet->color = dds3ParticleColorBlend(packet->color, effect->head.parameterWord);
            if (kind != 0) {
                VU0_LOAD_VF12(packet->radialOffset);
            }
            VU0_LOAD_VF10(packet->radialOffset);
            VU0_LENGTH_VF10(radius);
            radius += packet->radiusStep;
            scaleVector[0] = radius;
            scaleVector[1] = radius;
            scaleVector[2] = radius;
            VU0_NORMALIZE_VF10();
            VU0_LOAD_VF11(scaleVector);
            VU0_MUL_VF10_VF11();
            VU0_STORE_VF10(packet->radialOffset);
            VU0_COPY_VF24_TO_VF28();
            VU0_COPY_VF25_TO_VF29();
            VU0_COPY_VF26_TO_VF30();
            VU0_COPY_VF27_TO_VF31();
            rotationAxis[0] = packet->rotationAxis[0];
            rotationAxis[1] = packet->rotationAxis[1];
            rotationAxis[2] = packet->rotationAxis[2];
            rotationAxis[3] = 0;
            VU0_LOAD_VF10(rotationAxis);
            VU0_ROTATE_VF10();
            VU0_STORE_VF10_UNCLOBBERED(rotationAxis);
            SetRotateMatrixAxis(rotationStep, (const sdfVector3_t *)rotationAxis);
            VU0_LOAD_VF10(packet->radialOffset);
            VU0_ROTATE_VF10();
            VU0_LOAD_VF11(origin);
            VU0_STORE_VF10(packet->radialOffset);
            VU0_ADD_VF10_VF11();
            VU0_STORE_VF10(packet->position);
            if (kind != 0) {
                u32 color;

                history->points[history->nextIndex][0] = packet->radialOffset[0];
                history->points[history->nextIndex][1] = packet->radialOffset[1];
                history->points[history->nextIndex][2] = packet->radialOffset[2];
                color = (packet->color & 0xFF000000) | (effect->head.parameterWord & 0xFFFFFF);
                history->nextIndex++;
                if ((u32)history->nextIndex >= capacity) {
                    history->nextIndex = 0;
                }
                if ((u32)history->count < capacity) {
                    history->count++;
                }
                switch (effect->head.kindState.kind) {
                case 1:
                    dds3Particle_BlurSetExp(effect->head.kindState.payload.history.table, packetIndex,
                                               previousOrigin, packet->color, &effect->history,
                                               packet->billboardScale);
                    break;
                case 2:
                    dds3Particle_TrackSetExp(effect->head.kindState.payload.crossStrip.drawSystem,
                                               packetIndex, previousOrigin, color, &effect->history);
                    break;
                case 3:
                    dds3Particle_Track2SetExp(effect->head.kindState.payload.crossTriangle.drawSystem,
                                                  packetIndex, previousOrigin, color, &effect->history);
                    break;
                }
            }
        }
        age++;
        if (age >= lifetimeFrames) {
            if (repeatEnabled) {
                age = PAR_PACKET_INITIAL_TAG;
            } else {
                dds3Particle_EffectReset(&effect->head.kindState, packetIndex);
                completedCount++;
                if (completedCount >= packetCount) {
                    effect->head.restartFlag = 0;
                }
            }
            if (kind != 0) {
                history->nextIndex = 0;
                history->count = 0;
            }
        }
        packet->age = age;
    }
    effect->previousOrigin[0] = origin[0];
    effect->previousOrigin[1] = origin[1];
    effect->previousOrigin[2] = origin[2];
}

void dds3ParticleSetColorTable(parColorRamp_t *p, s32 frames) {
    u32 alpha;
    f32 span;
    u32 a, b;

    p->color0 &= 0xFFFFFF;
    p->color1 &= 0xFFFFFF;
    p->color2 &= 0xFFFFFF;
    alpha = p->alpha << 24;
    p->fadeIn = alpha;
    if (p->fadeInFrames > 0) {
        p->fadeIn = alpha / p->fadeInFrames;
    }
    p->fadeOut = alpha;
    if (p->fadeOutFrames > 0) {
        p->fadeOut = alpha / p->fadeOutFrames;
    }
    p->frames = frames;
    if (p->mode == 1) {
        return;
    }
    if (p->mode == 2) {
        p->rampFrames = frames;
    } else {
        p->rampFrames = frames >> 1;
    }
    span = p->rampFrames;
    if (span == 0) {
        span = 1.0f;
    }
    a = p->color1;
    b = p->color0;
    p->r01 = ((f32)(a & 0xFF) - (f32)(b & 0xFF)) / span;
    p->g01 = ((f32)((a >> 8) & 0xFF) - (f32)((b >> 8) & 0xFF)) / span;
    p->b01 = ((f32)((a >> 16) & 0xFF) - (f32)((b >> 16) & 0xFF)) / span;
    if (p->mode == 2) {
        return;
    }
    a = p->color2;
    b = p->color1;
    p->r12 = ((f32)(a & 0xFF) - (f32)(b & 0xFF)) / span;
    p->g12 = ((f32)((a >> 8) & 0xFF) - (f32)((b >> 8) & 0xFF)) / span;
    p->b12 = ((f32)((a >> 16) & 0xFF) - (f32)((b >> 16) & 0xFF)) / span;
}

INCLUDE_ASM(const s32, "effect/parManager", dds3ParticleColorAnim);

/* Allocate contiguous point slabs, slot records and the trailing table header. */
parTable_t *dds3Particle_BlurCreate(u32 slotCount, u16 pointCapacity, billObj_t **billboardRef) {
    u32 pointBytes = pointCapacity * sizeof(u128);
    u32 slabBytes = pointBytes + sizeof(parSlot_t);
    u32 totalBytes = slabBytes * slotCount;
    sdfMemHandle_t *allocation = sdfMemAllocDirect(totalBytes + sizeof(parTable_t));
    u8 *base = (u8 *)sdfMemLock(allocation);
    parTable_t *table = (parTable_t *)(base + totalBytes);
    u128 *points = (u128 *)base;
    parSlot_t *slotBase = (parSlot_t *)(points + pointCapacity * slotCount);
    parSlot_t *slots;
    u32 index = 0;

    table->pointCapacity = pointCapacity;
    table->billboardRef = billboardRef;
    table->allocation = allocation;
    table->slotCount = slotCount;
    table->slots = slotBase;
    if (slotCount != 0) {
        slots = table->slots;
        do {
            slots[index].points = points;
            slots[index].pointCount = 0;
            slots[index].color = 0;
            points += pointCapacity;
            index++;
        } while (index < slotCount);
    }
    return table;
}

/* Release the slot table's allocation handle, not a separate node object. */
void dds3Particle_BlurDelete(parTable_t *table) {
    sdfMemFreeDirect(table->allocation);
}

void dds3Particle_BlurSet(parTable_t *table, s32 slotIndex, u32 color, f32 billboardScale) {
    parSlot_t *slot = &table->slots[slotIndex];
    s32 last = table->pointCapacity - 1;
    u128 *point = slot->points + last;
    s32 i;

    for (i = 0; i < last; i++) {
        point--;
        PS2_COPY_QUAD(point + 1, point);
    }
    if (slot->pointCount <= last) {
        slot->pointCount++;
    }
    VU0_STORE_VF12(point);
    slot->color = color;
    slot->billboardScale = billboardScale;
}

void dds3Particle_BlurSetExp(parTable_t *table, s32 slotIndex, const f32 *origin, u32 color,
                   parHistoryTable_t *source, f32 scale) {
    f32 localOrigin[4];
    parHistory_t *history;
    parSlot_t *slot;
    f32 (*destination)[4];
    s32 pointCount;
    s32 pointCapacity;
    s32 pointIndex;

    PS2_COPY_QUAD(localOrigin, origin);
    history = &source->records[slotIndex];
    slot = &table->slots[slotIndex];
    pointCount = history->count;
    pointCapacity = source->capacity;
    slot->pointCount = pointCount;
    destination = (f32 (*)[4])slot->points;
    pointIndex = history->nextIndex;
    if (pointCount > 0) {
        f32 (*points)[3] = history->points;
        s32 lastIndex = pointCapacity - 1;
        s32 remaining = pointCount;

        do {
            --remaining;
            pointIndex = pointIndex == 0 ? lastIndex : pointIndex - 1;
            (*destination)[0] = localOrigin[0] + points[pointIndex][0];
            (*destination)[1] = localOrigin[1] + points[pointIndex][1];
            (*destination)[2] = localOrigin[2] + points[pointIndex][2];
            ++destination;
        } while (remaining != 0);
    }
    slot->color = color;
    slot->billboardScale = scale;
}

void dds3Particle_BlurDraw(parTable_t *table) {
    billObj_t *billboard;
    parSlot_t *slot;
    s32 remainingSlots;
    s32 pointCount;
    s32 remainingPoints;
    u32 color;
    u32 alpha;
    u32 alphaStep;
    u128 *point;

    billboard = *table->billboardRef;
    slot = table->slots;

    if (table->slotCount != 0) {
        remainingSlots = table->slotCount;

        do {
            pointCount = slot->pointCount;
            color = slot->color;
            point = slot->points;
            alpha = color & 0xFF000000;
            alphaStep = alpha / (pointCount + 1);

            dds3BillboardSetScale(billboard, slot->billboardScale, slot->billboardScale);
            color &= 0xFFFFFF;
            if (pointCount != 0) {
                remainingPoints = pointCount;
                do {
                    PS2_COPY_QUAD(billboard, point);
                    alpha -= alphaStep;
                    --remainingPoints;
                    billboard->childParam = color | (alpha & 0xFF000000);
                    ++point;
                    dds3BillboardDraw(billboard);
                } while (remainingPoints != 0);
            }
            --remainingSlots;
            ++slot;
        } while (remainingSlots != 0);
    }
}

/* Disable this slot's billboard points by clearing its count, not a bit flag. */
void dds3Particle_BlurClearItem(parTable_t *table, s32 slotIndex) {
    table->slots[slotIndex].pointCount = 0;
}

/* Rebuild one slot's two-vertex strip cells from its point history: each cell is
   the offset point +/- the cross product of the segment direction with the view axis,
   scaled by the strip width. */
void dds3Particle_TrackSetExp(parSystem_t *system, s32 index, const f32 *origin, u32 color, parHistoryTable_t *source) {
    parHistory_t *history = &source->records[index];
    parCell_t *cell = &system->cells[index];
    u128 *vertex = cell->history;
    u32 count = history->count;
    f32 point[4];
    f32 localOrigin[4];

    if (count >= 2) {
        s32 segments;
        s32 segmentIndex;
        s32 lastIndex;
        s32 ring;

        PS2_COPY_QUAD(localOrigin, origin);
        segments = count - 1;
        cell->vertexCount = segments * 2;
        ring = history->nextIndex;
        lastIndex = source->capacity - 1;
        ring = ring == 0 ? lastIndex : ring - 1;
        point[0] = localOrigin[0] + history->points[ring][0];
        point[1] = localOrigin[1] + history->points[ring][1];
        point[2] = localOrigin[2] + history->points[ring][2];
        point[3] = 0;
        for (segmentIndex = 0; segmentIndex < segments; segmentIndex++) {
            VU0_LOAD_VF10(point);
            ring = ring == 0 ? lastIndex : ring - 1;
            point[0] = localOrigin[0] + history->points[ring][0];
            point[1] = localOrigin[1] + history->points[ring][1];
            point[2] = localOrigin[2] + history->points[ring][2];
            VU0_LOAD_VF12(point);
            VU0_COPY_VF12_TO_VF11();
            VU0_SUB_VF10_VF11();
            VU0_LOAD_VF11(parLook);
            VU0_CROSS_VF10_VF11();
            VU0_NORMALIZE_VF10();
            VU0_LOAD_VF11(parTrack_w);
            VU0_MUL_VF10_VF11();
            VU0_COPY_VF10_TO_VF2();
            VU0_COPY_VF12_TO_VF10();
            VU0_COPY_VF2_TO_VF12();
            VU0_COPY_VF10_TO_VF11();
            VU0_ADD_VF10_VF12();
            VU0_STORE_VF10_UNCLOBBERED(vertex);
            VU0_COPY_VF12_TO_VF10();
            VU0_SUB_VF11_VF10();
            VU0_STORE_VF11_UNCLOBBERED(vertex + 1);
            vertex += 2;
        }
        dds3TrackPolyVCRefresh_ParType1(system, index);
    }
    dds3TrackPolyColorSet(system, index, color);
}

/* Triangle-cell variant of dds3Particle_TrackSetExp: three vertices per history segment. */
void dds3Particle_Track2SetExp(parSystem_t *system, s32 index, const f32 *origin, u32 color, parHistoryTable_t *source) {
    parHistory_t *history = &source->records[index];
    parCell_t *cell = &system->cells[index];
    u128 *vertex = cell->history;
    u32 count = history->count;
    f32 point[4];
    f32 localOrigin[4];

    if (count >= 2) {
        s32 segments;
        s32 segmentIndex;
        s32 lastIndex;
        s32 ring;

        PS2_COPY_QUAD(localOrigin, origin);
        segments = count - 1;
        cell->vertexCount = segments * 3;
        ring = history->nextIndex;
        lastIndex = source->capacity - 1;
        ring = ring == 0 ? lastIndex : ring - 1;
        point[0] = localOrigin[0] + history->points[ring][0];
        point[1] = localOrigin[1] + history->points[ring][1];
        point[2] = localOrigin[2] + history->points[ring][2];
        point[3] = 0;
        for (segmentIndex = 0; segmentIndex < segments; segmentIndex++) {
            VU0_LOAD_VF10(point);
            ring = ring == 0 ? lastIndex : ring - 1;
            point[0] = localOrigin[0] + history->points[ring][0];
            point[1] = localOrigin[1] + history->points[ring][1];
            point[2] = localOrigin[2] + history->points[ring][2];
            VU0_LOAD_VF12(point);
            VU0_COPY_VF12_TO_VF11();
            VU0_SUB_VF10_VF11();
            VU0_LOAD_VF11(parLook);
            VU0_CROSS_VF10_VF11();
            VU0_NORMALIZE_VF10();
            VU0_LOAD_VF11(parTrack_w);
            VU0_MUL_VF10_VF11();
            VU0_COPY_VF10_TO_VF2();
            VU0_COPY_VF12_TO_VF10();
            VU0_COPY_VF2_TO_VF12();
            VU0_STORE_VF10_UNCLOBBERED(vertex + 1);
            VU0_COPY_VF10_TO_VF11();
            VU0_ADD_VF10_VF12();
            VU0_STORE_VF10_UNCLOBBERED(vertex);
            VU0_COPY_VF12_TO_VF10();
            VU0_SUB_VF11_VF10();
            VU0_STORE_VF11_UNCLOBBERED(vertex + 2);
            vertex += 3;
        }
        dds3TrackPolyVCRefresh_ParType2(system, index);
    }
    dds3TrackPolyColorSet(system, index, color);
}

INCLUDE_ASM(const s32, "effect/parManager", dds3ParticleColorBlend);

/* Create through the selected dispatch entry and record its index; returns void. */
void dds3ParticleCreate(s32 dispatchIndex, void *creationData) {
    parObj_t *createdObject;

    createdObject = dds3ParticleFuncTable[dispatchIndex].func(creationData);
    createdObject->dispatchIndex = dispatchIndex;
}

/* Invoke the selected destructor with the existing empty argument list. */
void dds3ParticleDelete(parObj_t *obj) {
    dds3ParticleFuncTable[obj->dispatchIndex].destroy();
}

/* Draw live particles after applying the requested number of emitter updates. */
void dds3ParticleDraw(parObj_t *effect) {
    effectBufferRecord_t *record;
    billObj_t *billboard;
    u32 step;
    s32 particleCount;
    s32 lifetime;

    if (effect->restartFlag == 0) {
        return;
    }
    if (effect->pendingRestartSteps == 0) {
        dds3ParticleFuncTable[effect->dispatchIndex].update(effect);
    } else {
        for (step = 0; step < effect->pendingRestartSteps; step++) {
            dds3ParticleFuncTable[effect->dispatchIndex].update(effect);
        }
        effect->pendingRestartSteps = 0;
    }
    billboard = effect->billboard;
    record = effect->buffer->records;
    particleCount = effect->particleCount;
    lifetime = effect->lifetimeFrames;
    dds3BillboardSetScale(billboard, effect->billboardScale, effect->billboardScaleY);
    if (particleCount > 0) {
        step = particleCount;
        do {
            s32 age = record->age;

            if (age < lifetime && age >= 0) {
                PS2_COPY_QUAD(billboard, record->position);
                billboard->childParam = record->color;
                dds3BillboardDraw(billboard);
            }
            step--;
            record++;
        } while (step != 0);
    }
    SubmitKindDrawing(&effect->kindState);
}

/* Create another object of the same dispatch index and return the new instance. */
parObj_t *dds3ParticleCopy(parObj_t *obj) {
    parObj_t *createdObject;

    createdObject = dds3ParticleFuncTable[obj->dispatchIndex].func(obj);
    createdObject->dispatchIndex = obj->dispatchIndex;
    return createdObject;
}

/* Reissue the callback, reload the native updater's repeat count, and arm restart. */
void dds3ParticleReset(parObj_t *obj) {
    dds3ParticleAllBuffInitTable[obj->dispatchIndex]();
    obj->pendingRestartSteps = obj->restartStepCount;
    obj->restartFlag = 1;
}

/* Call the selected entry with factor, additionally scale kinds 2..4, then restart. */
void dds3ParticleSetScale(parObj_t *obj, f32 factor) {
    dds3ParticleSetScaleTable[obj->dispatchIndex](obj, factor);
    switch (obj->kindState.kind) {
    case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
        obj->kindState.payload.crossStrip.scale *= factor;
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
        obj->kindState.payload.crossTriangle.scale *= factor;
        break;
    case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
        obj->kindState.payload.trackPolygon.scale *= factor;
        break;
    }
    dds3ParticleReset(obj);
}

/* Return the full 16-bit restart state without converting it to a boolean. */
u16 dds3ParticleGetState(parObj_t *obj) {
    return obj->restartFlag;
}

/* Copy one 16-byte vector with the existing EE/VU copy primitive. */
void dds3ParticleSetPosition(void *destination, void *source) {
    PS2_COPY_QUAD(destination, source);
}

/* vu0 routine: compose the supplied matrix with the emitter's source matrix. */
void dds3ParticleSetRotateMatrix(parObj_t *effect, void *matrix) {
    VU0_LOAD_MATRIX(matrix);
    VU0_LOAD_MATRIX_B(effect->sourceMatrix);
    MultMatrixNoArg();
    VU0_STORE_MATRIX(effect->matrix);
}

void dds3ParticleSetRGBA(parObj_t *work, u32 parameterWord) {
    work->parameterWord = parameterWord;
}

void dds3ParticleSetTypeFlag(parObj_t *object, s32 mode) {
    mode &= 0xFF;
    switch (object->dispatchIndex) {
    case 1:
    case 5:
    case 11:
        ((parObjClientView_t *)object)->mode150 = mode;
        break;
    case 0:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 10:
    case 12:
        ((parObjClientView_t *)object)->mode151 = mode;
        break;
    case 9:
        break;
    }
    object->restartFlag = 1;
}

u8 dds3ParticleGetLoop(parObj_t *object) {
    switch (object->dispatchIndex) {
    case 1:
    case 5:
    case 11:
        return ((parObjClientView_t *)object)->mode150;
    case 0:
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 10:
    case 12:
        return ((parObjClientView_t *)object)->mode151;
    default:
        return 0;
    }
}

parObj_t *dds3ParticleExpCreate(s32 kind, parKindResource_t *resource) {
    parObj_t *object = (parObj_t *)((u8 *)resource + resource->offset + 0x10);
    billObj_t *billboard;

    if (resource->type != 3 || resource->offset != 0) {
        object->billboardCloneMarker = PAR_BILLBOARD_CLONE_FROM_RESOURCE;
        object = dds3ParticleFuncTable[kind].func(object);
        billboard = dds3BillboardCreate(resource->type, (u32)(resource + 1));
        dds3BillboardSetScale(billboard, object->billboardScale, object->billboardScaleY);
        dds3BillboardSetMode(billboard, object->billboardMode);
        dds3BillboardSetAnimColorAdvMode(billboard);
        object->billboard = billboard;
    } else {
        object = dds3ParticleFuncTable[kind].func(object);
    }
    object->dispatchIndex = kind;
    return object;
}

parObj_t *dds3ParticleExpCopy(parObj_t *source) {
    parObj_t *particle = dds3ParticleFuncTable[source->dispatchIndex].func(source);
    particle->dispatchIndex = source->dispatchIndex;
    if (source->billboardCloneMarker == PAR_BILLBOARD_CLONE_FROM_RESOURCE) {
        billObj_t *billboard = dds3BillboardCopy(source->billboard);
        dds3BillboardSetScale(billboard, particle->billboardScale, particle->billboardScaleY);
        dds3BillboardSetMode(billboard, particle->billboardMode);
        dds3BillboardSetAnimColorAdvMode(billboard);
        particle->billboard = billboard;
    }
    return particle;
}

void dds3ParticleExpDelete(parObj_t *object) {
    dds3ParticleFuncTable[object->dispatchIndex].destroy(object);
}

/* Apply queued updates, then draw live records and submit the kind's resource. */
void dds3ParticleExpDraw(parObj_t *effect) {
    effectBufferRecord_t *record;
    billObj_t *billboard;
    u32 step;
    s32 particleCount;
    s32 lifetime;
    f32 scaleStep;
    f32 spinStep;
    f32 spinDirection;
    f32 alternate;

    if (effect->restartFlag == 0) {
        return;
    }
    if (effect->pendingRestartSteps == 0) {
        dds3ParticleFuncTable[effect->dispatchIndex].update(effect);
    } else {
        for (step = 0; step < effect->pendingRestartSteps; step++) {
            dds3ParticleFuncTable[effect->dispatchIndex].update(effect);
        }
        particleCount = effect->particleCount;
        scaleStep = (effect->scaleEnd - effect->billboardScale) / effect->lifetimeFrames;
        spinStep = effect->spinDegrees * (3.14159265f / 180.0f);
        alternate = effect->alternateSpin ? -1.0f : 1.0f;
        for (step = 0; step < effect->pendingRestartSteps; step++) {
            s32 recordIndex;

            record = effect->buffer->records;
            spinDirection = 1.0f;
            for (recordIndex = 0; recordIndex < particleCount; recordIndex++, record++) {
                if (record->age >= 0) {
                    record->scale += scaleStep;
                    record->spin += spinStep * spinDirection;
                }
                spinDirection *= alternate;
            }
        }
        effect->pendingRestartSteps = 0;
    }
    /* Capture the draw owners before deriving this frame's scale/spin steps. */
    record = effect->buffer->records;
    billboard = effect->billboard;
    particleCount = effect->particleCount;
    scaleStep = (effect->scaleEnd - effect->billboardScale) / effect->lifetimeFrames;
    lifetime = effect->lifetimeFrames;
    spinStep = effect->spinDegrees * (3.14159265f / 180.0f);
    alternate = effect->alternateSpin ? -1.0f : 1.0f;
    spinDirection = 1.0f;
    if (particleCount > 0) {
        step = particleCount;
        do {
            s32 age = record->age;

            if (age < lifetime && age >= 0) {
                PS2_COPY_QUAD(billboard, record->position);
                billboard->rotationAngle = record->spin;
                billboard->childParam = record->color;
                billboard->childScaleX = billboard->childScaleY = record->scale;
                record->scale += scaleStep;
                if (record->scale < 0) {
                    record->scale = 0;
                }
                record->spin += spinStep * spinDirection;
                dds3BillboardSetAnimKey(billboard, age);
                dds3BillboardDraw(billboard);
            }
            spinDirection *= alternate;
            --step;
            ++record;
        } while (step != 0);
    }
    SubmitKindDrawing(&effect->kindState);
}

void dds3ParticleExpReset(parObj_t *obj) {
    dds3ParticleReset(obj);
}

void dds3ParticleExpSetScale(float scale, parObj_t *work) {
    dds3ParticleSetScale(work, scale);
    work->scaleEnd *= scale;
}

/* Descriptor query: return the complete 16-bit restart state. */
u16 dds3ParticleExpGetState(parObj_t *obj) {
    return dds3ParticleGetState(obj);
}

void dds3ParticleExpSetPosition(void *dst, void *src) {
    PS2_COPY_QUAD(dst, src);
}

void dds3ParticleExpSetRotateMatrix(parObj_t *effect, void *matrix) {
    dds3ParticleSetRotateMatrix(effect, matrix);
}

void dds3ParticleExpSetRGBA(parObj_t *work, u32 parameterWord) {
    work->parameterWord = parameterWord;
}

void dds3ParticleExpSetLoop(parObj_t *work, u8 mode) {
    dds3ParticleSetTypeFlag(work, mode);
}

/* Descriptor query: select the kind's current mode byte. */
u8 dds3ParticleExpGetLoop(parObj_t *work) {
    return dds3ParticleGetLoop(work);
}

/* Kinds 2-4 keep the scale at +8 of their own record; copy it into the
 * shared vector and store the (vf10 - vf11) difference. */
void dds3Particle_EffectInit(parKindState_t *obj) {
    f32 scale;

    switch (obj->kind) {
    case 0:
    case PAR_KIND_STATE_HISTORY:
        return;
    case PAR_KIND_STATE_BILLBOARD_CROSS_STRIP:
        scale = obj->payload.crossStrip.scale;
        parTrack_w[0] = parTrack_w[1] = parTrack_w[2] = scale;
        break;
    case PAR_KIND_STATE_BILLBOARD_CROSS_TRIANGLE:
        scale = obj->payload.crossTriangle.scale;
        parTrack_w[0] = parTrack_w[1] = parTrack_w[2] = scale;
        break;
    case PAR_KIND_STATE_TRACK_POLYGON_CROSS_AXES:
        scale = obj->payload.trackPolygon.scale;
        parTrack_w[0] = parTrack_w[1] = parTrack_w[2] = scale;
        break;
    default:
        return;
    }
    VU0_LOAD_VF10(aViewTarget);
    VU0_LOAD_VF11(aViewEye);
    VU0_SUB_VF10_VF11();
    VU0_STORE_VF10(parLook);
}

void dds3Particle_EffectSet(parKindState_t *work, s32 index, u32 color, f32 speed) {
    switch ((u16)work->kind) {
    case 1:
        dds3Particle_BlurSet(work->payload.history.table, index, color, speed);
        return;
    case 2:
        dds3Particle_TrackSet(work->payload.crossStrip.drawSystem, index, color);
        return;
    case 3:
        dds3Particle_Track2Set(work->payload.crossTriangle.drawSystem, index, color);
        return;
    case 4:
        dds3Particle_Track3Set(work->payload.trackPolygon.modelList, index, color);
        break;
    }
}

void dds3Particle_EffectReset(parKindState_t *work, s32 index) {
    switch ((u16)work->kind) {
    case 1:
        dds3Particle_BlurClearItem(work->payload.history.table, index);
        return;
    case 2:
        dds3TrackPolyListInit(work->payload.crossStrip.drawSystem, index);
        return;
    case 3:
        dds3TrackPolyListInit(work->payload.crossTriangle.drawSystem, index);
        return;
    case 4:
        Eff_ModelTrackPolyListReset(work->payload.trackPolygon.modelList, index);
        break;
    }
}

void dds3Particle_TrackSet(parSystem_t *system, s32 index, u32 color) {
    u128 axis[2];
    VU0_COPY_VF12_TO_VF11();
    VU0_SUB_VF10_VF11();
    VU0_LOAD_VF11(parLook);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_LOAD_VF11(parTrack_w);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF2();
    VU0_COPY_VF12_TO_VF10();
    VU0_COPY_VF2_TO_VF12();
    VU0_COPY_VF10_TO_VF11();
    VU0_ADD_VF10_VF12();
    VU0_STORE_VF10(&axis[0]);
    VU0_COPY_VF12_TO_VF10();
    VU0_SUB_VF11_VF10();
    VU0_STORE_VF11(&axis[1]);
    dds3TrackPolySet(system, index, axis);
    dds3TrackPolyVCRefresh_ParType1(system, index);
    dds3TrackPolyColorSet(system, index, (color & 0xFF000000) | 0x808080);
}

void dds3Particle_Track2Set(parSystem_t *system, s32 index, u32 color) {
    u128 axis[3];
    VU0_COPY_VF12_TO_VF11();
    VU0_SUB_VF10_VF11();
    VU0_LOAD_VF11(parLook);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_LOAD_VF11(parTrack_w);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF2();
    VU0_COPY_VF12_TO_VF10();
    VU0_COPY_VF2_TO_VF12();
    VU0_STORE_VF10(&axis[1]);
    VU0_COPY_VF10_TO_VF11();
    VU0_ADD_VF10_VF12();
    VU0_STORE_VF10(&axis[0]);
    VU0_COPY_VF12_TO_VF10();
    VU0_SUB_VF11_VF10();
    VU0_STORE_VF11(&axis[2]);
    dds3TrackPolySet2(system, index, axis);
    dds3TrackPolyVCRefresh_ParType2(system, index);
    dds3TrackPolyColorSet(system, index, (color & 0xFF000000) | 0x808080);
}

void dds3Particle_Track3Set(effTrackPolyList_t *list, s32 index, u32 color) {
    u128 axis[2];
    VU0_COPY_VF12_TO_VF11();
    VU0_SUB_VF10_VF11();
    VU0_LOAD_VF11(parLook);
    VU0_CROSS_VF10_VF11();
    VU0_NORMALIZE_VF10();
    VU0_LOAD_VF11(parTrack_w);
    VU0_MUL_VF10_VF11();
    VU0_COPY_VF10_TO_VF2();
    VU0_COPY_VF12_TO_VF10();
    VU0_COPY_VF2_TO_VF12();
    VU0_COPY_VF10_TO_VF11();
    VU0_ADD_VF10_VF12();
    VU0_STORE_VF10(&axis[0]);
    VU0_COPY_VF12_TO_VF10();
    VU0_SUB_VF11_VF10();
    VU0_STORE_VF11(&axis[1]);
    Eff_ModelTrackPolyListVertexSet(list, index, axis);
    Eff_ModelTrackPolyListColorSet(list, index, (color & 0xFF000000) | 0x808080);
}

u32 dds3Particle_TrackPolyGetRecTiming(void) {
    return 0;
}
