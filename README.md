# Xovi Extension Manager SDK

Optional C API headers, QML controls and translation helpers for extension
settings, navigation and session notifications. This is a build-time toolkit,
not an additional runtime plugin.

## Choose an integration path

**The SDK is optional.** A native plugin can use the QML/QMD path too; being a
native plugin does not require using the native settings API. Neither path below
requires a manifest or a JSON form schema. Keep your existing settings page,
backend, storage and translation catalogs.

| | QML API, optionally injected by QMD | Native provider API |
|---|---|---|
| Best fit | Reuse an existing QML page or QMD | Discover settings independently of a QML object's lifetime |
| SDK dependency | None | Use the small C ABI header; shared controls/i18n helpers are optional |
| Page and PIN support | `registerPage()`; `openPage()` alone does not create a PIN entry | Export a settings provider; optional presentation/default PIN metadata |
| Discovery | After registration code runs; its owner must stay alive | From an initialized native plugin's XOVI exports |
| Main tradeoff | Any added firmware QMD injection point needs version maintenance | Requires native compilation and compliance with the ABI/lifetime rules |
| Background work | Needs an available QML bridge to call UI services | Optional notification/navigation APIs can be called from native code without creating a settings page |

Native registration avoids adding a per-plugin firmware injection just for the
entry. Manager UI itself still needs firmware compatibility. Both paths load the
page on demand: registration/discovery does **not** prove the QML can render.
The host reports page creation errors, but cannot prevent arbitrary synchronous
plugin code from blocking the GUI thread.

## Minimal examples

- **No SDK:** [QML/QMD integration](https://github.com/Zen-Ink/xovi-extension-manager-ui/blob/master/docs/integration.md#1-qmlqmd-no-sdk) — open an existing page, or register it for the list and PINs.
- **Native:** [Native provider integration](docs/native-integration.md) — export a small descriptor from your existing plugin.

## Optional pieces

Use only what your plugin needs:

- `xovi-settings.h`: native settings provider descriptors.
- `xovi-navigation.h`: optional native API for opening another plugin's settings.
- `xovi-notifications.h`: optional notifications, progress and action delivery.
- `qrr-api.h`: qt-resource-rebuilder injection feedback; not proof of successful QML rendering.
- `xovi_controls.qrc`: shared QML controls; embed only if your page imports them.
- `i18n.pri` / `xovi-i18n.h`: optional translation helpers. Catalogs belong to each plugin.

For optional controls, add `RESOURCES += $$PWD/sdk/xovi_controls.qrc` to qmake.
For translation helpers, set `XOVI_TRANSLATION_ID` and `XOVI_TRANSLATION_DIR`
before `include($$PWD/sdk/i18n.pri)`. Neither is needed for the native example.
Embedded controls remain available when manager-ui is disabled.

## Tests

`tests/run-i18n-tests.sh` runs Qt 6 host tests using checked-in fixtures.
Catalog checks are workspace integration tests; set `XOVI_WORKSPACE` to the
rm-xovi-extensions checkout.

## Live language changes

The shared language service uses only the in-process `xoviNativeUiLanguage`
value. Manager-ui observes successful installation of xochitl's native
`reMarkable_*.qm` translator and reads `QTranslator::language()` from that
already-loaded object. Plugin translators are ignored by this observer.
No language settings files, file watchers, environment locale or new QML
engine defaults select the session language.

Before native language is observed, `currentLanguage()` is empty and the
application property `xoviUiLanguageReady` is false. This is an unsynchronized
state; source strings remain visible until the native value arrives. On the
first value and subsequent changes, catalogs reload and attached engines
retranslate. English catalogs remain the translation fallback for unsupported
native languages, not a guess about the native selection.

Plugins retain their own catalogs. The helper's old V2 watcher/list fields are
reserved for cross-DSO layout compatibility and are never populated or used.
Rebuild SDK consumers together to remove old file-reading implementations;
manager-ui must also be updated to supply the native runtime value. Without a
runtime observer, the SDK stays unsynchronized rather than guessing a locale.

## Text weight

Ordinary labels and buttons use `Typography.weight` (`Font.Normal`). Do not set
local `font.bold`, font names or literal font sizes on settings controls. For a
semantic emphasis such as an unread notification title, use:

```qml
ELabel { text: qsTr("Import completed"); emphasized: !notification.read }
```

`SettingsTitle` and the explicit Strong variants use `Typography.strong`.
Document and wallpaper content may retain the user's own typography settings.
