#include "Nodes/Exprs/StructConstructionExprNode.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Nodes/Exprs/LiteralSymbolExprNode.hpp"
#include "Name.hpp"

namespace Ace
{
    StructConstructionExprNode::StructConstructionExprNode(
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_typeName,
        std::vector<Arg>&& t_args
    ) : m_Scope{ t_scope },
        m_TypeName{ t_typeName },
        m_Args{ t_args }
    {
    }

    auto StructConstructionExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

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

    auto StructConstructionExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const StructConstructionExprNode>
    {
        std::vector<Arg> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const Arg& t_arg)
        {
            const auto clonedOptValue = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
            {
                if (!t_arg.OptValue.has_value())
                {
                    return std::nullopt;
                }

                return t_arg.OptValue.value()->CloneInScopeExpr(t_scope);
            }();

            return Arg{ t_arg.Name, clonedOptValue };
        });

        return std::make_shared<const StructConstructionExprNode>(
            t_scope,
            m_TypeName,
            std::move(clonedArgs)
        );
    }

    auto StructConstructionExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto StructConstructionExprNode::CreateBound() const -> Expected<std::shared_ptr<const StructConstructionExprBoundNode>>
    {
        ACE_TRY(structSymbol, m_Scope->ResolveStaticSymbol<StructTypeSymbol>(m_TypeName));

        const auto variables = structSymbol->GetVars();
        ACE_TRY_ASSERT(variables.size() == m_Args.size());

        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [&](const Arg& t_arg) -> Expected<StructConstructionExprBoundNode::Arg>
        {
            const auto symbolFoundIt = std::find_if(
                begin(variables),
                end  (variables),
                [&](const InstanceVarSymbol* const t_variable)
                {
                    return t_variable->GetName() == t_arg.Name;
                }
            );

            ACE_TRY_ASSERT(symbolFoundIt != end(variables));
            auto* const variableSymbol = *symbolFoundIt;

            ACE_TRY(boundValue, ([&]() -> Expected<std::shared_ptr<const IExprBoundNode>>
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

                return std::make_shared<const LiteralSymbolExprNode>(
                    m_Scope,
                    symbolName
                )->CreateBoundExpr();
            }()));

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(variableSymbol, m_Scope));

            return StructConstructionExprBoundNode::Arg
            {
                variableSymbol,
                boundValue
            };
        }));

        return std::make_shared<const StructConstructionExprBoundNode>(
            m_Scope,
            structSymbol,
            boundArgs
        );
    }

    auto StructConstructionExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
