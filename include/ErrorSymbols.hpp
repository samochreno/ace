#pragma once

namespace Ace
{
    class Compilation;

    class ISymbol;
    class ITypeSymbol;
    class ISizedTypeSymbol;
    class INominalTypeSymbol;
    class TraitTypeSymbol;
    class StructTypeSymbol;
    class IVarSymbol;
    class FieldVarSymbol;
    class ICallableSymbol;
    class FunctionSymbol;

    class ErrorSymbols
    {
    public:
        ErrorSymbols() = default;
        ErrorSymbols(Compilation* compilation);
        ~ErrorSymbols() = default;

        auto Contains(const ISymbol* const symbol) const -> bool;

        auto GetType() const -> ITypeSymbol*;
        auto GetSizedType() const -> ISizedTypeSymbol*;
        auto GetNominalType() const -> INominalTypeSymbol*;
        auto GetTrait() const -> TraitTypeSymbol*;
        auto GetStruct() const -> StructTypeSymbol*;
        auto GetVar() const -> IVarSymbol*;
        auto GetField() const -> FieldVarSymbol*;
        auto GetCallable() const -> ICallableSymbol*;
        auto GetFunction() const -> FunctionSymbol*;

    private:
        Compilation* m_Compilation{};

        TraitTypeSymbol* m_Trait{};
        StructTypeSymbol* m_Struct{};
        FieldVarSymbol* m_Field{};
        FunctionSymbol* m_Function{};
    };
}
