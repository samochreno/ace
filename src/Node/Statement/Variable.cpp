#include "Node/Statement/Variable.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Variable.hpp"
#include "Diagnostics.hpp"
#include "Symbol/Variable/Local.hpp"

namespace Ace::Node::Statement
{
    auto Variable::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        if (m_OptAssignedExpression.has_value())
        {
            AddChildren(children, m_OptAssignedExpression.value());
        }

        return children;
    }

    auto Variable::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Statement::Variable>
    {
        const auto optAssignedExpression = [&]() -> std::optional<std::shared_ptr<const Node::Expression::IBase>>
        {
            if (!m_OptAssignedExpression.has_value())
                return std::nullopt;

            return m_OptAssignedExpression.value()->CloneInScopeExpression(t_scope);
        }();

        return std::make_shared<const Node::Statement::Variable>(
            t_scope,
            m_Name,
            m_TypeName,
            optAssignedExpression
        );
    }

    auto Variable::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Variable>>
    {
        auto* selfSymbol = m_Scope->ExclusiveResolveSymbol<Symbol::Variable::Local>(m_Name).Unwrap();

        ACE_TRY(boundOptAssignedExpression, TransformExpectedOptional(m_OptAssignedExpression,
        [](const std::shared_ptr<const Node::Expression::IBase>& t_expression)
        {
            return t_expression->CreateBoundExpression();
        }));

        return std::make_shared<const BoundNode::Statement::Variable>(
            selfSymbol,
            boundOptAssignedExpression
            );
    }

    auto Variable::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::IBase>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));
        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Variable::Local>(
                m_Scope,
                m_Name,
                typeSymbol
            )
        };
    }
}
