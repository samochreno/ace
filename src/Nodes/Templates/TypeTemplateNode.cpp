#include "Nodes/Templates/TypeTemplateNode.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

namespace Ace
{
    TypeTemplateNode::TypeTemplateNode(
        const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& t_params,
        const std::shared_ptr<const ITypeNode>& t_ast
    ) : m_Params{ t_params },
        m_AST{ t_ast }
    {
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
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const TypeTemplateNode>
    {
        std::vector<std::shared_ptr<const NormalTemplateParamNode>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const NormalTemplateParamNode>& t_param)
            {
                return t_param->CloneInScope(m_AST->GetSelfScope());
            }
        );

        return std::make_shared<const TypeTemplateNode>(
            clonedParams,
            m_AST->CloneInScopeType(t_scope)
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

    auto TypeTemplateNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<TypeTemplateSymbol>(this)
        };
    }

    auto TypeTemplateNode::CollectImplParamNames() const -> std::vector<std::string>
    {
        return {};
    }

    auto TypeTemplateNode::CollectParamNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(names),
            [](const std::shared_ptr<const NormalTemplateParamNode>& t_param)
            {
                return t_param->GetName();
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
