#pragma once

#include <memory>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    class Module :
        public virtual Symbol::IBase,
        public virtual Symbol::ISelfScoped
    {
    public:
        Module(
            const std::shared_ptr<Scope>& t_selfScope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier
        );
        virtual ~Module() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

    private:
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
    };
}
