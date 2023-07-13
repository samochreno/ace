#include "Nodes/Exprs/FunctionCallExprNode.hpp"

#include <memory>
#include <vector>
#include <iterator>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Exprs/LiteralSymbolExprNode.hpp"
#include "Nodes/Exprs/MemberAccessExprNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    FunctionCallExprNode::FunctionCallExprNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<const IExprNode>& t_expr,
        const std::vector<std::shared_ptr<const IExprNode>>& t_args
    ) : m_SourceLocation{ t_sourceLocation },
        m_Expr{ t_expr },
        m_Args{ t_args }
    {
    }

    auto FunctionCallExprNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto FunctionCallExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto FunctionCallExprNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Expr);
        AddChildren(children, m_Args);

        return children;
    }

    auto FunctionCallExprNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const FunctionCallExprNode>
    {
        std::vector<std::shared_ptr<const IExprNode>> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const std::shared_ptr<const IExprNode>& t_arg)
        {
            return t_arg->CloneInScopeExpr(t_scope);
        });

        return std::make_shared<const FunctionCallExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(t_scope),
            clonedArgs
        );
    }

    auto FunctionCallExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(t_scope);
    }

    auto FunctionCallExprNode::CreateBound() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [](const std::shared_ptr<const IExprNode>& t_arg)
        {
            return t_arg->CreateBoundExpr();
        }));

        std::vector<ITypeSymbol*> argTypeSymbols{};
        std::transform(
            begin(boundArgs),
            end  (boundArgs),
            back_inserter(argTypeSymbols),
            [](const std::shared_ptr<const IExprBoundNode>& t_arg)
            {
                return t_arg->GetTypeInfo().Symbol;
            }
        );

        if (const auto* const literalSymbol = dynamic_cast<const LiteralSymbolExprNode*>(m_Expr.get()))
        {
            ACE_TRY(functionSymbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                literalSymbol->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const IExprBoundNode>
            {
                std::make_shared<const StaticFunctionCallExprBoundNode>(
                    GetScope(),
                    functionSymbol,
                    boundArgs
                )
            };
        }
        else if (const auto* const memberAccess = dynamic_cast<const MemberAccessExprNode*>(m_Expr.get()))
        {
            ACE_TRY(boundExpr, memberAccess->GetExpr()->CreateBoundExpr());

            ACE_TRY(functionSymbol, GetScope()->ResolveInstanceSymbol<FunctionSymbol>(
                boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
                memberAccess->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const IExprBoundNode>
            {
                std::make_shared<const InstanceFunctionCallExprBoundNode>(
                    boundExpr,
                    functionSymbol,
                    boundArgs
                )
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto FunctionCallExprNode::CreateBoundExpr() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateBound();
    }
}
