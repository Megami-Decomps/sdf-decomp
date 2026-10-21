#include "common.h"
#include "sdf_types.h"
#include "sdf_resource.h"

struct sdfMemHandle_t;

#define SDF_QWORD_ALIGNMENT_MASK 0xF

#define SDF_PACKET_BUFFER_ALIGNMENT_MASK 0x7F

#define SDF_PACKET_BUFFER_COUNT 2

void sdfMemFreeDirect(struct sdfMemHandle_t *allocation);

extern struct sdfMemHandle_t *sdfGrPacketBuffHandle;


u32 sdfMemLock(struct sdfMemHandle_t *allocation);

extern s32 sdfGrPacketBuffSize;

extern s32 D_00551870[2];

extern s32 sdfGrPacketBuffPtr;

extern s32 sdfGrPacketBuffLimit;

typedef struct effDebugPolyParams_t {
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
} effDebugPolyParams_t;

void sdfGrMakePacketBuff(s32 bufferBytes);

void sdfGrSetupPacketBuff(s32 bufferIndex);

s32 sdfGrAllocPacketBuff(s32 size);

void sdfGrClosePacketBuff(s32 cursorAddress);

void sdfGrMakePacketBuff(s32 bufferBytes) {
    s32 allocationAddress;
    if (sdfGrPacketBuffHandle != 0) {
        sdfMemFreeDirect(sdfGrPacketBuffHandle);
        sdfGrPacketBuffHandle = 0;
    }
    bufferBytes = (bufferBytes + SDF_PACKET_BUFFER_ALIGNMENT_MASK) & ~SDF_PACKET_BUFFER_ALIGNMENT_MASK;
    sdfGrPacketBuffSize = bufferBytes;
    sdfGrPacketBuffHandle = sdfMemAllocDirect(bufferBytes * SDF_PACKET_BUFFER_COUNT);
    allocationAddress = sdfMemLock(sdfGrPacketBuffHandle);
    D_00551870[0] = allocationAddress;
    D_00551870[1] = allocationAddress + bufferBytes;
}

void sdfGrSetupPacketBuff(s32 bufferIndex) {
    sdfGrPacketBuffPtr = D_00551870[bufferIndex];
    sdfGrPacketBuffLimit = D_00551870[bufferIndex] + sdfGrPacketBuffSize;
}

s32 sdfGrGetPacketBuffRemain(void) {
    return sdfGrPacketBuffLimit - sdfGrPacketBuffPtr;
}

s32 sdfGrAllocPacketBuff(s32 size) {
    s32 packet;

    packet = sdfGrPacketBuffPtr;
    sdfGrPacketBuffPtr = sdfGrPacketBuffPtr + ((size + 0xfU) & 0xfffffff0);
    return packet;
}

s32 sdfGrOpenPacketBuff(void) {
    return sdfGrPacketBuffPtr;
}

void sdfGrClosePacketBuff(s32 cursorAddress) {
    sdfGrPacketBuffPtr = (cursorAddress + SDF_QWORD_ALIGNMENT_MASK) & ~SDF_QWORD_ALIGNMENT_MASK;
}
