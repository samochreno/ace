#pragma once

#include <memory>
#include <vector>

#include "Node/Expression/Base.hpp"
#include "BoundNode/Expression/VariableReference/Static.hpp"
#include "Name.hpp"
#include "Scope.hpp"
#include "Error.hpp"

namespace Ace::Node::Expression
{
    class LiteralSymbol :
        public virtual Node::Expression::IBase,
        public virtual Node::ICloneable<Node::Expression::LiteralSymbol>, 
        public virtual Node::IBindable<BoundNode::Expression::VariableReference::Static>
    {
    public:
        LiteralSymbol(
            Scope* const t_scope,
            const SymbolName& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~LiteralSymbol() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetChildren() const -> std::vector<const Node::IBase*> final;
        auto CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::LiteralSymbol> final;
        auto CloneInScopeExpression(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::IBase> final { return CloneInScope(t_scope); }
        auto CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>> final;
        auto CreateBoundExpression() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>> final { return CreateBound(); }

        auto GetName() const -> const SymbolName& { return m_Name; }

    private:
        Scope* m_Scope{};
        SymbolName m_Name;
    };
}
