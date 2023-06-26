#include "Node/Expr/FunctionCall.hpp"

#include <memory>
#include <vector>
#include <iterator>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Node/Expr/Base.hpp"
#include "Node/Expr/LiteralSymbol.hpp"
#include "Node/Expr/MemberAccess.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "BoundNode/Expr/FunctionCall/Instance.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"

namespace Ace::Node::Expr
{
    FunctionCall::FunctionCall(
        const std::shared_ptr<const Node::Expr::IBase>& t_expr,
        const std::vector<std::shared_ptr<const Node::Expr::IBase>>& t_args
    ) : m_Expr{ t_expr },
        m_Args{ t_args }
    {
    }

    auto FunctionCall::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto FunctionCall::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Expr);
        AddChildren(children, m_Args);

        return children;
    }

    auto FunctionCall::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::FunctionCall>
    {
        std::vector<std::shared_ptr<const Node::Expr::IBase>> clonedArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(clonedArgs),
        [&](const std::shared_ptr<const Node::Expr::IBase>& t_arg)
        {
            return t_arg->CloneInScopeExpr(t_scope);
        });

        return std::make_shared<const Node::Expr::FunctionCall>(
            m_Expr->CloneInScopeExpr(t_scope),
            clonedArgs
        );
    }

    auto FunctionCall::CloneInScopeExpr(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Expr::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto FunctionCall::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        ACE_TRY(boundArgs, TransformExpectedVector(m_Args,
        [](const std::shared_ptr<const Node::Expr::IBase>& t_arg)
        {
            return t_arg->CreateBoundExpr();
        }));

        std::vector<ITypeSymbol*> argTypeSymbols{};
        std::transform(begin(boundArgs), end(boundArgs), back_inserter(argTypeSymbols),
        [](const std::shared_ptr<const BoundNode::Expr::IBase>& t_arg)
        {
            return t_arg->GetTypeInfo().Symbol;
        });

        if (const auto* const literalSymbol = dynamic_cast<const Node::Expr::LiteralSymbol*>(m_Expr.get()))
        {
            ACE_TRY(functionSymbol, GetScope()->ResolveStaticSymbol<FunctionSymbol>(
                literalSymbol->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const BoundNode::Expr::IBase>
            {
                std::make_shared<const BoundNode::Expr::FunctionCall::Static>(
                    GetScope(),
                    functionSymbol,
                    boundArgs
                )
            };
        }
        else if (const auto* const memberAccess = dynamic_cast<const Node::Expr::MemberAccess*>(m_Expr.get()))
        {
            ACE_TRY(boundExpr, memberAccess->GetExpr()->CreateBoundExpr());

            ACE_TRY(functionSymbol, GetScope()->ResolveInstanceSymbol<FunctionSymbol>(
                boundExpr->GetTypeInfo().Symbol->GetWithoutReference(),
                memberAccess->GetName(),
                Scope::CreateArgTypes(argTypeSymbols)
            ));

            return std::shared_ptr<const BoundNode::Expr::IBase>
            {
                std::make_shared<const BoundNode::Expr::FunctionCall::Instance>(
                    boundExpr,
                    functionSymbol,
                    boundArgs
                )
            };
        }

        ACE_TRY_UNREACHABLE();
    }

    auto FunctionCall::CreateBoundExpr() const -> Expected<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return CreateBound();
    }
}
