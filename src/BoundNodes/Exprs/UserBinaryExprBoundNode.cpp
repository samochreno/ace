#include "BoundNodes/Exprs/UserBinaryExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UserBinaryExprBoundNode::UserBinaryExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& lhsExpr,
        const std::shared_ptr<const IExprBoundNode>& rhsExpr,
        FunctionSymbol* const opSymbol
    ) : m_SrcLocation{ srcLocation },
        m_LHSExpr{ lhsExpr },
        m_RHSExpr{ rhsExpr },
        m_OpSymbol{ opSymbol }
    {
    }

    auto UserBinaryExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UserBinaryExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpr->GetScope();
    }

    auto UserBinaryExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_LHSExpr);
        AddChildren(children, m_RHSExpr);

        return children;
    }

    auto UserBinaryExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UserBinaryExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        auto convertedLHSExpr = m_LHSExpr;
        auto convertedRHSExpr = m_RHSExpr;
        if (!m_OpSymbol->IsError())
        {
            const auto argTypeInfos = m_OpSymbol->CollectArgTypeInfos();

            const auto dgnConvertedLHSExpr = CreateImplicitlyConverted(
                convertedLHSExpr,
                argTypeInfos.at(0)
            );
            diagnostics.Add(dgnConvertedLHSExpr);
            convertedLHSExpr = dgnConvertedLHSExpr.Unwrap();

            const auto dgnConvertedRHSExpr = CreateImplicitlyConverted(
                convertedRHSExpr,
                argTypeInfos.at(1)
            );
            diagnostics.Add(dgnConvertedRHSExpr);
            convertedRHSExpr = dgnConvertedRHSExpr.Unwrap();
        }

        const auto dgnCheckedLHSExpr =
            convertedLHSExpr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedLHSExpr);

        const auto dgnCheckedRHSExpr =
            convertedRHSExpr->CreateTypeCheckedExpr({});
        diagnostics.Add(dgnCheckedRHSExpr);

        if (
            (dgnCheckedLHSExpr.Unwrap() == m_LHSExpr) &&
            (dgnCheckedRHSExpr.Unwrap() == m_RHSExpr)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const UserBinaryExprBoundNode>(
                GetSrcLocation(),
                dgnCheckedLHSExpr.Unwrap(),
                dgnCheckedRHSExpr.Unwrap(),
                m_OpSymbol
            ),
            diagnostics,
        };
    }

    auto UserBinaryExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto UserBinaryExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticFunctionCallExprBoundNode>
    {
        const auto loweredLHSExpr = m_LHSExpr->CreateLoweredExpr({});
        const auto loweredRHSExpr = m_RHSExpr->CreateLoweredExpr({});

        std::vector<std::shared_ptr<const IExprBoundNode>> args{};
        if (!m_OpSymbol->IsError())
        {
            args.push_back(loweredLHSExpr);
            args.push_back(loweredRHSExpr);
        }

        return std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_OpSymbol,
            args
        )->CreateLowered({});
    }

    auto UserBinaryExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto UserBinaryExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinaryExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_OpSymbol->GetType(), ValueKind::R };
    }
}
