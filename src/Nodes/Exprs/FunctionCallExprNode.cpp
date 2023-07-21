#include "Nodes/Exprs/FunctionCallExprNode.hpp"

#include <memory>
#include <vector>
#include <iterator>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "Nodes/Exprs/SymbolLiteralExprNode.hpp"
#include "Nodes/Exprs/MemberAccessExprNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace
{
    FunctionCallExprNode::FunctionCallExprNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const std::vector<std::shared_ptr<const IExprNode>>& args
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr },
        m_Args{ args }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const FunctionCallExprNode>
    {
        std::vector<std::shared_ptr<const IExprNode>> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const std::shared_ptr<const IExprNode>& arg)
        {
            return arg->CloneInScopeExpr(scope);
        });

        return std::make_shared<const FunctionCallExprNode>(
            m_SourceLocation,
            m_Expr->CloneInScopeExpr(scope),
            clonedArgs
        );
    }

    auto FunctionCallExprNode::CloneInScopeExpr(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IExprNode>
    {
        return CloneInScope(scope);
    }

    auto FunctionCallExprNode::CreateBound() const -> Expected<std::shared_ptr<const IExprBoundNode>>
    {
        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [](const std::shared_ptr<const IExprNode>& arg)
        {
            return arg->CreateBoundExpr();
        }));

        std::vector<ITypeSymbol*> argTypeSymbols{};
        std::transform(
            begin(boundArgs),
            end  (boundArgs),
            back_inserter(argTypeSymbols),
            [](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                return arg->GetTypeInfo().Symbol;
            }
        );

        if (const auto* const symbolLiteralExpr = dynamic_cast<const SymbolLiteralExprNode*>(m_Expr.get()))
        {
            ACE_TRY(functionSymbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                symbolLiteralExpr->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const IExprBoundNode>
            {
                std::make_shared<const StaticFunctionCallExprBoundNode>(
                    GetSourceLocation(),
                    GetScope(),
                    functionSymbol,
                    boundArgs
                )
            };
        }
        
        if (const auto* const memberAccessExpr = dynamic_cast<const MemberAccessExprNode*>(m_Expr.get()))
        {
            ACE_TRY(boundExpr, memberAccessExpr->GetExpr()->CreateBoundExpr());

            ACE_TRY(functionSymbol, GetScope()->ResolveInstanceSymbol<FunctionSymbol>(
                boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
                memberAccessExpr->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const IExprBoundNode>
            {
                std::make_shared<const InstanceFunctionCallExprBoundNode>(
                    GetSourceLocation(),
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
