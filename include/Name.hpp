#pragma once

#include <vector>
#include <string>

#include "Ident.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    class Compilation;

    struct SymbolName;

    struct SymbolNameSection
    {
        SymbolNameSection();
        SymbolNameSection(const Ident& name);
        SymbolNameSection(
            const Ident& name,
            const std::vector<SymbolName>& templateArgs
        );
        ~SymbolNameSection();

        auto CreateSrcLocation() const -> SrcLocation;

        Ident Name;
        std::vector<SymbolName> TemplateArgs;
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
        StrongPtr,
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

        auto ToSymbolName(
            Compilation* const compilation
        ) const -> SymbolName;

        SymbolName SymbolName;
        std::vector<TypeNameModifier> Modifiers;
    };
}
