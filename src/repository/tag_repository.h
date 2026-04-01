#pragma once

#include "model/tag.h"
#include <QVector>
#include <optional>

class TagRepository {
public:
    TagRepository();

    int insert(const Tag& tag);
    bool update(const Tag& tag);
    bool remove(int id);
    std::optional<Tag> findById(int id);
    std::optional<Tag> findByName(const QString& name);
    QVector<Tag> findAll();
    QVector<Tag> findByPromptId(int promptId);
    QVector<Tag> findPopular(int limit = 10);
    bool exists(const QString& name);
    int getOrCreate(const QString& name);
    int count();

private:
    Tag tagFromQuery(class QSqlQuery& query);
};
