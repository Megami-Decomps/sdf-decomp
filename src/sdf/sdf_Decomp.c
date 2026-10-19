#include "common.h"
#include "ps2asm.h"

enum {
    SDF_DECODE_READ_COMMAND = 1,
    SDF_DECODE_READ_COUNT_LOW = 2,
    SDF_DECODE_READ_COUNT_HIGH = 3,
    SDF_DECODE_DISPATCH = 4,
    SDF_DECODE_COPY_LITERAL = 5,
    SDF_DECODE_FILL_BYTE = 6,
    SDF_DECODE_COPY_SHORT_BACKREF = 7,
    SDF_DECODE_READ_BACKREF_LOW = 8,
    SDF_DECODE_READ_BACKREF_HIGH = 9,
    SDF_DECODE_EXPAND_BYTES = 10
};
enum {
    SDF_DECODE_COMMAND_CLASS_MASK = 0xE0,
    SDF_DECODE_COMMAND_LENGTH_MASK = 0x1F,
    SDF_DECODE_COMMAND_END_MARKER = 0xFF,
    SDF_DECODE_COMMAND_RESERVED_MIN = 0xE0,
    SDF_DECODE_COMMAND_COPY_LITERAL = 0x00,
    SDF_DECODE_COMMAND_ZERO_FILL = 0x20,
    SDF_DECODE_COMMAND_REPEAT_BYTE = 0x40,
    SDF_DECODE_COMMAND_COPY_SHORT_BACKREFERENCE = 0x60,
    SDF_DECODE_COMMAND_COPY_LONG_BACKREFERENCE = 0x80,
    SDF_DECODE_COMMAND_EXPAND_BYTES = 0xA0
};

struct SdfPacket;

struct SdfPacDecoder;

/* Load matrix into the VU0 scratch bank and postmultiply the primary bank. */
void MultMatrixRev(const void *matrix);

/* Four-doubleword DMA packet payload (0x20); DDS1/2 game/code_002D33C8/0032C278.c. */
typedef struct SdfPacket {
    u64 unk0;
    u64 unk8;
    u64 unk10;
    u64 unk18;
} SdfPacket;

extern const u8 *D_00594780[16];

extern f32 sdfDrawFaceListParam[][4];

extern u8 sdf3DScreenTm[];

/* Incremental decoder access view shared by both providers. The backing
 * decoder block is allocated as 0x20 bytes; only this 0x14-byte prefix is
 * described here, and its trailing allocation bytes remain unknown. */
typedef struct SdfPacDecoder {
    u8 state;             /* 0x00 */
    u8 command;           /* 0x01 */
    u16 pendingBytes;     /* 0x02 */
    u8 *output;           /* 0x04 */
    const u8 *input;      /* 0x08 */
    s32 inputBytes;       /* 0x0C */
    u8 backrefLow;        /* 0x10 */
    u8 pad11[3];          /* 0x11: opaque */
} SdfPacDecoder;

/* Read the active packet cursor or restore it at a quadword boundary. */
s32 sdfGrOpenPacketBuff(void);

void sdfGrClosePacketBuff(s32 cursorAddress);

extern s32 D_00552170;

/* One row per administrative mode. The entry receives the restore flag and
 * mode data; destroy and cleanup return scheduler results. Each row is 12 bytes
 * on the EE. */
typedef struct Dds3AdminModeDispatch {
    void (*entry)(s32 restoring, void *data);
    s32 (*destroy)(void);
    s32 (*cleanup)(void);
} Dds3AdminModeDispatch;

extern void *memcpy(void *, const void *, unsigned int);

s32 sdfDecomp(SdfPacDecoder *decoder, const u8 *input, s32 inputBytes);

void sdfDecompInit(SdfPacDecoder *decoder, u8 *output);

SdfPacket *sdfDrawModelWithMatrix(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags);

SdfPacket *effDebugDrawWireModel(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags);

SdfPacket *sdfDrawRotatedModel(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags);

