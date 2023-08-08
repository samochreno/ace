#pragma once

#include <memory>

#include "Compilation.hpp"
#include "Emittable.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace::GlueGeneration
{
    auto GenerateAndBindGlue(Compilation* const compilation) -> void;

    auto CreateCopyGlueBody(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
    auto CreateDropGlueBody(
        Compilation* const compilation,
        FunctionSymbol* const glueSymbol,
        StructTypeSymbol* const structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
}
