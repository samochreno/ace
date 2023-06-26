#include "Node/Expr/StructConstruction.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Expr/StructConstruction.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Node/Expr/LiteralSymbol.hpp"
#include "Name.hpp"

namespace Ace::Node::Expr
{
    auto StructConstruction::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const Arg& t_arg)
        {
            if (t_arg.OptValue.has_value())
            {
                AddChildren(children, t_arg.OptValue.value());
            }
        });

        return children;
    }

    auto StructConstruction::CloneInScope(const std::shared_ptr<Scope>& t_scope) const -> std::shared_ptr<const Node::Expr::StructConstruction>
    {
        std::vector<Arg> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const Arg& t_arg)
        {
            auto clonedOptValue = [&]() -> std::optional<std::shared_ptr<const Node::Expr::IBase>>
            {
                if (!t_arg.OptValue.has_value())
                    return std::nullopt;

                return t_arg.OptValue.value()->CloneInScopeExpr(t_scope);
            }();

            return Arg{ t_arg.Name, clonedOptValue };
        });

        return std::make_shared<const Node::Expr::StructConstruction>(
            t_scope,
            m_TypeName,
            std::move(clonedArgs)
        );
    }

    auto StructConstruction::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::StructConstruction>>
    {
        ACE_TRY(structSymbol, m_Scope->ResolveStaticSymbol<StructTypeSymbol>(m_TypeName));

        const auto variables = structSymbol->GetVars();
        ACE_TRY_ASSERT(variables.size() == m_Args.size());

        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [&](const Arg& t_arg) -> Expected<BoundNode::Expr::StructConstruction::Arg>
        {
            const auto symbolFoundIt = std::find_if(begin(variables), end(variables),
            [&](const InstanceVarSymbol* const t_variable)
            {
                return t_variable->GetName() == t_arg.Name;
            });

            ACE_TRY_ASSERT(symbolFoundIt != end(variables));
            auto* const variableSymbol = *symbolFoundIt;

            ACE_TRY(boundValue, ([&]() -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
            {
                if (t_arg.OptValue.has_value())
                {
                    return t_arg.OptValue.value()->CreateBoundExpr();
                }

                const SymbolName symbolName
                {
                    SymbolNameSection{ t_arg.Name },
                    SymbolNameResolutionScope::Local,
                };

                return std::make_shared<const Node::Expr::LiteralSymbol>(
                    m_Scope,
                    symbolName
                )->CreateBoundExpr();
            }()));

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(variableSymbol, m_Scope));

            return BoundNode::Expr::StructConstruction::Arg
            {
                variableSymbol,
                boundValue
            };
        }));

        return std::make_shared<const BoundNode::Expr::StructConstruction>(
            m_Scope,
            structSymbol,
            boundArgs
            );
    }
}
