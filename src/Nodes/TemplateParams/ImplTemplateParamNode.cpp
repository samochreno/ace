#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"

namespace Ace
{
    ImplTemplateParamNode::ImplTemplateParamNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const Identifier& name
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Name{ name }
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
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ImplTemplateParamNode>
    {
        return std::make_shared<const ImplTemplateParamNode>(
            m_SourceLocation,
            scope,
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
                m_Name
            )
        };
    }

    auto ImplTemplateParamNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }
}
