#include "BoundNodes/Exprs/AddressOfExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    AddressOfExprBoundNode::AddressOfExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto AddressOfExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto AddressOfExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto AddressOfExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto AddressOfExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const AddressOfExprBoundNode>>
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
            std::make_shared<const AddressOfExprBoundNode>(
                GetSrcLocation(),
                checkedExpr
            ),
            diagnostics,
        };
    }

    auto AddressOfExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto AddressOfExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const AddressOfExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const AddressOfExprBoundNode>(
            GetSrcLocation(),
            loweredExpr
        )->CreateLowered({});
    }

    auto AddressOfExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto AddressOfExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        const auto exprEmitResult = m_Expr->Emit(emitter);
        tmps.insert(
            end(tmps),
            begin(exprEmitResult.Tmps),
            end  (exprEmitResult.Tmps)
        );

        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        auto* const type = llvm::PointerType::get(
            emitter.GetIRType(typeSymbol),
            0
        );

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(type);
        tmps.emplace_back(
            allocaInst, 
            GetCompilation()->GetNatives()->Ptr.GetSymbol()
        );

        emitter.GetBlockBuilder().Builder.CreateStore(
            exprEmitResult.Value, 
            allocaInst
        );

        return { allocaInst, tmps };
    }

    auto AddressOfExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return 
        { 
            GetCompilation()->GetNatives()->Ptr.GetSymbol(), 
            ValueKind::R
        };
    }
}
