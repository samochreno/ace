#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "AccessModifier.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class ITemplatableNode;

    struct TemplateSymbolsInstantationResult
    {
        ISymbol* const Symbol{};
        std::shared_ptr<const ITemplatableNode> AST{};
    };

    class ITemplateSymbol : public virtual ISymbol
    {
    public:
        virtual ~ITemplateSymbol() = default;

        virtual auto GetAccessModifier() const -> AccessModifier final;

        virtual auto GetAST() const -> std::shared_ptr<const ITemplatableNode> = 0;

        virtual auto CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*> final;
        virtual auto CollectParams()     const -> std::vector<NormalTemplateParamTypeSymbol*> final;

        virtual auto SetPlaceholderSymbol(ISymbol* const symbol) -> void = 0;
        virtual auto GetPlaceholderSymbol() const -> ISymbol* = 0;

        virtual auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& implArgs,
            const std::vector<ITypeSymbol*>& args
        ) -> Diagnosed<TemplateSymbolsInstantationResult> = 0;
        virtual auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const ITemplatableNode>& ast
        ) -> void = 0;
    };
}
