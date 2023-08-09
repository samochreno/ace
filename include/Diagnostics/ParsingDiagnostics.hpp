#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Token.hpp"

namespace Ace
{
    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedLiteralError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedNewError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> DiagnosticGroup;

    auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateEmptyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateExternInstanceFunctionError(
        const std::shared_ptr<const Token>& externKeywordToken
    ) -> DiagnosticGroup;

    auto CreateUnknownModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> DiagnosticGroup;

    auto CreateForbiddenModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> DiagnosticGroup;

    auto CreateEmptyModifiersError(
        const std::shared_ptr<const Token>& minusGreaterThanToken
    ) -> DiagnosticGroup;

    auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> DiagnosticGroup;

    auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> DiagnosticGroup;

    auto CreateUnexpectedUnaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup;

    auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup;

    auto CreateUnknownIdentOpError(
        const std::shared_ptr<const Token>& opToken
    ) -> DiagnosticGroup;

    auto CreateOpMustBePublicError(
        const std::shared_ptr<const Token>& nameToken
    ) -> DiagnosticGroup;

    auto CreateInstanceOpError(
        const std::shared_ptr<const Token>& nameToken
    ) -> DiagnosticGroup;

    auto CreateMissingSelfModifierAfterStrongPtrError(
        const std::shared_ptr<const Token>& strongPtrModifierToken
    ) -> DiagnosticGroup;
}
