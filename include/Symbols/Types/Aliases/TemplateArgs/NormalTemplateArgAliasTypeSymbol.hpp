#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "Assert.hpp"
#include "Symbols/Types/Aliases/TemplateArgs/TemplateArgAliasTypeSymbol.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class NormalTemplateArgAliasTypeSymbol :
        public virtual ITemplateArgAliasTypeSymbol
    {
    public:
        NormalTemplateArgAliasTypeSymbol(
            const std::shared_ptr<Scope>& t_scope,
            const std::string& t_name,
            ITypeSymbol* const t_aliasedType,
            const size_t t_index
        );
        virtual ~NormalTemplateArgAliasTypeSymbol() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetSelfScope() const -> std::shared_ptr<Scope> final;
        auto GetName() const -> const std::string& final;
        auto GetSymbolKind() const -> SymbolKind final;
        auto GetSymbolCategory() const -> SymbolCategory final;
        auto GetAccessModifier() const -> AccessModifier final;

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*> final;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*> final;

        auto GetSizeKind() const -> Expected<TypeSizeKind> final;
        auto SetAsUnsized() -> void final;
        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBody(
            FunctionSymbol* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBody(
            FunctionSymbol* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(FunctionSymbol* const t_glue) -> void final;
        auto GetCopyGlue() const -> std::optional<FunctionSymbol*> final;
        auto BindDropGlue(FunctionSymbol* const t_glue) -> void final;
        auto GetDropGlue() const -> std::optional<FunctionSymbol*> final;

        auto GetAliasedType() const -> ITypeSymbol* final;

        auto GetIndex() const -> size_t final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        std::string m_Name{};
        ITypeSymbol* m_AliasedType{};
        size_t m_Index{};
    };
}
