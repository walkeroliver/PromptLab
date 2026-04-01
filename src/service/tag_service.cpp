#include "tag_service.h"

TagService::TagService(QObject* parent)
    : QObject(parent)
{
}

int TagService::createTag(const QString& name)
{
    Tag tag;
    tag.name = name;

    int id = m_tagRepo.insert(tag);
    if (id > 0) {
        emit tagCreated(id);
        emit dataChanged();
    }

    return id;
}

bool TagService::updateTag(const Tag& tag)
{
    bool success = m_tagRepo.update(tag);
    if (success) {
        emit tagUpdated(tag.id);
        emit dataChanged();
    }

    return success;
}

bool TagService::deleteTag(int id)
{
    bool success = m_tagRepo.remove(id);
    if (success) {
        emit tagDeleted(id);
        emit dataChanged();
    }

    return success;
}

Tag TagService::getTag(int id)
{
    auto tag = m_tagRepo.findById(id);
    return tag.value_or(Tag());
}

Tag TagService::getTagByName(const QString& name)
{
    auto tag = m_tagRepo.findByName(name);
    return tag.value_or(Tag());
}

QVector<Tag> TagService::getAllTags()
{
    return m_tagRepo.findAll();
}

QVector<Tag> TagService::getTagsForPrompt(int promptId)
{
    return m_tagRepo.findByPromptId(promptId);
}

bool TagService::tagExists(const QString& name)
{
    return m_tagRepo.exists(name);
}

int TagService::getOrCreateTag(const QString& name)
{
    return m_tagRepo.getOrCreate(name);
}
