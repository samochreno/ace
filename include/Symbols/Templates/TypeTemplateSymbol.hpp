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
        TypeTemplateSymbol(const TypeTemplateNode* const templateNode);
        virtual ~TypeTemplateSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const Ident& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>   final;
        auto CollectParams()     const -> std::vector<NormalTemplateParamTypeSymbol*> final;

        auto GetASTName() const -> const Ident& final;
  
        auto SetPlaceholderSymbol(ISymbol* const symbol) -> void final;
        auto GetPlaceholderSymbol() const -> ISymbol* final;

        auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& implArgs,
            const std::vector<ITypeSymbol*>& args
        ) -> Expected<TemplateSymbolsInstantationResult> final;
        auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const INode>& ast
        ) -> void final;

    private:
        Ident m_Name{};
        AccessModifier m_AccessModifier{};
        const TypeTemplateNode* m_TemplateNode{};
        ISymbol* m_PlaceholderSymbol{};
    };
}
