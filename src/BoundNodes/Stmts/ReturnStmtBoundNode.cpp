#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace
{
    ReturnStmtBoundNode::ReturnStmtBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
    {
    }

    auto ReturnStmtBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto ReturnStmtBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ReturnStmtBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ReturnStmtBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        if (m_OptExpr.has_value())
        {
            AddChildren(children, m_OptExpr.value());
        }

        return children;
    }

    auto ReturnStmtBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ReturnStmtBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ReturnStmtBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_OptExpr
        );
    }

    auto ReturnStmtBoundNode::CloneWithDiagnosticsStmt(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto ReturnStmtBoundNode::GetOrCreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>>>
    {
        const bool isFunctionTypeVoid = 
            context.ParentFunctionTypeSymbol == 
            GetCompilation()->GetNatives()->Void.GetSymbol();

        ACE_TRY_ASSERT(m_OptExpr.has_value() != isFunctionTypeVoid);

        if (m_OptExpr.has_value())
        {
            auto* const exprTypeSymbol = 
                m_OptExpr.value()->GetTypeInfo().Symbol->GetUnaliased();

            const bool isExprTypeVoid = 
                exprTypeSymbol ==
                GetCompilation()->GetNatives()->Void.GetSymbol();

            ACE_TRY_ASSERT(!isExprTypeVoid);
        }

        ACE_TRY(mchCheckedOptExpr, TransformExpectedMaybeChangedOptional(m_OptExpr,
        [&](const std::shared_ptr<const IExprBoundNode>& expr)
        {
            return CreateImplicitlyConvertedAndTypeChecked(
                expr,
                { context.ParentFunctionTypeSymbol, ValueKind::R }
            );
        }));

        if (!mchCheckedOptExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ReturnStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            mchCheckedOptExpr.Value
        ));
    }

    auto ReturnStmtBoundNode::GetOrCreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IStmtBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto ReturnStmtBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        const auto mchLoweredOptExpr = TransformMaybeChangedOptional(m_OptExpr,
        [&](const std::shared_ptr<const IExprBoundNode>& expr)
        {
            return expr->GetOrCreateLoweredExpr({});
        });

        if (!mchLoweredOptExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ReturnStmtBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            mchLoweredOptExpr.Value
        )->GetOrCreateLowered(context).Value);
    }

    auto ReturnStmtBoundNode::GetOrCreateLoweredStmt(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IStmtBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto ReturnStmtBoundNode::Emit(Emitter& emitter) const -> void
    {
        if (m_OptExpr.has_value())
        {
            const auto exprEmitResult = m_OptExpr.value()->Emit(emitter);
            
            auto* const typeSymbol = m_OptExpr.value()->GetTypeInfo().Symbol;
            auto* const type = emitter.GetIRType(typeSymbol);

            auto* const allocaInst =
                emitter.GetBlockBuilder().Builder.CreateAlloca(type);

            emitter.EmitCopy(
                allocaInst,
                exprEmitResult.Value,
                typeSymbol
            );

            emitter.EmitDropTmps(exprEmitResult.Tmps);
            emitter.EmitDropLocalVarsBeforeStmt(this);
            
            auto* const loadInst = emitter.GetBlockBuilder().Builder.CreateLoad(
                type,
                allocaInst
            );

            emitter.GetBlockBuilder().Builder.CreateRet(loadInst);
        }
        else
        {
            emitter.EmitDropLocalVarsBeforeStmt(this);

            emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    }
}
