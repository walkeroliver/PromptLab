#pragma once

#include "model/prompt.h"
#include <QDialog>

class PromptService;
class CategoryService;
class TagService;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QListWidget;
class StarRating;

class PromptEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit PromptEditorDialog(PromptService* promptService,
                                 CategoryService* categoryService,
                                 TagService* tagService,
                                 QWidget* parent = nullptr);

    void setPrompt(const Prompt& prompt);
    Prompt getPrompt() const;

private slots:
    void onSave();
    void onManageTags();

private:
    void setupUi();
    void loadCategories();
    void loadTags();
    bool validateInput();

    PromptService* m_promptService;
    CategoryService* m_categoryService;
    TagService* m_tagService;

    QLineEdit* m_titleEdit;
    QTextEdit* m_contentEdit;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_categoryCombo;
    QListWidget* m_tagList;
    StarRating* m_ratingWidget;

    int m_promptId = 0;
    bool m_isEditMode = false;
};
