#include "ErrorSymbols.hpp"

#include "Compilation.hpp"
#include "Symbols/All.hpp"

namespace Ace
{
    ErrorSymbols::ErrorSymbols(
        Compilation* compilation
    ) : m_Compilation{ compilation }
    {
        const auto scope = m_Compilation->GetGlobalScope();
        const auto selfScope = scope->GetOrCreateChild({});

        m_StructType = scope->DefineSymbol(std::make_unique<StructTypeSymbol>(
            selfScope,
            Ident{ SrcLocation{}, "$error_struct_type" },
            AccessModifier::Public
        )).Unwrap();
        
        m_InstanceVar = scope->DefineSymbol(std::make_unique<InstanceVarSymbol>(
            selfScope,
            Ident{ SrcLocation{}, "$error_instance_var" },
            AccessModifier::Public,
            GetSizedType(),
            0
        )).Unwrap();

        m_Function = scope->DefineSymbol(std::make_unique<FunctionSymbol>(
            selfScope,
            Ident{ SrcLocation{}, "$error_function" },
            SymbolCategory::Static,
            AccessModifier::Public,
            GetType()
        )).Unwrap();
    }

    auto ErrorSymbols::Contains(const ISymbol* const symbol) const -> bool
    {
        return
            (symbol == m_StructType) ||
            (symbol == m_InstanceVar) ||
            (symbol == m_Function);
    }

    auto ErrorSymbols::GetType() const -> ITypeSymbol*
    {
        ACE_ASSERT(m_StructType);
        return m_StructType;
    }

    auto ErrorSymbols::GetSizedType() const -> ISizedTypeSymbol*
    {
        ACE_ASSERT(m_StructType);
        return m_StructType;
    }

    auto ErrorSymbols::GetStructType() const -> StructTypeSymbol*
    {
        ACE_ASSERT(m_StructType);
        return m_StructType;
    }

    auto ErrorSymbols::GetVar() const -> IVarSymbol*
    {
        ACE_ASSERT(m_InstanceVar);
        return m_InstanceVar;
    }

    auto ErrorSymbols::GetInstanceVar() const -> InstanceVarSymbol*
    {
        ACE_ASSERT(m_InstanceVar);
        return m_InstanceVar;
    }

    auto ErrorSymbols::GetFunction() const -> FunctionSymbol*
    {
        ACE_ASSERT(m_Function);
        return m_Function;
    }
}
