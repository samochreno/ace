#pragma once

#include <memory>
#include <vector>

#include "Node/Expr/Base.hpp"
#include "BoundNode/Expr/VarReference/Static.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"

namespace Ace::Node::Expr
{
    class LiteralSymbol :
        public virtual Node::Expr::IBase,
        public virtual Node::ICloneable<Node::Expr::LiteralSymbol>, 
        public virtual Node::IBindable<BoundNode::Expr::VarReference::Static>
    {
    public:
        LiteralSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const SymbolName& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~LiteralSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::LiteralSymbol> final;
        auto CloneInScopeExpr(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::VarReference::Static>> final;
        auto CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>> final { return CreateBound(); }

        auto GetName() const -> const SymbolName& { return m_Name; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        SymbolName m_Name;
    };
}
