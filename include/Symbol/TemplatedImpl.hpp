#pragma once

#include <memory>
#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Template/Type.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    class TemplatedImpl : 
        public virtual Symbol::IBase, 
        public virtual Symbol::ISelfScoped
    {
    public:
        TemplatedImpl(
            const std::shared_ptr<Scope>& t_scope,
            const std::shared_ptr<Scope>& t_selfScope,
            Symbol::Template::Type* const t_implementedTypeTemplate
        );
        virtual ~TemplatedImpl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto GetImplementedTypeTemplate() const -> Symbol::Template::Type*;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        Symbol::Template::Type* m_ImplementedTypeTemplate{};
    };
}
