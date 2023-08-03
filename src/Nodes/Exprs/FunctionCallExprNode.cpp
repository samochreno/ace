#include "Nodes/Exprs/FunctionCallExprNode.hpp"

#include <memory>
#include <vector>
#include <iterator>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
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
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprNode>& expr,
        const std::vector<std::shared_ptr<const IExprNode>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_Args{ args }
    {
    }

    auto FunctionCallExprNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionCallExprNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto FunctionCallExprNode::CollectChildren() const -> std::vector<const INode*>
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
            m_SrcLocation,
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

    auto FunctionCallExprNode::CreateBound() const -> std::shared_ptr<const IExprBoundNode>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const IExprBoundNode>> boundArgs{};
        std::transform(
            begin(m_Args),
            end  (m_Args),
            back_inserter(boundArgs),
            [](const std::shared_ptr<const IExprNode>& arg)
            {
                return arg->CreateBoundExpr();
            }
        );

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
            const auto expFunctionSymbol = GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                symbolLiteralExpr->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            );
            diagnostics.Add(expFunctionSymbol);

            auto* const functionSymbol = expFunctionSymbol.UnwrapOr(
                GetCompilation()->GetErrorSymbols().GetFunction()
            );

            return std::make_shared<const StaticFunctionCallExprBoundNode>(
                diagnostics,
                GetSrcLocation(),
                GetScope(),
                functionSymbol,
                boundArgs
            );
        }
        
        if (const auto* const memberAccessExpr = dynamic_cast<const MemberAccessExprNode*>(m_Expr.get()))
        {
            const auto boundExpr =
                memberAccessExpr->GetExpr()->CreateBoundExpr();

            const auto expFunctionSymbol = GetScope()->ResolveInstanceSymbol<FunctionSymbol>(
                boundExpr->GetTypeInfo().Symbol->GetWithoutRef(),
                memberAccessExpr->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            );
            diagnostics.Add(expFunctionSymbol);

            auto* const functionSymbol = expFunctionSymbol.UnwrapOr(
                GetCompilation()->GetErrorSymbols().GetFunction()
            );

            return std::shared_ptr<const IExprBoundNode>
            {
                std::make_shared<const InstanceFunctionCallExprBoundNode>(
                    diagnostics,
                    GetSrcLocation(),
                    boundExpr,
                    functionSymbol,
                    boundArgs
                )
            };
        }

        const auto boundExpr = m_Expr->CreateBoundExpr();

        diagnostics.Add(CreateExpectedFunctionError(
            boundExpr->GetSrcLocation(),
            boundExpr->GetTypeInfo().Symbol
        ));

        return boundExpr->CloneWithDiagnosticsExpr(diagnostics);
    }

    auto FunctionCallExprNode::CreateBoundExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateBound();
    }
}
