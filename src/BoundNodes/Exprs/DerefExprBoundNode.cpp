#include "BoundNodes/Exprs/DerefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    DerefExprBoundNode::DerefExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto DerefExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto DerefExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto DerefExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto DerefExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const DerefExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const DerefExprBoundNode>(
                GetSrcLocation(),
                checkedExpr
            ),
            diagnostics,
        };
    }
    auto DerefExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto DerefExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const DerefExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const DerefExprBoundNode>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto DerefExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto DerefExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );
            
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        auto* const type = llvm::PointerType::get(
            emitter.GetIRType(typeSymbol), 
            0
        );

        auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
            type,
            exprEmitResult.Value
        );
        tmps.emplace_back(loadInst, typeSymbol->GetWithoutRef());

        return { loadInst, tmps };
    }

    auto DerefExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expr->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsRef());

        return { typeSymbol->GetWithoutRef(), typeInfo.ValueKind };
    }

    auto DerefExprBoundNode::GetExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return m_Expr;
    }
}
