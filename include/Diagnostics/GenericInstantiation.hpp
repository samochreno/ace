#pragma once

#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/ConstraintSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TraitTypeSymbol.hpp"

namespace Ace
{
    auto CreateUnsatisfiedConstraintError(
        const SrcLocation& srcLocation,
        ConstraintSymbol* const constraint,
        TraitTypeSymbol* const trait,
        const std::vector<ITypeSymbol*>& typeArgs
    ) -> DiagnosticGroup;

    auto CreateUnsizedTypeArgError(
        const SrcLocation& srcLocation,
        ITypeSymbol* const typeArg
    ) -> DiagnosticGroup;
}
