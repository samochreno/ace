#pragma once

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Op.hpp"
#include "Diagnostic.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    auto ResolveUnaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        ITypeSymbol* const typeSymbol,
        const Op& op
    ) -> Expected<FunctionSymbol*>;

    auto ResolveBinaryOpSymbol(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<ITypeSymbol*>& typeSymbols, 
        const std::vector<TypeInfo>& argTypeInfos,
        const Op& op
    ) -> Expected<FunctionSymbol*>;
}
