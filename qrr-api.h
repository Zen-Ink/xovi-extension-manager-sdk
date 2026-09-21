#ifndef QRR_API_H
#define QRR_API_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define QRR_API_METADATA "qt-resource-rebuilder$api"
/* All JSON responses are UTF-8, owned by QRR; always release with freeString.
 * registerPatch accepts UTF-8 QMD, returns registration status, NOT UI health.
 * Call before resource processing begins. IDs: [A-Za-z0-9_.-], 1..128 bytes.
 * snapshot is read-only and never calls plugin/UI code. API lives until exit.
 */
typedef struct QrrApiV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*registerPatch)(const char *owner, const char *id, const char *data, size_t size);
    char *(*snapshot)(void);
    void (*freeString)(char *value);
} QrrApiV1;
const QrrApiV1 *qrr_get_api_v1(void);
#ifdef __cplusplus
}
#endif
#endif
