#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Symbols/Symbol.hpp"
#include "Symbols/Types/TemplateParams/ImplTemplateParamTypeSymbol.hpp"
#include "Symbols/Types/TemplateParams/NormalTemplateParamTypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"

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

        virtual auto GetASTName() const -> const std::string& = 0;

        virtual auto SetPlaceholderSymbol(
            ISymbol* const t_symbol
        ) -> void = 0;
        virtual auto GetPlaceholderSymbol() const -> ISymbol* = 0;

        virtual auto InstantiateSymbols(
            const std::vector<ITypeSymbol*>& t_implArgs,
            const std::vector<ITypeSymbol*>& t_args
        ) -> Expected<TemplateSymbolsInstantationResult> = 0;
        virtual auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const INode>& t_ast
        ) -> void = 0;
    };
}
