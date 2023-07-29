#pragma once

#include <memory>
#include <vector>

#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Ident.hpp"

namespace Ace
{
    class INode;

    struct TemplateSymbolsInstantationResult
    {
        ISymbol* const Symbol{};
        std::shared_ptr<const INode> AST{};
    };

    class ITemplateSymbol : public virtual ISymbol
    {
    public:
        virtual ~ITemplateSymbol() = default;

        virtual auto CollectImplParams() const -> std::vector<ImplTemplateParamTypeSymbol*>   = 0;
        virtual auto CollectParams()     const -> std::vector<NormalTemplateParamTypeSymbol*> = 0;

        virtual auto GetASTName() const -> const Ident& = 0;

        virtual auto SetPlaceholderSymbol(
            ISymbol* const symbol
        ) -> void = 0;
        virtual auto GetPlaceholderSymbol() const -> ISymbol* = 0;

        virtual auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& implArgs,
            const std::vector<ITypeSymbol*>& args
        ) -> Diagnosed<TemplateSymbolsInstantationResult> = 0;
        virtual auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const INode>& ast
        ) -> void = 0;
    };
}
