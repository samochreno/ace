#include "Nodes/Stmts/LabelStmtNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/Stmts/LabelStmtBoundNode.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace
{
    LabelStmtNode::LabelStmtNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto LabelStmtNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto LabelStmtNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto LabelStmtNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto LabelStmtNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const LabelStmtNode>
    {
        return std::make_shared<const LabelStmtNode>(
            m_SourceLocation,
            t_scope,
            m_Name
        );
    }

    auto LabelStmtNode::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const IStmtNode>
    {
        return CloneInScope(t_scope);
    }

    auto LabelStmtNode::CreateBound() const -> Expected<std::shared_ptr<const LabelStmtBoundNode>>
    {
        auto* const selfSymbol = m_Scope->ExclusiveResolveSymbol<LabelSymbol>(
            m_Name
        ).Unwrap();

        return std::make_shared<const LabelStmtBoundNode>(
            GetSourceLocation(),
            selfSymbol
        );
    }

    auto LabelStmtNode::CreateBoundStmt() const -> Expected<std::shared_ptr<const IStmtBoundNode>>
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

    auto LabelStmtNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<LabelSymbol>(
                m_Scope, 
                m_Name
            )
        };
    }

    auto LabelStmtNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }
}
