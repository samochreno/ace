#pragma once

#include <memory>
#include <string>

#include "Symbol/Type/Base.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"

namespace Ace::Symbol::Type::TemplateParameter
{
    class Impl : public virtual Symbol::Type::IBase
    {
    public:
        Impl(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name
        );
        virtual ~Impl() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*> final;

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

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
    };
}
