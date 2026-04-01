#include "main_window.h"
#include "prompt_list_widget.h"
#include "prompt_detail_widget.h"
#include "dashboard_widget.h"
#include "prompt_editor_dialog.h"
#include "category_manager_dialog.h"
#include "tag_manager_dialog.h"
#include "prompt_service.h"
#include "category_service.h"
#include "tag_service.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QActionGroup>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_promptService(new PromptService(this))
    , m_categoryService(new CategoryService(this))
    , m_tagService(new TagService(this))
{
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupConnections();
    loadCategories();
    loadTags();
    onShowPromptList();
    updateStatusBar();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    setWindowTitle(tr("PromptLab"));
    resize(1280, 800);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setHandleWidth(1);
    setCentralWidget(m_mainSplitter);

    m_leftPanel = new QWidget;
    m_leftPanel->setMinimumWidth(220);
    m_leftPanel->setMaximumWidth(300);
    m_leftPanel->setStyleSheet("background: #fafafa;");
    QVBoxLayout* leftLayout = new QVBoxLayout(m_leftPanel);
    leftLayout->setContentsMargins(12, 12, 12, 12);
    leftLayout->setSpacing(12);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(tr("搜索提示词..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setStyleSheet(
        "QLineEdit { background: #fff; border: 1px solid #d9d9d9; border-radius: 4px; padding: 0 12px; font-size: 13px; }"
        "QLineEdit:focus { border-color: #40a9ff; }"
    );
    leftLayout->addWidget(m_searchEdit);

    QLabel* categoryLabel = new QLabel(tr("分类"));
    categoryLabel->setStyleSheet("font-weight: 600; color: #333; font-size: 13px;");
    leftLayout->addWidget(categoryLabel);

    m_categoryTree = new QTreeWidget;
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setIndentation(16);
    m_categoryTree->setRootIsDecorated(false);
    m_categoryTree->setFrameShape(QFrame::NoFrame);
    m_categoryTree->setStyleSheet(
        "QTreeWidget { background: transparent; }"
        "QTreeWidget::item { padding: 8px; border-radius: 4px; }"
        "QTreeWidget::item:hover { background: #e6f7ff; }"
        "QTreeWidget::item:selected { background: #1890ff; color: #fff; }"
    );
    leftLayout->addWidget(m_categoryTree, 1);

    QHBoxLayout* categoryBtnLayout = new QHBoxLayout;
    QPushButton* manageCategoriesBtn = new QPushButton(tr("管理分类"));
    manageCategoriesBtn->setFlat(true);
    manageCategoriesBtn->setCursor(Qt::PointingHandCursor);
    manageCategoriesBtn->setStyleSheet("color: #1890ff; font-size: 12px; padding: 4px 8px;");
    categoryBtnLayout->addStretch();
    categoryBtnLayout->addWidget(manageCategoriesBtn);
    leftLayout->addLayout(categoryBtnLayout);
    connect(manageCategoriesBtn, &QPushButton::clicked, this, &MainWindow::onManageCategories);

    QLabel* tagLabel = new QLabel(tr("标签"));
    tagLabel->setStyleSheet("font-weight: 600; color: #333; font-size: 13px; margin-top: 8px;");
    leftLayout->addWidget(tagLabel);

    m_tagList = new QListWidget;
    m_tagList->setFrameShape(QFrame::NoFrame);
    m_tagList->setSpacing(2);
    m_tagList->setStyleSheet(
        "QListWidget { background: transparent; }"
        "QListWidget::item { padding: 6px 8px; border-radius: 4px; }"
        "QListWidget::item:hover { background: #e6f7ff; }"
        "QListWidget::item:selected { background: #1890ff; color: #fff; }"
    );
    leftLayout->addWidget(m_tagList, 1);

    QHBoxLayout* tagBtnLayout = new QHBoxLayout;
    QPushButton* manageTagsBtn = new QPushButton(tr("管理标签"));
    manageTagsBtn->setFlat(true);
    manageTagsBtn->setCursor(Qt::PointingHandCursor);
    manageTagsBtn->setStyleSheet("color: #1890ff; font-size: 12px; padding: 4px 8px;");
    tagBtnLayout->addStretch();
    tagBtnLayout->addWidget(manageTagsBtn);
    leftLayout->addLayout(tagBtnLayout);
    connect(manageTagsBtn, &QPushButton::clicked, this, &MainWindow::onManageTags);

    m_mainSplitter->addWidget(m_leftPanel);

    m_rightStack = new QStackedWidget;
    m_rightStack->setStyleSheet("background: #fff;");

    m_promptList = new PromptListWidget(m_promptService, m_categoryService, m_tagService);
    m_promptDetail = new PromptDetailWidget(m_promptService, m_tagService, m_categoryService);
    m_dashboard = new DashboardWidget(m_promptService, m_categoryService, m_tagService);

    m_promptViewPage = new QWidget;
    QVBoxLayout* promptViewLayout = new QVBoxLayout(m_promptViewPage);
    promptViewLayout->setContentsMargins(0, 0, 0, 0);
    promptViewLayout->setSpacing(0);

    QSplitter* contentSplitter = new QSplitter(Qt::Vertical);
    contentSplitter->setHandleWidth(1);
    contentSplitter->addWidget(m_promptList);
    contentSplitter->addWidget(m_promptDetail);
    contentSplitter->setSizes({450, 350});
    promptViewLayout->addWidget(contentSplitter);

    m_rightStack->addWidget(m_promptViewPage);
    m_rightStack->addWidget(m_dashboard);

    m_mainSplitter->addWidget(m_rightStack);
    m_mainSplitter->setSizes({260, 1020});
}

void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();
    menuBar->setStyleSheet(
        "QMenuBar { background: #fff; border-bottom: 1px solid #e8e8e8; padding: 4px 8px; }"
        "QMenuBar::item { padding: 6px 12px; border-radius: 4px; }"
        "QMenuBar::item:selected { background: #e6f7ff; }"
    );

    QMenu* fileMenu = menuBar->addMenu(tr("文件(&F)"));

    QAction* newAction = fileMenu->addAction(tr("新建提示词"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewPrompt);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction(tr("退出"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* editMenu = menuBar->addMenu(tr("编辑(&E)"));

    QAction* editAction = editMenu->addAction(tr("编辑提示词"));
    editAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    connect(editAction, &QAction::triggered, this, &MainWindow::onEditPrompt);

    QAction* deleteAction = editMenu->addAction(tr("删除提示词"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeletePrompt);

    QMenu* viewMenu = menuBar->addMenu(tr("视图(&V)"));

    QAction* promptListAction = viewMenu->addAction(tr("提示词列表"));
    promptListAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(promptListAction, &QAction::triggered, this, &MainWindow::onShowPromptList);

    QAction* dashboardAction = viewMenu->addAction(tr("仪表盘"));
    dashboardAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(dashboardAction, &QAction::triggered, this, &MainWindow::onShowDashboard);

    QMenu* helpMenu = menuBar->addMenu(tr("帮助(&H)"));
    QAction* aboutAction = helpMenu->addAction(tr("关于"));
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, tr("关于 PromptLab"),
            tr("PromptLab v1.0.0\n\n"
            "提示词管理与评估工具\n\n"
            "开源软件 - MIT 许可证"));
    });
}

void MainWindow::setupToolBar()
{
    QToolBar* toolbar = addToolBar(tr("主工具栏"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(18, 18));
    toolbar->setStyleSheet(
        "QToolBar { background: #fff; border-bottom: 1px solid #e8e8e8; spacing: 8px; padding: 8px 16px; }"
        "QToolBar QToolButton { background: transparent; border: none; border-radius: 4px; padding: 8px 16px; color: #333; font-size: 13px; }"
        "QToolBar QToolButton:hover { background: #e6f7ff; color: #1890ff; }"
        "QToolBar QToolButton:pressed { background: #bae7ff; }"
    );

    QAction* newAction = toolbar->addAction(tr("新建"));
    newAction->setToolTip(tr("新建提示词 (Ctrl+N)"));
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewPrompt);

    QAction* editAction = toolbar->addAction(tr("编辑"));
    editAction->setToolTip(tr("编辑选中的提示词 (Ctrl+E)"));
    connect(editAction, &QAction::triggered, this, &MainWindow::onEditPrompt);

    QAction* deleteAction = toolbar->addAction(tr("删除"));
    deleteAction->setToolTip(tr("删除选中的提示词"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeletePrompt);

    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    QAction* promptListAction = toolbar->addAction(tr("列表"));
    promptListAction->setToolTip(tr("显示提示词列表 (Ctrl+L)"));
    connect(promptListAction, &QAction::triggered, this, &MainWindow::onShowPromptList);

    QAction* dashboardAction = toolbar->addAction(tr("仪表盘"));
    dashboardAction->setToolTip(tr("显示仪表盘 (Ctrl+D)"));
    connect(dashboardAction, &QAction::triggered, this, &MainWindow::onShowDashboard);
}

void MainWindow::setupConnections()
{
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    connect(m_categoryTree, &QTreeWidget::itemClicked, this, &MainWindow::onCategorySelected);
    connect(m_tagList, &QListWidget::itemClicked, this, &MainWindow::onTagSelected);

    connect(m_promptList, &PromptListWidget::promptSelected, this, &MainWindow::onPromptSelected);
    connect(m_promptList, &PromptListWidget::promptDoubleClicked, this, &MainWindow::onEditPrompt);

    connect(m_promptService, &PromptService::dataChanged, this, &MainWindow::onDataChanged);
    connect(m_categoryService, &CategoryService::dataChanged, this, &MainWindow::onDataChanged);
    connect(m_tagService, &TagService::dataChanged, this, &MainWindow::onDataChanged);
}

void MainWindow::loadCategories()
{
    m_categoryTree->clear();

    QTreeWidgetItem* allItem = new QTreeWidgetItem(m_categoryTree);
    allItem->setText(0, tr("全部提示词"));
    allItem->setData(0, Qt::UserRole, -1);
    allItem->setSelected(true);

    QVector<Category> categories = m_categoryService->getAllCategories();
    for (const Category& cat : categories) {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_categoryTree);
        item->setText(0, cat.name);
        item->setData(0, Qt::UserRole, cat.id);
    }

    m_categoryTree->expandAll();
}

void MainWindow::loadTags()
{
    m_tagList->clear();

    QListWidgetItem* allItem = new QListWidgetItem(tr("全部标签"));
    allItem->setData(Qt::UserRole, -1);
    m_tagList->addItem(allItem);

    QVector<Tag> tags = m_tagService->getAllTags();
    for (const Tag& tag : tags) {
        QListWidgetItem* item = new QListWidgetItem(tag.name);
        item->setData(Qt::UserRole, tag.id);
        m_tagList->addItem(item);
    }
}

void MainWindow::updateStatusBar()
{
    int count = m_promptService->getPromptCount();
    statusBar()->showMessage(tr("共 %1 条提示词").arg(count));
}

void MainWindow::updateWindowTitle()
{
    if (m_selectedPromptId > 0) {
        Prompt prompt = m_promptService->getPrompt(m_selectedPromptId);
        if (prompt.isValid()) {
            setWindowTitle(tr("PromptLab - %1").arg(prompt.title));
            return;
        }
    }
    setWindowTitle(tr("PromptLab"));
}

void MainWindow::onNewPrompt()
{
    PromptEditorDialog dialog(m_promptService, m_categoryService, m_tagService, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_promptList->refreshList();
        updateStatusBar();
    }
}

void MainWindow::onEditPrompt()
{
    if (m_selectedPromptId <= 0) {
        QMessageBox::information(this, tr("提示"), tr("请先选择要编辑的提示词。"));
        return;
    }

    Prompt prompt = m_promptService->getPrompt(m_selectedPromptId);
    if (!prompt.isValid()) {
        return;
    }

    PromptEditorDialog dialog(m_promptService, m_categoryService, m_tagService, this);
    dialog.setPrompt(prompt);

    if (dialog.exec() == QDialog::Accepted) {
        m_promptList->refreshList();
        m_promptDetail->setPrompt(m_selectedPromptId);
        updateWindowTitle();
    }
}

void MainWindow::onDeletePrompt()
{
    if (m_selectedPromptId <= 0) {
        QMessageBox::information(this, tr("提示"), tr("请先选择要删除的提示词。"));
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("确认删除"),
        tr("确定要删除这条提示词吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_promptService->deletePrompt(m_selectedPromptId);
        m_selectedPromptId = 0;
        m_promptList->refreshList();
        m_promptDetail->clear();
        updateStatusBar();
        updateWindowTitle();
    }
}

void MainWindow::onPromptSelected(int promptId)
{
    m_selectedPromptId = promptId;
    m_promptDetail->setPrompt(promptId);
    updateWindowTitle();
}

void MainWindow::onSearchChanged(const QString& text)
{
    m_selectedCategoryId = -1;
    m_selectedTagId = -1;
    m_promptList->setSearchFilter(text);
}

void MainWindow::onCategorySelected(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    m_selectedCategoryId = item->data(0, Qt::UserRole).toInt();
    m_selectedTagId = -1;
    m_promptList->setCategoryFilter(m_selectedCategoryId);
}

void MainWindow::onTagSelected(QListWidgetItem* item)
{
    m_selectedTagId = item->data(Qt::UserRole).toInt();
    m_selectedCategoryId = -1;
    m_promptList->setTagFilter(m_selectedTagId);
}

void MainWindow::onManageCategories()
{
    CategoryManagerDialog dialog(m_categoryService, this);
    dialog.exec();
    loadCategories();
}

void MainWindow::onManageTags()
{
    TagManagerDialog dialog(m_tagService, this);
    dialog.exec();
    loadTags();
}

void MainWindow::onDataChanged()
{
    m_promptList->refreshList();
    loadCategories();
    loadTags();
    updateStatusBar();
}

void MainWindow::onShowDashboard()
{
    m_dashboard->refresh();
    m_rightStack->setCurrentWidget(m_dashboard);
}

void MainWindow::onShowPromptList()
{
    m_rightStack->setCurrentWidget(m_promptViewPage);
}
