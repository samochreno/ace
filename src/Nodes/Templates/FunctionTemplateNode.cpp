#include "Nodes/Templates/FunctionTemplateNode.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"

namespace Ace
{
    FunctionTemplateNode::FunctionTemplateNode(
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& t_implParams,
        const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& t_params,
        const std::shared_ptr<const FunctionNode>& t_ast
    ) : m_ImplParams{ t_implParams },
        m_Params{ t_params },
        m_AST{ t_ast }
    {
    }

    auto FunctionTemplateNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetScope();
    }

    auto FunctionTemplateNode::GetChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};
        
        AddChildren(children, m_ImplParams);
        AddChildren(children, m_Params);

        return children;
    }

    auto FunctionTemplateNode::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const FunctionTemplateNode>
    {
        std::vector<std::shared_ptr<const ImplTemplateParamNode>> clonedImplParams{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(clonedImplParams),
            [&](const std::shared_ptr<const ImplTemplateParamNode>& t_implParam)
            {
                return t_implParam->CloneInScope(m_AST->GetSelfScope());
            }
        );

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

        return std::make_shared<const FunctionTemplateNode>(
            clonedImplParams,
            clonedParams,
            m_AST->CloneInScope(t_scope)
        );
    }

    auto FunctionTemplateNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto FunctionTemplateNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::FunctionTemplate;
    }

    auto FunctionTemplateNode::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto FunctionTemplateNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<FunctionTemplateSymbol>(this)
        };
    }

    auto FunctionTemplateNode::CollectImplParamNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(names),
            [](const std::shared_ptr<const ImplTemplateParamNode>& t_implParam)
            {
                return t_implParam->GetName();
            }
        );

        return names;
    }

    auto FunctionTemplateNode::CollectParamNames() const -> std::vector<std::string>
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

    auto FunctionTemplateNode::GetAST() const -> const std::shared_ptr<const FunctionNode>&
    {
        return m_AST;
    }
}
