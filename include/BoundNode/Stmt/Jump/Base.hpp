#pragma once

#include "BoundNode/Stmt/Base.hpp"
#include "Symbol/Label.hpp"

namespace Ace::BoundNode::Stmt::Jump
{
    class IBase : public virtual BoundNode::Stmt::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetLabelSymbol() const -> Symbol::Label* = 0;
    };
}
