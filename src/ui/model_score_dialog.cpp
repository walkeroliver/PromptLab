#include "model_score_dialog.h"
#include "star_rating.h"
#include "repository/model_score_repository.h"

#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

ModelScoreDialog::ModelScoreDialog(int promptId, QWidget* parent)
    : QDialog(parent)
    , m_promptId(promptId)
{
    setupUi();
    loadModels();
}

 
void ModelScoreDialog::setupUi()
{
    setWindowTitle(tr("添加模型评分"));
    setMinimumSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QFormLayout* formLayout = new QFormLayout;

    m_modelCombo = new QComboBox;
    formLayout->addRow(tr("模型："), m_modelCombo);

    m_versionEdit = new QLineEdit;
    m_versionEdit->setPlaceholderText(tr("模型版本..."));
    formLayout->addRow(tr("版本："), m_versionEdit);

    QHBoxLayout* ratingLayout = new QHBoxLayout;
    m_ratingWidget = new StarRating;
    ratingLayout->addWidget(m_ratingWidget);
    ratingLayout->addStretch();
    formLayout->addRow(tr("评分："), ratingLayout);

    QHBoxLayout* noteLayout = new QHBoxLayout;
    noteLayout->addWidget(new QLabel(tr("备注：")));
    m_noteEdit = new QTextEdit;
    m_noteEdit->setPlaceholderText(tr("添加关于模型表现的备注..."));
    m_noteEdit->setMaximumHeight(80);
    noteLayout->addWidget(m_noteEdit, 1);
    mainLayout->addLayout(noteLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ModelScoreDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

 
void ModelScoreDialog::loadModels()
{
    ModelScoreRepository repo;
    QVector<Model> models = repo.findAllModels();

    for (const Model& model : models) {
        m_modelCombo->addItem(model.name, model.id);
    }
}

 
void ModelScoreDialog::onAccept()
{
    if (m_modelCombo->currentIndex() < 0) {
        QMessageBox::warning(this, tr("错误"), tr("请选择一个模型。"));
        return;
    }

    ModelScore score;
    score.promptId = m_promptId;
    score.modelId = getModelId();
    score.modelVersion = getModelVersion();
    score.score = getScore();
    score.note = getNote();

    ModelScoreRepository repo;
    int id = repo.insertScore(score);
    if (id <= 0) {
        QMessageBox::warning(this, tr("错误"), tr("保存模型评分失败。"));
        return;
    }

    accept();
}

 
int ModelScoreDialog::getModelId() const
{
    return m_modelCombo->currentData().toInt();
}
 
QString ModelScoreDialog::getModelVersion() const
{
    return m_versionEdit->text().trimmed();
}
 
double ModelScoreDialog::getScore() const
{
    return m_ratingWidget->rating();
}
 
QString ModelScoreDialog::getNote() const
{
    return m_noteEdit->toPlainText().trimmed();
}
