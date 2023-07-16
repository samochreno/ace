#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    struct StructConstructionExprBoundArg
    {
        InstanceVarSymbol* Symbol{};
        std::shared_ptr<const IExprBoundNode> Value{};
    };

    class StructConstructionExprBoundNode :
        public std::enable_shared_from_this<StructConstructionExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<StructConstructionExprBoundNode>,
        public virtual ILowerableBoundNode<StructConstructionExprBoundNode>
    {
    public:
        StructConstructionExprBoundNode(
            const SourceLocation& t_sourceLocation,
            const std::shared_ptr<Scope>& t_scope,
            StructTypeSymbol* const t_structSymbol,
            const std::vector<StructConstructionExprBoundArg>& t_args
        );
        virtual ~StructConstructionExprBoundNode() = default;

        auto GetSourceLocation() const -> const SourceLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SourceLocation m_SourceLocation{};
        std::shared_ptr<Scope> m_Scope{};
        StructTypeSymbol* m_StructSymbol{};
        std::vector<StructConstructionExprBoundArg> m_Args{};
    };
}
