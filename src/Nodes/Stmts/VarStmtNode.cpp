#include "Nodes/Stmts/VarStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace
{
    VarStmtNode::VarStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name,
        const TypeName& typeName,
        const std::optional<std::shared_ptr<const IExprNode>>& optAssignedExpr
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name },
        m_TypeName{ typeName },
        m_OptAssignedExpr{ optAssignedExpr }
    {
    }

    auto VarStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto VarStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VarStmtNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto VarStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const VarStmtNode>
    {
        const auto optAssignedExpr = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
        {
            if (!m_OptAssignedExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptAssignedExpr.value()->CloneInScopeExpr(scope);
        }();

        return std::make_shared<const VarStmtNode>(
            m_SrcLocation,
            scope,
            m_Name,
            m_TypeName,
            optAssignedExpr
        );
    }

    auto VarStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto VarStmtNode::CreateBound() const -> Expected<std::shared_ptr<const VarStmtBoundNode>>
    {
        auto* selfSymbol = m_Scope->ExclusiveResolveSymbol<LocalVarSymbol>(
            m_Name
        ).Unwrap();

        ACE_TRY(boundOptAssignedExpr, TransformExpectedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const IExprNode>& expr)
        {
            return expr->CreateBoundExpr();
        }));

        return std::make_shared<const VarStmtBoundNode>(
            GetSrcLocation(),
            selfSymbol,
            boundOptAssignedExpr
        );
    }

    auto VarStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }

    auto VarStmtNode::GetName() const -> const Ident&
    {
        return m_Name;
    }

    auto VarStmtNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VarStmtNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::LocalVar;
    }

    auto VarStmtNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto VarStmtNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        DiagnosticBag diagnosticBag{};

        const auto expTypeSymbol = m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        );
        diagnosticBag.Add(expTypeSymbol);

        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<LocalVarSymbol>(
                m_Scope,
                m_Name,
                expTypeSymbol.UnwrapOr(GetCompilation()->ErrorTypeSymbol)
            ),
            diagnosticBag,
        };
    }
}