s32 sdfDecomp(SdfPacDecoder *decoder, const u8 *input, s32 inputBytes) {
    s32 count;
    s32 offset;
    u8 *output;
    u32 sourceAddress;

    if (decoder->state == 0) {
        return 1;
    }

    D_00552170 = 0;
    output = decoder->output;
    if (inputBytes > 0) {
        do {
            switch (decoder->state) {
                case SDF_DECODE_READ_COMMAND: {
                    u8 command;
                    s32 commandBytes;
                    s32 historyIndex = D_00552170;

                    inputBytes--;
                    D_00594780[historyIndex] = input;
                    D_00552170 = (historyIndex + 1) & 0xF;
                    command = *input++;
                    if (command >= SDF_DECODE_COMMAND_RESERVED_MIN) {
                        if (command == SDF_DECODE_COMMAND_END_MARKER) {
                            decoder->output = output;
                            decoder->input = input;
                            decoder->inputBytes = inputBytes;
                            decoder->state = 0;
                            return 1;
                        }
                    } else {
                        decoder->command = command & SDF_DECODE_COMMAND_CLASS_MASK;
                        commandBytes = command & SDF_DECODE_COMMAND_LENGTH_MASK;
                        if (commandBytes == 0) {
                            decoder->state = SDF_DECODE_READ_COUNT_LOW;
                        } else {
                            decoder->pendingBytes = commandBytes;
                            decoder->state = SDF_DECODE_DISPATCH;
                        }
                    }
                    break;
                }
                case SDF_DECODE_READ_COUNT_LOW:
                    decoder->pendingBytes = *input++;
                    inputBytes--;
                    decoder->state = SDF_DECODE_READ_COUNT_HIGH;
                    break;
                case SDF_DECODE_READ_COUNT_HIGH:
                    decoder->pendingBytes |= *input++ << 8;
                    inputBytes--;
                    decoder->state = SDF_DECODE_DISPATCH;
                    break;
                case SDF_DECODE_DISPATCH:
                    switch (decoder->command) {
                    case SDF_DECODE_COMMAND_COPY_LITERAL:
                        decoder->state = SDF_DECODE_COPY_LITERAL;
                        break;
                    case SDF_DECODE_COMMAND_ZERO_FILL:
                        count = decoder->pendingBytes;
                        memset(output, 0, count);
                        output += count;
                        decoder->state = SDF_DECODE_READ_COMMAND;
                        break;
                    case SDF_DECODE_COMMAND_REPEAT_BYTE:
                        decoder->state = SDF_DECODE_FILL_BYTE;
                        break;
                    case SDF_DECODE_COMMAND_COPY_SHORT_BACKREFERENCE:
                        decoder->state = SDF_DECODE_COPY_SHORT_BACKREF;
                        break;
                    case SDF_DECODE_COMMAND_COPY_LONG_BACKREFERENCE:
                        decoder->state = SDF_DECODE_READ_BACKREF_LOW;
                        break;
                    case SDF_DECODE_COMMAND_EXPAND_BYTES:
                        decoder->state = SDF_DECODE_EXPAND_BYTES;
                        break;
                    }
                    break;
                case SDF_DECODE_COPY_LITERAL:
                    count = decoder->pendingBytes;
                    if (inputBytes < count) {
                        count = inputBytes;
                    }
                    memcpy(output, input, count);
                    output += count;
                    input += count;
                    inputBytes -= count;
                    decoder->pendingBytes -= count;
                    if (decoder->pendingBytes == 0) {
                        decoder->state = SDF_DECODE_READ_COMMAND;
                    }
                    break;
                case SDF_DECODE_FILL_BYTE:
                    count = decoder->pendingBytes;
                    memset(output, *input++, count);
                    output += count;
                    inputBytes--;
                    decoder->state = SDF_DECODE_READ_COMMAND;
                    break;
                case SDF_DECODE_COPY_SHORT_BACKREF:
                    sourceAddress = (u32)(output - *input++);
                    count = decoder->pendingBytes;
                    inputBytes--;
                    do {
                        *output++ = *(u8 *)sourceAddress++;
                    } while (--count != 0);
                    decoder->state = SDF_DECODE_READ_COMMAND;
                    break;
                case SDF_DECODE_READ_BACKREF_LOW:
                    decoder->backrefLow = *input++;
                    inputBytes--;
                    if (inputBytes == 0) {
                        decoder->state = SDF_DECODE_READ_BACKREF_HIGH;
                        break;
                    }
                    /* fallthrough */
                case SDF_DECODE_READ_BACKREF_HIGH: {
                    s32 combinedOffset;

                    offset = *input++ << 8;
                    sourceAddress = decoder->backrefLow;
                    inputBytes--;
                    combinedOffset = sourceAddress | offset;
                    sourceAddress = (u32)(output - combinedOffset);
                    count = decoder->pendingBytes;
                    do {
                        *output++ = *(u8 *)sourceAddress++;
                    } while (--count != 0);
                    decoder->state = SDF_DECODE_READ_COMMAND;
                    break;
                }
                case SDF_DECODE_EXPAND_BYTES:
                    count = decoder->pendingBytes;
                    if (inputBytes < count) {
                        count = inputBytes;
                        inputBytes = 0;
                        decoder->pendingBytes -= count;
                    } else {
                        decoder->pendingBytes = 0;
                        decoder->state = SDF_DECODE_READ_COMMAND;
                        inputBytes -= count;
                    }
                    do {
                        *output++ = *input++;
                        *output++ = 0;
                    } while (--count != 0);
                    break;
            }
        } while (inputBytes > 0);
    }
    decoder->input = input;
    decoder->inputBytes = inputBytes;
    decoder->output = output;
    return 0;
}

void sdfDecompInit(SdfPacDecoder *decoder, u8 *output) {
    decoder->output = output;
    decoder->state = 1;
}

