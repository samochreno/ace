#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

namespace Ace
{
    ImplTemplateParamNode::ImplTemplateParamNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name }
    {
    }

    auto ImplTemplateParamNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto ImplTemplateParamNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplTemplateParamNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto ImplTemplateParamNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const ImplTemplateParamNode>
    {
        return std::make_shared<const ImplTemplateParamNode>(
            m_SourceLocation,
            t_scope,
            m_Name
        );
    }

    auto ImplTemplateParamNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplTemplateParamNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::ImplTemplateParam;
    }

    auto ImplTemplateParamNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto ImplTemplateParamNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<ImplTemplateParamTypeSymbol>(
                m_Scope,
                m_Name.String
            )
        };
    }

    auto ImplTemplateParamNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }
}
