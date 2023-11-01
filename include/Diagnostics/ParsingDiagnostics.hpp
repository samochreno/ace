#pragma once

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Token.hpp"

namespace Ace
{
    auto CreateUnexpectedTokenError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenError(
        const Token& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedLiteralError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedNewError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateEmptyTypeParamsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateEmptyTypeArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExternInstanceFunctionError(
        const Token& externKeywordToken
    ) -> DiagnosticGroup;

    auto CreateUnknownModifierError(
        const Token& modifierToken
    ) -> DiagnosticGroup;

    auto CreateForbiddenModifierError(
        const Token& modifierToken
    ) -> DiagnosticGroup;

    auto CreateEmptyModifiersError(
        const Token& colonColonToken
    ) -> DiagnosticGroup;

    auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup;

    auto CreateMissingSelfModifierAfterStrongPtrError(
        const Token& strongPtrModifierToken
    ) -> DiagnosticGroup;

    auto CreateUnconstrainedTypeParamError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateConstrainedNonGenericSymbolError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateEmptyConstraintsError(
        const Token& whereToken
    ) -> DiagnosticGroup;
}
