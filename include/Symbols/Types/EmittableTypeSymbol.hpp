#pragma once

#include <memory>
#include <optional>

#include "Symbols/Types/SizedTypeSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class IConcreteTypeSymbol : public virtual ISizedTypeSymbol
    {
    public:
        virtual ~IConcreteTypeSymbol() = default;

        virtual auto SetAsPrimitivelyEmittable() -> void = 0;
        virtual auto IsPrimitivelyEmittable() const -> bool = 0;

        virtual auto SetAsTriviallyCopyable() -> void = 0;
        virtual auto IsTriviallyCopyable() const -> bool = 0;
        virtual auto SetAsTriviallyDroppable() -> void = 0;
        virtual auto IsTriviallyDroppable() const -> bool = 0;

        virtual auto CreateCopyGlueBlock(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;
        virtual auto CreateDropGlueBlock(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;

        virtual auto BindCopyGlue(FunctionSymbol* const glue) -> void = 0;
        virtual auto GetCopyGlue() const -> std::optional<FunctionSymbol*> = 0;
        virtual auto BindDropGlue(FunctionSymbol* const glue) -> void = 0;
        virtual auto GetDropGlue() const -> std::optional<FunctionSymbol*> = 0;
    };
}
