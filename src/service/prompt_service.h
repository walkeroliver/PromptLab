#pragma once

#include "model/prompt.h"
#include "repository/prompt_repository.h"
#include "repository/tag_repository.h"
#include <QObject>
#include <QVector>
#include <functional>

class PromptService : public QObject {
    Q_OBJECT

public:
    explicit PromptService(QObject* parent = nullptr);

    int createPrompt(const QString& title, const QString& content,
                     const QString& description, int categoryId,
                     const QVector<int>& tagIds, double rating);
    int createPrompt(const Prompt& prompt);
    bool updatePrompt(const Prompt& prompt);
    bool deletePrompt(int id);
    Prompt getPrompt(int id);
    QVector<Prompt> getAllPrompts();
    QVector<Prompt> getPromptsByCategory(int categoryId);
    QVector<Prompt> getPromptsByTag(int tagId);
    QVector<Prompt> searchPrompts(const QString& keyword);
    QVector<Prompt> filterPrompts(const QString& keyword, int categoryId,
                                   int tagId, double minRating, bool favoritesOnly);
    QVector<Prompt> findFavorites();

    bool setFavorite(int id, bool favorite);
    bool setArchive(int id, bool archive);
    bool updateRating(int id, double rating);
    void incrementUsage(int id);
    bool copyToClipboard(int id);

    int getPromptCount();
    int getPromptCountByCategory(int categoryId);
    int getFavoriteCount();
    double getAverageRating();

signals:
    void promptCreated(int id);
    void promptUpdated(int id);
    void promptDeleted(int id);
    void dataChanged();

private:
    PromptRepository m_promptRepo;
    TagRepository m_tagRepo;
};
