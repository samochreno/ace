#include "Noun.hpp"

#include <string>

namespace Ace
{
    auto ArticleToString(const Article article) -> std::string
    {
        switch (article)
        {
        case Article::A:
            return "a";

        case Article::An:
            return "an";

        case Article::The:
            return "the";

        default:
            return {};
        }
    }

    auto Noun::CreateStringWithArticle() const -> std::string
    {
        return ArticleToString(Article) + " " + String;
    }
}
