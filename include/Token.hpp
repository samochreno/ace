#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "SrcLocation.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    struct Token
    {
        Token(
        ) : String{},
            SrcLocation{},
            Kind{}
        {
        }
        Token(
            const SrcLocation& srcLocation,
            const TokenKind kind
        ) : SrcLocation{ srcLocation },
            Kind{ kind },
            String{}
            
        {
        }
        Token(
            const SrcLocation& srcLocation,
            const TokenKind kind,
            const std::string& string
        ) : SrcLocation{ srcLocation },
            Kind{ kind },
            String{ string }
        {
        }

        SrcLocation SrcLocation{};
        TokenKind Kind{};
        std::string String{};
    };

    auto operator==(const Token& token, const TokenKind kind) -> bool;
    auto operator==(
        const Token& token,
        const std::string_view string
    ) -> bool;
    template<typename T>
    auto operator!=(const Token& token, T&& value) -> bool
    {
        return !(token == value);
    }
}
