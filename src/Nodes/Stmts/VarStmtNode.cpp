#include "Nodes/Stmts/VarStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Nodes/Exprs/ExprNode.hpp"
#include "BoundNodes/Stmts/VarStmtBoundNode.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"

namespace Ace
{
    VarStmtNode::VarStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name,
        const TypeName& t_typeName,
        const std::optional<std::shared_ptr<const IExprNode>>& t_optAssignedExpr
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name },
        m_TypeName{ t_typeName },
        m_OptAssignedExpr{ t_optAssignedExpr }
    {
    }

    auto VarStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto VarStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto VarStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        if (m_OptAssignedExpr.has_value())
        {
            AddChildren(children, m_OptAssignedExpr.value());
        }

        return children;
    }

    auto VarStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const VarStmtNode>
    {
        const auto optAssignedExpr = [&]() -> std::optional<std::shared_ptr<const IExprNode>>
        {
            if (!m_OptAssignedExpr.has_value())
            {
                return std::nullopt;
            }

            return m_OptAssignedExpr.value()->CloneInScopeExpr(t_scope);
        }();

        return std::make_shared<const VarStmtNode>(
            m_SourceLocation,
            t_scope,
            m_Name,
            m_TypeName,
            optAssignedExpr
        );
    }

    auto VarStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto VarStmtNode::CreateBound() const -> Expected<std::shared_ptr<const VarStmtBoundNode>>
    {
        auto* selfSymbol = m_Scope->ExclusiveResolveSymbol<LocalVarSymbol>(
            m_Name.String
        ).Unwrap();

        ACE_TRY(boundOptAssignedExpr, TransformExpectedOptional(m_OptAssignedExpr,
        [](const std::shared_ptr<const IExprNode>& t_expr)
        {
            return t_expr->CreateBoundExpr();
        }));

        return std::make_shared<const VarStmtBoundNode>(
            selfSymbol,
            boundOptAssignedExpr
        );
    }

    auto VarStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }

    auto VarStmtNode::GetName() const -> const Identifier&
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

    auto VarStmtNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        ACE_TRY(typeSymbol, m_Scope->ResolveStaticSymbol<ITypeSymbol>(
            m_TypeName.ToSymbolName(GetCompilation())
        ));

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<LocalVarSymbol>(
                m_Scope,
                m_Name.String,
                typeSymbol
            )
        };
    }
}
