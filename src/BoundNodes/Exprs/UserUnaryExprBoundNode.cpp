#include "BoundNodes/Exprs/UserUnaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UserUnaryExprBoundNode::UserUnaryExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserUnaryExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserUnaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UserUnaryExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto UserUnaryExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UserUnaryExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        auto convertedExpr = m_Expr;
        if (!m_OpSymbol->IsError())
        {
            const auto argTypeInfos = m_OpSymbol->CollectArgTypeInfos();
            ACE_ASSERT(argTypeInfos.size() == 1);

            const auto dgnConvertedExpr = CreateImplicitlyConverted(
                convertedExpr,
                argTypeInfos.front()
            );
            diagnostics.Add(dgnConvertedExpr);
            convertedExpr = dgnConvertedExpr.Unwrap();
        }

        const auto dgnCheckedExpr = convertedExpr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedExpr);

        if (dgnCheckedExpr.Unwrap() == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const UserUnaryExprBoundNode>(
                GetSrcLocation(),
                dgnCheckedExpr.Unwrap(),
                m_OpSymbol
            ),
            diagnostics,
        };
    }

    auto UserUnaryExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto UserUnaryExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticFunctionCallExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprBoundNode>> args{};
        if (!m_OpSymbol->IsError())
        {
            args.push_back(loweredExpr);
        }

        return std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            args
        )->CreateLowered({});
    }

    auto UserUnaryExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto UserUnaryExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserUnaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
