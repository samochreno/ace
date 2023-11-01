#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "Assert.hpp"
#include "TokenKind.hpp"
#include "AccessModifier.hpp"
#include "SymbolCategory.hpp"
#include "Token.hpp"

namespace Ace
{
    auto CreateTokenKindString(const TokenKind tokenKind) -> std::string;
    auto CreateTokenKindStringWithArticle(
        const TokenKind tokenKind
    ) -> std::string;

    auto CreateAccessModifierString(
        const AccessModifier accessModifier
    ) -> std::string;

    auto CreateJsonTypeString(
        const nlohmann::json::value_t type
    ) -> std::string;
    auto CreateJsonTypeStringWithArticle(
        const nlohmann::json::value_t type
    ) -> std::string;

    auto CreateSymbolCategoryStringWithArticle(
        const SymbolCategory symbolCategory
    ) -> std::string;

    auto CreateOpString(
        const Token& opToken
    ) -> std::string;

    template<typename TSymbol>
    inline auto CreateSymbolTypeStringWithArticle() -> std::string
    {
        TSymbol::Error;
    }

    class ISymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ISymbol>() -> std::string
    {
        ACE_UNREACHABLE();
    }

    class IGenericSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<IGenericSymbol>() -> std::string
    {
        return "a generic";
    }

    class TypeParamTypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<TypeParamTypeSymbol>() -> std::string
    {
        return "a type param";
    }

    class ITypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ITypeSymbol>() -> std::string
    {
        return "a type";
    }

    class ICallableSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ICallableSymbol>() -> std::string
    {
        return "a function";
    }

    class FunctionSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<FunctionSymbol>() -> std::string
    {
        return "a function";
    }

    class ModSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ModSymbol>() -> std::string
    {
        return "a module";
    }

    class LabelSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<LabelSymbol>() -> std::string
    {
        return "a label";
    }

    class LocalVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<LocalVarSymbol>() -> std::string
    {
        return "a local variable";
    }

    class StructTypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<StructTypeSymbol>() -> std::string
    {
        return "a struct";
    }

    class FieldVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<FieldVarSymbol>() -> std::string
    {
        return "a field";
    }

    class NormalParamVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<NormalParamVarSymbol>() -> std::string
    {
        return "a parameter";
    }

    class SelfParamVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<SelfParamVarSymbol>() -> std::string
    {
        return "a self parameter";
    }

    class GlobalVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<GlobalVarSymbol>() -> std::string
    {
        return "a global variable";
    }

    class IVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<IVarSymbol>() -> std::string
    {
        return "a variable";
    }

    class ISizedTypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ISizedTypeSymbol>() -> std::string
    {
        return "a sized type";
    }

    class TraitTypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<TraitTypeSymbol>() -> std::string
    {
        return "a trait";
    }

    class PrototypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<PrototypeSymbol>() -> std::string
    {
        return "a function prototype";
    }

    class INominalTypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<INominalTypeSymbol>() -> std::string
    {
        return "a nominal type";
    }
}
