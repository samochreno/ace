#include "BoundNodes/Exprs/LogicalNegationExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    LogicalNegationExprBoundNode::LogicalNegationExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr }
    {
    }

    auto LogicalNegationExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto LogicalNegationExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto LogicalNegationExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const LogicalNegationExprBoundNode>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedExpr, CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (!mchConvertedAndCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            GetSourceLocation(),
            mchConvertedAndCheckedExpr.Value
        ));
    }

    auto LogicalNegationExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const LogicalNegationExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const LogicalNegationExprBoundNode>(
            GetSourceLocation(),
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto LogicalNegationExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto LogicalNegationExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            exprEmitResult.Value
        );

        auto* const negatedValue = emitter.GetBlockBuilder().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = emitter.GetBlockBuilder().Builder.CreateAlloca(
            boolType
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            negatedValue,
            allocaInst
        );

        return { allocaInst, exprEmitResult.Temporaries };
    }

    auto LogicalNegationExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->Natives->Bool.GetSymbol(), ValueKind::R };
    }
}
