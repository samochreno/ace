#pragma once

#include <string>
#include <unordered_map>

#include "Symbols/Symbol.hpp"
#include "TokenKind.hpp"

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

        inline const std::unordered_map<TokenKind, std::string> BinaryNameMap
        {
            { TokenKind::Asterisk, SpecialIdentifier::Operator::Multiplication },
            { TokenKind::Slash, SpecialIdentifier::Operator::Division },
            { TokenKind::Percent, SpecialIdentifier::Operator::Remainder },
            { TokenKind::Plus, SpecialIdentifier::Operator::Addition },
            { TokenKind::Minus, SpecialIdentifier::Operator::Subtraction },
            { TokenKind::LessThan, SpecialIdentifier::Operator::LessThan },
            { TokenKind::GreaterThan, SpecialIdentifier::Operator::GreaterThan },
            { TokenKind::LessThanEquals, SpecialIdentifier::Operator::LessThanEquals },
            { TokenKind::GreaterThanEquals, SpecialIdentifier::Operator::GreaterThanEquals },
            { TokenKind::LessThanLessThan, SpecialIdentifier::Operator::LeftShift },
            { TokenKind::GreaterThanGreaterThan, SpecialIdentifier::Operator::RightShift },
            { TokenKind::EqualsEquals, SpecialIdentifier::Operator::Equals },
            { TokenKind::ExclamationEquals, SpecialIdentifier::Operator::NotEquals },
            { TokenKind::Caret, SpecialIdentifier::Operator::XOR },
            { TokenKind::VerticalBar, SpecialIdentifier::Operator::OR },
            { TokenKind::Ampersand, SpecialIdentifier::Operator::AND },

            { TokenKind::PlusEquals, SpecialIdentifier::Operator::Addition },
            { TokenKind::MinusEquals, SpecialIdentifier::Operator::Subtraction },
            { TokenKind::AsteriskEquals, SpecialIdentifier::Operator::Multiplication },
            { TokenKind::SlashEquals, SpecialIdentifier::Operator::Division },
            { TokenKind::PercentEquals, SpecialIdentifier::Operator::Remainder },
            { TokenKind::LessThanLessThanEquals, SpecialIdentifier::Operator::LeftShift },
            { TokenKind::GreaterThanGreaterThanEquals, SpecialIdentifier::Operator::RightShift },
            { TokenKind::CaretEquals, SpecialIdentifier::Operator::XOR },
            { TokenKind::VerticalBarEquals, SpecialIdentifier::Operator::OR },
            { TokenKind::AmpersandEquals, SpecialIdentifier::Operator::AND },
        };

        inline const std::unordered_map<TokenKind, std::string> UnaryNameMap
        {
            { TokenKind::Plus, SpecialIdentifier::Operator::UnaryPlus },
            { TokenKind::Minus, SpecialIdentifier::Operator::UnaryNegation },
            { TokenKind::Tilde, SpecialIdentifier::Operator::OneComplement },
        };
    }
}
