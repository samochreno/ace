#include "Nodes/Templates/FunctionTemplateNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Nodes/TemplateParams/ImplTemplateParamNode.hpp"
#include "Nodes/TemplateParams/NormalTemplateParamNode.hpp"
#include "Ident.hpp"

namespace Ace
{
    FunctionTemplateNode::FunctionTemplateNode(
        const SrcLocation& srcLocation,
        const std::vector<std::shared_ptr<const ImplTemplateParamNode>>& implParams,
        const std::vector<std::shared_ptr<const NormalTemplateParamNode>>& params,
        const std::shared_ptr<const FunctionNode>& ast
    ) : m_SrcLocation{ srcLocation },
        m_ImplParams{ implParams },
        m_Params{ params },
        m_AST{ ast }
    {
    }

    auto FunctionTemplateNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto FunctionTemplateNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetScope();
    }

    auto FunctionTemplateNode::CollectChildren() const -> std::vector<const INode*>
    {
        std::vector<const INode*> children{};
        
        AddChildren(children, m_ImplParams);
        AddChildren(children, m_Params);

        return children;
    }

    auto FunctionTemplateNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const FunctionTemplateNode>
    {
        const auto clonedAST = m_AST->CloneInScope(scope);

        std::vector<std::shared_ptr<const ImplTemplateParamNode>> clonedImplParams{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(clonedImplParams),
            [&](const std::shared_ptr<const ImplTemplateParamNode>& implParam)
            {
                return implParam->CloneInScope(clonedAST->GetTemplateSelfScope());
            }
        );

        std::vector<std::shared_ptr<const NormalTemplateParamNode>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const NormalTemplateParamNode>& param)
            {
                return param->CloneInScope(clonedAST->GetTemplateSelfScope());
            }
        );

        return std::make_shared<const FunctionTemplateNode>(
            m_SrcLocation,
            clonedImplParams,
            clonedParams,
            clonedAST
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

    auto FunctionTemplateNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<FunctionTemplateSymbol>(shared_from_this()),
            DiagnosticBag::Create(),
        };
    }

    auto FunctionTemplateNode::GetAST() const -> std::shared_ptr<const ITemplatableNode>
    {
        return m_AST;
    }

    auto FunctionTemplateNode::CollectImplParamNames() const -> std::vector<Ident>
    {
        std::vector<Ident> names{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(names),
            [](const std::shared_ptr<const ImplTemplateParamNode>& implParam)
            {
                return implParam->GetName();
            }
        );

        return names;
    }

    auto FunctionTemplateNode::CollectParamNames() const -> std::vector<Ident>
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

    auto FunctionTemplateNode::GetConcreteAST() const -> const std::shared_ptr<const FunctionNode>&
    {
        return m_AST;
    }
}
