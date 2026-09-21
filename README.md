# Xovi Extension Manager SDK

Headers and embedded QML controls for extension settings, navigation and session notifications.

Add this repository as a `sdk` submodule and initialize dependencies with
`git submodule update --init --recursive`.

For qmake projects, add `INCLUDEPATH += $$PWD/sdk`,
`RESOURCES += $$PWD/sdk/xovi_controls.qrc`, and include `sdk/i18n.pri`
after setting `XOVI_TRANSLATION_ID` and `XOVI_TRANSLATION_DIR`.
Translation catalogs belong to each plugin. Controls are embedded in each consumer;
they do not require the manager UI to be enabled.

Native API headers: `xovi-settings.h`, `xovi-navigation.h`, and
`xovi-notifications.h`. Resolve optional APIs at runtime and handle unavailable
providers. `qrr-api.h` describes injection feedback from qt-resource-rebuilder.

Integration guides: https://github.com/Zen-Ink/xovi-extension-manager/tree/master/docs

`tests/run-i18n-tests.sh` runs Qt 6 host tests using small checked-in translation fixtures. The catalog checks are workspace
integration tests; set `XOVI_WORKSPACE` to the rm-xovi-extensions checkout.
