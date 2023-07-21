#include "Token.hpp"

#include <string_view>

namespace Ace
{
    auto operator==(const Token& token, const TokenKind kind) -> bool
    {
        return token.Kind == kind;
    }

    auto operator==(
        const Token& token,
        const std::string_view string
    ) -> bool
    {
        if (token.Kind != TokenKind::Identifier)
        {
            return false;
        }

        return token.String == string;
    }
}
