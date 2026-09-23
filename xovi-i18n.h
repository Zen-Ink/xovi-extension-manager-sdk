#pragma once
#include <QCoreApplication>
#include <QQmlEngine>
#include <QTranslator>
#include <QLocale>
#include <QTimer>
#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QFileSystemWatcher>
#include <QHash>
#include <QPointer>
#include <functional>

namespace XoviI18n {
class LanguageObserver : public QObject {
public:
    LanguageObserver(QObject *parent, std::function<void()> update):QObject(parent),update_(std::move(update)) {}
    bool eventFilter(QObject *, QEvent *event) override {
        const bool sessionChanged=event->type()==QEvent::DynamicPropertyChange
            && (static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName()=="xoviUiLanguage"
                || static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName()=="xoviNativeUiLanguage");
        if ((event->type()==QEvent::LanguageChange || event->type()==QEvent::LocaleChange || sessionChanged) && !pending_) {
            pending_=true;
            QTimer::singleShot(0,this,[this]() { pending_=false; update_(); });
        }
        return false;
    }
private:
    std::function<void()> update_;
    bool pending_=false;
};
inline QString catalogLanguage(const QString &requested) {
    if (requested.trimmed().isEmpty()) return {};
    const QLocale locale(requested);
    if(locale.language()!=QLocale::Chinese) return QStringLiteral("en");
    return locale.script()==QLocale::TraditionalHanScript ? QStringLiteral("zh_TW") : QStringLiteral("zh_CN");
}
// One process-wide owner, shared through the application object rather than
// header-local statics in separate DSOs. Catalogs stay independent and live for
// the session, never for the lifetime of a settings page or a QML engine.
class LanguageService : public QObject {
public:
    explicit LanguageService(QObject *app):QObject(app),watcher_(this) {
        setObjectName("xoviLanguageServiceV2");
        app->installEventFilter(new LanguageObserver(this,[this]() { synchronize(); }));
        synchronize();
    }
    QString language() const { return language_; }
    void addCatalog(const QString &id) {
        synchronize();
        if (!catalogs_.contains(id)) catalogs_.insert(id,nullptr);
        if (!catalogs_.value(id)) loadCatalog(id);
    }
    QString translate(const QString &id, const QString &context, const QString &source) {
        // Native runtime events own refresh; label lookup does not choose a locale.
        if (!catalogs_.contains(id)) addCatalog(id);
        const auto *catalog=catalogs_.value(id);
        const auto translated=catalog ? catalog->translate(context.toUtf8().constData(),source.toUtf8().constData()) : QString();
        return translated.isNull() ? source : translated;
    }
    void attachEngine(QQmlEngine *engine) {
        if (!engine) return;
        for (const auto &existing:engines_) if (existing==engine) return;
        engines_.append(engine);
        // Engine events can refresh the native session value, never replace it
        // with a new engine's default locale.
        QObject::connect(engine,&QQmlEngine::uiLanguageChanged,this,[this]() { schedule(); });
    }
private:
    QString resolve() const {
        return catalogLanguage(QCoreApplication::instance()->property("xoviNativeUiLanguage").toString());
    }
    void schedule() {
        if (pending_) return;
        pending_=true;
        QTimer::singleShot(0,this,[this]() { pending_=false; synchronize(); });
    }
    void loadCatalog(const QString &id) {
        if (language_.isEmpty()) return; // Native language has not been observed yet.
        auto *old=catalogs_.value(id);
        if (old && old->property("language").toString()==language_) return;
        auto *next=new QTranslator(this);
        const auto path=QStringLiteral(":/xovi/i18n/")+id+"/"+id+"_"+language_+".qm";
        if (!next->load(path)) {
            qWarning("[xovi-i18n] catalog unavailable: %s",qPrintable(path));
            delete next;
            return; // Keep a working previous translation if loading fails.
        }
        next->setObjectName("xoviUiTranslator:"+id);
        next->setProperty("language",language_);
        QCoreApplication::installTranslator(next);
        catalogs_[id]=next;
        if (old) { QCoreApplication::removeTranslator(old); delete old; }
    }
    void synchronize() {
        const auto next=resolve();
        const bool changed=next!=language_;
        language_=next;
        QCoreApplication::instance()->setProperty("xoviUiLanguage",language_);
        QCoreApplication::instance()->setProperty("xoviUiLanguageReady",!language_.isEmpty());
        for (const auto &id:catalogs_.keys()) loadCatalog(id);
        if (!changed) return;
        for (const auto &engine:engines_) {
            if (!engine || engine->property("xoviRetranslatePending").toBool()) continue;
            engine->setProperty("xoviRetranslatePending",true);
            QTimer::singleShot(0,engine,[engine]() {
                if (!engine) return;
                engine->setProperty("xoviRetranslatePending",false);
                engine->retranslate();
            });
        }
    }
    // Retain the V2 member layout across header-only consumers. These fields
    // are inert: no paths are populated, watched or read.
    QFileSystemWatcher watcher_;
    QStringList paths_;
    QString language_;
    QHash<QString,QTranslator *> catalogs_;
    QList<QPointer<QQmlEngine>> engines_;
    bool pending_=false;
};
inline LanguageService *service() {
    auto *app=QCoreApplication::instance();
    if (!app) return nullptr;
    auto *existing=app->findChild<QObject *>("xoviLanguageServiceV2",Qt::FindDirectChildrenOnly);
    return existing ? static_cast<LanguageService *>(existing) : new LanguageService(app);
}
inline QString currentLanguage() { auto *s=service(); return s ? s->language() : QString(); }
inline void prepareCatalog(const QString &pluginId) { if(auto *s=service()) s->addCatalog(pluginId); }
inline void attach(QQmlEngine *engine,const QString &pluginId) {
    if (auto *s=service()) { s->addCatalog(pluginId); s->attachEngine(engine); }
}
}
