#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    NormalTemplateParamNode::NormalTemplateParamNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const Identifier& name
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto NormalTemplateParamNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto NormalTemplateParamNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateParamNode::GetChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto NormalTemplateParamNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const NormalTemplateParamNode>
    {
        return std::make_shared<const NormalTemplateParamNode>(
            m_SourceLocation,
            scope,
            m_Name
        );
    }

    auto NormalTemplateParamNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateParamNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TemplateParam;
    }

    auto NormalTemplateParamNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto NormalTemplateParamNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<NormalTemplateParamTypeSymbol>(
                m_Scope,
                m_Name
            )
        };
    }

    auto NormalTemplateParamNode::GetName() const -> const Identifier&
    {
        return m_Name;
    }
}
