#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/ConstraintSymbol.hpp"
#include "Symbols/Types/TypeParamTypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    class IConstrainedSymbol : public virtual ISymbol
    {
    public:
        virtual ~IConstrainedSymbol() = default;

        virtual auto GetConstrainedScope() const -> std::shared_ptr<Scope> = 0;
        virtual auto CollectConstraints() const -> std::vector<ConstraintSymbol*> final;

        virtual auto DiagnoseUnsatisfiedConstraints(
            const SrcLocation& srcLocation,
            const std::vector<ITypeSymbol*>& typeArgs
        ) const -> Diagnosed<void> final;
    };
}
