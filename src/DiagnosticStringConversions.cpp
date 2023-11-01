#include "DiagnosticStringConversions.hpp"

#include <string>
#include <nlohmann/json.hpp>

#include "Assert.hpp"
#include "TokenKind.hpp"
#include "Keyword.hpp"
#include "AccessModifier.hpp"
#include "SymbolCategory.hpp"
#include "TokenKind.hpp"

namespace Ace
{
    auto CreateTokenKindString(const TokenKind tokenKind) -> std::string
    {
        switch (tokenKind)
        {
            case TokenKind::EndOfFile:
            {
                return "end of file";
            }

            case TokenKind::Colon:
            {
                return "`:`";
            }
            case TokenKind::ColonColon:
            {
                return "`::`";
            }
            case TokenKind::Semicolon:
            {
                return "`;`";
            }
            case TokenKind::Comma:
            {
                return "`,`";
            }
            case TokenKind::Exclamation:
            {
                return "`!`";
            }
            case TokenKind::Tilde:
            {
                return "`~`";
            }
            case TokenKind::Dot:
            {
                return "`.`";
            }
            case TokenKind::MinusGreaterThan:
            {
                return "`->`";
            }

            case TokenKind::OpenParen:
            {
                return "`(`";
            }
            case TokenKind::CloseParen:
            {
                return "`)`";
            }
            case TokenKind::OpenBrace:
            {
                return "`{`";
            }
            case TokenKind::CloseBrace:
            {
                return "`}`";
            }
            case TokenKind::OpenBracket:
            {
                return "`[`";
            }
            case TokenKind::CloseBracket:
            {
                return "`]`";
            }

            case TokenKind::Ident:
            {
                return "identifier";
            }

            case TokenKind::Int8:
            {
                return "`i8` literal";
            }
            case TokenKind::Int16:
            {
                return "`i16` literal";
            }
            case TokenKind::Int32:
            {
                return "`i32` literal";
            }
            case TokenKind::Int64:
            {
                return "`i64` literal";
            }
        
            case TokenKind::UInt8:
            {
                return "`u8` literal";
            }
            case TokenKind::UInt16:
            {
                return "`u16` literal";
            }
            case TokenKind::UInt32:
            {
                return "`u32` literal";
            }
            case TokenKind::UInt64:
            {
                return "`u64` literal";
            }
        
            case TokenKind::Int:
            {
                return "`int` literal";
            }

            case TokenKind::Float32:
            {
                return "`f32` literal";
            }
            case TokenKind::Float64:
            {
                return "`f64` literal";
            }

            case TokenKind::String:
            {
                return "`string` literal";
            }
            case TokenKind::Bool:
            {
                return "`bool` literal";
            }

            case TokenKind::Equals:
            {
                return "`=`";
            }
            case TokenKind::EqualsEquals:
            {
                return "`==`";
            }
            case TokenKind::ExclamationEquals:
            {
                return "`!=`";
            }
            case TokenKind::Plus:
            {
                return "`+`";
            }
            case TokenKind::PlusEquals:
            {
                return "`+=`";
            }
            case TokenKind::Minus:
            {
                return "`-`";
            }
            case TokenKind::MinusEquals:
            {
                return "`-=`";
            }
            case TokenKind::Asterisk:
            {
                return "`*`";
            }
            case TokenKind::AsteriskEquals:
            {
                return "`*=`";
            }
            case TokenKind::Slash:
            {
                return "`/`";
            }
            case TokenKind::SlashEquals:
            {
                return "`/=`";
            }
            case TokenKind::Percent:
            {
                return "`%`";
            }
            case TokenKind::LessThan:
            {
                return "`<`";
            }
            case TokenKind::GreaterThan:
            {
                return "`>`";
            }
            case TokenKind::LessThanEquals:
            {
                return "`<=`";
            }
            case TokenKind::GreaterThanEquals:
            {
                return "`>=`";
            }
            case TokenKind::LessThanLessThan:
            {
                return "`<<`";
            }
            case TokenKind::GreaterThanGreaterThan:
            {
                return "`>>`";
            }

            case TokenKind::Caret:
            {
                return "`^`";
            }
            case TokenKind::Ampersand:
            {
                return "`&`";
            }
            case TokenKind::VerticalBar:
            {
                return "`|`";
            }
            case TokenKind::AmpersandAmpersand:
            {
                return "`&&`";
            }
            case TokenKind::VerticalBarVerticalBar:
            {
                return "`||`";
            }
            case TokenKind::PercentEquals:
            {
                return "`%=`";
            }
            case TokenKind::LessThanLessThanEquals:
            {
                return "`<<=`";
            }
            case TokenKind::GreaterThanGreaterThanEquals:
            {
                return "`>>=`";
            }
            case TokenKind::AmpersandEquals:
            {
                return "`&=`";
            }
            case TokenKind::CaretEquals:
            {
                return "`^=`";
            }
            case TokenKind::VerticalBarEquals:
            {
                return "`|=`";
            }

            default:
            {
                const auto keywordTokenKindIt = TokenKindToKeywordMap.find(
                    tokenKind
                );
                ACE_ASSERT(keywordTokenKindIt != end(TokenKindToKeywordMap));
                return "`" + std::string{ keywordTokenKindIt->second } + "`";
            }
        }
    }

