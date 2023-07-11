#include "Token.hpp"

#include <string_view>

namespace Ace
{
    auto operator==(const Token& t_token, const TokenKind t_kind) -> bool
    {
        return t_token.Kind == t_kind;
    }

    auto operator==(
        const Token& t_token,
        const std::string_view t_string
    ) -> bool
    {
        if (t_token.Kind != TokenKind::Identifier)
        {
            return false;
        }

        return t_token.String == t_string;
    }
}
