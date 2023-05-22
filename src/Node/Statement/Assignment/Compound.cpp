#include "Node/Statement/Assignment/Compound.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Statement/Assignment/Compound.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node::Statement::Assignment
{
    auto Compound::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto Compound::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Assignment::Compound>
    {
        return std::make_shared<const Node::Statement::Assignment::Compound>(
            m_Scope,
            m_LHSExpression->CloneInScopeExpression(t_scope),
            m_RHSExpression->CloneInScopeExpression(t_scope),
            m_Operator
        );
    }

    auto Compound::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Assignment::Compound>>
    {
        ACE_TRY(boundLHSExpression, m_LHSExpression->CreateBoundExpression());
        ACE_TRY(boundRHSExpression, m_RHSExpression->CreateBoundExpression());

        auto* const lhsTypeSymbol = boundLHSExpression->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpression->GetTypeInfo().Symbol;

        const auto operatorNameIt = SpecialIdentifier::Operator::BinaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::BinaryNameMap));

        auto operatorFullName = lhsTypeSymbol->GetWithoutReference()->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, m_Scope->ResolveStaticSymbol<Symbol::Function>(
            operatorFullName,
            Scope::CreateArgumentTypes({ lhsTypeSymbol, rhsTypeSymbol })
        ));

        return std::make_shared<const BoundNode::Statement::Assignment::Compound>(
            boundLHSExpression,
            boundRHSExpression,
            operatorSymbol
        );
    }
}
