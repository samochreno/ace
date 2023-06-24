#pragma once

#include <memory>
#include <vector>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/Type/TemplateParam/Impl.hpp"
#include "Symbol/Type/TemplateParam/Normal.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Node
{
    class IBase;
}

namespace Ace
{
    struct TemplateSymbolsInstantationResult
    {
        Symbol::IBase* const Symbol{};
        std::shared_ptr<const Node::IBase> AST{};
    };
}

namespace Ace::Symbol::Template
{
    class IBase : public virtual Symbol::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto CollectImplParams() const -> std::vector<Symbol::Type::TemplateParam::Impl*>   = 0;
        virtual auto CollectParams()     const -> std::vector<Symbol::Type::TemplateParam::Normal*> = 0;

        virtual auto GetASTName() const -> const std::string& = 0;

        virtual auto SetPlaceholderSymbol(
            Symbol::IBase* const t_symbol
        ) -> void = 0;
        virtual auto GetPlaceholderSymbol() const -> Symbol::IBase* = 0;

        virtual auto InstantiateSymbols(
            const std::vector<Symbol::Type::IBase*>& t_implArgs,
            const std::vector<Symbol::Type::IBase*>& t_args
        ) -> Expected<TemplateSymbolsInstantationResult> = 0;
        virtual auto InstantiateSemanticsForSymbols(
            const std::shared_ptr<const Node::IBase>& t_ast
        ) -> void = 0;
    };
}
