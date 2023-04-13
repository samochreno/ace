#pragma once

#include <string>

#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "SymbolKind.hpp"
#include "Scope.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    class Self : public virtual Symbol::Variable::Parameter::IBase
    {
    public:
        Self(
            Scope* const t_scope,
            Symbol::Type::IBase* const t_type
        ) : m_Scope{ t_scope },
            m_Type{ t_type }
        {
        }
        virtual ~Self() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetName() const -> const std::string & final;
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::ParameterVariable; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Public; }
        auto IsInstance() const -> bool final { return false; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

    private:
        Scope* m_Scope{};
        Symbol::Type::IBase* m_Type{};
    };
}
