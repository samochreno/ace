#pragma once

#include <string>
#include <unordered_map>

#include "Symbol/Base.hpp"
#include "Token.hpp"

namespace Ace::SpecialIdentifier
{
    auto CreateAnonymous() -> std::string;
    auto CreateTemplate(const std::string& t_templateName) -> std::string;
    auto CreateCopyGlue(const std::string& t_typePartialSignature) -> std::string;
    auto CreateDropGlue(const std::string& t_typePartialSignature) -> std::string;

    inline constexpr const char* Self   = "self";
    inline constexpr const char* Main   = "main";
    inline constexpr const char* New    = "new";

    inline constexpr const char* Copy   = "copy";
    inline constexpr const char* Drop   = "drop";

    inline constexpr const char* Global = "$global";

    namespace Operator
    {
        inline constexpr const char* UnaryPlus          = "$operator_unary_plus";
        inline constexpr const char* UnaryNegation      = "$operator_unary_negation";
        inline constexpr const char* OneComplement      = "$operator_one_complement";
        inline constexpr const char* Multiplication     = "$operator_multiplication";
        inline constexpr const char* Division           = "$operator_division";
        inline constexpr const char* Remainder          = "$operator_remainder";
        inline constexpr const char* Addition           = "$operator_addition";
        inline constexpr const char* Subtraction        = "$operator_subtraction";
        inline constexpr const char* RightShift         = "$operator_right_shift";
        inline constexpr const char* LeftShift          = "$operator_left_shift";
        inline constexpr const char* LessThan           = "$operator_less_than";
        inline constexpr const char* GreaterThan        = "$operator_greater_than";
        inline constexpr const char* LessThanEquals     = "$operator_less_than_equals";
        inline constexpr const char* GreaterThanEquals  = "$operator_greater_than_equals";
        inline constexpr const char* Equals             = "$operator_equals";
        inline constexpr const char* NotEquals          = "$operator_not_equals";
        inline constexpr const char* AND                = "$operator_AND";
        inline constexpr const char* XOR                = "$operator_XOR";
        inline constexpr const char* OR                 = "$operator_OR";
        inline constexpr const char* Copy               = "$operator_copy";
        inline constexpr const char* Drop               = "$operator_drop";
        inline constexpr const char* ImplicitFrom       = "$operator_implicit_from";
        inline constexpr const char* ExplicitFrom       = "$operator_explicit_from";

        inline const std::unordered_map<TokenKind::Set, std::string> BinaryNameMap
        {
            { TokenKind::New(TokenKind::Asterisk), SpecialIdentifier::Operator::Multiplication },
            { TokenKind::New(TokenKind::Slash), SpecialIdentifier::Operator::Division },
            { TokenKind::New(TokenKind::Percent), SpecialIdentifier::Operator::Remainder },
            { TokenKind::New(TokenKind::Plus), SpecialIdentifier::Operator::Addition },
            { TokenKind::New(TokenKind::Minus), SpecialIdentifier::Operator::Subtraction },
            { TokenKind::New(TokenKind::LessThan), SpecialIdentifier::Operator::LessThan },
            { TokenKind::New(TokenKind::GreaterThan), SpecialIdentifier::Operator::GreaterThan },
            { TokenKind::New(TokenKind::LessThanEquals), SpecialIdentifier::Operator::LessThanEquals },
            { TokenKind::New(TokenKind::GreaterThanEquals), SpecialIdentifier::Operator::GreaterThanEquals },
            { TokenKind::New(TokenKind::LessThanLessThan), SpecialIdentifier::Operator::LeftShift },
            { TokenKind::New(TokenKind::GreaterThanGreaterThan), SpecialIdentifier::Operator::RightShift },
            { TokenKind::New(TokenKind::EqualsEquals), SpecialIdentifier::Operator::Equals },
            { TokenKind::New(TokenKind::ExclamationEquals), SpecialIdentifier::Operator::NotEquals },
            { TokenKind::New(TokenKind::Caret), SpecialIdentifier::Operator::XOR },
            { TokenKind::New(TokenKind::VerticalBar), SpecialIdentifier::Operator::OR },
            { TokenKind::New(TokenKind::Ampersand), SpecialIdentifier::Operator::AND },

            { TokenKind::New(TokenKind::PlusEquals), SpecialIdentifier::Operator::Addition },
            { TokenKind::New(TokenKind::MinusEquals), SpecialIdentifier::Operator::Subtraction },
            { TokenKind::New(TokenKind::AsteriskEquals), SpecialIdentifier::Operator::Multiplication },
            { TokenKind::New(TokenKind::SlashEquals), SpecialIdentifier::Operator::Division },
            { TokenKind::New(TokenKind::PercentEquals), SpecialIdentifier::Operator::Remainder },
            { TokenKind::New(TokenKind::LessThanLessThanEquals), SpecialIdentifier::Operator::LeftShift },
            { TokenKind::New(TokenKind::GreaterThanGreaterThanEquals), SpecialIdentifier::Operator::RightShift },
            { TokenKind::New(TokenKind::CaretEquals), SpecialIdentifier::Operator::XOR },
            { TokenKind::New(TokenKind::VerticalBarEquals), SpecialIdentifier::Operator::OR },
            { TokenKind::New(TokenKind::AmpersandEquals), SpecialIdentifier::Operator::AND },
        };

        inline const std::unordered_map<TokenKind::Set, std::string> UnaryNameMap
        {
            { TokenKind::New(TokenKind::Plus), SpecialIdentifier::Operator::UnaryPlus },
            { TokenKind::New(TokenKind::Minus), SpecialIdentifier::Operator::UnaryNegation },
            { TokenKind::New(TokenKind::Tilde), SpecialIdentifier::Operator::OneComplement },
        };
    }
}
