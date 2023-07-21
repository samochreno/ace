#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    DerefAsExprBoundNode::DerefAsExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        ITypeSymbol* const typeSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_TypeSymbol{ typeSymbol },
        m_Expr{ expr }
    {
    }

    auto DerefAsExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto DerefAsExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefAsExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefAsExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const DerefAsExprBoundNode>>>
    {
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol->GetUnaliased();

        ACE_TRY_ASSERT(
            (typeSymbol == GetCompilation()->Natives->Pointer.GetSymbol()) ||
            (typeSymbol->IsReference())
        );

        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DerefAsExprBoundNode>(
            GetSourceLocation(),
            m_Expr,
            m_TypeSymbol
        ));
    }

    auto DerefAsExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto DerefAsExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const DerefAsExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DerefAsExprBoundNode>(
            GetSourceLocation(),
            m_Expr,
            m_TypeSymbol
        )->GetOrCreateLowered({}).Value);
    }

    auto DerefAsExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto DerefAsExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );

        auto* const pointerType = GetCompilation()->Natives->Pointer.GetIRType();

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            pointerType,
            exprEmitResult.Value
        );

        return { loadInst, temporaries };
    }

    auto DerefAsExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
