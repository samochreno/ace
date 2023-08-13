#pragma once

#include <memory>

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

    auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const Token& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateEmptyTemplateArgsError(
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
        const Token& minusGreaterThanToken
    ) -> DiagnosticGroup;

    auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup;

    auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedUnaryOpParamCountError(
        const Token& opToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedBinaryOpParamCountError(
        const Token& opToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const Token& opToken
    ) -> DiagnosticGroup;

    auto CreateUnknownIdentOpError(
        const Token& opToken
    ) -> DiagnosticGroup;

    auto CreateOpMustBePublicError(
        const Token& nameToken
    ) -> DiagnosticGroup;

    auto CreateInstanceOpError(
        const Token& nameToken
    ) -> DiagnosticGroup;

    auto CreateMissingSelfModifierAfterStrongPtrError(
        const Token& strongPtrModifierToken
    ) -> DiagnosticGroup;
}
