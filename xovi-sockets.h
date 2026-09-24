#ifndef XOVI_SOCKETS_H
#define XOVI_SOCKETS_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define XEM_SOCKETS_ABI 1
#define XEM_SOCKETS_METADATA "xovi-extension-manager$socketsApi"
/* Managed Unix stream sockets: UTF-8 JSON object per line (max 64 KiB).
 * All calls except freeString and all callbacks run on the QCoreApplication thread. Callbacks are
 * queued, never invoked by registerService/send/unregisterService. JSON passed
 * to callbacks is borrowed; returned JSON must be freed with freeString.
 * Register JSON: {"ownerId":"plugin","serviceId":"control"}.
 * Returns {ok,serviceHandle,path}; handles are process-local, not persistent.
 * Unregister before destroying userData or unloading code. Unregister cancels
 * queued callbacks and disconnects clients. External applications reconnect
 * after xochitl restarts. No sockets are created in xochitl_pdf_renderer.
 */
typedef void (*XemSocketCallback)(const char *eventJson, void *userData);
typedef struct XemSocketsApi {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*registerService)(const char *description, XemSocketCallback callback, void *userData);
    char *(*unregisterService)(uint64_t serviceHandle);
    char *(*query)(uint64_t serviceHandle); /* 0 lists service metadata */
    char *(*send)(uint64_t serviceHandle, uint64_t connectionId, const char *jsonObject);
    char *(*disconnectClient)(uint64_t serviceHandle, uint64_t connectionId);
    void (*freeString)(char *value);
} XemSocketsApi;
typedef const XemSocketsApi *(*XemSocketsApiGetter)(void);
#ifdef __cplusplus
}
#endif
#endif
