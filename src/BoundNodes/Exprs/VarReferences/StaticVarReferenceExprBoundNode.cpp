#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    auto StaticVarReferenceExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarReferenceExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto StaticVarReferenceExprBoundNode::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
    {
        if (
            dynamic_cast<LocalVarSymbol*>(m_VarSymbol) || 
            dynamic_cast<IParamVarSymbol*>(m_VarSymbol)
            )
        {
            return { t_emitter.GetLocalVarMap().at(m_VarSymbol), {} };
        }
        else if (auto* const variableSymbol = dynamic_cast<StaticVarSymbol*>(m_VarSymbol))
        {
            return { t_emitter.GetStaticVarMap().at(variableSymbol), {} };
        }

        ACE_UNREACHABLE();
    }

    auto StaticVarReferenceExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }
}
