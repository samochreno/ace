#pragma once

#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Variable/Base.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    class IBase : public virtual BoundNode::Expression::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetVariableSymbol() const -> Symbol::Variable::IBase* = 0;
    };
}
