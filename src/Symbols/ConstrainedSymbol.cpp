#include "Symbols/ConstrainedSymbol.hpp"

#include <vector>

#include "Diagnostic.hpp"
#include "Diagnostics/GenericInstantiation.hpp"
#include "SrcLocation.hpp"
#include "Symbols/ConstraintSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"

namespace Ace
{
    auto IConstrainedSymbol::CollectConstraints() const -> std::vector<ConstraintSymbol*>
    {
        return GetConstrainedScope()->CollectSymbols<ConstraintSymbol>();
    }

    auto DiagnoseUnsatisfiedConstraint(
        const SrcLocation& srcLocation,
        ConstraintSymbol* const constraint,
        const std::vector<ITypeSymbol*>& typeArgs
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();
        
        const auto& traits = constraint->GetTraits();
        std::for_each(begin(traits), end(traits),
        [&](TraitTypeSymbol* const trait)
        {
            auto* const constrainedType = CreateInstantiated<ITypeSymbol>(
                constraint->GetType(),
                InstantiationContext{ typeArgs, std::nullopt }
            );

            const bool doesTypeImplementTrait = Scope::CollectImplOfFor(
                trait,
                constrainedType
            ).has_value();

            if (doesTypeImplementTrait)
            {
                return;
            }

            diagnostics.Add(CreateUnsatisfiedConstraintError(
                srcLocation,
                constraint,
                trait,
                typeArgs
            ));
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto IConstrainedSymbol::DiagnoseUnsatisfiedConstraints(
        const SrcLocation& srcLocation,
        const std::vector<ITypeSymbol*>& typeArgs
    ) const -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        const auto constraints = CollectConstraints();
        std::for_each(begin(constraints), end(constraints),
        [&](ConstraintSymbol* const constraint)
        {
            diagnostics.Collect(DiagnoseUnsatisfiedConstraint(
                srcLocation,
                constraint,
                typeArgs
            ));  
        });

        return Diagnosed<void>{ std::move(diagnostics) };
    }
}
