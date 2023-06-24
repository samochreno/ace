#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Symbol/Template/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Template/Type.hpp"
#include "Symbol/Type/TemplateParameter/Impl.hpp"
#include "Symbol/Type/TemplateParameter/Normal.hpp"
#include "Node/Template/Type.hpp"
#include "Node/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Symbol::Template
{
    class Type :
        public virtual Symbol::Template::IBase,
        public virtual Symbol::ISelfScoped
    {
    public:
        Type(const Node::Template::Type* const t_templateNode);
        virtual ~Type() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectImplParameters() const -> std::vector<Symbol::Type::TemplateParameter::Impl*>   final;
        auto CollectParameters()     const -> std::vector<Symbol::Type::TemplateParameter::Normal*> final;

        auto GetASTName() const -> const std::string& final;
  
        auto SetPlaceholderSymbol(Symbol::IBase* const t_symbol) -> void final;
        auto GetPlaceholderSymbol() const -> Symbol::IBase* final;

        auto InstantiateSymbols(
            const std::vector<Symbol::Type::IBase*>& t_implArguments,
            const std::vector<Symbol::Type::IBase*>& t_arguments
        ) -> Expected<TemplateSymbolsInstantationResult> final;
        auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const Node::IBase>& t_ast
        ) -> void final;

    private:
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        const Node::Template::Type* m_TemplateNode{};
        Symbol::IBase* m_PlaceholderSymbol{};
    };
}
