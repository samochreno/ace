#include "Node/Expression/UnaryUser.hpp"

#include <memory>
#include <vector>

#include "SpecialIdentifier.hpp"
#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression/UnaryUser.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Function.hpp"

namespace Ace::Node::Expression
{
    auto UnaryUser::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto UnaryUser::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::UnaryUser>
    {
        return std::make_unique<Node::Expression::UnaryUser>(
            m_Expression->CloneInScopeExpression(t_scope),
            m_Operator
            );
    }

    auto UnaryUser::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::UnaryUser>>
    {
        ACE_TRY(boundExpresssion, m_Expression->CreateBoundExpression());

        const auto operatorNameIt = SpecialIdentifier::Operator::UnaryNameMap.find(m_Operator);
        ACE_TRY_ASSERT(operatorNameIt != end(SpecialIdentifier::Operator::UnaryNameMap));

        auto* const typeSymbol = boundExpresssion->GetTypeInfo().Symbol;

        auto operatorFullName = typeSymbol->CreateFullyQualifiedName();
        operatorFullName.Sections.emplace_back(operatorNameIt->second);

        ACE_TRY(operatorSymbol, GetScope()->ResolveStaticSymbol<Symbol::Function>(
            operatorFullName,
            Scope::CreateArgumentTypes(typeSymbol)
        ));

        return std::make_shared<const BoundNode::Expression::UnaryUser>(
            boundExpresssion,
            operatorSymbol
            );
    }
}
