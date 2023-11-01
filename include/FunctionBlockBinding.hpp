#pragma once

#include <memory>
#include <optional>

namespace Ace
{
    class FunctionSymbol;
    class BlockStmtSyntax;

    struct FunctionBlockBinding
    {
        FunctionBlockBinding(
            FunctionSymbol* const symbol,
            const std::optional<std::shared_ptr<const BlockStmtSyntax>>& optBlockSyntax
        );

        FunctionSymbol* Symbol{};
        std::optional<std::shared_ptr<const BlockStmtSyntax>> OptBlockSyntax{};
    };
}
