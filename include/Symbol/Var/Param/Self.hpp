#pragma once

#include <memory>
#include <string>

#include "Symbol/Var/Param/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Var::Param
{
    class Self : public virtual Symbol::Var::Param::IBase
    {
    public:
        Self(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Type::IBase* const t_type
        );
        virtual ~Self() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> Symbol::Type::IBase* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Type::IBase* m_Type{};
    };
}
