#pragma once

#include <vector>
#include <string>

#include "Identifier.hpp"

namespace Ace
{
    class Compilation;

    struct SymbolName;

    struct SymbolNameSection
    {
        SymbolNameSection();
        SymbolNameSection(const Identifier& name);
        SymbolNameSection(
            const Identifier& name,
            const std::vector<SymbolName>& templateArgs
        );
        ~SymbolNameSection();

        Identifier Name;
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

        std::vector<SymbolNameSection> Sections;
        bool IsGlobal;
    };

    enum class TypeNameModifier
    {
        Reference,
        StrongPointer,
        WeakPointer,
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
            const Compilation* const compilation
        ) const -> SymbolName;

        SymbolName SymbolName;
        std::vector<TypeNameModifier> Modifiers;
    };
}
