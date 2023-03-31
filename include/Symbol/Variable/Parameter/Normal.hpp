#pragma once

#include <string>

#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    class Normal : public virtual Symbol::Variable::Parameter::IBase
    {
    public:
        Normal(
            Scope* const t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_type,
            const size_t& t_index
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_Type{ t_type },
            m_Index{ t_index }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::ParameterVariable; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Public; }
        auto IsInstance() const -> bool final { return false; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

        auto GetIndex() const -> size_t { return m_Index; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_Type{};
        size_t m_Index{};
    };
}
