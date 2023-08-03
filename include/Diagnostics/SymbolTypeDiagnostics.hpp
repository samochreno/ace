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
    ) -> std::shared_ptr<const DiagnosticGroup>
    {
        auto group = std::make_shared<DiagnosticGroup>();

        group->Diagnostics.emplace_back(
            DiagnosticSeverity::Error,
            srcLocation,
            "symbol is not " + CreateSymbolTypeStringWithArticle<TSymbol>()
        );

        return group;
    }
}
