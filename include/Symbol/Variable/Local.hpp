#pragma once

#include <string>

#include "Symbol/Variable/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Variable
{
    class Local : public Symbol::Variable::IBase
    {
    public:
        Local(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_type
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_Type{ t_type }
        {
        }
        virtual ~Local() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::LocalVariable; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Private; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_Type;
    };
}
