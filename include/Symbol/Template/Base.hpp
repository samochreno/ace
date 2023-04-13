#pragma once

#include <string>

#include "Symbol/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "SpecialIdentifier.hpp"

namespace Ace::Symbol::Template
{
    class IBase : public virtual Symbol::IBase
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetASTName() const -> const std::string& = 0;
        virtual auto InstantiateSymbols(
            const std::vector<Symbol::Type::IBase*>& t_implArguments,
            const std::vector<Symbol::Type::IBase*>& t_arguments
        ) -> Symbol::IBase* = 0;
        virtual auto HasUninstantiatedSemanticsForSymbols() const -> bool = 0;
        virtual auto InstantiateSemanticsForSymbols() -> void = 0;
    };
}
