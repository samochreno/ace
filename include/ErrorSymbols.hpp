#pragma once

namespace Ace
{
    class Compilation;

    class ISymbol;
    class ITypeSymbol;
    class StructTypeSymbol;
    class IVarSymbol;
    class InstanceVarSymbol;
    class FunctionSymbol;

    class ErrorSymbols
    {
    public:
        ErrorSymbols(Compilation* const compilation);
        ~ErrorSymbols() = default;

        auto Contains(const ISymbol* const symbol) const -> bool;

        auto GetType() const -> ITypeSymbol*;
        auto GetStructType() const -> StructTypeSymbol*;
        auto GetVar() const -> IVarSymbol*;
        auto GetInstanceVar() const -> InstanceVarSymbol*;
        auto GetFunction() const -> FunctionSymbol*;

    private:
        Compilation* m_Compilation{};

        StructTypeSymbol* m_StructType{};
        InstanceVarSymbol* m_InstanceVar{};
        FunctionSymbol* m_Function{};
    };
}
