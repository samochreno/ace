#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Types/TypeSymbol.hpp"
#include "Diagnostic.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class SizeOfExprBoundNode :
        public std::enable_shared_from_this<SizeOfExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<SizeOfExprBoundNode>,
        public virtual ILowerableBoundNode<SizeOfExprBoundNode>
    {
    public:
        SizeOfExprBoundNode(
            const SrcLocation& srcLocation,
            const std::shared_ptr<Scope>& scope,
            ITypeSymbol* const typeSymbol
        );
        virtual ~SizeOfExprBoundNode() = default;

        auto GetSrcLocation() const -> const SrcLocation& final;
        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto CollectChildren() const -> std::vector<const IBoundNode*> final;
        auto CreateTypeChecked(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const SizeOfExprBoundNode>> final;
        auto CreateTypeCheckedExpr(
            const TypeCheckingContext& context
        ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>> final;
        auto CreateLowered(
            const LoweringContext& context
        ) const -> std::shared_ptr<const SizeOfExprBoundNode> final;
        auto CreateLoweredExpr(
            const LoweringContext& context
        ) const -> std::shared_ptr<const IExprBoundNode> final;
        auto Emit(Emitter& emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

    private:
        DiagnosticBag m_Diagnostics{};
        SrcLocation m_SrcLocation{};
        std::shared_ptr<Scope> m_Scope{};
        ITypeSymbol* m_TypeSymbol{};
    };
}
