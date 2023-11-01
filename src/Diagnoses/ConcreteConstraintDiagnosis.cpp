#include "Diagnoses/ConcreteConstraintDiagnosis.hpp"

#include "Compilation.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/DiagnosisDiagnostics.hpp"
#include "Symbols/ConstraintSymbol.hpp"

namespace Ace
{
    static auto DiagnoseConcreteConstraint(
        ConstraintSymbol* const constraint
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto concreteTraitIt = std::find_if_not(
            begin(constraint->GetTraits()),
            end  (constraint->GetTraits()),
            [](TraitTypeSymbol* const trait) { return trait->IsPlaceholder(); }
        );

        const auto areAllTraitsPlaceholders =
            concreteTraitIt == end(constraint->GetTraits());

        const bool isValid =
            constraint->GetType()->IsPlaceholder() || areAllTraitsPlaceholders;

        if (!isValid)
        {
            diagnostics.Add(CreateConcreteConstraintError(
                constraint->GetName().SrcLocation
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto DiagnoseConcreteConstraints(
        Compilation* const compilation
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto constraints =
            compilation->GetGlobalScope()->CollectSymbolsRecursive<ConstraintSymbol>();

        std::for_each(begin(constraints), end(constraints),
        [&](ConstraintSymbol* const constraint)
        {
            diagnostics.Collect(DiagnoseConcreteConstraint(constraint));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
