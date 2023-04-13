#pragma once

#include <string>

#include "Symbol/Variable/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "SymbolKind.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Variable
{
    class Local : public Symbol::Variable::IBase
    {
    public:
        Local(
            Scope* const t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_type
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_Type{ t_type }
        {
        }
        virtual ~Local() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::LocalVariable; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Private; }
        auto IsInstance() const -> bool final { return false; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_Type;
    };
}
