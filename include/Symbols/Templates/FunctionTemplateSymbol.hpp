#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Symbols/Templates/TemplateSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Diagnostics.hpp"
#include "Name.hpp"

namespace Ace
{
    class FunctionTemplateSymbol : public virtual ITemplateSymbol
    {
    public:
        FunctionTemplateSymbol(
            const FunctionTemplateNode* const t_templateNode
        );
        virtual ~FunctionTemplateSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>   final;
        auto CollectParams()     const -> std::vector<NormalTemplateParamTypeSymbol*> final;

        auto GetASTName() const -> const std::string& final;

        auto SetPlaceholderSymbol(ISymbol* const t_symbol) -> void final;
        auto GetPlaceholderSymbol() const -> ISymbol* final;

        auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& t_implArgs,
            const std::vector<ITypeSymbol*>& t_args
        ) -> Expected<TemplateSymbolsInstantationResult> final; auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const INode>& t_ast
        ) -> void final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        const FunctionTemplateNode* m_TemplateNode{};
        ISymbol* m_PlaceholderSymbol{};
        std::vector<std::shared_ptr<const FunctionNode>> m_InstantiatedOnlySymbolsASTs{};
    };
}
