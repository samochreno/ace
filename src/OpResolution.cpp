#include "OpResolution.hpp"

#include "Compilation.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "TypeConversions.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/PrototypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"

namespace Ace
{
    static auto GetOpTraitTypePrototypeNamePair(
        Compilation* const compilation,
        const Op& op
    ) -> std::pair<ITypeSymbol*, std::string_view>
    {
        const auto& natives = compilation->GetNatives();
        
        switch (op)
        {
            case Op::UnaryNegation:
                return { natives.Minus.GetSymbol(), "minus" };

            case Op::Multiplication:
                return { natives.Multiply.GetSymbol(), "multiply" };
            case Op::Division:
                return { natives.Divide.GetSymbol(), "divide" };

            case Op::Remainder:
                return { natives.Remainder.GetSymbol(), "remainder" };

            case Op::Addition:
                return { natives.Add.GetSymbol(), "add" };
            case Op::Subtraction:
                return { natives.Subtract.GetSymbol(), "subtract" };

            case Op::Equals:
                return { natives.Equal.GetSymbol(), "equal" };
            case Op::NotEquals:
                return { natives.Equal.GetSymbol(), "not_equal" };

            case Op::AND:
                return { natives.AND.GetSymbol(), "and" };
            case Op::XOR:
                return { natives.XOR.GetSymbol(), "xor" };
            case Op::OR:
                return { natives.OR.GetSymbol(), "or" };

            default:
                ACE_UNREACHABLE();
        }

    }

    static auto CollectOpPrototype(
        Compilation* const compilation,
        const Op& op
    ) -> PrototypeSymbol*
    {
        const auto [traitType, prototypeName] =
            GetOpTraitTypePrototypeNamePair(compilation, op);

        auto* const trait = dynamic_cast<TraitTypeSymbol*>(traitType);
        ACE_ASSERT(trait);

        return trait->CollectPrototypes() // TODO
    }

    static auto ResolveNativeUnaryOpSymbol(
        ITypeSymbol* const typeSymbol,
        const Op op
    ) -> std::optional<FunctionSymbol*>
    {
        auto* const compilation = typeSymbol->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetUnaryOpMap();

        const auto typeOpsMapIt = opMap.find(typeSymbol->GetUnaliasedType()); 
        if (typeOpsMapIt == end(opMap))
        {
            return std::nullopt;
        }

        const auto& typeOpMap = typeOpsMapIt->second;

        const auto opSymbolIt = typeOpMap.find(op);
        if (opSymbolIt == end(typeOpMap))
        {
            return std::nullopt;
        }

        return opSymbolIt->second;
    }

    auto ResolveUnaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol,
        const Op& op
    ) -> Expected<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (typeSymbol->IsError())
        {
            return std::move(diagnostics);
        }

        const auto optSymbol = ResolveNativeUnaryOpSymbol(typeSymbol, op);
        if (!optSymbol.has_value())
        {
            diagnostics.Add(CreateUndeclaredUnaryOpError(
                srcLocation,
                typeSymbol
            ));
        }

        auto* const symbol = optSymbol.value_or(
            scope->GetCompilation()->GetErrorSymbols().GetFunction()
        );

        return Expected{ symbol, std::move(diagnostics) };
    }

    static auto ResolveNativeBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol,
        const Op op,
        const std::vector<TypeInfo>& argTypeInfos
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const compilation = scope->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetBinaryOpMap();

        const auto opSymbolsIt = opMap.find(typeSymbol->GetUnaliasedType());
        if (opSymbolsIt == end(opMap))
        {
            return std::nullopt;
        }

        const auto& opSymbols = opSymbolsIt->second;

        const auto opSymbolIt = opSymbols.find(op);
        if (opSymbolIt == end(opSymbols))
        {
            return std::nullopt;
        }

        auto* const opSymbol = opSymbolIt->second;

        const bool areArgsConvertible = AreTypesConvertible(
            scope,
            argTypeInfos,
            opSymbol->CollectArgTypeInfos()
        );
        if (!areArgsConvertible)
        {
            return std::nullopt;
        }

        return opSymbol;
    }

    static auto ResolveBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol,
        const Op op,
        const std::vector<TypeInfo>& argTypeInfos
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optNativeSymbol = ResolveNativeBinaryOpSymbol(
            srcLocation,
            scope,
            typeSymbol,
            op,
            argTypeInfos
        );
        if (optNativeSymbol.has_value())
        {
            return optNativeSymbol;
        }

        return std::nullopt;
    }

    auto ResolveBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<ITypeSymbol*>& typeSymbols, 
        const std::vector<TypeInfo>& argTypeInfos,
        const Op& op
    ) -> Expected<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        ACE_ASSERT(typeSymbols.size() == 1 || typeSymbols.size() == 2);
        ACE_ASSERT(argTypeInfos.size() == 2);

        const auto hasErrorType = std::find_if(
            begin(typeSymbols),
            end  (typeSymbols),
            [](ITypeSymbol* const typeSymbol) { return typeSymbol->IsError(); }
        ) != end(typeSymbols);

        const auto hasErrorArg =
            argTypeInfos.front().Symbol->IsError() ||
            argTypeInfos.back().Symbol->IsError();

        if (hasErrorType || hasErrorArg)
        {
            return std::move(diagnostics);
        }

        std::set<FunctionSymbol*> opSymbols{};
        std::for_each(begin(typeSymbols), end(typeSymbols),
        [&](ITypeSymbol* const typeSymbol)
        {
            const auto optOpSymbol = ResolveBinaryOpSymbol(
                srcLocation,
                scope,
                typeSymbol,
                op,
                argTypeInfos
            );
            if (optOpSymbol.has_value())
            {
                opSymbols.insert(dynamic_cast<FunctionSymbol*>(
                    optOpSymbol.value()->GetUnaliased()
                ));
            }
        });

        if (opSymbols.empty())
        {
            diagnostics.Add(CreateUndeclaredBinaryOpError(
                srcLocation,
                argTypeInfos.front().Symbol,
                argTypeInfos.back().Symbol
            ));
            return std::move(diagnostics);
        }

        if (opSymbols.size() > 1)
        {
            diagnostics.Add(CreateAmbiguousBinaryOpRefError(
                srcLocation,
                argTypeInfos.front().Symbol,
                argTypeInfos.back().Symbol
            ));
            return std::move(diagnostics);
        }

        return Expected{ *opSymbols.begin(), std::move(diagnostics) };
    }
}
