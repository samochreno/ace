#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "SourceLocation.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    struct Token
    {
        Token(
        ) : String{},
            SourceLocation{},
            Kind{}
        {
        }
        Token(
            const SourceLocation& t_sourceLocation,
            const TokenKind t_kind
        ) : SourceLocation{ t_sourceLocation },
            Kind{ t_kind },
            String{}
            
        {
        }
        Token(
            const SourceLocation& t_sourceLocation,
            const TokenKind t_kind,
            const std::string& t_string
        ) : SourceLocation{ t_sourceLocation },
            Kind{ t_kind },
            String{ t_string }
        {
        }

        SourceLocation SourceLocation{};
        TokenKind Kind{};
        std::string String{};
    };

    auto operator==(const Token& t_token, const TokenKind t_kind) -> bool;
    auto operator==(
        const Token& t_token,
        const std::string_view t_string
    ) -> bool;
    template<typename T>
    auto operator!=(const Token& t_token, T&& t_value) -> bool
    {
        return !(t_token == t_value);
    }
    template<typename T>
    auto operator==(
        const std::shared_ptr<const Token>& t_token,
        T&& t_value
    ) -> bool
    {
        return (*t_token.get()) == t_value;
    }
    template<typename T>
    auto operator!=(
        const std::shared_ptr<const Token>& t_token,
        T&& t_value
    ) -> bool
    {
        return !(t_token == t_value);
    }
}
