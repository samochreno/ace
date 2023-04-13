#pragma once

#include <vector>
#include <string>

#include "Symbol/Template/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Template/Type.hpp"
#include "SymbolKind.hpp"
#include "Node/Template/Type.hpp"
#include "Node/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Symbol::Template
{
    class Type : public virtual Symbol::Template::IBase, public virtual Symbol::ISelfScoped
    {
    public:
        Type(
            const Node::Template::Type* const t_templateNode
        ) : m_Name{ SpecialIdentifier::CreateTemplate(t_templateNode->GetAST()->GetName()) },
            m_TemplateNode{ t_templateNode }
        {
        }
        virtual ~Type() = default;

        auto GetScope() const -> Scope* final { return m_TemplateNode->GetScope(); }
        auto GetSelfScope() const -> Scope* final { return m_TemplateNode->GetSelfScope(); }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TypeTemplate; }
        auto GetAccessModifier() const -> AccessModifier final { return m_TemplateNode->GetAST()->GetAccessModifier(); }
        auto IsInstance() const -> bool final { return false; }

        auto GetASTName() const -> const std::string& final { return m_TemplateNode->GetAST()->GetName(); }
        auto InstantiateSymbols(
            const std::vector<Symbol::Type::IBase*>& t_implArguments,
            const std::vector<Symbol::Type::IBase*>& t_arguments
        ) -> Symbol::IBase* final;
        auto HasUninstantiatedSemanticsForSymbols() const -> bool final { return m_InstantiatedOnlySymbolsASTs.size() > 0; }
        auto InstantiateSemanticsForSymbols() -> void final;

    private:
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        const Node::Template::Type* m_TemplateNode{};
        std::vector<std::shared_ptr<const Node::Type::IBase>> m_InstantiatedOnlySymbolsASTs{};
    };
}
