#ifndef XOVI_NOTIFICATIONS_H
#define XOVI_NOTIFICATIONS_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define XEM_NOTIFICATIONS_ABI 3
#define XEM_NOTIFICATIONS_METADATA "xovi-extension-manager$notificationsApi"
/* Breaking ABI: no V1/V2 tables or pollActions. All returned strings are owned
 * by the caller and must be released with freeString. Callback JSON is borrowed
 * until the callback returns. post/dismiss/query/acknowledge are thread-safe.
 * subscribe/unsubscribe MUST run on the QCoreApplication thread; callbacks run
 * queued on that thread, without store locks. Keep callbacks short.
 * ownerId="" subscribes to store changes only; a nonempty owner also receives
 * its actions. Only one action subscription per owner is permitted. Subscribe
 * before querying a snapshot; compare revision to reconcile races. unsubscribe
 * prevents future callbacks, including queued ones (the current call may finish).
 * Unsubscribe before destroying userData or unloading callback code.
 */
typedef void (*XemNotificationCallback)(const char *eventJson, void *userData);
typedef struct XemNotificationsApi {
    uint32_t abiVersion;
    uint32_t structSize;
    char *(*post)(const char *description);
    char *(*dismiss)(const char *ownerId, const char *notificationId);
    char *(*query)(const char *requestJson);
    uint64_t (*subscribe)(const char *ownerId, XemNotificationCallback callback, void *userData);
    void (*unsubscribe)(uint64_t subscription);
    char *(*acknowledge)(const char *requestJson);
    void (*freeString)(char *value);
} XemNotificationsApi;
typedef const XemNotificationsApi *(*XemNotificationsApiGetter)(void);
#ifdef __cplusplus
}
#endif
#endif
