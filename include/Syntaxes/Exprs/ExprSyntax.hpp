#pragma once

#include <memory>

#include "Syntaxes/Syntax.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    class IExprSyntax : public virtual ISyntax
    {
    public:
        virtual ~IExprSyntax() = default;
        
        virtual auto CreateExprSema() const -> Diagnosed<std::shared_ptr<const IExprSema>> = 0;
    };
}
