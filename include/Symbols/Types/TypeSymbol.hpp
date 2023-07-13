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
            FunctionSymbol* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;
        virtual auto CreateDropGlueBody(
            FunctionSymbol* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;

        virtual auto BindCopyGlue(FunctionSymbol* const t_glue) -> void = 0;
        virtual auto GetCopyGlue() const -> std::optional<FunctionSymbol*> = 0;
        virtual auto BindDropGlue(FunctionSymbol* const t_glue) -> void = 0;
        virtual auto GetDropGlue() const -> std::optional<FunctionSymbol*> = 0;

        virtual auto IsReference() const -> bool final;
        virtual auto GetWithoutReference() -> ITypeSymbol* final;
        virtual auto GetWithoutReference() const -> const ITypeSymbol* final;
        virtual auto GetWithReference() -> ITypeSymbol* final;
        virtual auto IsStrongPointer() const -> bool final;
        virtual auto GetWithoutStrongPointer() -> ITypeSymbol* final;
        virtual auto GetWithStrongPointer() -> ITypeSymbol* final;
        virtual auto GetUnaliased() -> ITypeSymbol* final;
        virtual auto GetUnaliased() const -> const ITypeSymbol* final;
        virtual auto GetTemplate() const -> std::optional<TypeTemplateSymbol*> final;
    };
}
