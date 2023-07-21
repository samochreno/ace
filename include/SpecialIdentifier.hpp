#pragma once

#include <string>
#include <unordered_map>

#include "Symbols/Symbol.hpp"
#include "TokenKind.hpp"

namespace Ace::SpecialIdentifier
{
    auto CreateAnonymous() -> std::string;
    auto CreateTemplate(const std::string& templateName) -> std::string;
    auto CreateCopyGlue(const std::string& typePartialSignature) -> std::string;
    auto CreateDropGlue(const std::string& typePartialSignature) -> std::string;

    inline constexpr const char* Self   = "self";
    inline constexpr const char* Main   = "main";
    inline constexpr const char* New    = "new";

    inline constexpr const char* Copy   = "copy";
    inline constexpr const char* Drop   = "drop";

    inline constexpr const char* Global = "$global";

    namespace Op
    {
        inline constexpr const char* UnaryPlus          = "$op_unary_plus";
        inline constexpr const char* UnaryNegation      = "$op_unary_negation";
        inline constexpr const char* OneComplement      = "$op_one_complement";
        inline constexpr const char* Multiplication     = "$op_multiplication";
        inline constexpr const char* Division           = "$op_division";
        inline constexpr const char* Remainder          = "$op_remainder";
        inline constexpr const char* Addition           = "$op_addition";
        inline constexpr const char* Subtraction        = "$op_subtraction";
        inline constexpr const char* RightShift         = "$op_right_shift";
        inline constexpr const char* LeftShift          = "$op_lefshift";
        inline constexpr const char* LessThan           = "$op_less_than";
        inline constexpr const char* GreaterThan        = "$op_greater_than";
        inline constexpr const char* LessThanEquals     = "$op_less_than_equals";
        inline constexpr const char* GreaterThanEquals  = "$op_greater_than_equals";
        inline constexpr const char* Equals             = "$op_equals";
        inline constexpr const char* NotEquals          = "$op_noequals";
        inline constexpr const char* AND                = "$op_AND";
        inline constexpr const char* XOR                = "$op_XOR";
        inline constexpr const char* OR                 = "$op_OR";
        inline constexpr const char* Copy               = "$op_copy";
        inline constexpr const char* Drop               = "$op_drop";

        inline const std::unordered_map<TokenKind, std::string> BinaryNameMap
        {
            { TokenKind::Asterisk, SpecialIdentifier::Op::Multiplication },
            { TokenKind::Slash, SpecialIdentifier::Op::Division },
            { TokenKind::Percent, SpecialIdentifier::Op::Remainder },
            { TokenKind::Plus, SpecialIdentifier::Op::Addition },
            { TokenKind::Minus, SpecialIdentifier::Op::Subtraction },
            { TokenKind::LessThan, SpecialIdentifier::Op::LessThan },
            { TokenKind::GreaterThan, SpecialIdentifier::Op::GreaterThan },
            { TokenKind::LessThanEquals, SpecialIdentifier::Op::LessThanEquals },
            { TokenKind::GreaterThanEquals, SpecialIdentifier::Op::GreaterThanEquals },
            { TokenKind::LessThanLessThan, SpecialIdentifier::Op::LeftShift },
            { TokenKind::GreaterThanGreaterThan, SpecialIdentifier::Op::RightShift },
            { TokenKind::EqualsEquals, SpecialIdentifier::Op::Equals },
            { TokenKind::ExclamationEquals, SpecialIdentifier::Op::NotEquals },
            { TokenKind::Caret, SpecialIdentifier::Op::XOR },
            { TokenKind::VerticalBar, SpecialIdentifier::Op::OR },
            { TokenKind::Ampersand, SpecialIdentifier::Op::AND },

            { TokenKind::PlusEquals, SpecialIdentifier::Op::Addition },
            { TokenKind::MinusEquals, SpecialIdentifier::Op::Subtraction },
            { TokenKind::AsteriskEquals, SpecialIdentifier::Op::Multiplication },
            { TokenKind::SlashEquals, SpecialIdentifier::Op::Division },
            { TokenKind::PercentEquals, SpecialIdentifier::Op::Remainder },
            { TokenKind::LessThanLessThanEquals, SpecialIdentifier::Op::LeftShift },
            { TokenKind::GreaterThanGreaterThanEquals, SpecialIdentifier::Op::RightShift },
            { TokenKind::CaretEquals, SpecialIdentifier::Op::XOR },
            { TokenKind::VerticalBarEquals, SpecialIdentifier::Op::OR },
            { TokenKind::AmpersandEquals, SpecialIdentifier::Op::AND },
        };

        inline const std::unordered_map<TokenKind, std::string> UnaryNameMap
        {
            { TokenKind::Plus, SpecialIdentifier::Op::UnaryPlus },
            { TokenKind::Minus, SpecialIdentifier::Op::UnaryNegation },
            { TokenKind::Tilde, SpecialIdentifier::Op::OneComplement },
        };
    }
}
