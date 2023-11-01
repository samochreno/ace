#pragma once

#include <memory>
#include <vector>

#include "Syntaxes/Syntax.hpp"
#include "Semas/Stmts/StmtSema.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IStmtSyntax : public virtual ISyntax
    {
    public:
        virtual ~IStmtSyntax() = default;

        virtual auto CreateStmtSema() const -> Diagnosed<std::shared_ptr<const IStmtSema>> = 0;
    };
}
