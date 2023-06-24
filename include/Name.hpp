#pragma once

#include <vector>
#include <string>

namespace Ace
{
    class Compilation;

    struct SymbolName;

    struct SymbolNameSection
    {
        SymbolNameSection();
        SymbolNameSection(const std::string& t_name);
        SymbolNameSection(
            const std::string& t_name,
            const std::vector<SymbolName>& t_templateArgs
        );
        ~SymbolNameSection();

        std::string Name;
        std::vector<SymbolName> TemplateArgs;
    };

    enum class SymbolNameResolutionScope
    {
        None,
        Local,
        Global,
    };

    struct SymbolName
    {
        SymbolName();
        SymbolName(
            const SymbolNameSection& t_section,
            const SymbolNameResolutionScope& t_resolutionScope
        );
        SymbolName(
            const std::vector<SymbolNameSection>& t_sections,
            const SymbolNameResolutionScope& t_resolutionScope
        );
        ~SymbolName();

        std::vector<SymbolNameSection> Sections;
        bool IsGlobal;
    };

    enum class TypeNameModifier
    {
        None,
        Reference,
        StrongPointer,
        WeakPointer,
    };

    struct TypeName
    {
        TypeName();
        TypeName(
            const SymbolName& t_symbolName,
            const std::vector<TypeNameModifier>& t_modifiers
        );
        ~TypeName();

        auto ToSymbolName(
            const Compilation* const t_compilation
        ) const -> SymbolName;

        SymbolName SymbolName;
        std::vector<TypeNameModifier> Modifiers;
    };
}
