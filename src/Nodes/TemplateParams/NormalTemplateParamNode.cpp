#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    NormalTemplateParamNode::NormalTemplateParamNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::string& t_name
    ) : m_Scope{ t_scope },
        m_Name{ t_name }
    {
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

    auto NormalTemplateParamNode::GetName() const -> const std::string&
    {
        return m_Name;
    }
}
