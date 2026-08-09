#include <cstddef>

#include "Assert.hpp"
#include "DiagnosticStringConversions.hpp"
#include "Keyword.hpp"
#include "TokenKind.hpp"

namespace
{
    auto IsKeywordTokenKind(const Ace::TokenKind tokenKind) -> bool
    {
        return (tokenKind >= Ace::TokenKind::AddressOfKeyword) &&
               (tokenKind <= Ace::TokenKind::VoidKeyword);
    }
}

auto main() -> int
{
    using namespace Ace;

    for (size_t index = 0; index < static_cast<size_t>(TokenKind::Count); ++index)
    {
        const auto tokenKind = static_cast<TokenKind>(index);
        ACE_ASSERT(!CreateTokenKindString(tokenKind).empty());

        const bool isKeyword = IsKeywordTokenKind(tokenKind);
        ACE_ASSERT(TokenKindToKeywordMap.contains(tokenKind) == isKeyword);
    }

    ACE_ASSERT(KeywordToTokenKindMap.size() == TokenKindToKeywordMap.size());

    for (const auto& [keyword, tokenKind] : KeywordToTokenKindMap)
    {
        const auto tokenKindIt = TokenKindToKeywordMap.find(tokenKind);
        ACE_ASSERT(tokenKindIt != end(TokenKindToKeywordMap));
        ACE_ASSERT(tokenKindIt->second == keyword);
    }

    for (const auto& [tokenKind, keyword] : TokenKindToKeywordMap)
    {
        ACE_ASSERT(IsKeywordTokenKind(tokenKind));

        const auto keywordIt = KeywordToTokenKindMap.find(keyword);
        ACE_ASSERT(keywordIt != end(KeywordToTokenKindMap));
        ACE_ASSERT(keywordIt->second == tokenKind);
    }
}
