#pragma once

#include <vector>
#include <string>

#include "Symbol/Template/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Type/TemplateParameter/Impl.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"
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
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return m_TemplateNode->GetAST()->GetAccessModifier(); }

        auto CollectImplParameters() const -> std::vector<Symbol::Type::TemplateParameter::Impl*>   final;
        auto CollectParameters()     const -> std::vector<Symbol::Type::TemplateParameter::Normal*> final;

        auto GetASTName() const -> const std::string& final { return m_TemplateNode->GetAST()->GetName(); }

        auto SetPlaceholderSymbol(
            Symbol::IBase* const t_symbol
        ) -> void final { m_PlaceholderSymbol = t_symbol; }
        auto GetPlaceholderSymbol() const -> Symbol::IBase* final { return m_PlaceholderSymbol; }

        auto InstantiateSymbols(
            const std::vector<Symbol::Type::IBase*>& t_implArguments,
            const std::vector<Symbol::Type::IBase*>& t_arguments
        ) -> Expected<TemplateSymbolsInstantationResult> final;
        auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const Node::IBase>& t_ast
        ) -> void final;

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        const Node::Template::Function* m_TemplateNode{};
        Symbol::IBase* m_PlaceholderSymbol{};
        std::vector<std::shared_ptr<const Node::Function>> m_InstantiatedOnlySymbolsASTs{};
    };
}
