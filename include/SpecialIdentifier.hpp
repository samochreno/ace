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

        inline const std::unordered_map<Token::Kind::Set, std::string> BinaryNameMap
        {
            { Token::Kind::New(Token::Kind::Asterisk), SpecialIdentifier::Operator::Multiplication },
            { Token::Kind::New(Token::Kind::Slash), SpecialIdentifier::Operator::Division },
            { Token::Kind::New(Token::Kind::Percent), SpecialIdentifier::Operator::Remainder },
            { Token::Kind::New(Token::Kind::Plus), SpecialIdentifier::Operator::Addition },
            { Token::Kind::New(Token::Kind::Minus), SpecialIdentifier::Operator::Subtraction },
            { Token::Kind::New(Token::Kind::LessThan), SpecialIdentifier::Operator::LessThan },
            { Token::Kind::New(Token::Kind::GreaterThan), SpecialIdentifier::Operator::GreaterThan },
            { Token::Kind::New(Token::Kind::LessThanEquals), SpecialIdentifier::Operator::LessThanEquals },
            { Token::Kind::New(Token::Kind::GreaterThanEquals), SpecialIdentifier::Operator::GreaterThanEquals },
            { Token::Kind::New(Token::Kind::LessThanLessThan), SpecialIdentifier::Operator::LeftShift },
            { Token::Kind::New(Token::Kind::GreaterThanGreaterThan), SpecialIdentifier::Operator::RightShift },
            { Token::Kind::New(Token::Kind::EqualsEquals), SpecialIdentifier::Operator::Equals },
            { Token::Kind::New(Token::Kind::ExclamationEquals), SpecialIdentifier::Operator::NotEquals },
            { Token::Kind::New(Token::Kind::Caret), SpecialIdentifier::Operator::XOR },
            { Token::Kind::New(Token::Kind::VerticalBar), SpecialIdentifier::Operator::OR },
            { Token::Kind::New(Token::Kind::Ampersand), SpecialIdentifier::Operator::AND },

            { Token::Kind::New(Token::Kind::PlusEquals), SpecialIdentifier::Operator::Addition },
            { Token::Kind::New(Token::Kind::MinusEquals), SpecialIdentifier::Operator::Subtraction },
            { Token::Kind::New(Token::Kind::AsteriskEquals), SpecialIdentifier::Operator::Multiplication },
            { Token::Kind::New(Token::Kind::SlashEquals), SpecialIdentifier::Operator::Division },
            { Token::Kind::New(Token::Kind::PercentEquals), SpecialIdentifier::Operator::Remainder },
            { Token::Kind::New(Token::Kind::LessThanLessThanEquals), SpecialIdentifier::Operator::LeftShift },
            { Token::Kind::New(Token::Kind::GreaterThanGreaterThanEquals), SpecialIdentifier::Operator::RightShift },
            { Token::Kind::New(Token::Kind::CaretEquals), SpecialIdentifier::Operator::XOR },
            { Token::Kind::New(Token::Kind::VerticalBarEquals), SpecialIdentifier::Operator::OR },
            { Token::Kind::New(Token::Kind::AmpersandEquals), SpecialIdentifier::Operator::AND },
        };

        inline const std::unordered_map<Token::Kind::Set, std::string> UnaryNameMap
        {
            { Token::Kind::New(Token::Kind::Plus), SpecialIdentifier::Operator::UnaryPlus },
            { Token::Kind::New(Token::Kind::Minus), SpecialIdentifier::Operator::UnaryNegation },
            { Token::Kind::New(Token::Kind::Tilde), SpecialIdentifier::Operator::OneComplement },
        };
    }
}
