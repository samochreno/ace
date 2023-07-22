#pragma once

#include <memory>
#include <vector>
#include <string>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "LiteralKind.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
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
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            const LiteralKind kind,
            const std::string& string
        );
        virtual ~LiteralExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const LiteralExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        LiteralKind m_Kind{};
        std::string m_String{};
    };
}
