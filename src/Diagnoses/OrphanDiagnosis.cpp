#include "Diagnoses/OrphanDiagnosis.hpp"

#include "Compilation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"
#include "Symbols/Impls/InherentImplSymbol.hpp"
#include "Symbols/Impls/TraitImplSymbol.hpp"

namespace Ace
{
    static auto DiagnoseOrphanInherentImpl(
        InherentImplSymbol* const impl
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const implPackageMod = impl->GetScope()->FindPackageMod();
        auto* const typePackageMod =
            impl->GetType()->GetUnaliased()->GetScope()->FindPackageMod();

        if (implPackageMod != typePackageMod)
        {
            diagnostics.Add(CreateOrphanInherentImplError(impl));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseOrphanTraitImpl(
        TraitImplSymbol* const impl
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const implPackageModSymbol = impl->GetScope()->FindPackageMod();
        auto* const traitPackageModSymbol =
            impl->GetTrait()->GetUnaliased()->GetScope()->FindPackageMod();
        auto* const typePackageModSymbol =
            impl->GetType()->GetUnaliased()->GetScope()->FindPackageMod();

        if (
            (implPackageModSymbol != typePackageModSymbol) &&
            (implPackageModSymbol != traitPackageModSymbol)
            )
        {
            diagnostics.Add(CreateOrphanTraitImplError(impl));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnoseOrphans(Compilation* const compilation) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& scope = compilation->GetPackageBodyScope();
        
        const auto inherentImpls = scope->CollectSymbols<InherentImplSymbol>();
        std::for_each(begin(inherentImpls), end(inherentImpls),
        [&](InherentImplSymbol* const impl)
        {
            diagnostics.Collect(DiagnoseOrphanInherentImpl(impl));
        });

        const auto traitImpls = scope->CollectSymbols<TraitImplSymbol>();
        std::for_each(begin(traitImpls), end(traitImpls),
        [&](TraitImplSymbol* const impl)
        {
            diagnostics.Collect(DiagnoseOrphanTraitImpl(impl));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
