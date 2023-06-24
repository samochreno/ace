#pragma once

#include <memory>

#include "BoundNode/Base.hpp"
#include "Symbol/Type/Base.hpp"
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
                Symbol::Type::IBase* const t_parentFunctionType
            ) : ParentFunctionTypeSymbol{ t_parentFunctionType }
            {
            }

            Symbol::Type::IBase* ParentFunctionTypeSymbol{};
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
