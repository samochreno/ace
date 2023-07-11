#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    NormalTemplateParamNode::NormalTemplateParamNode(
        const SourceLocation& t_sourceLocation,
        const std::shared_ptr<Scope>& t_scope,
        const Identifier& t_name
    ) : m_SourceLocation{ t_sourceLocation },
        m_Scope{ t_scope },
        m_Name{ t_name }
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
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const NormalTemplateParamNode>
    {
        return std::make_shared<const NormalTemplateParamNode>(
            m_SourceLocation,
            t_scope,
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
