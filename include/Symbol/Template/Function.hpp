#pragma once

#include <vector>
#include <string>

#include "Symbol/Template/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "SymbolKind.hpp"
#include "Node/Template/Function.hpp"
#include "Node/Function.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"
#include "Error.hpp"
#include "Name.hpp"

namespace Ace::Symbol::Template
{
    class Function : public virtual Symbol::Template::IBase
    {
    public:
        Function(
            const Node::Template::Function* const t_templateNode
        ) : m_Name{ SpecialIdentifier::CreateTemplate(t_templateNode->GetAST()->GetName()) },
            m_TemplateNode{ t_templateNode }
        {
        }
        virtual ~Function() = default;

        auto GetScope() const -> Scope* final { return m_TemplateNode->GetScope(); }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::FunctionTemplate; }
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
        Scope* m_Scope{};
        std::string m_Name{};
        const Node::Template::Function* m_TemplateNode{};
        std::vector<std::shared_ptr<const Node::Function>> m_InstantiatedOnlySymbolsASTs{};
    };
}
