#include "model_score_repository.h"
#include "database/database_manager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ModelScoreRepository::ModelScoreRepository() {}

QVector<Model> ModelScoreRepository::findAllModels()
{
    QVector<Model> models;
    QSqlQuery query(DatabaseManager::instance().database());
    query.exec("SELECT * FROM models ORDER BY is_builtin DESC, sort_order, name");

    while (query.next()) {
        models.append(modelFromQuery(query));
    }

    return models;
}

std::optional<Model> ModelScoreRepository::findModelById(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM models WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        return modelFromQuery(query);
    }

    return std::nullopt;
}

std::optional<Model> ModelScoreRepository::findModelByName(const QString& name)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM models WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        return modelFromQuery(query);
    }

    return std::nullopt;
}

int ModelScoreRepository::insertModel(const QString& name, const QString& provider)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("INSERT INTO models (name, provider, is_builtin) VALUES (?, ?, 0)");
    query.addBindValue(name);
    query.addBindValue(provider);

    if (!query.exec()) {
        qDebug() << "Failed to insert model:" << query.lastError().text();
        return 0;
    }

    return query.lastInsertId().toInt();
}

bool ModelScoreRepository::updateModel(const Model& model)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("UPDATE models SET name = ?, provider = ?, sort_order = ? WHERE id = ?");
    query.addBindValue(model.name);
    query.addBindValue(model.provider);
    query.addBindValue(model.sortOrder);
    query.addBindValue(model.id);

    return query.exec();
}

bool ModelScoreRepository::removeModel(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM models WHERE id = ? AND is_builtin = 0");
    query.addBindValue(id);

    return query.exec();
}

int ModelScoreRepository::insertScore(const ModelScore& score)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        INSERT OR REPLACE INTO model_scores (prompt_id, model_id, model_version, score, note, tested_at)
        VALUES (?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(score.promptId);
    query.addBindValue(score.modelId);
    query.addBindValue(score.modelVersion);
    query.addBindValue(score.score);
    query.addBindValue(score.note);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Failed to insert model score:" << query.lastError().text();
        return 0;
    }

    return query.lastInsertId().toInt();
}

bool ModelScoreRepository::updateScore(const ModelScore& score)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(R"(
        UPDATE model_scores SET
            model_id = ?,
            model_version = ?,
            score = ?,
            note = ?,
            tested_at = ?
        WHERE id = ?
    )");

    query.addBindValue(score.modelId);
    query.addBindValue(score.modelVersion);
    query.addBindValue(score.score);
    query.addBindValue(score.note);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    query.addBindValue(score.id);

    return query.exec();
}

bool ModelScoreRepository::removeScore(int id)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM model_scores WHERE id = ?");
    query.addBindValue(id);

    return query.exec();
}

bool ModelScoreRepository::removeScoresByPrompt(int promptId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM model_scores WHERE prompt_id = ?");
    query.addBindValue(promptId);

    return query.exec();
}

QVector<ModelScore> ModelScoreRepository::findByPromptId(int promptId)
{
    QVector<ModelScore> scores;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM model_scores WHERE prompt_id = ? ORDER BY score DESC, tested_at DESC");
    query.addBindValue(promptId);

    if (query.exec()) {
        while (query.next()) {
            scores.append(scoreFromQuery(query));
        }
    }

    return scores;
}

QVector<ModelScore> ModelScoreRepository::findByModelId(int modelId)
{
    QVector<ModelScore> scores;
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM model_scores WHERE model_id = ? ORDER BY score DESC, tested_at DESC");
    query.addBindValue(modelId);

    if (query.exec()) {
        while (query.next()) {
            scores.append(scoreFromQuery(query));
        }
    }

    return scores;
}

std::optional<ModelScore> ModelScoreRepository::findByPromptAndModel(int promptId, int modelId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT * FROM model_scores WHERE prompt_id = ? AND model_id = ?");
    query.addBindValue(promptId);
    query.addBindValue(modelId);

    if (query.exec() && query.next()) {
        return scoreFromQuery(query);
    }

    return std::nullopt;
}

double ModelScoreRepository::getAverageScoreForModel(int modelId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT AVG(score) FROM model_scores WHERE model_id = ? AND score > 0");
    query.addBindValue(modelId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

double ModelScoreRepository::getAverageScoreForPrompt(int promptId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT AVG(score) FROM model_scores WHERE prompt_id = ? AND score > 0");
    query.addBindValue(promptId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

int ModelScoreRepository::countScoresForPrompt(int promptId)
{
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT COUNT(*) FROM model_scores WHERE prompt_id = ?");
    query.addBindValue(promptId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

Model ModelScoreRepository::modelFromQuery(QSqlQuery& query)
{
    Model model;
    model.id = query.value("id").toInt();
    model.name = query.value("name").toString();
    model.provider = query.value("provider").toString();
    model.isBuiltin = query.value("is_builtin").toInt() == 1;
    model.sortOrder = query.value("sort_order").toInt();
    model.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    return model;
}

ModelScore ModelScoreRepository::scoreFromQuery(QSqlQuery& query)
{
    ModelScore score;
    score.id = query.value("id").toInt();
    score.promptId = query.value("prompt_id").toInt();
    score.modelId = query.value("model_id").toInt();
    score.modelVersion = query.value("model_version").toString();
    score.score = query.value("score").toDouble();
    score.note = query.value("note").toString();
    score.testedAt = QDateTime::fromString(query.value("tested_at").toString(), Qt::ISODate);
    score.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    score.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return score;
}
