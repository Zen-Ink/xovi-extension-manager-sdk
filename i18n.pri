# Optional Qt build helper. Catalogs and translation ownership stay in each plugin.
isEmpty(XOVI_TRANSLATION_ID): error("Set XOVI_TRANSLATION_ID before including i18n.pri")
isEmpty(XOVI_TRANSLATION_DIR): error("Set XOVI_TRANSLATION_DIR before including i18n.pri")
CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /xovi/i18n/$$XOVI_TRANSLATION_ID
TRANSLATIONS += $${XOVI_TRANSLATION_DIR}/$${XOVI_TRANSLATION_ID}_en.ts \
                $${XOVI_TRANSLATION_DIR}/$${XOVI_TRANSLATION_ID}_zh_CN.ts \
                $${XOVI_TRANSLATION_DIR}/$${XOVI_TRANSLATION_ID}_zh_TW.ts
INCLUDEPATH += $$PWD
HEADERS += $$PWD/xovi-i18n.h
# RCC emits identical exported initializer names for qmake_qmake_qm_files in
# different plugins. Bind definitions locally so one plugin cannot initialize
# another plugin's resource table instead of its own under RTLD_GLOBAL.
unix: QMAKE_LFLAGS += -Wl,-Bsymbolic-functions
