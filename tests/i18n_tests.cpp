#include "../xovi-i18n.h"
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlApplicationEngine>
#include <QTemporaryDir>
#include <QSaveFile>
#include <QEventLoop>
#include <memory>
#include <cstdio>
#include <cstdlib>
void check(bool value,const char *message){if(!value){std::fprintf(stderr,"FAIL %s\n",message);std::exit(1);}std::printf("PASS %s\n",message);}
static void settle() { QEventLoop loop; QTimer::singleShot(100,&loop,&QEventLoop::quit); loop.exec(); }
static void language(const QString &code) {
    QSaveFile file(XoviI18n::languageSettingsPath());
    check(file.open(QIODevice::WriteOnly),"open native language fixture");
    file.write(("[General]\nConversionLanguage=ignored\nLanguage="+code+"\n").toUtf8());
    check(file.commit(),"atomically replace native language fixture");
    settle();
}
int main(int argc,char **argv){
    QGuiApplication app(argc,argv);
    QTemporaryDir config;
    qputenv("XOVI_LANGUAGE_SETTINGS",(config.path()+"/xochitl.conf").toUtf8());
    language("en");
    std::unique_ptr<QQmlEngine> owner(argc>1 ? new QQmlApplicationEngine : new QQmlEngine);
    auto &engine=*owner;
    for (const auto &id:{"xovi-extension-manager-ui","advanced_settings","keyboardcjk","epub-preloader"}) {
        XoviI18n::prepareCatalog(id); XoviI18n::attach(&engine,id);
    }
    check(QCoreApplication::translate("ManagerSettings","键盘设置")=="Keyboard settings","Keyboard CJK catalog available before opening its page");
    auto probe=engine.newObject();probe.setProperty("evaluations",0);
    engine.globalObject().setProperty("translationProbe",probe);
    QQmlComponent component(&engine);
    component.setData(R"(import QtQml
QtObject {
    property string heading: { translationProbe.evaluations++; return qsTranslate("advanced_settings", "Control Center") }
    property string pin: qsTranslate("Manager", "Pin to sidebar")
    property string keyboard: qsTranslate("ManagerSettings", "键盘设置")
})",QUrl("qrc:/i18n-test.qml"));
    QScopedPointer<QObject> page(component.create());check(bool(page),"translation fixture loaded");
    const int initial=probe.property("evaluations").toInt();
    settle();
    for(int i=0;i<8;++i) {
        XoviI18n::attach(&engine,"keyboardcjk");
        QEvent event(QEvent::LanguageChange);QCoreApplication::sendEvent(&app,&event);
    }
    settle();
    check(probe.property("evaluations").toInt()==initial,"startup and repeated attachment do not retranslate host bindings");
    language("zh_CN");
    check(page->property("pin")==QString::fromUtf8("固定到侧边栏"),"native config change translates manager");
    check(page->property("heading")==QString::fromUtf8("控制中心"),"native config change translates Advanced Settings");
    check(page->property("keyboard")==QString::fromUtf8("键盘设置"),"native config change translates Keyboard CJK");
    check(probe.property("evaluations").toInt()==initial+1,"multiple catalog changes produce one engine refresh");
    for(int i=0;i<4;++i) {
        QQmlEngine other; other.setUiLanguage("en");
        XoviI18n::attach(&other,"advanced_settings");XoviI18n::attach(&other,"keyboardcjk");
        XoviI18n::attach(&other,"xovi-extension-manager-ui");settle();
        check(QCoreApplication::translate("advanced_settings","Control Center")==QString::fromUtf8("控制中心"),"alternate entry/new English engine cannot replace native Chinese language");
    }
    engine.setUiLanguage("en");settle();
    check(page->property("pin")==QString::fromUtf8("固定到侧边栏"),"opening and closing pages preserves translations");
    QFile::remove(XoviI18n::languageSettingsPath());settle();
    check(page->property("pin")==QString::fromUtf8("固定到侧边栏"),"temporary missing config does not reset session to English");
    for (const auto &locale:{"zh_TW","zh-HK","zh-Hant"}) {
        language(locale);
        check(page->property("pin")==QString::fromUtf8("固定到側邊欄"),"Traditional Chinese region/script mapping");
    }
    language("de_DE");check(page->property("keyboard")=="Keyboard settings","unsupported language uses English catalog");
    check(app.findChildren<QObject *>("xoviLanguageServiceV2",Qt::FindDirectChildrenOnly).size()==1,"one language owner serves every plugin and engine");
    check(QCoreApplication::translate("UnrelatedApplication","Pin to sidebar")=="Pin to sidebar","unrelated translation contexts are untouched");
}
