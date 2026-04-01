#include "ui/main_window.h"
#include "database/database_manager.h"
#include "utils/translation_manager.h"

#include <QApplication>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PromptLab");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PromptLab");

    TranslationManager::instance().initialize(&app);

    QString dataPath = QCoreApplication::applicationDirPath() + "/data";
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString dbPath = dataPath + "/promptlab.db";

    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.initialize(dbPath)) {
        QMessageBox::critical(nullptr, QObject::tr("数据库错误"),
            QObject::tr("初始化数据库失败：%1").arg(dbManager.lastError()));
        return -1;
    }

    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    MainWindow mainWindow;
    mainWindow.setWindowTitle(QCoreApplication::translate("MainWindow", "PromptLab"));
    mainWindow.resize(1200, 800);
    mainWindow.show();

    return app.exec();
}
