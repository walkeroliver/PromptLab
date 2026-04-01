#pragma once

#include "model/tag.h"
#include "repository/tag_repository.h"
#include <QObject>
#include <QVector>

class TagService : public QObject {
    Q_OBJECT

public:
    explicit TagService(QObject* parent = nullptr);

    int createTag(const QString& name);
    bool updateTag(const Tag& tag);
    bool deleteTag(int id);
    Tag getTag(int id);
    Tag getTagByName(const QString& name);
    QVector<Tag> getAllTags();
    QVector<Tag> getTagsForPrompt(int promptId);
    bool tagExists(const QString& name);
    int getOrCreateTag(const QString& name);

signals:
    void tagCreated(int id);
    void tagUpdated(int id);
    void tagDeleted(int id);
    void dataChanged();

private:
    TagRepository m_tagRepo;
};
