#pragma once

#include <memory>
#include <vector>

#include "Symbols/Templates/TemplateSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Diagnostic.hpp"
#include "Name.hpp"

namespace Ace
{
    class FunctionTemplateSymbol : public virtual ITemplateSymbol
    {
    public:
        FunctionTemplateSymbol(
            const std::shared_ptr<const FunctionTemplateNode>& templateNode
        );
        virtual ~FunctionTemplateSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
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
        std::shared_ptr<Scope> m_Scope{};
        Ident m_Name{};
        std::shared_ptr<const FunctionTemplateNode> m_TemplateNode{};
        ISymbol* m_PlaceholderSymbol{};
        std::vector<std::shared_ptr<const FunctionNode>> m_InstantiatedOnlySymbolsASTs{};
    };
}
