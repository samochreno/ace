#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Ident.hpp"
#include "SrcLocation.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class Compilation;
    class Scope;
    class ITypeSymbol;
    class ISizedTypeSymbol;

    struct SymbolName;

    struct SymbolNameSection
    {
        SymbolNameSection();
        SymbolNameSection(const Ident& name);
        SymbolNameSection(
            const Ident& name,
            const std::vector<SymbolName>& typeArgs
        );
        ~SymbolNameSection();

        auto CreateSrcLocation() const -> SrcLocation;

        Ident Name;
        std::vector<SymbolName> TypeArgs;
    };

    enum class SymbolNameResolutionScope
    {
        Local,
        Global,
    };

    struct SymbolName
    {
        SymbolName();
        SymbolName(
            const SymbolNameSection& section,
            const SymbolNameResolutionScope& resolutionScope
        );
        SymbolName(
            const std::vector<SymbolNameSection>& sections,
            const SymbolNameResolutionScope& resolutionScope
        );
        ~SymbolName();

        auto CreateSrcLocation() const -> SrcLocation;

        std::vector<SymbolNameSection> Sections;
        bool IsGlobal;
    };

    enum class TypeNameModifier
    {
        Ref,
        AutoStrongPtr,
        DynStrongPtr,
        WeakPtr,
    };

    struct TypeName
    {
        TypeName();
        TypeName(
            const SymbolName& symbolName,
            const std::vector<TypeNameModifier>& modifiers
        );
        ~TypeName();

        SymbolName SymbolName;
        std::vector<TypeNameModifier> Modifiers;
    };

    auto ModifyTypeSymbol(
        ITypeSymbol* const pureTypeSymbol,
        std::vector<TypeNameModifier> modifiers
    ) -> ISizedTypeSymbol*;
}
