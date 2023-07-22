#pragma once

#include <vector>
#include <optional>

#include "Symbols/Symbol.hpp"
#include "Symbols/SelfScopedSymbol.hpp"
#include "Symbols/TemplatableSymbol.hpp"
#include "Diagnostic.hpp"
#include "TypeSizeKind.hpp"
#include "Emittable.hpp"

namespace Ace
{
    class FunctionSymbol;
    class TypeTemplateSymbol;

    class ITypeSymbol :
        public virtual ISymbol,
        public virtual ISelfScopedSymbol,
        public virtual ITemplatableSymbol
    {
    public:
        virtual ~ITypeSymbol() = default;

        virtual auto GetSizeKind() const -> Expected<TypeSizeKind> = 0;
        virtual auto SetAsUnsized() -> void = 0;
        virtual auto SetAsPrimitivelyEmittable() -> void = 0;
        virtual auto IsPrimitivelyEmittable() const -> bool = 0;

        virtual auto SetAsTriviallyCopyable() -> void = 0;
        virtual auto IsTriviallyCopyable() const -> bool = 0;
        virtual auto SetAsTriviallyDroppable() -> void = 0;
        virtual auto IsTriviallyDroppable() const -> bool = 0;

        virtual auto CreateCopyGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;
        virtual auto CreateDropGlueBody(
            FunctionSymbol* const glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;

        virtual auto BindCopyGlue(FunctionSymbol* const glue) -> void = 0;
        virtual auto GetCopyGlue() const -> std::optional<FunctionSymbol*> = 0;
        virtual auto BindDropGlue(FunctionSymbol* const glue) -> void = 0;
        virtual auto GetDropGlue() const -> std::optional<FunctionSymbol*> = 0;

        virtual auto IsRef() const -> bool final;
        virtual auto GetWithoutRef() -> ITypeSymbol* final;
        virtual auto GetWithoutRef() const -> const ITypeSymbol* final;
        virtual auto GetWithRef() -> ITypeSymbol* final;
        virtual auto IsStrongPtr() const -> bool final;
        virtual auto GetWithoutStrongPtr() -> ITypeSymbol* final;
        virtual auto GetWithStrongPtr() -> ITypeSymbol* final;
        virtual auto IsWeakPtr() const -> bool final;
        virtual auto GetWithoutWeakPtr() -> ITypeSymbol* final;
        virtual auto GetWithWeakPtr() -> ITypeSymbol* final;
        virtual auto GetUnaliased() -> ITypeSymbol* final;
        virtual auto GetUnaliased() const -> const ITypeSymbol* final;
        virtual auto GetTemplate() const -> std::optional<TypeTemplateSymbol*> final;
    };
}
