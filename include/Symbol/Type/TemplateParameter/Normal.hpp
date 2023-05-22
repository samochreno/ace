
#pragma once

#include <string>

#include "Symbol/Type/Base.hpp"

namespace Ace::Symbol::Type::TemplateParameter
{
    class Normal : public virtual Symbol::Type::IBase
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        ) : m_Scope{ t_scope },
            m_Name{ t_name }
        {
        }
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetSelfScope() const -> std::shared_ptr<Scope> final { ACE_UNREACHABLE(); }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TemplateParameter; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Private; }

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;

        auto SetAsUnsized() -> void final { }
        auto IsSized() const -> bool final { return false; }

        auto SetAsNativeSized() -> void final { ACE_UNREACHABLE(); }
        auto IsNativeSized() const -> bool final { ACE_UNREACHABLE(); }

        auto CanResolveSize() const -> bool final { return true; }

        auto SetAsTriviallyCopyable() -> void final { ACE_UNREACHABLE(); }
        auto IsTriviallyCopyable() const -> bool final { ACE_UNREACHABLE(); }
        auto SetAsTriviallyDroppable() -> void final { ACE_UNREACHABLE(); }
        auto IsTriviallyDroppable() const -> bool final { ACE_UNREACHABLE(); }

        auto CreateCopyGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final { ACE_UNREACHABLE(); }
        auto CreateDropGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final { ACE_UNREACHABLE(); }

        auto BindCopyGlue(Symbol::Function* const t_glue) -> void final { ACE_UNREACHABLE(); }
        auto GetCopyGlue() const -> std::optional<Symbol::Function*> final { ACE_UNREACHABLE(); }
        auto BindDropGlue(Symbol::Function* const t_glue) -> void { ACE_UNREACHABLE(); }
        auto GetDropGlue() const -> std::optional<Symbol::Function*> { ACE_UNREACHABLE(); }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}


