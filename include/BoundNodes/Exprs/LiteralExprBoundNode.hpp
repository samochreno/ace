#pragma once

#include <memory>
#include <vector>
#include <string>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "TypeInfo.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class LiteralExprBoundNode :
        public std::enable_shared_from_this<LiteralExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<LiteralExprBoundNode>,
        public virtual ILowerableBoundNode<LiteralExprBoundNode>
    {
    public:
        LiteralExprBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            const LiteralKind& t_kind,
            const std::string& t_string
        );
        virtual ~LiteralExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
