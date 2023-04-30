#include "Node/Expression/FunctionCall.hpp"

#include <memory>
#include <vector>
#include <iterator>

#include "Scope.hpp"
#include "Error.hpp"
#include "Node/Expression/Base.hpp"
#include "Node/Expression/LiteralSymbol.hpp"
#include "Node/Expression/MemberAccess.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "BoundNode/Expression/FunctionCall/Instance.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Function.hpp"

namespace Ace::Node::Expression
{
    auto FunctionCall::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expression);
        AddChildren(children, m_Arguments);

        return children;
    }

    auto FunctionCall::CloneInScope(Scope* const t_scope) const -> std::shared_ptr<const Node::Expression::FunctionCall>
    {
        std::vector<std::shared_ptr<const Node::Expression::IBase>> clonedArguments{};
        std::transform(begin(m_Arguments), end(m_Arguments), back_inserter(clonedArguments),
        [&](const std::shared_ptr<const Node::Expression::IBase>& t_argument)
        {
            return t_argument->CloneInScopeExpression(t_scope);
        });

        return std::make_unique<const Node::Expression::FunctionCall>(
            m_Expression->CloneInScopeExpression(t_scope),
            clonedArguments
            );
    }

    auto FunctionCall::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        ACE_TRY(boundArguments, TransformExpectedVector(m_Arguments,
        [](const std::shared_ptr<const Node::Expression::IBase>& t_argument)
        {
            return t_argument->CreateBoundExpression();
        }));

        std::vector<Symbol::Type::IBase*> argumentTypeSymbols{};
        std::transform(begin(boundArguments), end(boundArguments), back_inserter(argumentTypeSymbols),
        [](const std::shared_ptr<const BoundNode::Expression::IBase>& t_argument)
        {
            return t_argument->GetTypeInfo().Symbol;
        });

        if (auto literalSymbol = dynamic_cast<const Node::Expression::LiteralSymbol*>(m_Expression.get()))
        {
            ACE_TRY(functionSymbol, GetScope()->ResolveStaticSymbol<Symbol::Function>(literalSymbol->GetName()));

            return std::shared_ptr<const BoundNode::Expression::IBase>
            {
                std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
                    GetScope(),
                    functionSymbol,
                    boundArguments
                    )
            };
        }
        else if (auto memberAccess = dynamic_cast<const Node::Expression::MemberAccess*>(m_Expression.get()))
        {
            ACE_TRY(boundExpression, memberAccess->GetExpression()->CreateBoundExpression());

            ACE_TRY(functionSymbol, GetScope()->ResolveInstanceSymbol<Symbol::Function>(
                boundExpression->GetTypeInfo().Symbol->GetWithoutReference(),
                memberAccess->GetName()
                ));

            return std::shared_ptr<const BoundNode::Expression::IBase>
            {
                std::make_shared<const BoundNode::Expression::FunctionCall::Instance>(
                    boundExpression,
                    functionSymbol,
                    boundArguments
                    )
            };
        }

        ACE_TRY_UNREACHABLE();
    }
}
