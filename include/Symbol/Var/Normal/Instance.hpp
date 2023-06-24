#pragma once

#include <memory>
#include <string>

#include "Symbol/Var/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Var::Normal
{
    class Instance : public virtual Symbol::Var::IBase
    {
    public:
        Instance(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier,
            Symbol::Type::IBase* const t_type,
            const size_t& t_index
        );
        virtual ~Instance() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetType() const -> Symbol::Type::IBase* final;

        auto GetIndex() const -> size_t;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
        Symbol::Type::IBase* m_Type{};
        size_t m_Index{};
    };
}
