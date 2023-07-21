#include "BoundNodes/Exprs/DereferenceExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    DereferenceExprBoundNode::DereferenceExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr }
    {
    }

    auto DereferenceExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto DereferenceExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DereferenceExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DereferenceExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DereferenceExprBoundNode>(
            GetSourceLocation(),
            mchCheckedExpr.Value
        ));
    }
    auto DereferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto DereferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const DereferenceExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const DereferenceExprBoundNode>(
            GetSourceLocation(),
            mchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto DereferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto DereferenceExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );
            
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        auto* const type = llvm::PointerType::get(
            emitter.GetIRType(typeSymbol), 
            0
        );

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            type,
            exprEmitResult.Value
        );
        temporaries.emplace_back(loadInst, typeSymbol->GetWithoutReference());

        return { loadInst, temporaries };
    }

    auto DereferenceExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expr->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        return { typeSymbol->GetWithoutReference(), typeInfo.ValueKind };
    }

    auto DereferenceExprBoundNode::GetExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return m_Expr;
    }
}
