#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "TokenKind.hpp"
#include "AccessModifier.hpp"
#include "SymbolKind.hpp"
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

    auto CreateSymbolKindString(const SymbolKind symbolKind) -> std::string;
    auto CreateSymbolKindStringWithArticle(
        const SymbolKind symbolKind
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
        TSymbol::fail;
    }
    class ITypeSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ITypeSymbol>() -> std::string
    {
        return "a type";
    }
    class FunctionSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<FunctionSymbol>() -> std::string
    {
        return "a function";
    }
    class ModuleSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<ModuleSymbol>() -> std::string
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
    class InstanceVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<InstanceVarSymbol>() -> std::string
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
    class StaticVarSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<StaticVarSymbol>() -> std::string
    {
        return "a global variable";
    }
    class TypeTemplateSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<TypeTemplateSymbol>() -> std::string
    {
        return "a type template";
    }
    class FunctionTemplateSymbol;
    template<>
    inline auto CreateSymbolTypeStringWithArticle<FunctionTemplateSymbol>() -> std::string
    {
        return "a function template";
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
}
