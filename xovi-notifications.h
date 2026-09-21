#ifndef XOVI_NOTIFICATIONS_H
#define XOVI_NOTIFICATIONS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XEM_NOTIFICATIONS_ABI 1
#define XEM_NOTIFICATIONS_METADATA "xovi-extension-manager$notificationsApi"
#define XEM_NOTIFICATIONS_V2_ABI 2
#define XEM_NOTIFICATIONS_V2_METADATA "xovi-extension-manager$notificationsApiV2"

/* Optional manager API. Discover the getter through XOVI metadata; do not
 * import the manager. post accepts a UTF-8 JSON notification description and
 * returns a malloc-owned UTF-8 JSON response. A successful post means queued
 * in the manager's in-memory store, not that a UI has displayed it. dismiss
 * and freeString have the same ownership rules. All function pointers and the
 * table remain valid until process exit. */
typedef struct XemNotificationsApiV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*post)(const char *description);
    char *(*dismiss)(const char *ownerId, const char *notificationId);
    void (*freeString)(char *value);
} XemNotificationsApiV1;

typedef const XemNotificationsApiV1 *(*XemNotificationsApiGetterV1)(void);

/* Version 2 extends the V1 prefix with action delivery. Discover it through
 * XEM_NOTIFICATIONS_V2_METADATA. pollActions atomically takes pending action
 * requests for one owner; it never invokes plugin code. Returned strings use
 * freeString and all table pointers remain valid until process exit. */
typedef struct XemNotificationsApiV2 {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*post)(const char *description);
    char *(*dismiss)(const char *ownerId, const char *notificationId);
    void (*freeString)(char *value);
    char *(*pollActions)(const char *ownerId);
} XemNotificationsApiV2;

typedef const XemNotificationsApiV2 *(*XemNotificationsApiGetterV2)(void);

#ifdef __cplusplus
}
#endif

#endif
