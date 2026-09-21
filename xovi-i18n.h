#pragma once
#include <QCoreApplication>
#include <QQmlEngine>
#include <QTranslator>
#include <QLocale>
#include <QTimer>
#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QHash>
#include <QPointer>
#include <functional>

namespace XoviI18n {
class LanguageObserver : public QObject {
public:
    LanguageObserver(QObject *parent, std::function<void()> update):QObject(parent),update_(std::move(update)) {}
    bool eventFilter(QObject *, QEvent *event) override {
        const bool sessionChanged=event->type()==QEvent::DynamicPropertyChange
            && static_cast<QDynamicPropertyChangeEvent *>(event)->propertyName()=="xoviUiLanguage";
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
    const QLocale locale(requested.isEmpty() ? QLocale().name() : requested);
    if(locale.language()!=QLocale::Chinese) return QStringLiteral("en");
    return locale.script()==QLocale::TraditionalHanScript ? QStringLiteral("zh_TW") : QStringLiteral("zh_CN");
}
inline QString languageSettingsPath() {
    const auto overridePath=qEnvironmentVariable("XOVI_LANGUAGE_SETTINGS");
    return overridePath.isEmpty() ? QStringLiteral("/data/xochitl.conf") : overridePath;
}
inline QString readNativeLanguage(const QString &path) {
    // Match AppLoad's native config parser; do not ask each plugin/QML engine
    // to guess the locale from its own defaults or QSettings' cached state.
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    bool general=false;
    while (!file.atEnd()) {
        auto line=QString::fromUtf8(file.readLine()).trimmed();
        if (line.startsWith(QChar(0xfeff))) line.remove(0,1);
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) continue;
        if (line.startsWith('[')) {
            general=line.compare("[General]",Qt::CaseInsensitive)==0;
            continue;
        }
        const int equals=line.indexOf('=');
        if (general && equals>0 && line.left(equals).trimmed().compare("Language",Qt::CaseInsensitive)==0)
            return line.mid(equals+1).trimmed();
    }
    return {};
}
// One process-wide owner, shared through the application object rather than
// header-local statics in separate DSOs. Catalogs stay independent and live for
// the session, never for the lifetime of a settings page or a QML engine.
class LanguageService : public QObject {
public:
    explicit LanguageService(QObject *app):QObject(app),watcher_(this) {
        setObjectName("xoviLanguageServiceV2");
        paths_ << languageSettingsPath();
        if (qEnvironmentVariableIsEmpty("XOVI_LANGUAGE_SETTINGS"))
            paths_ << QStringLiteral("/home/root/.config/remarkable/xochitl.conf");
        observePaths();
        auto changed=[this](const QString &) { observePaths(); schedule(); };
        QObject::connect(&watcher_,&QFileSystemWatcher::fileChanged,this,changed);
        QObject::connect(&watcher_,&QFileSystemWatcher::directoryChanged,this,changed);
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
        // Watchers own language refresh; resolving a label must not reread the
        // device config or walk every catalog on each QML binding evaluation.
        if (!catalogs_.contains(id)) addCatalog(id);
        const auto *catalog=catalogs_.value(id);
        const auto translated=catalog ? catalog->translate(context.toUtf8().constData(),source.toUtf8().constData()) : QString();
        return translated.isNull() ? source : translated;
    }
    void attachEngine(QQmlEngine *engine) {
        if (!engine) return;
        for (const auto &existing:engines_) if (existing==engine) return;
        engines_.append(engine);
        // An engine can request that native language be checked again. It must
        // never replace the session language with a new engine's default English.
        QObject::connect(engine,&QQmlEngine::uiLanguageChanged,this,[this]() { schedule(); });
    }
private:
    QString resolve() const {
        for (const auto &path:paths_) {
            const auto language=readNativeLanguage(path);
            if (!language.isEmpty()) return catalogLanguage(language);
            // Do not read a stale legacy copy when the canonical file exists.
            if (QFileInfo::exists(path)) break;
        }
        // Atomic replacement, temporary unreadability or a newly-created engine
        // must not discard a language already selected by xochitl.
        if (!language_.isEmpty()) return language_;
        const auto explicitLocale=qEnvironmentVariable("APP_LOCALE");
        return catalogLanguage(explicitLocale.isEmpty() ? QLocale::system().name() : explicitLocale);
    }
    void observePaths() {
        for (const auto &path:paths_) {
            const auto directory=QFileInfo(path).absolutePath();
            if (QFileInfo(directory).isDir() && !watcher_.directories().contains(directory)) watcher_.addPath(directory);
            if (QFileInfo::exists(path) && !watcher_.files().contains(path)) watcher_.addPath(path);
        }
    }
    void schedule() {
        if (pending_) return;
        pending_=true;
        QTimer::singleShot(0,this,[this]() { pending_=false; synchronize(); });
    }
    void loadCatalog(const QString &id) {
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
        const bool changed=!language_.isEmpty() && next!=language_;
        language_=next;
        QCoreApplication::instance()->setProperty("xoviUiLanguage",language_);
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
inline QString currentLanguage() { auto *s=service(); return s ? s->language() : QStringLiteral("en"); }
inline void prepareCatalog(const QString &pluginId) { if(auto *s=service()) s->addCatalog(pluginId); }
inline void attach(QQmlEngine *engine,const QString &pluginId) {
    if (auto *s=service()) { s->addCatalog(pluginId); s->attachEngine(engine); }
}
}
