#pragma once

#include <memory>

#include "BoundNode/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Emittable.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Statement
{
    struct Context
    {
        Context() = delete;

        struct TypeChecking : public BoundNode::Context::TypeChecking
        {
            TypeChecking(Symbol::Type::IBase* const t_parentFunctionType)
                : ParentFunctionTypeSymbol{ t_parentFunctionType }
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

        virtual auto GetOrCreateTypeCheckedStatement(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>> = 0;
        virtual auto GetOrCreateLoweredStatement(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>> = 0;
    };
}
