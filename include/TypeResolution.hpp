#pragma once

#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    template<typename TSymbol>
    auto ResolveTypeSymbol(
        const std::shared_ptr<Scope>& scope,
        const TypeName& typeName
    ) -> Expected<TSymbol*>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optPureTypeSymbol = diagnostics.Collect(
            scope->ResolveStaticSymbol<ITypeSymbol>(typeName.SymbolName)
        );
        if (!optPureTypeSymbol.has_value())
        {
            return std::move(diagnostics);
        }

        auto* const pureTypeSymbol = optPureTypeSymbol.value();

        if (typeName.Modifiers.empty())
        {
            auto* const castedPureTypeSymbol = dynamic_cast<TSymbol*>(
                pureTypeSymbol
            );
            if (!castedPureTypeSymbol)
            {
                diagnostics.Add(CreateIncorrectSymbolTypeError<TSymbol>(
                    typeName.SymbolName.CreateSrcLocation()
                ));
                return std::move(diagnostics);
            }

            return Expected{ castedPureTypeSymbol, std::move(diagnostics) };
        }

        auto* const modifiedTypeSymbol = ModifyTypeSymbol(
            pureTypeSymbol,
            typeName.Modifiers
        );
        return Expected
        {
            dynamic_cast<TSymbol*>(modifiedTypeSymbol),
            std::move(diagnostics),
        };
    }
}
