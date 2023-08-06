#include "BoundNodes/Stmts/ReturnStmtBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

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
        DiagnosticBag diagnostics{};

        auto* const compilation = functionTypeSymbol->GetCompilation();

        const bool isFunctionTypeVoid =
            functionTypeSymbol == compilation->GetNatives()->Void.GetSymbol();

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

        return Diagnosed<void>{ diagnostics };
    }

    static auto DiagnoseUnsizedExpr(
        const std::optional<std::shared_ptr<const IExprBoundNode>>& optExpr
    ) -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        if (!optExpr.has_value())
        {
            return Diagnosed<void>{ diagnostics };
        }

        const auto& expr = optExpr.value();

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol->GetUnaliased();

        const auto expExprTypeSizeKind = exprTypeSymbol->GetSizeKind();
        const bool isExprTypeUnsized = 
            (expExprTypeSizeKind) &&
            (expExprTypeSizeKind.Unwrap() == TypeSizeKind::Unsized);

        if (isExprTypeUnsized)
        {
            diagnostics.Add(CreateReturningUnsizedExprError(
                expr->GetSrcLocation()
            ));
        }

        return Diagnosed<void>{ diagnostics };
    }

    auto ReturnStmtBoundNode::CreateTypeChecked(
        const StmtTypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ReturnStmtBoundNode>>
    {
        DiagnosticBag diagnostics{};

        diagnostics.Add(DiagnoseMissingOrUnexpectedExpr(
            GetSrcLocation(),
            context.ParentFunctionTypeSymbol,
            m_OptExpr
        ));
        diagnostics.Add(DiagnoseUnsizedExpr(m_OptExpr));

        std::optional<std::shared_ptr<const IExprBoundNode>> checkedOptExpr{};
        if (m_OptExpr.has_value())
        {
            const auto dgnExpr = CreateImplicitlyConvertedAndTypeChecked(
                m_OptExpr.value(),
                TypeInfo{ context.ParentFunctionTypeSymbol, ValueKind::R }
            );
            diagnostics.Add(dgnExpr);
            checkedOptExpr = dgnExpr.Unwrap();
        }

        if (checkedOptExpr == m_OptExpr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const ReturnStmtBoundNode>(
                GetSrcLocation(),
                GetScope(),
                checkedOptExpr
            ),
            diagnostics,
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
