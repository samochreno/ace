#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/SizeOf.hpp"
#include "Scope.hpp"
#include "Name.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class SizeOf :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::SizeOf>,
        public virtual Node::IBindable<BoundNode::Expr::SizeOf>
    {
    public:
        SizeOf(
            const std::shared_ptr<Scope>& t_scope,
            const TypeName& t_typeName
        ) : m_Scope{ t_scope}, 
            m_TypeName{ t_typeName }
        {
        }
        virtual ~SizeOf() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::SizeOf> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::SizeOf>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        TypeName m_TypeName{};
    };
}
