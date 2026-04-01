#pragma once

#include <QDialog>

class TagService;
class QListWidget;
class QLineEdit;

class TagManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit TagManagerDialog(TagService* tagService, QWidget* parent = nullptr);

private slots:
    void onAddTag();
    void onEditTag();
    void onDeleteTag();

private:
    void setupUi();
    void loadTags();

    TagService* m_tagService;

    QListWidget* m_tagList;
    QLineEdit* m_nameEdit;
};
