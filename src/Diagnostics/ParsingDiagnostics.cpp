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
        const Token& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken.Kind);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken.SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenError(
        const Token& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken.Kind) +
            ", expected " + CreateTokenKindStringWithArticle(expectedTokenKind);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken.SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedLiteralError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken.Kind) +
            ", expected a literal";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken.SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedNewError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken.Kind) +
            ", expected `" + SpecialIdent::New + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken.SrcLocation,
            message
        );

        return group;
    }

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "unexpected " + CreateTokenKindString(unexpectedToken.Kind) +
            ", expected a compound assignment operator";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            unexpectedToken.SrcLocation,
            message
        );

        return group;
    }

    auto CreateExpectedTraitError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "expected a trait"
        );

        return group;
    }

    auto CreateEmptyTypeParamsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty type parameters"
        );

        return group;
    }

    auto CreateEmptyTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty type arguments"
        );

        return group;
    }

    auto CreateExternInstanceFunctionError(
        const Token& externKeywordToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            externKeywordToken.SrcLocation,
            "extern instance function"
        );

        return group;
    }

    auto CreateUnknownModifierError(
        const Token& modifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            modifierToken.SrcLocation,
            "unknown modifier"
        );

        return group;
    }

    auto CreateForbiddenModifierError(
        const Token& modifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            modifierToken.SrcLocation,
            "forbidden modifier"
        );

        return group;
    }

    auto CreateEmptyModifiersError(
        const Token& colonColonToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            colonColonToken.SrcLocation,
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

    auto CreateMissingSelfModifierAfterStrongPtrError(
        const Token& strongPtrModifierToken
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        const std::string message =
            "missing `" +
            std::string{ TokenKindToKeywordMap.at(TokenKind::SelfKeyword) } +
            "` modifier after " + CreateTokenKindString(TokenKind::Asterisk);

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            strongPtrModifierToken.SrcLocation.CreateLast(),
            message
        );

        return group;
    }

    auto CreateUnconstrainedTypeParamError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unconstrained type parameter"
        );

        return group;
    }

    auto CreateConstrainedNonGenericSymbolError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "constrained non-generic symbol"
        );

        return group;
    }

    auto CreateEmptyConstraintsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "empty constraint list"
        );

        return group;
    }
}
