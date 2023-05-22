#include "Node/Expression/StructConstruction.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Error.hpp"
#include "BoundNode/Expression/StructConstruction.hpp"
#include "Symbol/Type/Struct.hpp"
#include "Node/Expression/LiteralSymbol.hpp"
#include "Name.hpp"

namespace Ace::Node::Expression
{
    auto StructConstruction::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        std::for_each(begin(m_Arguments), end(m_Arguments),
        [&](const Argument& t_argument)
        {
            if (t_argument.OptValue.has_value())
            {
                AddChildren(children, t_argument.OptValue.value());
            }
        });

        return children;
    }

    auto StructConstruction::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expression::StructConstruction>
    {
        std::vector<Argument> clonedArguments{};
        std::transform(begin(m_Arguments), end(m_Arguments), back_inserter(clonedArguments),
        [&](const Argument& t_argument)
        {
            auto clonedOptValue = [&]() -> std::optional<std::shared_ptr<const Node::Expression::IBase>>
            {
                if (!t_argument.OptValue.has_value())
                    return std::nullopt;

                return t_argument.OptValue.value()->CloneInScopeExpression(t_scope);
            }();

            return Argument{ t_argument.Name, clonedOptValue };
        });

        return std::make_shared<const Node::Expression::StructConstruction>(
            t_scope,
            m_TypeName,
            std::move(clonedArguments)
        );
    }

    auto StructConstruction::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expression::StructConstruction>>
    {
        ACE_TRY(structSymbol, m_Scope->ResolveStaticSymbol<Symbol::Type::Struct>(m_TypeName));

        const auto variables = structSymbol->GetVariables();
        ACE_TRY_ASSERT(variables.size() == m_Arguments.size());

        ACE_TRY(boundArguments, TransformExpectedVector(m_Arguments,
        [&](const Argument& t_argument) -> Expected<BoundNode::Expression::StructConstruction::Argument>
        {
            const auto symbolFoundIt = std::find_if(begin(variables), end(variables),
            [&](const Symbol::Variable::Normal::Instance* const t_variable)
            {
                return t_variable->GetName() == t_argument.Name;
            });

            ACE_TRY_ASSERT(symbolFoundIt != end(variables));
            auto* const variableSymbol = *symbolFoundIt;

            ACE_TRY(boundValue, ([&]() -> Expected<std::shared_ptr<const BoundNode::Expression::IBase>>
            {
                if (t_argument.OptValue.has_value())
                {
                    return t_argument.OptValue.value()->CreateBoundExpression();
                }

                const SymbolName symbolName
                {
                    SymbolNameSection{ t_argument.Name },
                    SymbolNameResolutionScope::Local,
                };

                return std::make_shared<const Node::Expression::LiteralSymbol>(
                    m_Scope,
                    symbolName
                )->CreateBoundExpression();
            }()));

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(variableSymbol, m_Scope));

            return BoundNode::Expression::StructConstruction::Argument
            {
                variableSymbol,
                boundValue
            };
        }));

        return std::make_shared<const BoundNode::Expression::StructConstruction>(
            m_Scope,
            structSymbol,
            boundArguments
            );
    }
}
