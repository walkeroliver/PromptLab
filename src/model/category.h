#pragma once

#include <QString>
#include <QDateTime>

struct Category {
    int id = 0;
    QString name;
    int parentId = 0;
    int sortOrder = 0;
    QDateTime createdAt;
    QDateTime updatedAt;

    Category() = default;
    Category(const QString& n) : name(n) {}

    bool isValid() const { return id > 0; }
    bool isTopLevel() const { return parentId == 0; }
    bool hasParent() const { return parentId > 0; }
};
