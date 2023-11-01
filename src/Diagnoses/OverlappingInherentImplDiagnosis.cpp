#include "Diagnoses/OverlappingInherentImplDiagnosis.hpp"

#include <vector>

#include "Diagnostic.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"
#include "Compilation.hpp"
#include "PlaceholderOverlapping.hpp"
#include "Symbols/Impls/InherentImplSymbol.hpp"

namespace Ace
{
    struct SymbolPair
    {
        ISymbol* LHS{};
        ISymbol* RHS{};
    };

    static auto IsGenericInstance(ISymbol* const symbol) -> bool
    {
        auto* const generic = dynamic_cast<IGenericSymbol*>(symbol);
        return generic && generic->IsInstance();
    }

    static auto CollectOverlappingNameImplSymbols(
        InherentImplSymbol* const lhsImpl,
        InherentImplSymbol* const rhsImpl
    ) -> std::vector<SymbolPair>
    {
        const auto lhsSymbols = lhsImpl->GetBodyScope()->CollectAllSymbols();
        const auto rhsSymbols = rhsImpl->GetBodyScope()->CollectAllSymbols();

        std::vector<SymbolPair> overlappingSymbols{};
        std::for_each(begin(lhsSymbols), end(lhsSymbols),
        [&](ISymbol* const lhsSymbol)
        {
            if (dynamic_cast<TypeParamTypeSymbol*>(lhsSymbol))
            {
                return;
            }

            if (IsGenericInstance(lhsSymbol))
            {
                return;
            }

            const auto matchingNameSymbolIt = std::find_if(
                begin(rhsSymbols),
                end  (rhsSymbols),
                [&](ISymbol* const rhsSymbol)
                {
                    return
                        lhsSymbol->GetName().String ==
                        rhsSymbol->GetName().String;
                }
            );
            if (matchingNameSymbolIt == end(rhsSymbols))
            {
                return;
            }

            overlappingSymbols.emplace_back(lhsSymbol, *matchingNameSymbolIt);
        });

        return overlappingSymbols;
    }

    static auto CollectOverlappingImplSymbols(
        InherentImplSymbol* const lhsImpl,
        InherentImplSymbol* const rhsImpl
    ) -> std::vector<SymbolPair>
    {
        ACE_ASSERT(lhsImpl != rhsImpl);

        auto* const lhsType = lhsImpl->GetType();
        auto* const rhsType = rhsImpl->GetType();

        const bool couldOverlap =
            DoPlaceholdersOverlap(lhsType, rhsType) || 
            DoPlaceholdersOverlap(rhsType, lhsType);

        if (!couldOverlap)
        {
            return {};
        }

        return CollectOverlappingNameImplSymbols(lhsImpl, rhsImpl);
    }

    static auto DiagnoseOverlappingImpls(
        InherentImplSymbol* const lhsImpl,
        InherentImplSymbol* const rhsImpl
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto overlappingSymbols =
            CollectOverlappingImplSymbols(lhsImpl, rhsImpl);

        std::for_each(begin(overlappingSymbols), end(overlappingSymbols),
        [&](const SymbolPair& symbolPair)
        {
            diagnostics.Add(CreateOverlappingInherentImplSymbolError(
                symbolPair.LHS,
                symbolPair.RHS
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnoseOverlappingInherentImpls(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& scope = compilation->GetPackageBodyScope();

        const auto impls = scope->CollectSymbolsRecursive<InherentImplSymbol>();

        for (int i = 0; i < impls.size(); i++)
        {
            for (int j = (i + 1); j < impls.size(); j++)
            {
                diagnostics.Collect(
                    DiagnoseOverlappingImpls(impls.at(i), impls.at(j))
                );
            }
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
