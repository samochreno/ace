#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"

namespace Ace
{
    NormalTemplateParamNode::NormalTemplateParamNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const Ident& name
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Name{ name }
    {
    }

    auto NormalTemplateParamNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalTemplateParamNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto NormalTemplateParamNode::CollectChildren() const -> std::vector<const INode*>
    {
        return {};
    }

    auto NormalTemplateParamNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const NormalTemplateParamNode>
    {
        return std::make_shared<const NormalTemplateParamNode>(
            m_SrcLocation,
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

    auto NormalTemplateParamNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<NormalTemplateParamTypeSymbol>(
                m_Scope,
                m_Name
            ),
            DiagnosticBag::Create(),
        };
    }

    auto NormalTemplateParamNode::GetName() const -> const Ident&
    {
        return m_Name;
    }
}
