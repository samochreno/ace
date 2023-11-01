#include "ErrorSymbols.hpp"

#include "Compilation.hpp"
#include "Symbols/All.hpp"
#include "Syntaxes/All.hpp"
#include "Application.hpp"
#include "AnonymousIdent.hpp"

namespace Ace
{
    ErrorSymbols::ErrorSymbols(
        Compilation* compilation
    ) : m_Compilation{ compilation }
    {
        auto diagnostics = DiagnosticBag::CreateNoError();

        const auto scope = m_Compilation->GetGlobalScope();

        const SrcLocation srcLocation{ compilation };

        auto ownedTrait = std::make_unique<TraitTypeSymbol>(
            scope->CreateChild(),
            scope->CreateChild(),
            AccessModifier::Pub,
            Ident{ srcLocation, AnonymousIdent::Create("error_trait") },
            std::vector<ITypeSymbol*>{}
        );
        m_Trait = diagnostics.Collect(
            Scope::DeclareSymbol(std::move(ownedTrait))
        );

        auto ownedStruct = std::make_unique<StructTypeSymbol>(
            scope->CreateChild(),
            AccessModifier::Pub,
            Ident{ srcLocation, AnonymousIdent::Create("error_struct") },
            std::vector<ITypeSymbol*>{}
        );
        m_Struct = diagnostics.Collect(
            Scope::DeclareSymbol(std::move(ownedStruct))
        );

        auto ownedField = std::make_unique<FieldVarSymbol>(
            m_Struct,
            AccessModifier::Pub,
            Ident{ srcLocation, AnonymousIdent::Create("error_field") },
            GetSizedType(),
            0
        );
        m_Field = diagnostics.Collect(
            Scope::DeclareSymbol(std::move(ownedField))
        );

        auto ownedFunction = std::make_unique<FunctionSymbol>(
            scope->CreateChild(),
            SymbolCategory::Static,
            AccessModifier::Pub,
            Ident{ srcLocation, AnonymousIdent::Create("error_function") },
            GetType(),
            std::vector<ITypeSymbol*>{}
        );
        m_Function = diagnostics.Collect(
            Scope::DeclareSymbol(std::move(ownedFunction))
        );
    }

    auto ErrorSymbols::Contains(const ISymbol* const symbol) const -> bool
    {
        return 
            (symbol == m_Trait) ||
            (symbol == m_Struct) ||
            (symbol == m_Field) ||
            (symbol == m_Function);
    }

    auto ErrorSymbols::GetType() const -> ITypeSymbol*
    {
        return GetStruct();
    }

    auto ErrorSymbols::GetSizedType() const -> ISizedTypeSymbol*
    {
        return GetStruct();
    }

    auto ErrorSymbols::GetNominalType() const -> INominalTypeSymbol*
    {
        return GetStruct();
    }

    auto ErrorSymbols::GetTrait() const -> TraitTypeSymbol*
    {
        ACE_ASSERT(m_Trait);
        return m_Trait;
    }

    auto ErrorSymbols::GetStruct() const -> StructTypeSymbol*
    {
        ACE_ASSERT(m_Struct);
        return m_Struct;
    }

    auto ErrorSymbols::GetVar() const -> IVarSymbol*
    {
        return GetField();
    }

    auto ErrorSymbols::GetField() const -> FieldVarSymbol*
    {
        ACE_ASSERT(m_Field);
        return m_Field;
    }

    auto ErrorSymbols::GetCallable() const -> ICallableSymbol*
    {
        return GetFunction();
    }

    auto ErrorSymbols::GetFunction() const -> FunctionSymbol*
    {
        ACE_ASSERT(m_Function);
        return m_Function;
    }
}
