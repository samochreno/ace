#include "Node/Template/Function.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Node/TemplateParam/Impl.hpp"
#include "Node/TemplateParam/Normal.hpp"

namespace Ace::Node::Template
{
    Function::Function(
        const std::vector<std::shared_ptr<const Node::TemplateParam::Impl>>& t_implParams,
        const std::vector<std::shared_ptr<const Node::TemplateParam::Normal>>& t_params,
        const std::shared_ptr<const Node::Function>& t_ast
    ) : m_ImplParams{ t_implParams },
        m_Params{ t_params },
        m_AST{ t_ast }
    {
    }

    auto Function::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetScope();
    }

    auto Function::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};
        
        AddChildren(children, m_ImplParams);
        AddChildren(children, m_Params);

        return children;
    }

    auto Function::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Template::Function>
    {
        std::vector<std::shared_ptr<const Node::TemplateParam::Impl>> clonedImplParams{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(clonedImplParams),
            [&](const std::shared_ptr<const Node::TemplateParam::Impl>& t_implParam)
            {
                return t_implParam->CloneInScope(m_AST->GetSelfScope());
            }
        );

        std::vector<std::shared_ptr<const Node::TemplateParam::Normal>> clonedParams{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(clonedParams),
            [&](const std::shared_ptr<const Node::TemplateParam::Normal>& t_param)
            {
                return t_param->CloneInScope(m_AST->GetSelfScope());
            }
        );

        return std::make_shared<const Node::Template::Function>(
            clonedImplParams,
            clonedParams,
            m_AST->CloneInScope(t_scope)
        );
    }

    auto Function::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto Function::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::FunctionTemplate;
    }

    auto Function::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Function::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<FunctionTemplateSymbol>(this)
        };
    }

    auto Function::CollectImplParamNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_ImplParams),
            end  (m_ImplParams),
            back_inserter(names),
            [](const std::shared_ptr<const Node::TemplateParam::Impl>& t_implParam)
            {
                return t_implParam->GetName();
            }
        );

        return names;
    }

    auto Function::CollectParamNames() const -> std::vector<std::string>
    {
        std::vector<std::string> names{};
        std::transform(
            begin(m_Params),
            end  (m_Params),
            back_inserter(names),
            [](const std::shared_ptr<const Node::TemplateParam::Normal>& t_param)
            {
                return t_param->GetName();
            }
        );

        return names;
    }

    auto Function::GetAST() const -> const std::shared_ptr<const Node::Function>&
    {
        return m_AST;
    }
}
