#pragma once

#include "model/prompt.h"
#include <QVector>
#include <optional>
#include <functional>

class PromptRepository {
public:
    PromptRepository();

    int insert(const Prompt& prompt);
    bool update(const Prompt& prompt);
    bool remove(int id);
    std::optional<Prompt> findById(int id);
    QVector<Prompt> findAll();
    QVector<Prompt> findAllOrderByRating();
    QVector<Prompt> findAllOrderByUpdated();
    QVector<Prompt> findByCategoryId(int categoryId);
    QVector<Prompt> findByTagId(int tagId);
    QVector<Prompt> search(const QString& keyword);
    QVector<Prompt> searchWithFilter(const QString& keyword, int categoryId,
                                      int tagId, double minRating, bool favoritesOnly);
    QVector<Prompt> findFavorites();
    QVector<Prompt> findArchived();

    int count();
    int countByCategory(int categoryId);
    int countFavorites();

    bool addTag(int promptId, int tagId);
    bool removeTag(int promptId, int tagId);
    bool removeAllTags(int promptId);
    QVector<int> findTagIds(int promptId);

    bool updateRating(int promptId, double rating);
    bool setFavorite(int id, bool favorite);
    bool setArchive(int id, bool archive);
    bool incrementUsage(int id);

    double getAverageRating();
    QVector<Prompt> findTopRated(int limit = 10);
    QVector<Prompt> findRecent(int limit = 10);

private:
    Prompt promptFromQuery(class QSqlQuery& query);
    void loadTagsForPrompt(Prompt& prompt);
};
