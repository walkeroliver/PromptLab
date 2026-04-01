#pragma once

#include <QString>
#include <QDateTime>
#include <QVector>

struct Prompt {
    int id = 0;
    QString title;
    QString content;
    QString description;
    int categoryId = 0;
    double overallRating = 0.0;
    bool isFavorite = false;
    bool isArchived = false;
    int usageCount = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
    QVector<int> tagIds;

    Prompt() = default;

    bool isValid() const { return id > 0; }
    bool hasCategory() const { return categoryId > 0; }
    bool hasTags() const { return !tagIds.isEmpty(); }
    bool isRated() const { return overallRating > 0; }
};
