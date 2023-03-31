#include "Node/Statement/Return.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "Scope.hpp"
#include "Node/Expression/Base.hpp"
#include "BoundNode/Statement/Return.hpp"
#include "Error.hpp"

namespace Ace::Node::Statement
{
    auto Return::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        if (m_OptExpression.has_value())
        {
            AddChildren(children, m_OptExpression.value());
        }

        return children;
    }

    auto Return::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Statement::Return>
    {
        const auto clonedOptExpression = [&]() -> std::optional<std::shared_ptr<const Node::Expression::IBase>>
        {
            if (!m_OptExpression.has_value())
            {
                return std::nullopt;
            }

            return m_OptExpression.value()->CloneInScopeExpression(t_scope);
        }();

        return std::make_unique<const Node::Statement::Return>(
            t_scope,
            clonedOptExpression
            );
    }

    auto Return::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Statement::Return>>
    {
        ACE_TRY(boundOptExpression, TransformExpectedOptional(m_OptExpression, []
        (const std::shared_ptr<const Node::Expression::IBase>& t_expression)
        {
            return t_expression->CreateBoundExpression();
        }));

        return std::make_shared<const BoundNode::Statement::Return>(
            m_Scope,
            boundOptExpression
            );
    }
}
