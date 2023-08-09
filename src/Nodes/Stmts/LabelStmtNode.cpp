#include "Nodes/Stmts/LabelStmtNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    LabelStmtNode::LabelStmtNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto LabelStmtNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto LabelStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelStmtNode::CollectChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto LabelStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const LabelStmtNode>
    {
        return std::make_shared<const LabelStmtNode>(
            m_SrcLocation,
            scope,
            m_Name
        );
    }

    auto LabelStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(scope);
    }

    auto LabelStmtNode::CreateBound() const -> Diagnosed<std::shared_ptr<const LabelStmtBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<LabelSymbol>(
            m_Name
        ).Unwrap();

        return Diagnosed
        {
            std::make_shared<const LabelStmtBoundNode>(
                GetSrcLocation(),
                selfSymbol
            ),
            std::move(diagnostics),
        };
    }

    auto LabelStmtNode::CreateBoundStmt() const -> Diagnosed<std::shared_ptr<const IStmtBoundNode>>
    {
        return CreateBound();
    }

    auto LabelStmtNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelStmtNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Label;
    }

    auto LabelStmtNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto LabelStmtNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<LabelSymbol>(
                m_Scope, 
                m_Name
            ),
            DiagnosticBag::Create(),
        };
    }

    auto LabelStmtNode::GetName() const -> const Ident&
    {
        return m_Name;
    }
}
