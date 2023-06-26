#pragma once

#include <memory>

#include "BoundNode/Base.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Emittable.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Stmt
{
    struct Context
    {
        Context() = delete;

        struct TypeChecking : public BoundNode::Context::TypeChecking
        {
            TypeChecking(
                ITypeSymbol* const t_parentFunctionType
            ) : ParentFunctionTypeSymbol{ t_parentFunctionType }
            {
            }

            ITypeSymbol* ParentFunctionTypeSymbol{};
        };
    };

    class IBase :
        public virtual BoundNode::IBase,
        public virtual IEmittable<void>
    {
    public:
        virtual ~IBase() = default;

        virtual auto GetOrCreateTypeCheckedStmt(
            const BoundNode::Stmt::Context::TypeChecking& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>>> = 0;
        virtual auto GetOrCreateLoweredStmt(
            const BoundNode::Context::Lowering& t_context
        ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Stmt::IBase>> = 0;
    };
}
