#pragma once

#include "model/prompt.h"
#include <QWidget>
#include <QHash>

class PromptService;
class CategoryService;
class TagService;
class QListWidget;
class QListWidgetItem;
class QComboBox;

class PromptListWidget : public QWidget {
    Q_OBJECT

public:
    explicit PromptListWidget(PromptService* promptService,
                               CategoryService* categoryService,
                               TagService* tagService,
                               QWidget* parent = nullptr);

    void refreshList();
    void setSearchFilter(const QString& keyword);
    void setCategoryFilter(int categoryId);
    void setTagFilter(int tagId);
    void setRatingFilter(double minRating);
    void clearFilters();

signals:
    void promptSelected(int promptId);
    void promptDoubleClicked(int promptId);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onSortChanged(int index);

private:
    void setupUi();
    void loadPrompts();
    void updateList();
    QWidget* createPromptCard(const Prompt& prompt);

    PromptService* m_promptService;
    CategoryService* m_categoryService;
    TagService* m_tagService;

    QListWidget* m_listWidget;
    QComboBox* m_sortCombo;

    QVector<Prompt> m_prompts;
    QHash<int, QString> m_categoryNames;
    QHash<int, QString> m_tagNames;

    QString m_searchKeyword;
    int m_filterCategoryId = -1;
    int m_filterTagId = -1;
    double m_minRating = 0.0;
    int m_sortMode = 0;
};
