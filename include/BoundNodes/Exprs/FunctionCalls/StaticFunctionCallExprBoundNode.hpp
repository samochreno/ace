#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    class StaticFunctionCallExprBoundNode :
        public std::enable_shared_from_this<StaticFunctionCallExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<StaticFunctionCallExprBoundNode>,
        public virtual ILowerableBoundNode<StaticFunctionCallExprBoundNode>
    {
    public:
        StaticFunctionCallExprBoundNode(
            const SourceLocation& sourceLocation,
            const std::shared_ptr<Scope>& scope,
            FunctionSymbol* const functionSymbol,
            const std::vector<std::shared_ptr<const IExprBoundNode>>& args
        );
        virtual ~StaticFunctionCallExprBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const StaticFunctionCallExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        FunctionSymbol* m_FunctionSymbol{};
        std::vector<std::shared_ptr<const IExprBoundNode>> m_Args{};
    };
}
