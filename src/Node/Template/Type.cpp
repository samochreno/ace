#include "Node/Template/Type.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "Diagnostics.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Node/TemplateParam/Normal.hpp"

namespace Ace::Node::Template
{
    Type::Type(
        const std::vector<std::shared_ptr<const Node::TemplateParam::Normal>>& t_params,
        const std::shared_ptr<const Node::Type::IBase>& t_ast
    ) : m_Params{ t_params },
        m_AST{ t_ast }
    {
    }

    auto Type::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetScope();
    }

    auto Type::GetChildren() const -> std::vector<const Node::IBase*>
    {
        std::vector<const Node::IBase*> children{};

        AddChildren(children, m_Params);

        return children;
    }

    auto Type::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Template::Type>
    {
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

        return std::make_shared<const Node::Template::Type>(
            clonedParams,
            m_AST->CloneInScopeType(t_scope)
        );
    }

    auto Type::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return GetScope();
    }

    auto Type::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::TypeTemplate;
    }

    auto Type::GetSymbolCreationSuborder() const -> size_t
    {
        return 0;
    }

    auto Type::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        return std::unique_ptr<ISymbol>
        {
            std::make_unique<TypeTemplateSymbol>(this)
        };
    }

    auto Type::CollectImplParamNames() const -> std::vector<std::string>
    {
        return {};
    }

    auto Type::CollectParamNames() const -> std::vector<std::string>
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

    auto Type::GetAST() const -> const std::shared_ptr<const Node::Type::IBase>&
    {
        return m_AST;
    }

    auto Type::GetSelfScope() const -> std::shared_ptr<Scope>
    {
        return m_AST->GetSelfScope();
    }
}
