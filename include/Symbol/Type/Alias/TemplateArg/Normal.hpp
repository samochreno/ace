#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Asserts.hpp"
#include "Symbol/Type/Alias/TemplateArg/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Emittable.hpp"

namespace Ace::Symbol::Type::Alias::TemplateArg
{
    class Normal : public virtual Symbol::Type::Alias::TemplateArg::IBase
    {
    public:
        Normal(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            Symbol::Type::IBase* const t_aliasedType,
            const size_t& t_index
        );
        virtual ~Normal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectTemplateArgs() const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArgs() const -> std::vector<Symbol::Type::IBase*> final;

        auto GetSizeKind() const -> Expected<TypeSizeKind> final;
        auto SetAsUnsized() -> void final;
        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(Symbol::Function* const t_glue) -> void final;
        auto GetCopyGlue() const -> std::optional<Symbol::Function*> final;
        auto BindDropGlue(Symbol::Function* const t_glue) -> void final;
        auto GetDropGlue() const -> std::optional<Symbol::Function*> final;

        auto GetAliasedType() const -> Symbol::Type::IBase* final;

        auto GetIndex() const -> size_t final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        Symbol::Type::IBase* m_AliasedType{};
        size_t m_Index{};
    };
}
