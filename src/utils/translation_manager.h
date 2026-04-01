#pragma once

#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <memory>

class TranslationManager {
public:
    static TranslationManager& instance();

    void initialize(QApplication* app);
    bool loadLanguage(const QString& languageCode);
    QString currentLanguage() const;
    QStringList availableLanguages() const;

private:
    TranslationManager() = default;
    ~TranslationManager() = default;

    std::unique_ptr<QTranslator> m_translator;
    QString m_currentLanguage;
    QString m_translationsPath;
};
