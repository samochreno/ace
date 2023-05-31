#pragma once

#include <vector>
#include <string>
#include <optional>

#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "Symbol/Templatable.hpp"
#include "Error.hpp"
#include "TypeSizeKind.hpp"
#include "Emittable.hpp"

namespace Ace::Symbol
{
    class Function;
}

namespace Ace::Symbol::Template
{
    class Type;
}

namespace Ace::Symbol::Type
{
    class IBase :
        public virtual Symbol::IBase,
        public virtual Symbol::ISelfScoped,
        public virtual Symbol::ITemplatable
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetSizeKind() const -> Expected<TypeSizeKind> = 0;
        virtual auto SetAsUnsized() -> void = 0;
        virtual auto SetAsPrimitivelyEmittable() -> void = 0;
        virtual auto IsPrimitivelyEmittable() const -> bool = 0;

        virtual auto SetAsTriviallyCopyable() -> void = 0;
        virtual auto IsTriviallyCopyable() const -> bool = 0;
        virtual auto SetAsTriviallyDroppable() -> void = 0;
        virtual auto IsTriviallyDroppable() const -> bool = 0;

        virtual auto CreateCopyGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;
        virtual auto CreateDropGlueBody(
            Symbol::Function* const t_glueSymbol
        ) -> std::shared_ptr<const IEmittable<void>> = 0;

        virtual auto BindCopyGlue(Symbol::Function* const t_glue) -> void = 0;
        virtual auto GetCopyGlue() const -> std::optional<Symbol::Function*> = 0;
        virtual auto BindDropGlue(Symbol::Function* const t_glue) -> void = 0;
        virtual auto GetDropGlue() const -> std::optional<Symbol::Function*> = 0;

        virtual auto IsReference() const -> bool final;
        virtual auto GetWithoutReference() -> Symbol::Type::IBase* final;
        virtual auto GetWithoutReference() const -> const Symbol::Type::IBase* final;
        virtual auto GetWithReference() -> Symbol::Type::IBase* final;
        virtual auto IsStrongPointer() const -> bool final;
        virtual auto GetWithoutStrongPointer() -> Symbol::Type::IBase* final;
        virtual auto GetWithStrongPointer() -> Symbol::Type::IBase* final;
        virtual auto GetUnaliased() -> Symbol::Type::IBase* final;
        virtual auto GetUnaliased() const -> const Symbol::Type::IBase* final;
        virtual auto GetTemplate() const -> std::optional<Symbol::Template::Type*> final;
    };

    auto GetImplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_targetType
    ) -> Expected<Symbol::Function*>;
    auto GetExplicitConversionOperator(
        const std::shared_ptr<Scope>& t_scope,
        Symbol::Type::IBase* t_fromType,
        Symbol::Type::IBase* t_targetType
    ) -> Expected<Symbol::Function*>;
}
