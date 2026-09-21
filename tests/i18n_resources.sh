#!/usr/bin/env bash
set -euo pipefail
root=${XOVI_WORKSPACE:-$(cd "$(dirname "$0")/fixtures" && pwd)}
output=$1
mkdir -p "$output"
printf '<RCC>\n' > "$output/translations.qrc"
for pair in 'xovi-extension-manager-ui:xovi-extension-manager-ui' 'advanced_settings:advanced_settings' 'keyboardcjk:keyboardcjk' 'epub-preloader:epub-preloader' 'rm-librarian:rm-librarian' 'rmfakecloud-control:xovi-rmfakecloud-plugin/plugin'; do
    id=${pair%%:*}
    directory=${pair#*:}
    printf '<qresource prefix="/xovi/i18n/%s">\n' "$id" >> "$output/translations.qrc"
    for language in en zh_CN zh_TW; do
        name="${id}_${language}.qm"
        /usr/lib/qt6/bin/lrelease "$root/$directory/translations/${id}_${language}.ts" -qm "$output/$name" > /dev/null
        printf '<file>%s</file>\n' "$name" >> "$output/translations.qrc"
    done
    printf '</qresource>\n' >> "$output/translations.qrc"
done
printf '</RCC>\n' >> "$output/translations.qrc"
"$(pkg-config --variable=libexecdir Qt6Core)/rcc" -name translations "$output/translations.qrc" -o "$output/translations.cpp"
