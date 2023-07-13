#pragma once

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"

namespace Ace
{
    class StaticVarReferenceExprBoundNode :
        public std::enable_shared_from_this<StaticVarReferenceExprBoundNode>,
        public virtual IExprBoundNode,
        public virtual ITypeCheckableBoundNode<StaticVarReferenceExprBoundNode>,
        public virtual ILowerableBoundNode<StaticVarReferenceExprBoundNode>
    {
    public:
        StaticVarReferenceExprBoundNode(
            const std::shared_ptr<Scope>& t_scope,
            IVarSymbol* const t_variableSymbol
        ) : m_Scope{ t_scope },
            m_VarSymbol{ t_variableSymbol }
        {
        }
        virtual ~StaticVarReferenceExprBoundNode() = default;

        auto GetScope() const -> std::shared_ptr<Scope> final;
        auto GetChildren() const -> std::vector<const IBoundNode*> final;
        auto GetOrCreateTypeChecked(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>>> final;
        auto GetOrCreateTypeCheckedExpr(
            const TypeCheckingContext& t_context
        ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>> final;
        auto GetOrCreateLowered(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>> final;
        auto GetOrCreateLoweredExpr(
            const LoweringContext& t_context
        ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>> final;
        auto Emit(Emitter& t_emitter) const -> ExprEmitResult final;

        auto GetTypeInfo() const -> TypeInfo final;

        auto GetVarSymbol() const -> IVarSymbol* { return m_VarSymbol; }

    private:
        std::shared_ptr<Scope> m_Scope{};
        IVarSymbol* m_VarSymbol{};
    };
}
