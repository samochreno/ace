#pragma once

#include <vector>
#include <string>

#include "Symbol/Type/Base.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type
{
    class Struct : public virtual Symbol::Type::IBase
    {
    public:
        Struct(
            Scope* const t_selfScope,
            const std::string& t_name,
            const AccessModifier& t_accessModifier
        ) : m_SelfScope{ t_selfScope },
            m_Name{ t_name },
            m_AccessModifier{ t_accessModifier }
        {
        }
        virtual ~Struct() = default;

        auto GetScope() const -> Scope* final { return m_SelfScope->GetParent(); }
        auto GetSelfScope() const -> Scope* final { return m_SelfScope; }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::Struct; }
        auto GetAccessModifier() const -> AccessModifier final { return m_AccessModifier; }
        auto IsInstance() const -> bool final { return false; }

        auto SetAsNativeSized() -> void final { m_IsNativeSized = true; }
        auto IsNativeSized() const -> bool final { return m_IsNativeSized; }

        auto CanResolveSize() const -> bool final;

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_SelfScope->CollectTemplateArguments(); }
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_SelfScope->CollectImplTemplateArguments(); }

        auto GetVariables() const -> std::vector<Symbol::Variable::Normal::Instance*>;
        auto GetVariableIndex(Symbol::Variable::Normal::Instance* const t_variable) const -> size_t;

    private:
        Scope* m_SelfScope{};
        std::string m_Name{};
        AccessModifier m_AccessModifier{};

        bool m_IsNativeSized{};
        
        mutable bool m_IsResolvingSize = false;
        mutable bool m_DidResolveSize = false;
    };
}
