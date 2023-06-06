#pragma once

#include <string>
#include <bitset>
#include <optional>

#include "SourceLocation.hpp"

namespace Ace
{
    struct TokenKind
    {
        TokenKind() = delete;

        enum : uint8_t
        {
            None,
            EndOfFile,

            Colon,
            ColonColon,
            Semicolon,
            Comma,
            Exclamation,
            Tilde,
            Dot,
            MinusGreaterThan,

            OpenParen,
            CloseParen,
            OpenBrace,
            CloseBrace,
            OpenBracket,
            CloseBracket,

            Identifier,

            Int8,
            Int16,
            Int32,
            Int64,
            
            UInt8,
            UInt16,
            UInt32,
            UInt64,
            
            Int,

            Float32,
            Float64,

            ErrorNumber,

            String,
            Bool,

            Equals,
            EqualsEquals,
            ExclamationEquals,
            Plus,
            PlusEquals,
            Minus,
            MinusEquals,
            Asterisk,
            AsteriskEquals,
            Slash,
            SlashEquals,
            Percent,
            LessThan,
            GreaterThan,
            LessThanEquals,
            GreaterThanEquals,
            LessThanLessThan,
            GreaterThanGreaterThan,

            Caret,
            Ampersand,
            VerticalBar,
            AmpersandAmpersand,
            VerticalBarVerticalBar,
            PercentEquals,
            LessThanLessThanEquals,
            GreaterThanGreaterThanEquals,
            AmpersandEquals,
            CaretEquals,
            VerticalBarEquals,

            IfKeyword,
            ElseKeyword,
            ElifKeyword,
            WhileKeyword,
            ReturnKeyword,
            StructKeyword,
            OperatorKeyword,
            PublicKeyword,
            ExternKeyword,
            CastKeyword,
            ExitKeyword,
            AssertKeyword,
            ModuleKeyword,
            ImplKeyword,
            ExplKeyword,
            AddressOfKeyword,
            SizeOfKeyword,
            DerefAsKeyword,
            BoxKeyword,
            UnboxKeyword,
            TrueKeyword,
            FalseKeyword,

            Length,
        };

        using Set = std::bitset<static_cast<size_t>(Length)>;

        static inline auto New() -> Set
        {
            return Set{};
        }

        static inline auto New(const uint8_t& t_kind) -> Set
        {
            return Set{}.set(t_kind);
        }
    };

    struct Token
    {
        Token(
        ) : String{},
            OptSourceLocation{},
            Kind{}
        {
        }
        Token(
            const std::optional<SourceLocation>& t_optSourceLocation,
            const TokenKind::Set& t_kind
        ) : OptSourceLocation{ t_optSourceLocation },
            Kind{ t_kind },
            String{}
            
        {
        }
        Token(
            const std::optional<SourceLocation>& t_optSourceLocation,
            const TokenKind::Set& t_kind,
            const std::string& t_string
        ) : OptSourceLocation{ t_optSourceLocation },
            Kind{ t_kind },
            String{ t_string }
        {
        }

        std::optional<SourceLocation> OptSourceLocation{};
        TokenKind::Set Kind{};
        std::string String{};
    };
}
