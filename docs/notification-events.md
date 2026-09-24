# Notifications: subscribe, then query

Manager **0.3.0** exposes notification ABI **3**. This intentionally replaces
V1/V2: there is no old getter, `pollActions`, broker `notificationsPollActions`
or QML `takeNotificationActions()`. Rebuild native notification consumers with
`xovi-notifications.h`; update manager and manager-ui together. The SDK remains
optional, and notifications do not require a settings manifest.

## Discover the optional native API

Include the plugin's generated `xovi.h` and `xovi-notifications.h`. This helper
also works for the Socket API:

```cpp
#include <cstring>

template<class Api, class Getter>
const Api *managerApi(const char *metadata, int abi) {
    if (!Environment || !Environment->createMetadataSearchingIterator ||
        !Environment->nextFunctionMetadataEntry || !Environment->getExtensionLoadState)
        return nullptr;
    ExtensionMetadataIterator it{};
    Environment->createMetadataSearchingIterator(&it, metadata);
    while (auto *m = Environment->nextFunctionMetadataEntry(&it)) {
        if (!it.extensionName || std::strcmp(it.extensionName, "xovi-extension-manager") ||
            m->type != METADATA_TYPE_INT || m->value.i != abi || !it.functionAddress ||
            Environment->getExtensionLoadState(it.extensionName) != XOVI_EXTENSION_INITIALIZED)
            continue;
        auto *api = reinterpret_cast<Getter>(it.functionAddress)();
        if (api && api->abiVersion == abi && api->structSize >= sizeof(Api)) return api;
    }
    return nullptr;
}
auto *api = managerApi<XemNotificationsApi, XemNotificationsApiGetter>(
    XEM_NOTIFICATIONS_METADATA, XEM_NOTIFICATIONS_ABI);
```

Treat `nullptr` as service unavailable. Discovery needs no mandatory import or
manifest. If notification support is mandatory, require manager >=0.3.0 in the
plugin's normal dependency declaration. Never cast an older table to the new one.

## Subscribe before reading a snapshot

Call `subscribe`/`unsubscribe` on the Qt application thread, after that thread's
application object exists. Posting/querying/acknowledging is thread-safe.
Callbacks are queued on the application thread, outside store locks.

```cpp
// `api` has already been checked; `controller` outlives the subscription.
auto subscription = api->subscribe("my-plugin", onNotificationEvent, controller);
if (!subscription) { /* report unavailable/duplicate owner subscription */ }
char *snapshot = api->query(R"({"ownerId":"my-plugin"})");
// Parse/copy snapshot, including revision, entries and action states.
api->freeString(snapshot);
// Before destroying controller or unloading callback code:
api->unsubscribe(subscription);
```

The callback has signature `void(const char *eventJson, void *userData)`; its JSON
pointer is valid only for that callback. Each returned string, including errors,
must be freed using `api->freeString`. No caller callback is invoked inline from
post/subscribe/acknowledge. Unsubscribe suppresses queued callbacks; a callback
already running may finish.

Events:

```json
{"type":"changed","revision":12}
{"type":"action","revision":13,"action":{"ownerId":"my-plugin","notificationId":"import","actionId":"cancel","sequence":4,"actionSequence":1,"status":"delivered"}}
```

On `changed`, query `{"ownerId":"my-plugin","sinceRevision":12}` using the last
**snapshot** revision. Equal revisions return `unchanged:true`. Change events may
coalesce; use them as invalidation hints, not as an audit log. Subscribe before
querying to close the snapshot/subscription race. `ownerId:""` subscribes only to
change hints; a nonempty owner additionally receives its actions. There is one
action subscription per owner and at most 128 total subscriptions. Duplicate or
invalid subscriptions return zero. Query does not consume anything.

## Post and acknowledge

```cpp
char *reply = api->post(R"({
  "ownerId":"my-plugin", "notificationId":"import",
  "title":"Importing", "message":"Preparing documents", "state":"running",
  "progress":{"value":0}, "actions":[{"id":"cancel","label":"Cancel"}]
})");
api->freeString(reply);
```

A successful post or button invocation means **queued**, not displayed or done.
After an `action` event, start plugin-owned asynchronous work. Only when it
finishes call `acknowledge` with that event's `actionSequence`:

```cpp
char *reply = api->acknowledge(R"({
  "ownerId":"my-plugin", "actionSequence":1,
  "status":"completed", "result":{"cancelled":true}
})");
api->freeString(reply);
```

Use `status:"failed"` and a result explaining failure when appropriate. Query
returns a bounded action journal: `queued → delivered → completed|failed`.
The button stays pending through delivery and re-enables on acknowledgment.
Acknowledgments with the same final status are idempotent; conflicting final states are rejected.
The manager never infers business success from delivery and never blindly replays
a delivered action. After reconnecting/reopening, query delivered actions and
reconcile them with the plugin's own operation state before acknowledging.

Dismissal, eviction, clearing, ordinary notification replacement and retiring an
action invalidate its journal records. A late acknowledgment then returns
`action-not-found`: it must not cancel or repeat business work. Acknowledge before
posting a terminal notification update that retires actions. No consumer leaves
an invocation queued; no acknowledgment leaves it delivered, never completed.
Keep long-lived native subscriptions on the task controller, not a settings page.

The store and journal each hold at most 100 items; finished journal records are
evicted first. An all-pending journal rejects new actions with `action-queue-full`.
Notifications survive page closure, but not xochitl restart. All native plugins
share the process: owner IDs are routing identifiers, not a security boundary.

## QML page example (no SDK required)

```qml
Component.onCompleted: {
    settingsContext.notificationActionsEnabled = true
    var snapshot = settingsContext.notificationState()
    // Reconcile any delivered actions with this plugin's backend.
}
Connections {
    target: settingsContext
    function onNotificationStateChanged() {
        var snapshot = settingsContext.notificationState()
    }
    function onNotificationAction(action) {
        if (action.actionId !== "cancel") {
            settingsContext.completeNotificationAction(action.actionSequence, false,
                                                       {error:"unsupported-action"})
            return
        }
        // Example plugin controller; its completion callback is asynchronous.
        importController.cancelAsync(function(ok) {
            settingsContext.completeNotificationAction(action.actionSequence, ok)
        })
    }
}
```

The page/controller must stay alive until that callback finishes. For tasks that
outlive the page, let a native controller own the subscription and acknowledgment.
Posting actions enables the page subscription automatically; a failed subscription
is exposed as the context's error. Destroying the context unsubscribes it.

The UI subscribes once and queries after changes: there is no notification timer
or action polling. Short prompts display for five seconds in FIFO order, coalescing
updates to the same key. Progress-only patches update the current view without
restarting the prompt. Queued prompts become visible after earlier prompts;
event delivery still depends on the xochitl event loop being responsive.
