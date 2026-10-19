#ifndef SDF_RESOURCE_H
#define SDF_RESOURCE_H

/* Engine resource records the packet builders need but the engine owners in
 * libsdf.h do not have to drag in with them. Same intent as sdf_types.h one
 * layer down: a unit that builds a VIF packet from a material or a TMX header
 * includes this and gets the canonical layout, without the queue, file and
 * motion definitions it has no use for.
 *
 * libsdf.h includes this, so there is exactly one definition of each record.
 * The pointed-to types stay forward declarations on purpose: a unit that also
 * carries its own copy of one of them keeps compiling. */

#include "common.h"

struct sdfMatPacket_t;
struct sdfTexHandle_t;

typedef u8 sdfTextureClampMode_t;

/* One material plus its two prebuilt VIF packets, one per texture context.
 * sdfhook gives this struct a trailing reserved[88] and 160 bytes; sdfMatNew
 * allocates 0x48, which is where the struct ends here. */
typedef struct sdfMat_t
{
    u32 id;
    u16 flags;
    u8 duty;
    u8 pad;
    struct sdfMatPacket_t *packet[2];
    u32 ambient;
    u32 diffuse;
    u32 multiTexDiffuse;
    f32 constant;
    u32 specular;
    u32 specularParm;
    u32 refmap;
    struct sdfTexHandle_t *texHandle;
    struct sdfTexHandle_t *multiTexHandle;
    u32 multiTexMethod;
    /* Pointee left untyped on purpose: libsdf.h and the IDB spell it the
     * struct sdfMatUvwParm_t, while sdf3DPrim.c and effPCPMisc.c spell the
     * same 24 bytes a union of three views. Nothing dereferences these two
     * through a material; they are stored, passed on and freed. Unifying the
     * two readings is type recovery, not a header move. */
    void *uvwParm;
    void *multiTexUvwParm;
    f32 fdBias;
    f32 fdSoften;
} sdfMat_t;

/* Native 0x40-byte TMX header; sdfTexSetEx and resource loaders consume it. */
typedef struct TMX_HEADER_t
{
    u8 fileType;
    u8 compression;
    s16 userId;
    u32 fileSize;
    u32 formatId;
    s32 realSize;
    u8 numOfCLUT;
    u8 CLUTFormat;
    s16 width;
    s16 height;
    u8 pixelFormat;
    u8 numOfMipmap;
    u16 mipmapParam;
    u8 flags;
    sdfTextureClampMode_t texWrapMode;
    /* The IDB calls 0x1C onward userArea and comment. These two are what this
     * game puts in its user area; sdfTmx reads both as words. */
    s32 resourceKey;
    s32 battleTextureSlot;
    u8 pad24[0x1C];
} TMX_HEADER_t;

#endif /* SDF_RESOURCE_H */
