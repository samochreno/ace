#include "Diagnostics/ParseDiagnostics.hpp"

#include <memory>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"
#include "Token.hpp"
#include "Keyword.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected " + CreateTokenKindStringWithArticle(expectedTokenKind);

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedTokenExpectedLiteralError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a literal";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedTokenExpectedNewError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected `" + SpecialIdent::New + "`";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a compound assignment operator";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected an overloadable operator";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );
    }

    auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template parameters"
        );
    }

    auto CreateEmptyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template arguments"
        );
    }

    auto CreateExternInstanceFunctionError(
        const std::shared_ptr<const Token>& externKeywordToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            externKeywordToken->SrcLocation,
            "extern method"
        );
    }

    auto CreateUnknownModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "unknown modifier"
        );
    }

    auto CreateForbiddenModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "forbidden modifier"
        );
    }

    auto CreateEmptyModifiersError(
        const std::shared_ptr<const Token>& minusGreaterThanToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            minusGreaterThanToken->SrcLocation,
            "empty function modifier list"
        );
    }

    auto CreateMissingTokenError(
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

    auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "template specialization"
        );
    }

    auto CreateUnexpectedUnaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "operator " + CreateOpString(opToken) + " must have 1 parameter";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "operator " + CreateOpString(opToken) + " must have 2 parameters";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        const std::string message = 
            "operator " + CreateOpString(opToken) +
            " must have 1 or 2 parameters";

        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );
    }

    auto CreateUnknownIdentOpError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            "unknown operator"
        );
    }

    auto CreateOpMustBePublicError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "operator must be public"
        );
    }

    auto CreateInstanceOpError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "instance operator"
        );
    }
}
