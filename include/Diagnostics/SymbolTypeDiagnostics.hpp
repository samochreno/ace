#pragma once

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
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

    template<typename TSymbol>
    auto CreateIncorrectSymbolTypeError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "symbol is not " + CreateSymbolTypeStringWithArticle<TSymbol>()
        );
    }
}
