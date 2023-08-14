#pragma once

#include "Symbols/Types/Aliases/AliasTypeSymbol.hpp"
#include "Symbols/Types/SizedTypeSymbol.hpp"

namespace Ace
{
    class ITemplateArgAliasTypeSymbol :
        public virtual IAliasTypeSymbol,
        public virtual ISizedTypeSymbol
    {
    public:
        virtual ~ITemplateArgAliasTypeSymbol() = default;

        virtual auto GetIndex() const -> size_t = 0;

        auto SetAsPrimitivelyEmittable() -> void final;
        auto IsPrimitivelyEmittable() const -> bool final;

        auto SetAsTriviallyCopyable() -> void final;
        auto IsTriviallyCopyable() const -> bool final;
        auto SetAsTriviallyDroppable() -> void final;
        auto IsTriviallyDroppable() const -> bool final;

        auto CreateCopyGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;
        auto CreateDropGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> final;

        auto BindCopyGlue(FunctionSymbol* const glue) -> void final;
        auto GetCopyGlue() const -> std::optional<FunctionSymbol*> final;
        auto BindDropGlue(FunctionSymbol* const glue) -> void final;
        auto GetDropGlue() const -> std::optional<FunctionSymbol*> final;
    };
}
