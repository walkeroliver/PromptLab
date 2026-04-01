#pragma once

#include <QString>
#include <QDateTime>

struct Tag {
    int id = 0;
    QString name;
    QString color;
    int usageCount = 0;
    QDateTime createdAt;

    Tag() = default;
    Tag(const QString& n) : name(n), color("#3498db") {}

    bool isValid() const { return id > 0; }
};
