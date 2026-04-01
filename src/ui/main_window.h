#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QListWidgetItem>
#include <QStackedWidget>

class PromptListWidget;
class PromptDetailWidget;
class DashboardWidget;
class PromptService;
class CategoryService;
class TagService;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onNewPrompt();
    void onEditPrompt();
    void onDeletePrompt();
    void onPromptSelected(int promptId);
    void onSearchChanged(const QString& text);
    void onCategorySelected(QTreeWidgetItem* item, int column);
    void onTagSelected(QListWidgetItem* item);
    void onManageCategories();
    void onManageTags();
    void onDataChanged();
    void onShowDashboard();
    void onShowPromptList();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupConnections();
    void loadCategories();
    void loadTags();
    void updateStatusBar();
    void updateWindowTitle();

    QSplitter* m_mainSplitter;
    QWidget* m_leftPanel;
    QStackedWidget* m_rightStack;

    QTreeWidget* m_categoryTree;
    QListWidget* m_tagList;
    QLineEdit* m_searchEdit;

    PromptListWidget* m_promptList;
    PromptDetailWidget* m_promptDetail;
    DashboardWidget* m_dashboard;
    QWidget* m_promptViewPage;

    PromptService* m_promptService;
    CategoryService* m_categoryService;
    TagService* m_tagService;

    int m_selectedCategoryId = -1;
    int m_selectedTagId = -1;
    int m_selectedPromptId = 0;
};
