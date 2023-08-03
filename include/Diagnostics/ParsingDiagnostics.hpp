#pragma once

#include <memory>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Token.hpp"

namespace Ace
{
    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedTokenError(
        const std::shared_ptr<const Token>& unexpectedToken,
        const TokenKind expectedTokenKind
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedTokenExpectedLiteralError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedTokenExpectedNewError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedTokenExpectedCompoundAssignmentOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedTokenExpectedOverloadableOpError(
        const std::shared_ptr<const Token>& unexpectedToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateEmptyTemplateParamsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateEmptyTemplateArgsError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateExternInstanceFunctionError(
        const std::shared_ptr<const Token>& externKeywordToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnknownModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateForbiddenModifierError(
        const std::shared_ptr<const Token>& modifierToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateEmptyModifiersError(
        const std::shared_ptr<const Token>& minusGreaterThanToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateMissingTokenError(
        const SrcLocation& lastTokenSrcLocation,
        const TokenKind expectedTokenKind
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateTemplateSpecializationError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedUnaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnexpectedUnaryOrBinaryOpParamCountError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateUnknownIdentOpError(
        const std::shared_ptr<const Token>& opToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateOpMustBePublicError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const DiagnosticGroup>;

    auto CreateInstanceOpError(
        const std::shared_ptr<const Token>& nameToken
    ) -> std::shared_ptr<const DiagnosticGroup>;
}
