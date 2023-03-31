#pragma once

#include "BoundNode/Statement/Base.hpp"
#include "Symbol/Label.hpp"

namespace Ace::BoundNode::Statement::Jump
{
    class IBase : public virtual BoundNode::Statement::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetLabelSymbol() const -> Symbol::Label* = 0;
    };
}
