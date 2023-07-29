#pragma once

#include <string>
#include <unordered_map>

#include "TokenKind.hpp"

namespace Ace::SpecialIdent
{
    auto CreateAnonymous() -> std::string;
    auto CreateTemplate(const std::string& templateName) -> std::string;
    auto CreateCopyGlue(const std::string& typePartialSignature) -> std::string;
    auto CreateDropGlue(const std::string& typePartialSignature) -> std::string;

    inline constexpr const char* Self = "self";
    inline constexpr const char* Main = "main";
    inline constexpr const char* New  = "new";

    inline constexpr const char* Copy = "copy";
    inline constexpr const char* Drop = "drop";

    inline constexpr const char* Global = "$global";

    inline constexpr const char* Error = "$error";

    namespace Op
    {
        inline constexpr const char* UnaryPlus         = "$op_unary_plus";
        inline constexpr const char* UnaryNegation     = "$op_unary_negation";
        inline constexpr const char* OneComplement     = "$op_one_complement";
        inline constexpr const char* Multiplication    = "$op_multiplication";
        inline constexpr const char* Division          = "$op_division";
        inline constexpr const char* Remainder         = "$op_remainder";
        inline constexpr const char* Addition          = "$op_addition";
        inline constexpr const char* Subtraction       = "$op_subtraction";
        inline constexpr const char* RightShift        = "$op_right_shift";
        inline constexpr const char* LeftShift         = "$op_lefshift";
        inline constexpr const char* LessThan          = "$op_less_than";
        inline constexpr const char* GreaterThan       = "$op_greater_than";
        inline constexpr const char* LessThanEquals    = "$op_less_than_equals";
        inline constexpr const char* GreaterThanEquals = "$op_greater_than_equals";
        inline constexpr const char* Equals            = "$op_equals";
        inline constexpr const char* NotEquals         = "$op_noequals";
        inline constexpr const char* AND               = "$op_AND";
        inline constexpr const char* XOR               = "$op_XOR";
        inline constexpr const char* OR                = "$op_OR";
        inline constexpr const char* Copy              = "$op_copy";
        inline constexpr const char* Drop              = "$op_drop";

        inline const std::unordered_map<TokenKind, std::string> BinaryNameMap
        {
            { TokenKind::Asterisk, SpecialIdent::Op::Multiplication },
            { TokenKind::Slash, SpecialIdent::Op::Division },
            { TokenKind::Percent, SpecialIdent::Op::Remainder },
            { TokenKind::Plus, SpecialIdent::Op::Addition },
            { TokenKind::Minus, SpecialIdent::Op::Subtraction },
            { TokenKind::LessThan, SpecialIdent::Op::LessThan },
            { TokenKind::GreaterThan, SpecialIdent::Op::GreaterThan },
            { TokenKind::LessThanEquals, SpecialIdent::Op::LessThanEquals },
            { TokenKind::GreaterThanEquals, SpecialIdent::Op::GreaterThanEquals },
            { TokenKind::LessThanLessThan, SpecialIdent::Op::LeftShift },
            { TokenKind::GreaterThanGreaterThan, SpecialIdent::Op::RightShift },
            { TokenKind::EqualsEquals, SpecialIdent::Op::Equals },
            { TokenKind::ExclamationEquals, SpecialIdent::Op::NotEquals },
            { TokenKind::Caret, SpecialIdent::Op::XOR },
            { TokenKind::VerticalBar, SpecialIdent::Op::OR },
            { TokenKind::Ampersand, SpecialIdent::Op::AND },

            { TokenKind::PlusEquals, SpecialIdent::Op::Addition },
            { TokenKind::MinusEquals, SpecialIdent::Op::Subtraction },
            { TokenKind::AsteriskEquals, SpecialIdent::Op::Multiplication },
            { TokenKind::SlashEquals, SpecialIdent::Op::Division },
            { TokenKind::PercentEquals, SpecialIdent::Op::Remainder },
            { TokenKind::LessThanLessThanEquals, SpecialIdent::Op::LeftShift },
            { TokenKind::GreaterThanGreaterThanEquals, SpecialIdent::Op::RightShift },
            { TokenKind::CaretEquals, SpecialIdent::Op::XOR },
            { TokenKind::VerticalBarEquals, SpecialIdent::Op::OR },
            { TokenKind::AmpersandEquals, SpecialIdent::Op::AND },
        };

        inline const std::unordered_map<TokenKind, std::string> UnaryNameMap
        {
            { TokenKind::Plus, SpecialIdent::Op::UnaryPlus },
            { TokenKind::Minus, SpecialIdent::Op::UnaryNegation },
            { TokenKind::Tilde, SpecialIdent::Op::OneComplement },
        };
    }
}
