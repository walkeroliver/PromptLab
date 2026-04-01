#pragma once

#include <QString>
#include <QDateTime>

struct Model {
    int id = 0;
    QString name;
    QString provider;
    bool isBuiltin = false;
    int sortOrder = 0;
    QDateTime createdAt;

    Model() = default;
    Model(const QString& n, const QString& p = QString(), bool builtin = false)
        : name(n), provider(p), isBuiltin(builtin) {}

    bool isValid() const { return id > 0; }
};

struct ModelScore {
    int id = 0;
    int promptId = 0;
    int modelId = 0;
    QString modelVersion;
    double score = 0.0;
    QString note;
    QDateTime testedAt;
    QDateTime createdAt;
    QDateTime updatedAt;

    ModelScore() = default;

    bool isValid() const { return id > 0; }
    bool isScored() const { return score > 0; }
};

struct Version {
    int id = 0;
    int promptId = 0;
    int version = 0;
    QString title;
    QString content;
    QString description;
    QString changeNote;
    QDateTime createdAt;

    Version() = default;
    bool isValid() const { return id > 0; }
};

struct Example {
    int id = 0;
    int promptId = 0;
    QString inputText;
    QString outputText;
    QString note;
    int sortOrder = 0;
    QDateTime createdAt;

    Example() = default;
    bool isValid() const { return id > 0; }
};

struct Setting {
    QString key;
    QString value;
    QDateTime updatedAt;

    Setting() = default;
    Setting(const QString& k, const QString& v) : key(k), value(v) {}

    bool isValid() const { return !key.isEmpty(); }
};
