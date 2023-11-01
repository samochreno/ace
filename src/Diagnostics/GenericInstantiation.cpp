#include "Diagnostics/GenericInstantiation.hpp"

#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"

namespace Ace
{
    auto CreateUnsatisfiedConstraintError(
        const SrcLocation& srcLocation,
        ConstraintSymbol* const constraint,
        TraitTypeSymbol* const trait,
        const std::vector<ITypeSymbol*>& typeArgs
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        auto* const constrainedType = CreateInstantiated<ITypeSymbol>(
            constraint->GetType(),
            InstantiationContext{ typeArgs, std::nullopt }
        );

        const std::string message =
            "type argument `" + constrainedType->CreateDisplayName() +
            "` does not implement `" + trait->CreateDisplayName() + "`";

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            message
        );

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Note,
            constraint->GetName().SrcLocation,
            "constraint declaration"
        );

        return group;
    }

    auto CreateUnsizedTypeArgError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const typeArg
    ) -> DiagnosticGroup
    {
        DiagnosticGroup group{};

        group.Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "unsized type argument"
        );

        return group;
    }
}
