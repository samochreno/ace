#pragma once

#include <memory>
#include <string>

#include "Diagnostic.hpp"
#include "DiagnosticStringConversions.hpp"
#include "SrcLocation.hpp"

namespace Ace
{
    template<typename TSymbol>
    auto CreateIncorrectSymbolTypeError(
        const SrcLocation& srcLocation
    ) -> std::shared_ptr<const Diagnostic>
    {
        return std::make_shared<const Diagnostic>(
            DiagnosticSeverity::Error,
            srcLocation,
            "symbol is not " + CreateSymbolTypeStringWithArticle<TSymbol>()
        );
    }
}
