# Managed Unix Socket services

Manager >=0.3.0 provides **`xovi-sockets.h`, ABI 1** independently of manager-ui.
A native plugin registers its own named service; manager owns its listener,
connections and bounded asynchronous transport. The plugin owns its message
schema and business operations. No settings page, QMD or manifest is required.
The SDK is a header convenience, not a runtime dependency. External Python/Rust/
other programs use the socket protocol and need no SDK.

This API is not a public, catch-all administration endpoint. An external program
connects to a service registered by a loaded plugin; it cannot register arbitrary
native callbacks, install plugins or execute commands through a built-in RPC.
A plugin can explicitly implement those business operations when appropriate.

## Register an echo service

Use `managerApi` from [the discovery example](notification-events.md#discover-the-optional-native-api)
with `XEM_SOCKETS_METADATA` and `XEM_SOCKETS_ABI`:

```cpp
#include "xovi-sockets.h"
#include <QJsonDocument>
#include <QJsonObject>

static const XemSocketsApi *sockets;
static uint64_t service;

static void onSocketEvent(const char *json, void *) {
    const auto event = QJsonDocument::fromJson(json).object();
    if (event.value("type") == "resync") {
        char *snapshot = sockets->query(service);
        // Reconcile the current connection list with plugin-owned client state.
        sockets->freeString(snapshot);
        return;
    }
    if (event.value("type") != "message") return;
    const auto payload = QJsonDocument(event.value("message").toObject())
                             .toJson(QJsonDocument::Compact);
    char *reply = sockets->send(service, event.value("connectionId").toInteger(),
                               payload.constData());
    // Inspect reply.ok; "queued" means accepted by the local send buffer.
    sockets->freeString(reply);
}

// Run after QCoreApplication exists, on its thread, in xochitl.
void startService() {
    sockets = managerApi<XemSocketsApi, XemSocketsApiGetter>(
        XEM_SOCKETS_METADATA, XEM_SOCKETS_ABI);
    if (!sockets) return;
    char *raw = sockets->registerService(
        R"({"ownerId":"my-plugin","serviceId":"control"})", onSocketEvent, nullptr);
    const auto result = QJsonDocument::fromJson(raw ? raw : "{}").object();
    sockets->freeString(raw);
    if (!result.value("ok").toBool()) return; // Surface error; do not block startup.
    service = result.value("serviceHandle").toInteger();
    // Publish result.path to the plugin's external client/configuration.
}
void stopService() {
    if (!service) return;
    sockets->freeString(sockets->unregisterService(service));
    service = 0;
}
```

In real services validate messages, correlate requests with IDs, return explicit
results, and run expensive operations asynchronously. A send result of `queued`
is not proof of receipt or successful processing; define business acknowledgments
in your own protocol. Do not call `startService` twice without stopping first.

## External client

The returned path is `/tmp/xovi-em-<effective-uid>/<ownerId>.<serviceId>.sock`;
for root and the example it is `/tmp/xovi-em-0/my-plugin.control.sock`.
The directory is private (`0700`); the listener allows its owning user. IDs are
1–32 ASCII letters, digits, `_`, `-` or `.`, starting with a letter/digit/underscore.
Dots in IDs are allowed, but address collisions are rejected, never overwritten.
Use the returned path rather than inventing a different pathname.

```python
import json, socket
with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
    client.settimeout(3)
    client.connect('/tmp/xovi-em-0/my-plugin.control.sock')
    client.sendall(json.dumps({'id': 1, 'op': 'status'}).encode() + b'\n')
    with client.makefile('rb') as stream:
        result = json.loads(stream.readline(65538))
        print(result)  # The sample plugin echoes; real plugins define their response.
```

Framing: one UTF-8 JSON **object** per newline, at most 64 KiB excluding newline.
Partial frames and several frames per read are supported. Binary or bulk data
must use a plugin-defined reference/chunk protocol within those bounds. Invalid
JSON and oversized frames disconnect that client; incomplete frames time out
after 15 seconds. Same-UID filesystem permissions are not per-plugin authorization;
the service must enforce any additional policy its operations need.

## API, events and lifetime

All API calls except `freeString`, and all callbacks, run on the Qt application
thread. Off-thread calls return `application-thread-required`. Registration outside
the `xochitl` executable returns `wrong-process`, including `xochitl_pdf_renderer`.
No listener is created merely by loading manager. The restriction covers this API;
it does not itself change XOVI's process injection policy.

| Call | Result |
| --- | --- |
| `registerService(description, callback, userData)` | `{ok,serviceHandle,path,connections,state,protocol}` |
| `query(handle)` | Current service/connection snapshot; `query(0)` lists services |
| `send(handle, connectionId, jsonObject)` | `{ok:true,status:"queued"}`, or error |
| `disconnectClient(handle, connectionId)` | Disconnect one client |
| `unregisterService(handle)` | Close listener/clients, remove socket, cancel queued callbacks |

Events include `serviceHandle`:

```json
{"type":"connected","serviceHandle":1,"connectionId":2}
{"type":"message","serviceHandle":1,"connectionId":2,"message":{"id":1,"op":"status"}}
{"type":"disconnected","serviceHandle":1,"connectionId":2}
{"type":"resync","serviceHandle":1}
```

Callbacks are queued and may call the API. Event JSON is borrowed; free returned
API JSON with `freeString`. Unregister before freeing callback data or unloading
plugin code. A current callback may finish; later queued callbacks are suppressed.
Connections and handles are session-local. Unregistering is explicit; manager
cannot infer arbitrary native controller destruction. Clients reconnect and query
business state after xochitl restarts; requests are not automatically replayed.

Limits: 32 services, 16 clients per service, 64 total clients, 128 queued regular
events, 128 KiB outstanding output per client. Manager caps reads and callback
batches; it never uses blocking client I/O or `waitFor*`. Slow readers and input
queue overflow disconnect the affected client. Under overflow lifecycle events
may coalesce into `resync`; query connections to reconcile instead of assuming
every disconnect event was delivered. Excess connections are rejected without
creating plugin callbacks. A plugin's own blocking callback can still block xochitl.

Address collisions fail with `socket-in-use`; manager preserves foreign listeners
and regular files. A same-user stale socket is removed only after acquiring the
service lock and verifying there is no live listener. Directory ownership/type/
permissions are checked without following a final symlink. Startup failures return
an error and leave xochitl running; the caller decides whether/when to retry.

| Error | Handling |
| --- | --- |
| `invalid-request`, `invalid-service-id`, `invalid-message`, `socket-path-too-long` | Correct the request; no restart remedy |
| `application-thread-required`, `wrong-process`, `service-already-registered` | Fix caller lifecycle/thread/process |
| `service-not-found`, `connection-not-found` | Drop stale handles and query state |
| `service-limit` | Release unused services |
| `socket-in-use` | Choose a different service ID or stop its existing owner |
| `unsafe-runtime-directory`, `unsafe-socket-path`, `runtime-directory-unavailable`, `socket-path-unavailable`, `stale-socket-cleanup-failed` | Inspect ownership, permissions and conflicting paths |
| `socket-unavailable`, `listen-failed`, `socket-write-failed`, `slow-client` | Check resources/client, reconnect and query; do not assume business success |

These responses include manager's structured diagnostic classification. They do
not mark a plugin as needing restart. No service registration writes activation
state or waits for an external program during startup.
