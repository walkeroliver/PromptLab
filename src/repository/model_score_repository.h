#pragma once

#include "model/model_score.h"
#include <QVector>
#include <optional>

class ModelScoreRepository {
public:
    ModelScoreRepository();

    QVector<Model> findAllModels();
    std::optional<Model> findModelById(int id);
    std::optional<Model> findModelByName(const QString& name);
    int insertModel(const QString& name, const QString& provider = QString());
    bool updateModel(const Model& model);
    bool removeModel(int id);

    int insertScore(const ModelScore& score);
    bool updateScore(const ModelScore& score);
    bool removeScore(int id);
    bool removeScoresByPrompt(int promptId);
    QVector<ModelScore> findByPromptId(int promptId);
    QVector<ModelScore> findByModelId(int modelId);
    std::optional<ModelScore> findByPromptAndModel(int promptId, int modelId);
    double getAverageScoreForModel(int modelId);
    double getAverageScoreForPrompt(int promptId);
    int countScoresForPrompt(int promptId);

private:
    Model modelFromQuery(class QSqlQuery& query);
    ModelScore scoreFromQuery(class QSqlQuery& query);
};
