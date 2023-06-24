#pragma once

#include <memory>
#include <string>

#include "Symbol/Var/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Var
{
    class Local : public Symbol::Var::IBase
    {
    public:
        Local(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_type
        );
        virtual ~Local() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> Symbol::Type::IBase* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_Type;
    };
}
