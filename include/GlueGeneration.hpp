#pragma once

#include <memory>

#include "Compilation.hpp"
#include "Emittable.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace::GlueGeneration
{
    auto GenerateAndBindGlue(
        const Compilation* const t_compilation
    ) -> void;

    auto CreateCopyGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        StructTypeSymbol* const t_structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
    auto CreateDropGlueBody(
        const Compilation* const t_compilation,
        FunctionSymbol* const t_glueSymbol,
        StructTypeSymbol* const t_structSymbol
    ) -> std::shared_ptr<const IEmittable<void>>;
}
