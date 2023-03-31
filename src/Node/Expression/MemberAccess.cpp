#include "Node/Expression/MemberAccess.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/VariableReference/Instance.hpp"
#include "Error.hpp"
#include "Symbol/Variable/Base.hpp"
#include "Symbol/Type/Base.hpp"

namespace Ace::Node::Expression
{
    auto MemberAccess::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto MemberAccess::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::MemberAccess>
    {
        return std::make_unique<const Node::Expression::MemberAccess>(
            m_Expression->CloneInScopeExpression(t_scope),
            m_Name
            );
    }

    auto MemberAccess::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>
    {
        ACE_TRY(boundExpression, m_Expression->CreateBoundExpression());

        ACE_TRY_ASSERT(m_Name.TemplateArguments.empty());
        ACE_TRY(memberSymbol, GetScope()->ResolveInstanceSymbol<Symbol::Variable::IBase>(
            boundExpression->GetTypeInfo().Symbol->GetWithoutReference(),
            m_Name
            ));

        return std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
            boundExpression,
            memberSymbol
            );
    }
}
