#pragma once

#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Template/Type.hpp"
#include "Scope.hpp"
#include "SpecialIdentifier.hpp"

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
        ) : m_Scope{ t_scope },
            m_SelfScope{ t_selfScope },
            m_Name{ SpecialIdentifier::CreateAnonymous() },
            m_ImplementedTypeTemplate{ t_implementedTypeTemplate }
        {
        }
        virtual ~TemplatedImpl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetSelfScope() const -> std::shared_ptr<Scope> final { return m_SelfScope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TemplatedImpl; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Public; }

        auto GetImplementedTypeTemplate() const -> Symbol::Template::Type* { return m_ImplementedTypeTemplate; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::shared_ptr<Scope> m_SelfScope{};
        std::string m_Name{};
        Symbol::Template::Type* m_ImplementedTypeTemplate{};
    };
}
