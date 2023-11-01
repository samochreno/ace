#pragma once

#include <vector>
#include <string_view>

#include "Diagnostic.hpp"
#include "Compilation.hpp"
#include "Syntaxes/Syntax.hpp"
#include "Syntaxes/Stmts/BlockStmtSyntax.hpp"
#include "Semas/Sema.hpp"
#include "FunctionBlockBinding.hpp"

namespace Ace::Application
{
    auto CollectSyntaxes(
        const std::shared_ptr<const ISyntax>& ast
    ) -> std::vector<const ISyntax*>;

    auto CreateAndDeclareSymbols(
        const std::vector<const ISyntax*>& syntaxes
    ) -> Diagnosed<std::vector<FunctionBlockBinding>>;
    auto CreateAndBindFunctionBodies(
        const std::vector<FunctionBlockBinding>& functionBlockBindings
    ) -> Diagnosed<void>;
    auto CreateVerifiedFunctionBlock(
        std::shared_ptr<const BlockStmtSema> functionBlock,
        ITypeSymbol* const functionTypeSymbol
    ) -> Diagnosed<std::shared_ptr<const BlockStmtSema>>;

    auto Main(const std::vector<std::string_view>& args) -> void;
}
