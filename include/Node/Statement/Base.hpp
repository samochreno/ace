#pragma once

#include <memory>
#include <vector>

#include "Node/Base.hpp"
#include "BoundNode/Statement/Base.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Statement
{
    class IBase : public virtual Node::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto CloneInScopeStatement(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::IBase> = 0;
        
        virtual auto CreateBoundStatement() const -> Expected<std::shared_ptr<const BoundNode::Statement::IBase>> = 0;
    };
}
