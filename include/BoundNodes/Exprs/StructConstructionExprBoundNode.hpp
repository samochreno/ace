#pragma once

#include <memory>
#include <vector>

#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    struct StructConstructionExprBoundArg
    {
        auto operator<=>(const StructConstructionExprBoundArg&) const = default;

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
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            StructTypeSymbol* const structSymbol,
            const std::vector<StructConstructionExprBoundArg>& args
        );
        virtual ~StructConstructionExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const StructConstructionExprBoundNode>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const StructConstructionExprBoundNode> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprBoundNode> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        StructTypeSymbol* m_StructSymbol{};
        std::vector<StructConstructionExprBoundArg> m_Args{};
    };
}
