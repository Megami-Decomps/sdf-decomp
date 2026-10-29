#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

s32 strlen(const char *aStr);

enum SdfFileIOCmdStat {
    SDF_FILE_CMD_STAT_INIT = 0,
    SDF_FILE_CMD_STAT_DORMANT = 1,
    SDF_FILE_CMD_STAT_OPEN = 2,
    SDF_FILE_CMD_STAT_SEEK = 3,
    SDF_FILE_CMD_STAT_READ = 4,
    SDF_FILE_CMD_STAT_WRITE = 5,
    SDF_FILE_CMD_STAT_CLOSE = 6,
    SDF_FILE_CMD_STAT_SUSPEND = 7,
    SDF_FILE_CMD_STAT_DONE = 8,
    SDF_FILE_CMD_STAT_ERROR = 9,
    SDF_FILE_CMD_STAT_CANCEL = 10
};

enum SdfFileIOCmd {
    SDF_FILE_CMD_NONE = 0,
    SDF_FILE_CMD_READOPEN = 1,
    SDF_FILE_CMD_WRITEOPEN = 2,
    SDF_FILE_CMD_SEEK = 3,
    SDF_FILE_CMD_FILESIZE = 4,
    SDF_FILE_CMD_READ = 5,
    SDF_FILE_CMD_WRITE = 6,
    SDF_FILE_CMD_CLOSE = 7,
    SDF_FILE_CMD_READFILE = 8,
    SDF_FILE_CMD_WRITEFILE = 9
};

enum sdf_tex_resource {
    SDF_TEX_RESOURCE_FREE = 0,
    SDF_TEX_RESOURCE_FRAME = 1,
    SDF_TEX_RESOURCE_TEXTURE = 2,
    SDF_TEX_RESOURCE_CLUT = 3,
    SDF_TEX_RESOURCE_KNOWN_MODE_COUNT = 4
};

enum sdf_stream_output {
    SDF_STREAM_OUTPUT_RGBA32 = 0,
    SDF_STREAM_OUTPUT_RGBA16 = 1,
    SDF_STREAM_OUTPUT_RGBA16_DITHERED = 2
};

enum sdf_stream_playback {
    SDF_STREAM_PLAYBACK_INITIAL = 0,
    SDF_STREAM_PLAYBACK_FIRST_COMPLETION = 1,
    SDF_STREAM_PLAYBACK_CADENCED = 2
};

extern void *memcpy(void *, const void *, unsigned int);

void *sdfChipAllocDirect(s32 aSize);

extern char D_00549878[];

extern u8 sdfDev9LoadFlag;

extern u8 sdfDev9Ready;

extern s32 sceSifLoadStartModule(const char *modulePath, s32 flags, void *moduleArguments, s32 *resident);

s32 printf(const char *format, ...);

extern s32 (*p2sdSmgBuf)(void);

void sdfDev9LoadModule(void) {
    s32 resident;
    if (sdfDev9LoadFlag == 0) {
        sdfDev9Ready = 0;
        if (sceSifLoadStartModule("cdrom0:\\IRX\\DEV9.IRX;1", 0, NULL, &resident) < 0) {
            printf("cdrom0:\\IRX\\DEV9.IRX;1 could't load.\n");
            return;
        }
        if (resident != 0) {
            printf(D_00549878);
            return;
        }
        sdfDev9Ready = 1;
        sdfDev9LoadFlag = 1;
    }
}

#ifdef NON_MATCHING
extern void *memset(void *dst, s32 value, u32 size);

u64 sdf_2BFC58(void *dst, s32 size)
{
    u32 address = (u32)dst;
    s32 index, i, counter;
    u32 entry;

    if (size <= 0) return (u64)(s64)size | address;
    if (((address | (u32)size) & 15) != 0) return (u32)memset(dst, 0, size);

    /* Retail overwrites the aligned-path counter with its jump-table address. */
    index = (-(size >> 8)) & 7;
    counter = 0x002BFCAC;
    entry = (u32)counter + (u32)index * 4;
    for (i = 7 - index; i >= 0; i--) {
        PS2_STORE_ZERO_QUAD((void *)(address + (u32)i * 16));
    }
    while ((counter -= 8) >= 0) {
        address += 0x80;
        for (i = 7; i >= 0; i--) {
            PS2_STORE_ZERO_QUAD((void *)(address + (u32)i * 16));
        }
    }
    return entry;
}
#else
INCLUDE_ASM(const s32, "sdf/sdfDev9", sdf_2BFC58);
#endif

/* Clear `count` quadwords at dst: four per pass, then a falling 3/2/1
 * remainder. The pass is a goto loop (no loop notes, so no loop alignment),
 * and its exit test is taken before the four stores. */
u128 *sdfZeroMemQuad(u128 *dst, s32 count) {
    u128 *p = dst;
    s32 done;
    if (count >= 4) {
loop:
        count -= 4;
        done = count < 4;
        PS2_STORE_ZERO_QUAD(p);
        PS2_STORE_ZERO_QUAD_AT16(p);
        PS2_STORE_ZERO_QUAD_AT32(p);
        PS2_STORE_ZERO_QUAD_AT48(p);
        p += 4;
        if (!done) {
            goto loop;
        }
    }
    switch (count) {
    case 3:
        PS2_STORE_ZERO_QUAD_AT32(p);
    case 2:
        PS2_STORE_ZERO_QUAD_AT16(p);
    case 1:
        PS2_STORE_ZERO_QUAD(p);
    }
    return dst;
}

char *sdfStrdupChip(const char *text) {
    u32 length;
    char *copy;
    if (text == NULL) {
        return NULL;
    }
    length = strlen(text);
    copy = sdfChipAllocDirect(length + 1);
    memcpy(copy, text, length);
    copy[length] = 0;
    return copy;
}

s32 sdfBcdToDecimal(s32 aBcd)
{
    s32 result = 0;
    s32 scale = 1;

    while (aBcd > 0) {
        result += (aBcd & 0xF) * scale;
        aBcd >>= 4;
        scale *= 10;
    }
    return result;
}

/* Pack positive decimal digits into nibbles; non-positive input returns zero.
 * The original signed shifts and lack of an overflow check are retained.
 */
s32 sdf_2BFE08(s32 number) {
    s32 digitShift = 0;
    s32 packedDigits = 0;
    while (number > 0) {
        s32 quotient = number / 10;
        packedDigits |= (number - quotient * 10) << digitShift;
        number = quotient;
        digitShift += 4;
    }
    return packedDigits;
}

INCLUDE_RODATA(const s32, "sdf/sdfDev9", D_00549878);

