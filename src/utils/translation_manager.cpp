#include "translation_manager.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

TranslationManager& TranslationManager::instance()
{
    static TranslationManager instance;
    return instance;
}

void TranslationManager::initialize(QApplication* app)
{
    m_translationsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_translationsPath += "/PromptLab/translations";

    QDir dir(m_translationsPath);
    if (!dir.exists()) {
        dir.mkpath(m_translationsPath);
    }

    QString systemLang = QLocale::system().name();
    
    if (systemLang.startsWith("zh")) {
        loadLanguage("zh_CN");
    } else {
        loadLanguage("zh_CN");
    }
}

bool TranslationManager::loadLanguage(const QString& languageCode)
{
    if (m_translator) {
        qApp->removeTranslator(m_translator.get());
    }

    m_translator = std::make_unique<QTranslator>();

    QString translationFile = QString("%1/promptlab_%2.qm").arg(m_translationsPath, languageCode);
    
    if (QFile::exists(translationFile)) {
        if (m_translator->load(translationFile)) {
            qApp->installTranslator(m_translator.get());
            m_currentLanguage = languageCode;
            qDebug() << "Loaded translation:" << languageCode;
            return true;
        }
    }

    qApp->installTranslator(m_translator.get());
    m_currentLanguage = "zh_CN";
    qDebug() << "Using default Chinese translation";
    return true;
}

QString TranslationManager::currentLanguage() const
{
    return m_currentLanguage;
}

QStringList TranslationManager::availableLanguages() const
{
    return {"zh_CN", "en_US"};
}
