#ifndef XOVI_NAVIGATION_H
#define XOVI_NAVIGATION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define XEM_NAVIGATION_ABI 1
#define XEM_NAVIGATION_METADATA "xovi-extension-manager-ui$navigationApi"
/* Optional metadata-discovered UI service. Safe to call from worker threads.
 * ownerId is a provider or manifest package ID; NULL pageId means "main".
 * UTF-8 JSON result is owned by the caller; release with freeString.
 * accepted/queued means navigation was scheduled, NOT that QML loaded.
 * Page readiness/failure remains available through manager uiStates.
 * Table and function pointers remain valid for the process lifetime. */
typedef struct XemNavigationApiV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*openSettings)(const char *ownerId, const char *pageId);
    void (*freeString)(char *value);
} XemNavigationApiV1;
typedef const XemNavigationApiV1 *(*XemNavigationApiGetterV1)(void);
#ifdef __cplusplus
}
#endif
#endif
