#pragma once

#include <memory>
#include <string>

#include "Symbol/Variable/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Variable::Normal
{
    class Static : public virtual Symbol::Variable::IBase
    {
    public:
        Static(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier,
            Symbol::Type::IBase* const t_type
        );
        virtual ~Static() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> Symbol::Type::IBase* final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        Symbol::Type::IBase* m_Type{};
    };
}
