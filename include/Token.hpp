#pragma once

#include <string>
#include <bitset>

namespace Ace
{
    struct Token
    {
        struct Kind
        {
            Kind() = delete;

            enum : uint8_t
            {
                End,

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

        Token(const uint8_t& t_kind) = delete;

        Token()
            : Token{ {}, std::string{} }
        {
        }

        Token(
            const Token::Kind::Set& t_kind
        ) : TokenKind{ t_kind },
            String{}
        {
        }

        Token(
            const Token::Kind::Set& t_kind,
            const std::string::const_iterator& t_begin,
            const std::string::const_iterator& t_end
        ) : TokenKind{ t_kind },
            String{ t_begin, t_end }
        {
        }

        Token(
            const std::string& t_string
        ) : TokenKind{ Token::Kind::New(Token::Kind::Identifier) },
            String{ t_string }
        {
        }

        Token(
            const Token::Kind::Set& t_kind,
            const std::string& t_string
        ) : TokenKind{ t_kind },
            String{ t_string }
        {
        }

        Token::Kind::Set TokenKind{};
        std::string String{};
    };
}
