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

The shared language service follows the session property `xoviNativeUiLanguage`
set by manager-ui's native localization startup and Language Settings adapters. Runtime language changes
take precedence over the configuration file, which may not yet be saved. Files
remain the startup fallback when no native language has been observed. New QML
engines never become the language authority. Rebuild consumers together when
updating this header-only helper. Plugins retain their own translation catalogs.

The native runtime value wins even if `xochitl.conf` still contains another
language. Do not use `LANG`, keyboard `InputLocale`, or a newly created QML
engine's default English as the native UI language. Manager-ui observes the
native model at localization startup as well as in language selectors, so users
do not need to open Language settings first. Without that adapter, the SDK can
only fall back to configuration/environment; it cannot infer an unobserved native
runtime value. AppLoad's separate environment-first selector is not used by this
helper. Each plugin continues to own its own translation catalogs.
