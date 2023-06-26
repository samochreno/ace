#include "Node/Stmt/Label.hpp"

#include <memory>
#include <vector>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Stmt/Label.hpp"
#include "Symbols/LabelSymbol.hpp"
#include "Symbols/Symbol.hpp"

namespace Ace::Node::Stmt
{
    Label::Label(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto Label::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Label::GetChildren() const -> std::vector<const Node::IBase*>
    {
        return {};
    }

    auto Label::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::Label>
    {
        return std::make_shared<const Node::Stmt::Label>(
            t_scope,
            m_Name
        );
    }

    auto Label::CloneInScopeStmt(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Stmt::IBase>
    {
        return CloneInScope(t_scope);
    }

    auto Label::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Stmt::Label>>
    {
        auto* const selfSymbol =
            m_Scope->ExclusiveResolveSymbol<LabelSymbol>(m_Name).Unwrap();

        return std::make_shared<const BoundNode::Stmt::Label>(selfSymbol);
    }

    auto Label::CreateBoundStmt() const -> Expected<std::shared_ptr<const BoundNode::Stmt::IBase>>
    {
        return CreateBound();
    }

    auto Label::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Label::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Label;
    }

    auto Label::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Label::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<LabelSymbol>(
                m_Scope, 
                m_Name
            )
        };
    }

    auto Label::GetName() const -> const std::string&
    {
        return m_Name;
    }
}