    auto CreateTokenKindStringWithArticle(
        const TokenKind tokenKind
    ) -> std::string
    {
        switch (tokenKind)
        {
            case TokenKind::EndOfFile:
            {
                return "the end of the file";
            }

            case TokenKind::UInt8:
            case TokenKind::UInt16:
            case TokenKind::UInt32:
            case TokenKind::UInt64:
            case TokenKind::String:
            case TokenKind::Bool:
            {
                return "a " + CreateTokenKindString(tokenKind);
            }

            case TokenKind::Ident:
            case TokenKind::Int8:
            case TokenKind::Int16:
            case TokenKind::Int32:
            case TokenKind::Int64:
            case TokenKind::Int:
            case TokenKind::Float32:
            case TokenKind::Float64:
            {
                return "an " + CreateTokenKindString(tokenKind);
            }

            default:
            {
                return CreateTokenKindString(tokenKind);
            }
        }
    }

    auto CreateAccessModifierString(
        const AccessModifier accessModifier
    ) -> std::string
    {
        switch (accessModifier)
        {
            case AccessModifier::Priv:
            {
                return "private";
            }

            case AccessModifier::Pub:
            {
                return "public";
            }
        }
    }

    auto CreateJsonTypeString(
        const nlohmann::json::value_t type
    ) -> std::string
    {
        switch (type)
        {
            case nlohmann::json::value_t::null:
            {
                return "null";
            }

            case nlohmann::json::value_t::object:
            {
                return "object";
            }

            case nlohmann::json::value_t::array:
            {
                return "array";
            }

            case nlohmann::json::value_t::string:
            {
                return "string";
            }

            case nlohmann::json::value_t::boolean:
            {
                return "boolean";
            }

            case nlohmann::json::value_t::number_integer:
            {
                return "signed integer";
            }

            case nlohmann::json::value_t::number_unsigned:
            {
                return "unsigned integer";
            }

            case nlohmann::json::value_t::number_float:
            {
                return "float";
            }

            case nlohmann::json::value_t::binary:
            {
                return "binary array";
            }

            case nlohmann::json::value_t::discarded:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    auto CreateJsonTypeStringWithArticle(
        const nlohmann::json::value_t type
    ) -> std::string
    {
        switch (type)
        {
            case nlohmann::json::value_t::null:
            case nlohmann::json::value_t::string:
            case nlohmann::json::value_t::boolean:
            case nlohmann::json::value_t::number_integer:
            case nlohmann::json::value_t::number_float:
            case nlohmann::json::value_t::binary:
            {
                return "a " + CreateJsonTypeString(type);
            }

            case nlohmann::json::value_t::number_unsigned:
            case nlohmann::json::value_t::array:
            case nlohmann::json::value_t::object:
            {
                return "an " + CreateJsonTypeString(type);
            }

            case nlohmann::json::value_t::discarded:
            {
                ACE_UNREACHABLE();
            }
        }
    }

    auto CreateSymbolCategoryStringWithArticle(
        const SymbolCategory symbolCategory
    ) -> std::string
    {
        switch (symbolCategory)
        {
            case SymbolCategory::Instance:
            {
                return "an instance";
            }

            case SymbolCategory::Static:
            {
                return "a static";
            }
        }
    }

    auto CreateOpString(const Token& opToken) -> std::string
    {
        if (opToken.Kind == TokenKind::Ident)
        {
            return "`" + opToken.String + "`";
        }

        return CreateTokenKindString(opToken.Kind);
    }
}
