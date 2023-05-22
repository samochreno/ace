#pragma once

#include <string>

#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"

namespace Ace::Symbol::Variable::Parameter
{
    class Self : public virtual Symbol::Variable::Parameter::IBase
    {
    public:
        Self(
            const std::shared_ptr<Scope>& t_scope,
            Symbol::Type::IBase* const t_type
        ) : m_Scope{ t_scope },
            m_Type{ t_type }
        {
        }
        virtual ~Self() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetName() const -> const std::string & final;
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::ParameterVariable; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Public; }

        auto GetType() const -> Symbol::Type::IBase* final { return m_Type; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        Symbol::Type::IBase* m_Type{};
    };
}
