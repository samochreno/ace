#pragma once

#include <memory>
#include <vector>

#include "Symbols/Templates/TemplateSymbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/Templates/TypeTemplateSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Nodes/Templates/TypeTemplateNode.hpp"
#include "Nodes/Types/TypeNode.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdent.hpp"

namespace Ace
{
    class TypeTemplateSymbol :
        public virtual ITemplateSymbol,
        public virtual ISelfScopedSymbol
    {
    public:
        TypeTemplateSymbol(
            const std::shared_ptr<const TypeTemplateNode>& templateNode
        );
        virtual ~TypeTemplateSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetKind() const -> SymbolKind final;
        auto GetCategory() const -> SymbolCategory final;

        auto GetAST() const -> std::shared_ptr<const ITemplatableNode> final;
  
        auto SetPlaceholderSymbol(ISymbol* const symbol) -> void final;
        auto GetPlaceholderSymbol() const -> ISymbol* final;

        auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& implArgs,
            const std::vector<ITypeSymbol*>& args
        ) -> Diagnosed<TemplateSymbolsInstantationResult> final;
        auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const ITemplatableNode>& ast
        ) -> void final;

    private:
        Ident m_Name{};
        AccessModifier m_AccessModifier{};
        std::shared_ptr<const TypeTemplateNode> m_TemplateNode{};
        ISymbol* m_PlaceholderSymbol{};
    };
}
