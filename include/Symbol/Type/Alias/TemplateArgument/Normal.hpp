#pragma once

#include <vector>
#include <string>

#include "Symbol/Type/Alias/TemplateArgument/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "Asserts.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArgument
{
    class Normal : public virtual Symbol::Type::Alias::TemplateArgument::IBase
    {
    public:
        Normal(
            Scope* const t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_aliasedType,
            const size_t& t_index
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_AliasedType{ t_aliasedType },
            m_Index{ t_index }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> Scope* final { return m_Scope; }
        auto GetSelfScope() const -> Scope* final { return m_AliasedType->GetSelfScope(); }
        auto GetName() const -> const std::string & final { return m_Name; }
        auto GetSymbolKind() const -> Symbol::Kind final { return Symbol::Kind::TypeAlias; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Private; }
        auto IsInstance() const -> bool final { return false; }

        auto SetAsNativeSized() -> void final { ACE_UNREACHABLE(); }
        auto IsNativeSized() const -> bool final { return m_AliasedType->IsNativeSized(); }

        auto CanResolveSize() const -> bool final { return m_AliasedType->CanResolveSize(); }

        auto SetAsTriviallyCopyable() -> void final { m_AliasedType->SetAsTriviallyCopyable(); }
        auto IsTriviallyCopyable() const -> bool final { return m_AliasedType->IsTriviallyCopyable(); }

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_AliasedType->CollectTemplateArguments(); }
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_AliasedType->CollectImplTemplateArguments(); }

        auto GetAliasedType() const -> Symbol::Type::IBase* final { return m_AliasedType; }

        auto GetIndex() const -> size_t final { return m_Index; }

    private:
        Scope* m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_AliasedType{};
        size_t m_Index{};
    };
}
