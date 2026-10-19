#include "common.h"

/* The SDK multi-RPC client. sceSifMBindRpc leaves the server handle at 0x24,
 * which is how the bind loop below knows the server answered. */
typedef struct
{
    u8 opaque00[0x24];
    void *serve;
} sdfIopRpcClient_t;

extern s32 printf(const char *format, ...);
extern s32 sceSifLoadModule(const char *path, s32 argSize, const char *args);
extern void sceSifMInitRpc(s32 mode);
extern s32 sceSifMBindRpc(sdfIopRpcClient_t *client, s32 number, s32 mode);
extern s32 sdfGetRunTimeCountBase(void);
extern s32 sdfGetRunTimeCount(s32 startTicks);

extern sdfIopRpcClient_t sdfIopRpcClient;
extern char D_00549040[];

void SdfIopInit(const char *driverPath, const char *modulePath)
{
    s32 startTicks;

    if (modulePath != NULL) {
        while (sceSifLoadModule(modulePath, 0, NULL) < 0) {
            printf(D_00549040, modulePath);
        }
    }
    while (sceSifLoadModule(driverPath, 0, NULL) < 0) {
        printf(D_00549040, driverPath);
    }
    if (modulePath != NULL) {
        sceSifMInitRpc(0);
    }
    while (1) {
        sceSifMBindRpc(&sdfIopRpcClient, 0x6F496453, 0);
        if (sdfIopRpcClient.serve != NULL) {
            break;
        }
        startTicks = sdfGetRunTimeCountBase();
        while (sdfGetRunTimeCount(startTicks) < 0x1ED2) {
        }
    }
}

INCLUDE_RODATA(const s32, "code_002A99B0", D_00549040);

