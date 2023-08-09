#include "Diagnostics/ParsingDiagnostics.hpp"

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
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected " + CreateTokenKindStringWithArticle(expectedTokenKind);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedLiteralError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a literal";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedNewError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected `" + SpecialIdent::New + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected a compound assignment operator";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken->Kind) +
            ", expected an overloadable operator";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template parameters"
        );

        return group;
    }

    auto CreateEmptyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty template arguments"
        );

        return group;
    }

    auto CreateExternInstanceFunctionError(
        const std::shared_ptr<const Token>& externKeywordToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            externKeywordToken->SrcLocation,
            "extern instance function"
        );

        return group;
    }

    auto CreateUnknownModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "unknown modifier"
        );

        return group;
    }

    auto CreateForbiddenModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            modifierToken->SrcLocation,
            "forbidden modifier"
        );

        return group;
    }

    auto CreateEmptyModifiersError(
        const std::shared_ptr<const Token>& minusGreaterThanToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            minusGreaterThanToken->SrcLocation,
            "empty modifier list"
        );

        return group;
    }

    auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const SrcLocation srcLocation
        {
            lastTokenSrcLocation.Buffer,
            lastTokenSrcLocation.CharacterEndIterator - 1,
            lastTokenSrcLocation.CharacterEndIterator,
        };

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "missing " + CreateTokenKindString(expectedTokenKind)
        );

        return group;
    }

    auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "template specialization"
        );

        return group;
    }

    auto CreateUnexpectedUnaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = 
            "operator " + CreateOpString(opToken) + " must have 1 parameter";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = 
            "operator " + CreateOpString(opToken) + " must have 2 parameters";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = 
            "operator " + CreateOpString(opToken) +
            " must have 1 or 2 parameters";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnknownIdentOpError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            opToken->SrcLocation,
            "unknown operator"
        );

        return group;
    }

    auto CreateOpMustBePublicError(
        const std::shared_ptr<const Token>& nameToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "operator is not public"
        );

        return group;
    }

    auto CreateInstanceOpError(
        const std::shared_ptr<const Token>& nameToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            nameToken->SrcLocation,
            "instance operator"
        );

        return group;
    }

    auto CreateMissingSelfModifierAfterStrongPtrError(
        const std::shared_ptr<const Token>& strongPtrModifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message = std::string{} +
            "missing `" + SpecialIdent::Self + "` modifier after " +
            CreateTokenKindString(TokenKind::Asterisk);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            strongPtrModifierToken->SrcLocation.CreateLast(),
            message
        );

        return group;
    }
}
