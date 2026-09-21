# Native settings provider: minimal adaptation

Use this path when your native plugin should expose settings without first
creating a QML registration object. For an existing QMD/QML page,
[QML registration requires no SDK](https://github.com/Zen-Ink/xovi-extension-manager-ui/blob/master/docs/integration.md#1-qmlqmd-no-sdk).
The native example uses only a header, not shared controls or translation helpers.

## 1. Add the header dependency

```sh
git submodule add https://github.com/Zen-Ink/xovi-extension-manager-sdk.git sdk
git submodule update --init --recursive
```

For qmake:

```qmake
INCLUDEPATH += $$PWD/sdk
```

For a Makefile, add `-I$(CURDIR)/sdk` to your compiler flags.

## 2. Export your existing page

Assume the plugin already registers `Settings.qml` at
`qrc:/my-plugin/Settings.qml` through its Qt resources:

```cpp
#include "xovi-settings.h"

static const char pageUrl[] = "qrc:/my-plugin/Settings.qml";
static const XemSettingsProviderV1 page = {
    XEM_SETTINGS_ABI, sizeof(XemSettingsProviderV1),
    "main", "My plugin",
    0, pageUrl, sizeof(pageUrl) - 1, nullptr
};

extern "C" const XemSettingsProviderV1 *my_settings_provider_v1()
{
    return &page;
}
```

Add to the plugin's `.xovi` definition, then regenerate its bindings using its
normal build script:

```text
export my_settings_provider_v1
with
    xovi-extension-manager$settingsProvider = 1
end
```

No import of manager or manager-ui is needed. When manager is absent, the
plugin continues to work; keep its existing entry if it needs standalone access.
The owner ID comes from the native plugin's XOVI runtime identity.

The getter must return immutable process-lifetime data and be thread safe and
side-effect free: no Qt object creation, I/O or callbacks into manager.
A `.xovi resource` embeds bytes but does **not** register a Qt `qrc:` URL. Keep
using your existing Qt RCC registration, or use the documented inline-source ABI.

## 3. Optional icon, translated title and default PIN

```cpp
static const XemSettingsPresentationV1 presentation = {
    XEM_SETTINGS_ABI, sizeof(XemSettingsPresentationV1),
    "main", "My plugin", "MySettings", "qrc:/my-plugin/icon.svg"
};
static const XemLauncherDefaultsV1 defaults = {
    XEM_SETTINGS_ABI, sizeof(XemLauncherDefaultsV1),
    "main", XEM_LAUNCHER_SETTINGS
};
extern "C" const XemSettingsPresentationV1 *my_settings_presentation_v1()
{
    return &presentation;
}
extern "C" const XemLauncherDefaultsV1 *my_launcher_defaults_v1()
{
    return &defaults;
}
```

```text
export my_settings_presentation_v1
with
    xovi-extension-manager$settingsPresentation = 1
end
export my_launcher_defaults_v1
with
    xovi-extension-manager$launcherDefaults = 1
end
```

The plugin owns the icon and translations (`MySettings` is the Qt translation
context). Default PINs apply on first discovery; saved user choices take priority.
Without these optional exports the page is still discoverable.

## Optional native services

Use `xovi-notifications.h` for background progress and notification actions;
use `xovi-navigation.h` to request another plugin's settings. Discover the optional
API through XOVI metadata, check its version and size, and handle an absent service.
Queued navigation is not confirmation that a page rendered; a notification being
stored is not confirmation that it was displayed.

See [navigation examples](https://github.com/Zen-Ink/xovi-extension-manager/blob/master/docs/settings-navigation-api.md#native-c-api)
and [notifications](https://github.com/Zen-Ink/xovi-extension-manager/blob/master/docs/launchers-and-notifications.md)
for the full contracts. The page can keep its own controls, translations and
settings storage. Discovery does not validate its QML until the page is opened.
