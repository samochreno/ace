#include "BoundNodes/Exprs/AddressOfExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "ExprDropData.hpp"

namespace Ace
{
    AddressOfExprBoundNode::AddressOfExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto AddressOfExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto AddressOfExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const AddressOfExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const AddressOfExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Expr
        );
    }

    auto AddressOfExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto AddressOfExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const AddressOfExprBoundNode>>>
    {
        ACE_TRY(cchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!cchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }
         
        return CreateChanged(std::make_shared<const AddressOfExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Expr
        ));
    }

    auto AddressOfExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto AddressOfExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const AddressOfExprBoundNode>>
    {
        const auto cchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!cchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const AddressOfExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            cchLoweredExpr.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto AddressOfExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
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
