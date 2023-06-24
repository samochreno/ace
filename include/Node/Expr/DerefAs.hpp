#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/DerefAs.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class DerefAs :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::DerefAs>,
        public virtual Node::IBindable<BoundNode::Expr::DerefAs>
    {
    public:
        DerefAs(
            const TypeName& t_typeName, 
            const std::shared_ptr<const Node::Expr::IBase>& t_expr
        ) : m_TypeName{ t_typeName },
            m_Expr{ t_expr }
        {
        }
        virtual ~DerefAs() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Expr->GetScope(); }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::DerefAs> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::DerefAs>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }
        
    private:
        TypeName m_TypeName{};
        std::shared_ptr<const Node::Expr::IBase> m_Expr{};
    };
}
