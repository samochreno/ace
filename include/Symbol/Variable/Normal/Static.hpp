#pragma once

#include <string>

#include "Symbol/Variable/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "SymbolKind.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Variable::Normal
{
    class Static : public virtual Symbol::Variable::IBase
    {
    public:
        Static(
            Scope* const t_scope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier,
            Symbol::Type::IBase* const t_type
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier },
            m_Type{ t_type }
        {
        }
        virtual ~Static() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::StaticVariable; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }
        auto IsInstance() const -> bool final { return false; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        Symbol::Type::IBase* m_Type{};
    };
}
