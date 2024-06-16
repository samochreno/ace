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
    ) -> std::optional<std::pair<ITypeSymbol*, std::string_view>>
    {
        const auto& natives = compilation->GetNatives();
        
        switch (op)
        {
            case Op::UnaryNegation:
                return std::pair{ natives.Minus.GetSymbol(), "minus" };

            case Op::Multiplication:
                return std::pair{ natives.Multiply.GetSymbol(), "multiply" };
            case Op::Division:
                return std::pair{ natives.Divide.GetSymbol(), "divide" };

            case Op::Remainder:
                return std::pair{ natives.Remainder.GetSymbol(), "remainder" };

            case Op::Addition:
                return std::pair{ natives.Add.GetSymbol(), "add" };
            case Op::Subtraction:
                return std::pair{ natives.Subtract.GetSymbol(), "subtract" };

            case Op::Equals:
                return std::pair{ natives.Equal.GetSymbol(), "equal" };
            case Op::NotEquals:
                return std::pair{ natives.Equal.GetSymbol(), "not_equal" };

            case Op::AND:
                return std::pair{ natives.AND.GetSymbol(), "and" };
            case Op::XOR:
                return std::pair{ natives.XOR.GetSymbol(), "xor" };
            case Op::OR:
                return std::pair{ natives.OR.GetSymbol(), "or" };

            default:
                return std::nullopt;
        }
    }

    static auto CollectOpPrototype(
        Compilation* const compilation,
        const Op& op
    ) -> std::optional<PrototypeSymbol*>
    {
        const auto optPair = GetOpTraitTypePrototypeNamePair(compilation, op);
        if (!optPair.has_value())
        {
            return std::nullopt;
        }

        auto* const traitType = optPair.value().first;
        const auto& prototypeName = optPair.value().second;


        auto* const trait = dynamic_cast<TraitTypeSymbol*>(traitType);
        ACE_ASSERT(trait);

        const auto prototypes = trait->CollectPrototypes();
        const auto prototypeIt = std::find_if(
            begin(prototypes),
            end  (prototypes),
            [&](PrototypeSymbol* const prototype)
            {
                return prototype->GetName().String == prototypeName;
            }
        );


        ACE_ASSERT(prototypeIt != end(prototypes));
        return *prototypeIt;
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

    static auto ResolveUserBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& lhsTypeInfo,
        const TypeInfo& rhsTypeInfo,
        const Op op
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const compilation = scope->GetCompilation();

        const auto optPrototype = CollectOpPrototype(compilation, op);
        if (!optPrototype.has_value())
        {
            return std::nullopt;
        }

        return optPrototype.has_value()
            ? Scope::CollectImplOfFor(
                optPrototype.value(),
                lhsTypeInfo.Symbol
            )
            : std::nullopt;
    }

    static auto ResolveNativeBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& lhsTypeInfo,
        const TypeInfo& rhsTypeInfo,
        const Op op
    ) -> std::optional<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const compilation = scope->GetCompilation();

        const auto& opMap = compilation->GetNatives().GetBinaryOpMap();

        const auto opSymbolsIt = opMap.find(
            lhsTypeInfo.Symbol->GetUnaliasedType()
        );
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

        return opSymbolIt->second;
    }

    auto ResolveBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const TypeInfo& lhsTypeInfo,
        const TypeInfo& rhsTypeInfo,
        const Op& op
    ) -> Expected<FunctionSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (lhsTypeInfo.Symbol->IsError() || rhsTypeInfo.Symbol->IsError())
        {
            return std::move(diagnostics);
        }

        const auto optNativeSymbol = ResolveNativeBinaryOpSymbol(
            srcLocation,
            scope,
            lhsTypeInfo,
            rhsTypeInfo,
            op
        );
        const auto optUserSymbol = ResolveUserBinaryOpSymbol(
            srcLocation,
            scope,
            lhsTypeInfo,
            rhsTypeInfo,
            op
        );

        if (!optNativeSymbol.has_value() && !optUserSymbol.has_value())
        {
            diagnostics.Add(CreateUndeclaredBinaryOpError(
                srcLocation,
                lhsTypeInfo.Symbol,
                rhsTypeInfo.Symbol
            ));

            const auto optUserSymbol = ResolveUserBinaryOpSymbol(
                srcLocation,
                scope,
                lhsTypeInfo,
                rhsTypeInfo,
                op
            );

            return std::move(diagnostics);
        }

        if (optNativeSymbol.has_value() && optUserSymbol.has_value())
        {
            diagnostics.Add(CreateAmbiguousBinaryOpRefError(
                srcLocation,
                lhsTypeInfo.Symbol,
                rhsTypeInfo.Symbol
            ));
            return std::move(diagnostics);
        }

        auto* const symbol = optNativeSymbol.has_value()
            ? optNativeSymbol.value()
            : optUserSymbol.value();

        const bool areArgsConvertible = AreTypesConvertible(
            scope,
            { lhsTypeInfo, rhsTypeInfo },
            symbol->CollectAllArgTypeInfos()
        );
        if (!areArgsConvertible)
        {
            return std::move(diagnostics);
        }

        return Expected{ symbol, std::move(diagnostics) };
    }
}
