#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;
class QTextEdit;
class StarRating;

class ModelScoreDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelScoreDialog(int promptId, QWidget* parent = nullptr);

    int getModelId() const;
    QString getModelVersion() const;
    double getScore() const;
    QString getNote() const;

private:
    void setupUi();
    void loadModels();

private slots:
    void onAccept();

private:

    int m_promptId;

    QComboBox* m_modelCombo;
    QLineEdit* m_versionEdit;
    StarRating* m_ratingWidget;
    QTextEdit* m_noteEdit;
};
