#include "FunctionBlockBinding.hpp"

#include <optional>
#include <memory>

#include "Symbols/FunctionSymbol.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"

namespace Ace
{
    FunctionBlockBinding::FunctionBlockBinding(
        FunctionSymbol* const symbol,
        const std::optional<std::shared_ptr<const BlockStmtSyntax>>& optBlockSyntax
    ) : Symbol{ symbol },
        OptBlockSyntax{ optBlockSyntax }
    {
    }
}
