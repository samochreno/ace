#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Token.hpp"
#include "Keyword.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    inline auto CreateTokenKindString(
        const TokenKind tokenKind
    ) -> std::string
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
                return "ident";
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

            case TokenKind::IfKeyword:
            {
                return std::string{} + "`" + Keyword::If + "`";
            }
            case TokenKind::ElseKeyword:
            {
                return std::string{} + "`" + Keyword::Else + "`";
            }
            case TokenKind::ElifKeyword:
            {
                return std::string{} + "`" + Keyword::Elif + "`";
            }
            case TokenKind::WhileKeyword:
            {
                return std::string{} + "`" + Keyword::While + "`";
            }
            case TokenKind::ReturnKeyword:
            {
                return Keyword::Return;
            }
            case TokenKind::StructKeyword:
            {
                return std::string{} + "`" + Keyword::Struct + "`";
            }
            case TokenKind::OpKeyword:
            {
                return std::string{} + "`" + Keyword::Op + "`";
            }
            case TokenKind::PublicKeyword:
            {
                return std::string{} + "`" + Keyword::Public + "`";
            }
            case TokenKind::ExternKeyword:
            {
                return std::string{} + "`" + Keyword::Extern + "`";
            }
            case TokenKind::CastKeyword:
            {
                return std::string{} + "`" + Keyword::Cast + "`";
            }
            case TokenKind::ExitKeyword:
            {
                return std::string{} + "`" + Keyword::Exit + "`";
            }
            case TokenKind::AssertKeyword:
            {
                return std::string{} + "`" + Keyword::Assert + "`";
            }
            case TokenKind::ModuleKeyword:
            {
                return std::string{} + "`" + Keyword::Module + "`";
            }
            case TokenKind::ImplKeyword:
            {
                return std::string{} + "`" + Keyword::Impl + "`";
            }
            case TokenKind::AddressOfKeyword:
            {
                return std::string{} + "`" + Keyword::AddressOf + "`";
            }
            case TokenKind::SizeOfKeyword:
            {
                return std::string{} + "`" + Keyword::SizeOf + "`";
            }
            case TokenKind::DerefAsKeyword:
            {
                return std::string{} + "`" + Keyword::DerefAs + "`";
            }
            case TokenKind::BoxKeyword:
            {
                return std::string{} + "`" + Keyword::Box + "`";
            }
            case TokenKind::UnboxKeyword:
            {
                return std::string{} + "`" + Keyword::Unbox + "`";
            }
            case TokenKind::TrueKeyword:
            {
                return std::string{} + "`" + Keyword::True + "`";
            }
            case TokenKind::FalseKeyword:
            {
                return std::string{} + "`" + Keyword::False + "`";
            }
        }
    }

    inline auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind) + ", expected " +
            CreateTokenKindString(expectedTokenKind);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedTokenExpectedLiteralError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a literal";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedTokenExpectedNewError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind) +
            ", expected " + SpecialIdent::New;

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a compound assignment op";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " +
            CreateTokenKindString(unexpectedToken->Kind) +
            ", expected an overloadable op";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    inline auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template parameters"
        );
    }

    inline auto CreateEmptyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template arguments"
        );
    }

    inline auto CreateExternInstanceFunctionError(
        const std::shared_ptr<const Token>& externKeywordToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            externKeywordToken->SrcLocation,
            "extern method"
        );
    }

    inline auto CreateUnknownModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "unknown modifier"
        );
    }

    inline auto CreateForbiddenModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "forbidden modifier"
        );
    }

    inline auto CreateEmptyModifiersError(
        const std::shared_ptr<const Token>& minusGreaterThanToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            minusGreaterThanToken->SrcLocation,
            "empty function modifier list"
        );
    }

    inline auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> std::shared_ptr<const Diagnostic>
    {
        const SrcLocation srcLocation
        {
            lastTokenSrcLocation.Buffer,
            lastTokenSrcLocation.CharacterEndIterator - 1,
            lastTokenSrcLocation.CharacterEndIterator,
        };

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing " + CreateTokenKindString(expectedTokenKind)
        );
    }

    inline auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "template specialization"
        );
    }

    inline auto CreateOpString(
        const std::shared_ptr<const Token>& opToken
    ) -> std::string
    {
        if (opToken->Kind == TokenKind::Ident)
        {
            return "`" + opToken->String + "`";
        }

        return CreateTokenKindString(opToken->Kind);
    }

    inline auto CreateUnexpectedUnaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "op" + CreateOpString(opToken) +
            "must have 1 parameter";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "op" + CreateOpString(opToken) +
            "must have 2 parameters";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "op" + CreateOpString(opToken) +
            "must have 1 or 2 parameters";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    inline auto CreateUnknownIdentOpError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            "unknown op"
        );
    }

    inline auto CreateOpMustBePublicError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "op must be public"
        );
    }

    inline auto CreateInstanceOpError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "instance op"
        );
    }
}
