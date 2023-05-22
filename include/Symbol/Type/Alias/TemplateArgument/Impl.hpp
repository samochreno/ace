#pragma once

#include <vector>
#include <string>
#include <optional>

#include "Symbol/Type/Alias/TemplateArgument/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "Asserts.hpp"
#include "Emittable.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArgument
{
    class Impl : public virtual Symbol::Type::Alias::TemplateArgument::IBase
    {
    public:
        Impl(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_aliasedType,
            const size_t& t_index
        ) : m_Scope{ t_scope },
            m_Name{ t_name },
            m_AliasedType{ t_aliasedType },
            m_Index{ t_index }
        {
        }
        virtual ~Impl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetSelfScope() const -> std::shared_ptr<Scope> final { return m_AliasedType->GetSelfScope(); }
        auto GetName() const -> const std::string& final { return m_Name; }
        auto GetSymbolKind() const -> SymbolKind final { return SymbolKind::TypeAlias; }
        auto GetSymbolCategory() const -> SymbolCategory final { return SymbolCategory::Static; }
        auto GetAccessModifier() const -> AccessModifier final { return AccessModifier::Private; }

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_AliasedType->CollectTemplateArguments(); }
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final { return m_AliasedType->CollectImplTemplateArguments(); }

        auto SetAsUnsized() -> void final { m_AliasedType->SetAsUnsized(); }
        auto IsSized() const -> bool final { return m_AliasedType->IsSized(); }

        auto SetAsNativeSized() -> void final { ACE_UNREACHABLE(); }
        auto IsNativeSized() const -> bool final { return m_AliasedType->IsNativeSized(); }

        auto CanResolveSize() const -> bool final { return m_AliasedType->CanResolveSize(); }

        auto SetAsTriviallyCopyable() -> void final { m_AliasedType->SetAsTriviallyCopyable(); }
        auto IsTriviallyCopyable() const -> bool final { return m_AliasedType->IsTriviallyCopyable(); }
        auto SetAsTriviallyDroppable() -> void final { m_AliasedType->SetAsTriviallyDroppable(); }
        auto IsTriviallyDroppable() const -> bool final { return m_AliasedType->IsTriviallyDroppable(); }

        auto CreateCopyGlueBody(Symbol::Function* const t_glueSymbol) -> std::shared_ptr<const IEmittable<void>> final { return m_AliasedType->CreateCopyGlueBody(t_glueSymbol); }
        auto CreateDropGlueBody(Symbol::Function* const t_glueSymbol) -> std::shared_ptr<const IEmittable<void>> final { return m_AliasedType->CreateDropGlueBody(t_glueSymbol); }

        auto BindCopyGlue(Symbol::Function* const t_glue) -> void final { m_AliasedType->BindCopyGlue(t_glue); }
        auto GetCopyGlue() const -> std::optional<Symbol::Function*> final { return m_AliasedType->GetCopyGlue(); }
        auto BindDropGlue(Symbol::Function* const t_glue) -> void final { m_AliasedType->BindDropGlue(t_glue); }
        auto GetDropGlue() const -> std::optional<Symbol::Function*> final { return m_AliasedType->GetDropGlue(); }

        auto GetAliasedType() const -> Symbol::Type::IBase* final { return m_AliasedType; }

        auto GetIndex() const -> size_t final { return m_Index; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_AliasedType{};
        size_t m_Index{};
    };
}
