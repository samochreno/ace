#pragma once

#include <string_view>
#include <unordered_map>

#include "TokenKind.hpp"
#include "Compilation.hpp"
#include "Symbols/Types/TypeSymbol.hpp"

namespace Ace
{
    auto CreateKeywordToTokenKindMap() -> std::unordered_map<std::string_view, TokenKind>;
    auto CreateTokenToKeywordKindMap() -> std::unordered_map<TokenKind, std::string_view>;

    const std::unordered_map<std::string_view, TokenKind> KeywordToTokenKindMap = CreateKeywordToTokenKindMap();
    const std::unordered_map<TokenKind, std::string_view> TokenKindToKeywordMap = CreateTokenToKeywordKindMap();

    auto GetTokenKindNativeTypeSymbol(
        Compilation* const compilation,
        const TokenKind tokenKind
    ) -> ITypeSymbol*;
}
