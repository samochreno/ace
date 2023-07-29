#include "Nodes/Templates/TypeTemplateNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"
#include "Ident.hpp"

namespace Ace
{
    TypeTemplateNode::TypeTemplateNode(
        const SrcLocation& srcLocation,
        const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& params,
        const std::shared_ptr<const ITypeNode>& ast
    ) : m_SrcLocation{ srcLocation },
        m_Params{ params },
        m_AST{ ast }
    {
    }

    auto TypeTemplateNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto TypeTemplateNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetScope();
    }

    auto TypeTemplateNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Params);

        return children;
    }

    auto TypeTemplateNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const TypeTemplateNode>
    {
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const NormalTemplateParamNode>& param)
            {
                return param->CloneInScope(m_AST->GetSelfScope());
            }
        );

        return std::make_shared<const TypeTemplateNode>(
            m_SrcLocation,
            clonedParams,
            m_AST->CloneInScopeType(scope)
        );
    }

    auto TypeTemplateNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto TypeTemplateNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeTemplate;
    }

    auto TypeTemplateNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto TypeTemplateNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<TypeTemplateSymbol>(this),
            DiagnosticBag{},
        };
    }

    auto TypeTemplateNode::CollectImplParamNames() const -> std::vector<Ident>
    {
        return {};
    }

    auto TypeTemplateNode::CollectParamNames() const -> std::vector<Ident>
    {
        std::vector<Ident> names{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(names),
            [](const std::shared_ptr<const NormalTemplateParamNode>& param)
            {
                return param->GetName();
            }
        );

        return names;
    }

    auto TypeTemplateNode::GetAST() const -> const std::shared_ptr<const ITypeNode>&
    {
        return m_AST;
    }

    auto TypeTemplateNode::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetSelfScope();
    }
}
