#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "CFA.hpp"

namespace Ace
{
    ReturnStmtBoundNode::ReturnStmtBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_OptExpr{ optExpr }
    {
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

    static auto DiagnoseMissingOrUnexpectedExpr(
        const SrcLocation& srcLocation,
        ITypeSymbol* const functionTypeSymbol,
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const compilation = functionTypeSymbol->GetCompilation();

        const bool isFunctionTypeVoid =
            functionTypeSymbol == compilation->GetVoidTypeSymbol();

        if (!isFunctionTypeVoid && !optExpr.has_value())
        {
            diagnostics.Add(CreateMissingReturnExprError(srcLocation));
        }

        if (isFunctionTypeVoid && optExpr.has_value())
        {
            diagnostics.Add(CreateReturningExprFromVoidFunctionError(
                optExpr.value()->GetSrcLocation()
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    static auto DiagnoseUnsizedExpr(
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
    ) -> Diagnosed<void>
    {
        auto diagnostics = DiagnosticBag::Create();

        if (!optExpr.has_value())
        {
            return Diagnosed<void>{ std::move(diagnostics) };
        }

        const auto& expr = optExpr.value();

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol->GetUnaliased();
        if (dynamic_cast<ISizedTypeSymbol*>(exprTypeSymbol) == nullptr)
        {
            diagnostics.Add(CreateReturningUnsizedExprError(
                expr->GetSrcLocation()
            ));
        }

        return Diagnosed<void>{ std::move(diagnostics) };
    }

    auto ReturnStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        diagnostics.Collect(DiagnoseMissingOrUnexpectedExpr(
            GetSrcLocation(),
            context.ParentFunctionTypeSymbol,
            m_OptExpr
        ));
        diagnostics.Collect(DiagnoseUnsizedExpr(m_OptExpr));

        std::optional<std::shared_ptr<const IExprBoundNode>> checkedOptExpr{};
        if (m_OptExpr.has_value())
        {
            checkedOptExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
                m_OptExpr.value(),
                TypeInfo{ context.ParentFunctionTypeSymbol, ValueKind::R }
            ));
        }

        if (checkedOptExpr == m_OptExpr)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const ReturnStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                checkedOptExpr
            ),
            std::move(diagnostics),
        };
    }

    auto ReturnStmtBoundNode::CreateTypeCheckedStmt(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto ReturnStmtBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ReturnStmtBoundNode>
    {
        const auto loweredOptExpr = m_OptExpr.has_value() ?
            std::optional{ m_OptExpr.value()->CreateLoweredExpr({}) } :
            std::nullopt;

        if (loweredOptExpr == m_OptExpr)
        {
            return shared_from_this();
        }

        return std::make_shared<const ReturnStmtBoundNode>(
            GetSrcLocation(),
            GetScope(),
            loweredOptExpr
        )->CreateLowered({});
    }

    auto ReturnStmtBoundNode::CreateLoweredStmt(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IStmtBoundNode>
    {
        return CreateLowered(context);
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
                dynamic_cast<ISizedTypeSymbol*>(typeSymbol)
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

    auto ReturnStmtBoundNode::CreateCFANodes() const -> std::vector<CFANode>
    {
        return std::vector{ CFANode{ CFANodeKind::Return } };
    }
}
