#include "Nodes/Exprs/StructConstructionExprNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "Nodes/Exprs/LiteralSymbolExprNode.hpp"
#include "Name.hpp"

namespace Ace
{
    StructConstructionExprNode::StructConstructionExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const SymbolName& t_typeName,
        std::vector<StructConstructionExprArg>&& t_args
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_TypeName{ t_typeName },
        m_Args{ t_args }
    {
    }

    auto StructConstructionExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto StructConstructionExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprArg& t_arg)
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
        std::vector<StructConstructionExprArg> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const StructConstructionExprArg& t_arg)
        {
            const auto clonedOptValue = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
            {
                if (!t_arg.OptValue.has_value())
                {
                    return std::nullopt;
                }

                return t_arg.OptValue.value()->CloneInScopeExpr(t_scope);
            }();

            return StructConstructionExprArg
            {
                t_arg.Name,
                clonedOptValue,
            };
        });

        return std::make_shared<const StructConstructionExprNode>(
            m_SourceLocation,
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

        const auto variableSymbols = structSymbol->GetVars();
        ACE_TRY_ASSERT(variableSymbols.size() == m_Args.size());

        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [&](const StructConstructionExprArg& t_arg) -> Expected<StructConstructionExprBoundArg>
        {
            const auto matchingVariableSymbolIt = std::find_if(
                begin(variableSymbols),
                end  (variableSymbols),
                [&](const InstanceVarSymbol* const t_variableSymbol)
                {
                    return
                        t_variableSymbol->GetName().String ==
                        t_arg.Name.String;
                }
            );

            ACE_TRY_ASSERT(matchingVariableSymbolIt != end(variableSymbols));
            auto* const variableSymbol = *matchingVariableSymbolIt;

            ACE_TRY(boundValue, ([&]() -> Expected<std::shared_ptr<const IExprBoundNode>>
            {
                if (t_arg.OptValue.has_value())
                {
                    return t_arg.OptValue.value()->CreateBoundExpr();
                }

                const SymbolName symbolName
                {
                    SymbolNameSection{ t_arg.Name.String },
                    SymbolNameResolutionScope::Local,
                };

                return std::make_shared<const LiteralSymbolExprNode>(
                    t_arg.Name.SourceLocation,
                    m_Scope,
                    symbolName
                )->CreateBoundExpr();
            }()));

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(variableSymbol, m_Scope));

            return StructConstructionExprBoundArg
            {
                variableSymbol,
                boundValue,
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
