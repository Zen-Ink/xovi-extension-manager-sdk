#ifndef XOVI_SETTINGS_H
#define XOVI_SETTINGS_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define XEM_SETTINGS_ABI 1
#define XEM_SETTINGS_METADATA "xovi-extension-manager$settingsProvider"
/* Export a getter tagged with XEM_SETTINGS_METADATA = 1. No manager import.
 * All pointers remain valid until process exit. Getters are side-effect free.
 * source is either a URL (sourceKind=0) or UTF-8 QML (sourceKind=1).
 * sourceSize is the exact byte count (excluding NUL). baseUrl is required for
 * inline QML and must be unique to the provider. The manager copies the data.
 * Provider functions may be queried from a broker thread; do not touch Qt UI.
 */
typedef struct XemSettingsProviderV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    const char *pageId;
    const char *title;
    uint32_t sourceKind;
    const char *source;
    size_t sourceSize;
    const char *baseUrl;
} XemSettingsProviderV1;
typedef const XemSettingsProviderV1 *(*XemSettingsProviderGetterV1)(void);
#define XEM_PRESENTATION_METADATA "xovi-extension-manager$settingsPresentation"
/* Optional presentation for a settings page, independent of pin locations.
 * Strings are immutable process-lifetime storage. title is a translation key;
 * translationContext names the Qt catalog context, iconSource is a qrc URL.
 */
typedef struct XemSettingsPresentationV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    const char *pageId;
    const char *title;
    const char *translationContext;
    const char *iconSource;
} XemSettingsPresentationV1;
typedef const XemSettingsPresentationV1 *(*XemSettingsPresentationGetterV1)(void);
/* V2 is optional. The V1 prefix preserves title/icon metadata; ABI/version and
 * metadata value are 2. PAGE means a self-contained screen: the host reserves
 * no header, footer or margins. The page must provide settingsContext.close(). */
#define XEM_CHROME_HOST 0u
#define XEM_CHROME_PAGE 1u
typedef struct XemSettingsPresentationV2 {
    XemSettingsPresentationV1 base;
    uint32_t chromeMode;
} XemSettingsPresentationV2;
typedef const XemSettingsPresentationV2 *(*XemSettingsPresentationGetterV2)(void);
/* Optional first-discovery defaults. User choices take precedence and are
 * retained across upgrades and disable/re-enable. No UI injection is needed. */
#define XEM_LAUNCHER_DEFAULTS_METADATA "xovi-extension-manager$launcherDefaults"
#define XEM_LAUNCHER_SIDEBAR 1u
#define XEM_LAUNCHER_BOTTOM 2u
#define XEM_LAUNCHER_SETTINGS 4u
#define XEM_LAUNCHER_QUICK 8u
typedef struct XemLauncherDefaultsV1 {
    uint32_t abiVersion;
    uint32_t structSize;
    const char *pageId;
    uint32_t locations;
} XemLauncherDefaultsV1;
typedef const XemLauncherDefaultsV1 *(*XemLauncherDefaultsGetterV1)(void);
#ifdef __cplusplus
}
#endif
#endif
