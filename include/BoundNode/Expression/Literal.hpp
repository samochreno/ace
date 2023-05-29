#pragma once

#include <memory>
#include <vector>
#include <string>

#include "BoundNode/Expression/Base.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "TypeInfo.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    class Literal :
        public std::enable_shared_from_this<BoundNode::Expression::Literal>,
        public virtual BoundNode::Expression::IBase,
        public virtual BoundNode::ITypeCheckable<BoundNode::Expression::Literal>,
        public virtual BoundNode::ILowerable<BoundNode::Expression::Literal>
    {
    public:
        Literal(
            const std::shared_ptr<Scope>& t_scope,
            const LiteralKind& t_kind,
            const std::string& t_string
        ) : m_Scope{ t_scope },
            m_Kind{ t_kind },
            m_String{ t_string }
        {
        }
        virtual ~Literal() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final { return m_Scope; }
        auto GetChildren() const -> std::vector<const BoundNode::IBase*> final;
        auto GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Literal>>> final;
        auto GetOrCreateTypeCheckedExpression(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>> final { return GetOrCreateTypeChecked(t_context); }
        auto GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::Literal>> final;
        auto GetOrCreateLoweredExpression(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>> final { return GetOrCreateLowered(t_context); }
        auto Emit(Emitter& t_emitter) const -> ExpressionEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
