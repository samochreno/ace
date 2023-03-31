#pragma once

#include <string>

#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Kind.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol
{
    class Module : public virtual Symbol::IBase, public virtual Symbol::ISelfScoped
    {
    public:
        Module(
            Scope* const t_selfScope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier
        ) : m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier }
        {
        }
        virtual ~Module() = default;

        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent(); }
        auto GetSelfScope() const -> Scope* final { return m_SelfScope; }
        auto GetName() const -> const std::string& { return m_Name; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::Module; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }
        auto IsInstance() const -> bool final { return false; }

    private:
        Scope* m_SelfScope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};
    };
}