SdfPacket *sdfDrawModelWithMatrix(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags) {
    SdfPacket *packet;
    f32 (*viewport)[4];
    u128 *cursor;
    s32 i;
    s32 emitted;
    s32 plane;
    u32 clipFlags;
    u32 firstClip;
    u32 secondClip;
    u32 quadwordCount;

    packet = (SdfPacket *)sdfGrOpenPacketBuff();
    cursor = (u128 *)(packet + 1);
    viewport = sdfDrawFaceListParam;
    VU0_LOAD_VF14(viewport);
    VU0_LOAD_VF15_AT16(viewport);
    emitted = 0;
    for (i = 0; i < vertexCount; i += 2) {
        SDF_VU0_LOADVECTOR(vertices[i]);
        EE_MMI_RGBA_TO_VF8(colors[i]);
        SDF_VU0_APPLYMATRIX_XYZ1();
        SDF_VU0_LOADVECTOR_B(vertices[i + 1]);
        EE_MMI_RGBA_TO_VF9(colors[i + 1]);
        VU0_TRANSFORM_POINT_VF11();
        SDF_VU0_CLIPTEST();
        SDF_VU0_CLIPTEST_B();
        SDF_VU0_CLIPTESTWAIT(clipFlags);
        firstClip = (clipFlags >> 6) & 0x3F;
        secondClip = clipFlags & 0x3F;
        if ((firstClip & secondClip) != 0 || (firstClip & 0x20) != 0 || (secondClip & 0x20) != 0) {
            continue;
        }
        if (firstClip != 0 || secondClip != 0) {
            for (plane = 4; plane != -1; plane--) {
                if (firstClip & (1 << plane)) {
                    if ((plane & 1) == 0) {
                        VU0_POSITIVE_CLIP_PLANE_VF2();
                    } else {
                        VU0_NEGATIVE_CLIP_PLANE_VF2();
                    }
                    VU0_CLIP_SEGMENT_PREP_FIRST();
                    switch (plane) {
                    case 0:
                    case 1:
                        VU0_CLIP_RATIO_X();
                        break;
                    case 2:
                    case 3:
                        VU0_CLIP_RATIO_Y();
                        break;
                    default:
                        VU0_CLIP_RATIO_Z();
                        break;
                    }
                    VU0_CLIP_SEGMENT_UPDATE_FIRST();
                    SDF_VU0_CLIPTESTWAIT(firstClip);
                    firstClip &= 0x3F;
                    if (firstClip & secondClip) {
                        break;
                    }
                }
                if (secondClip & (1 << plane)) {
                    if ((plane & 1) == 0) {
                        VU0_POSITIVE_CLIP_PLANE_VF2();
                    } else {
                        VU0_NEGATIVE_CLIP_PLANE_VF2();
                    }
                    VU0_CLIP_SEGMENT_PREP_SECOND();
                    switch (plane) {
                    case 0:
                    case 1:
                        VU0_CLIP_RATIO_X();
                        break;
                    case 2:
                    case 3:
                        VU0_CLIP_RATIO_Y();
                        break;
                    default:
                        VU0_CLIP_RATIO_Z();
                        break;
                    }
                    VU0_CLIP_SEGMENT_UPDATE_SECOND();
                    SDF_VU0_CLIPTESTWAIT(secondClip);
                    secondClip &= 0x3F;
                    if (firstClip & secondClip) {
                        break;
                    }
                }
            }
            if (firstClip & secondClip) {
                continue;
            }
        }
        VU0_PROJECT_COLORED_SEGMENT();
        VU0_STORE_VF8_AT0(cursor);
        VU0_STORE_VF10_AT16(cursor);
        VU0_STORE_VF9_AT32(cursor);
        VU0_STORE_VF11_AT48(cursor);
        cursor += 4;
        emitted += 2;
    }
    sdfGrClosePacketBuff((s32)cursor & 0x0FFFFFFF);
    quadwordCount = emitted * 2 + 1;
    packet->unk0 = (quadwordCount & 0xFFFF) | 0x20000000;
    packet->unk8 = ((u64)(quadwordCount | 0x50000000) << 32) | 0x10000000;
    packet->unk10 = emitted | ((u64)(primitiveFlags | 9) << 47) | 0x2000400000008000ULL;
    packet->unk18 = 0x41;
    return packet;
}

SdfPacket *effDebugDrawWireModel(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags) {
    SDF_VU0_LOADMATRIX(sdf3DScreenTm);
    return sdfDrawModelWithMatrix(vertices, colors, vertexCount, primitiveFlags);
}

SdfPacket *sdfDrawRotatedModel(const f32 vertices[][4], const u32 *colors, s32 vertexCount, u32 primitiveFlags) {
    MultMatrixRev(sdf3DScreenTm);
    return sdfDrawModelWithMatrix(vertices, colors, vertexCount, primitiveFlags);
}
