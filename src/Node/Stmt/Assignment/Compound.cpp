#include "Node/Stmt/Assignment/Compound.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Assignment/Compound.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node::Stmt::Assignment
{
    auto Compound::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto Compound::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Stmt::Assignment::Compound>
    {
        return std::make_shared<const Node::Stmt::Assignment::Compound>(
            m_Scope,
            m_LHSExpr->CloneInScopeExpr(t_scope),
            m_RHSExpr->CloneInScopeExpr(t_scope),
            m_Operator
        );
    }

    auto Compound::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Assignment::Compound>>
    {
        ACE_TRY(boundLHSExpr, m_LHSExpr->CreateBoundExpr());
        ACE_TRY(boundRHSExpr, m_RHSExpr->CreateBoundExpr());

        auto* const lhsTypeSymbol = boundLHSExpr->GetTypeInfo().Symbol;
        auto* const rhsTypeSymbol = boundRHSExpr->GetTypeInfo().Symbol;

        const auto operatorNameIt = SpecialIdentifier::Operator::BinaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::BinaryNameMap));

        auto operatorFullName = lhsTypeSymbol->GetWithoutReference()->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, m_Scope->ResolveStaticSymbol<FunctionSymbol>(
            operatorFullName,
            Scope::CreateArgTypes({ lhsTypeSymbol, rhsTypeSymbol })
        ));

        return std::make_shared<const BoundNode::Stmt::Assignment::Compound>(
            boundLHSExpr,
            boundRHSExpr,
            operatorSymbol
        );
    }
}
