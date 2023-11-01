#pragma once

#include <string>

namespace Ace
{
    enum class Article
    {
        A,
        An,
        The,
    };

    auto ArticleToString(const Article article) -> std::string;

    struct Noun
    {
        auto CreateStringWithArticle() const -> std::string;

        Article Article{};
        std::string String{};
    };
}
