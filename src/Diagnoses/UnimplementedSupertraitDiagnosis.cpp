#include "Diagnoses/UnimplementedSupertraitDiagnosis.hpp"

#include "Compilation.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Impls/TraitImplSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"
#include "Symbols/SupertraitSymbol.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"

namespace Ace
{
    static auto DiagnoseUnimplementedSupertrait(
        const std::map<TraitTypeSymbol*, std::set<TraitImplSymbol*>>& traitImplMap,
        TraitImplSymbol* const impl,
        SupertraitSymbol* const supertrait
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto optSupertraitImpl = Scope::CollectImplOfFor(
            supertrait->GetTrait(),
            impl->GetType()
        );
        if (!optSupertraitImpl.has_value())
        {
            diagnostics.Add(CreateUnimplementedSupertraitError(
                supertrait,
                impl
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseUnimplementedSupertraits(
        const std::map<TraitTypeSymbol*, std::set<TraitImplSymbol*>>& traitImplMap,
        TraitImplSymbol* const impl
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto supertraits = impl->GetTrait()->CollectSupertraits();
        std::for_each(begin(supertraits), end(supertraits),
        [&](SupertraitSymbol* const supertrait)
        {
            diagnostics.Collect(DiagnoseUnimplementedSupertrait(
                traitImplMap,
                impl,
                supertrait
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseUnimplementedSupertraits(
        const std::map<TraitTypeSymbol*, std::set<TraitImplSymbol*>>& traitImplMap,
        TraitTypeSymbol* const trait
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!traitImplMap.contains(trait))
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        const auto& impls = traitImplMap.at(trait);
        std::for_each(begin(impls), end(impls),
        [&](TraitImplSymbol* const impl)
        {
            diagnostics.Collect(DiagnoseUnimplementedSupertraits(
                traitImplMap,
                impl
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto CollectTraitImplMap(
        const std::shared_ptr<Scope>& scope
    ) -> std::map<TraitTypeSymbol*, std::set<TraitImplSymbol*>>
    {
        std::map<TraitTypeSymbol*, std::set<TraitImplSymbol*>> map{};

        const auto impls = scope->CollectSymbolsRecursive<TraitImplSymbol>();
        std::for_each(begin(impls), end(impls),
        [&](TraitImplSymbol* const impl)
        {
            auto* const trait = dynamic_cast<TraitTypeSymbol*>(
                impl->GetTrait()->GetUnaliased()
            );

            map[trait].insert(dynamic_cast<TraitImplSymbol*>(
                impl->GetUnaliased()
            ));
        });

        return map;
    }

    auto DiagnoseUnimplementedSupertraits(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto& scope = compilation->GetPackageBodyScope();

        const auto traitImplMap = CollectTraitImplMap(scope);

        const auto traits = scope->CollectSymbolsRecursive<TraitTypeSymbol>();
        std::for_each(begin(traits), end(traits),
        [&](TraitTypeSymbol* const trait)
        {
            diagnostics.Collect(DiagnoseUnimplementedSupertraits(
                traitImplMap,
                trait
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
