#include "prompt_service.h"
#include "repository/tag_repository.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>

PromptService::PromptService(QObject* parent)
    : QObject(parent)
{
}

int PromptService::createPrompt(const QString& title, const QString& content,
                                 const QString& description, int categoryId,
                                 const QVector<int>& tagIds, double rating)
{
    Prompt prompt;
    prompt.title = title;
    prompt.content = content;
    prompt.description = description;
    prompt.categoryId = categoryId;
    prompt.tagIds = tagIds;
    prompt.overallRating = rating;

    int id = m_promptRepo.insert(prompt);
    if (id > 0) {
        emit promptCreated(id);
        emit dataChanged();
    }

    return id;
}

int PromptService::createPrompt(const Prompt& prompt)
{
    int id = m_promptRepo.insert(prompt);
    if (id > 0) {
        emit promptCreated(id);
        emit dataChanged();
    }
    return id;
}

bool PromptService::updatePrompt(const Prompt& prompt)
{
    bool success = m_promptRepo.update(prompt);
    if (success) {
        emit promptUpdated(prompt.id);
        emit dataChanged();
    }

    return success;
}

bool PromptService::deletePrompt(int id)
{
    bool success = m_promptRepo.remove(id);
    if (success) {
        emit promptDeleted(id);
        emit dataChanged();
    }

    return success;
}

Prompt PromptService::getPrompt(int id)
{
    auto prompt = m_promptRepo.findById(id);
    return prompt.value_or(Prompt());
}

QVector<Prompt> PromptService::getAllPrompts()
{
    return m_promptRepo.findAllOrderByRating();
}

QVector<Prompt> PromptService::getPromptsByCategory(int categoryId)
{
    return m_promptRepo.findByCategoryId(categoryId);
}

QVector<Prompt> PromptService::getPromptsByTag(int tagId)
{
    return m_promptRepo.findByTagId(tagId);
}

QVector<Prompt> PromptService::searchPrompts(const QString& keyword)
{
    if (keyword.isEmpty()) {
        return getAllPrompts();
    }
    return m_promptRepo.search(keyword);
}

QVector<Prompt> PromptService::filterPrompts(const QString& keyword, int categoryId,
                                              int tagId, double minRating, bool favoritesOnly)
{
    return m_promptRepo.searchWithFilter(keyword, categoryId, tagId, minRating, favoritesOnly);
}

QVector<Prompt> PromptService::findFavorites()
{
    return m_promptRepo.findFavorites();
}

bool PromptService::setFavorite(int id, bool favorite)
{
    return m_promptRepo.setFavorite(id, favorite);
}

bool PromptService::setArchive(int id, bool archive)
{
    return m_promptRepo.setArchive(id, archive);
}

bool PromptService::updateRating(int id, double rating)
{
    return m_promptRepo.updateRating(id, rating);
}

void PromptService::incrementUsage(int id)
{
    m_promptRepo.incrementUsage(id);
}

bool PromptService::copyToClipboard(int id)
{
    auto prompt = m_promptRepo.findById(id);
    if (!prompt) {
        return false;
    }

    QApplication::clipboard()->setText(prompt->content);
    m_promptRepo.incrementUsage(id);
    return true;
}

int PromptService::getPromptCount()
{
    return m_promptRepo.count();
}

int PromptService::getPromptCountByCategory(int categoryId)
{
    return m_promptRepo.countByCategory(categoryId);
}

int PromptService::getFavoriteCount()
{
    return m_promptRepo.countFavorites();
}

double PromptService::getAverageRating()
{
    return m_promptRepo.getAverageRating();
}
