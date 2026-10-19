#include "fpu.h"
#include "common.h"
#include "libsdf.h"
#include "ps2asm.h"

#define SDF_PI_2 1.5707963f

f32 sdfSin(f32 aRad);

enum sdf_dev_state {
    SDF_DEV_STATE_OPENING = 2,
    SDF_DEV_STATE_SEEKING = 3,
    SDF_DEV_STATE_READING = 4,
    SDF_DEV_STATE_WRITING = 5,
    SDF_DEV_STATE_CLOSING = 6,
    SDF_DEV_STATE_ACTIVE = 7,
    SDF_DEV_STATE_COMPLETE = 8,
    SDF_DEV_STATE_INACTIVE = 9
};

enum sdf_dev_operation {
    SDF_DEV_OPERATION_NONE = 0,
    SDF_DEV_OPERATION_OPEN_READ = 1,
    SDF_DEV_OPERATION_OPEN_WRITE = 2,
    SDF_DEV_OPERATION_SEEK = 3,
    SDF_DEV_OPERATION_GET_SIZE = 4,
    SDF_DEV_OPERATION_READ = 5,
    SDF_DEV_OPERATION_WRITE = 6,
    SDF_DEV_OPERATION_CLOSE = 7,
    SDF_DEV_OPERATION_READ_BUFFER = 8,
    SDF_DEV_OPERATION_WRITE_BUFFER = 9
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

extern f32 D_0052D944[];

extern s32 (*p2sdSmgBuf)(void);

/* Mirror the phase into a quarter turn, then evaluate an odd ninth-degree polynomial. */
f32 sdfSin(f32 angle) {
    f32 phase = angle * 0.15915494f;
    f32 polynomialInput;
    f32 inputSquared;
    f32 inputCubed;
    f32 inputFifthPower;
    f32 inputSeventhPower;
    f32 inputNinthPower;
    phase -= (s32)phase;
    if (phase > 0.5f) {
        phase -= 1.0f;
    } else if (phase < -0.5f) {
        phase += 1.0f;
    }
    if (phase > 0.25f) {
        phase = 0.5f - phase;
    } else if (phase < -0.25f) {
        phase = -0.5f - phase;
    }
    polynomialInput = phase * 3.9999996f;
    inputSquared = polynomialInput * polynomialInput;
    inputCubed = inputSquared * polynomialInput;
    inputFifthPower = inputCubed * inputSquared;
    inputSeventhPower = inputFifthPower * inputSquared;
    inputNinthPower = inputSeventhPower * inputSquared;
    return polynomialInput * 1.5707963f + inputCubed * -0.64596367f + inputFifthPower * 0.07968968f + inputSeventhPower * -0.0046737656f + inputNinthPower * 0.00015148419f;
}

f32 sdfCos(f32 aRad)
{
    return sdfSin(aRad + SDF_PI_2);
}

/* Binary-search samples, using zero as the implicit lower endpoint.
 * The step is integer 1/sampleCount converted to f32, not a float reciprocal.
 * The caller must provide valid table bounds for the final sample index.
 */
f32 sdfTableInterpolation(f32 lookupValue, const f32 *table, s32 sampleCount) {
    f32 positionStep = 1 / sampleCount;
    s32 lowerIndex = 0;
    s32 upperIndex = sampleCount;
    s32 sampleIndex;
    f32 lowerValue;
    f32 upperValue;
    do {
        sampleIndex = lowerIndex + upperIndex;
        sampleIndex >>= 1;
        upperValue = table[sampleIndex];
        if (lookupValue < upperValue) {
            upperIndex = sampleIndex;
        } else {
            sampleIndex++;
            lowerIndex = sampleIndex;
        }
    } while (lowerIndex < upperIndex);
    upperValue = table[sampleIndex];
    lowerValue = 0.0f;
    if (sampleIndex != 0) {
        lowerValue = table[sampleIndex - 1];
    }
    return sampleIndex * positionStep + (lookupValue - lowerValue) * positionStep / (upperValue - lowerValue);
}

f32 sdfAtan(f32 aX)
{
    f32 x2 = aX * aX;
    f32 x3 = x2 * aX;
    f32 x5 = x2 * x3;

    return aX * 0.9999998f + x3 * -0.33325735f + x5 * 0.19388643f;
}

f32 sdfAtan2(f32 y, f32 x) {
    s32 xNegative = 0;
    s32 yNegative;
    f32 angle;
    if (x < 0.0f) {
        x = -x;
        xNegative = 1;
    }
    yNegative = 0;
    if (y < 0.0f) {
        y = -y;
        yNegative = 1;
    }
    if (y < x) {
        angle = sdfAtan(y / x);
    } else {
        angle = 1.5707963f - sdfAtan(x / y);
    }
    if (xNegative != 0) {
        angle = 3.1415926f - angle;
    }
    if (yNegative != 0) {
        angle = -angle;
    }
    return angle;
}

/* Restore the input sign after sampling; magnitudes at or above one use half pi. */
f32 sdfAsin(f32 x) {
    f32 inputSign;
    f32 angleMagnitude;
    if (x < 0.0f) {
        x = -x;
        inputSign = -1.0f;
    } else {
        inputSign = 1.0f;
    }
    angleMagnitude = x >= 1.0f ? 1.5707963f : sdfTableInterpolation(x, D_0052D944, 128) * 1.5707963f;
    return angleMagnitude * inputSign;
}

/* Return the input-signed complement of the sampled angle, not standard acos(x).
 * Magnitudes at or above one leave the angle magnitude at zero.
 */
f32 sdfAcos(f32 x) {
    f32 inputSign;
    f32 angleMagnitude;
    if (x < 0.0f) {
        x = -x;
        inputSign = -1.0f;
    } else {
        inputSign = 1.0f;
    }
    angleMagnitude = 0.0f;
    if (!(x >= 1.0f)) {
        angleMagnitude = (1.0f - sdfTableInterpolation(x, D_0052D944, 128)) * 1.5707963f;
    }
    return angleMagnitude * inputSign;
}

/* Keep the cast-plus/minus-one turn adjustment; this is not general modulo.
 * Inputs already within the pi thresholds are returned unchanged.
 */
f32 sdfClipAngleValuePI(f32 angle) {
    s32 adjustedTurns;
    if (angle > 3.1415926f) {
        adjustedTurns = (s32)(angle / 6.2831852f) + 1;
        return angle - (f32)adjustedTurns * 6.2831852f;
    }
    if (angle < -3.1415926f) {
        adjustedTurns = (s32)(angle / 6.2831852f) - 1;
        return angle - (f32)adjustedTurns * 6.2831852f;
    }
    return angle;
}
