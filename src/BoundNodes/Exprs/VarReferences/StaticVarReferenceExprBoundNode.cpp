#include "BoundNodes/Exprs/VarReferences/StaticVarReferenceExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticVarReferenceExprBoundNode::StaticVarReferenceExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        IVarSymbol* const varSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_VarSymbol{ varSymbol }
    {
    }

    auto StaticVarReferenceExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto StaticVarReferenceExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarReferenceExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StaticVarReferenceExprBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto StaticVarReferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto StaticVarReferenceExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        if (
            dynamic_cast<LocalVarSymbol*>(m_VarSymbol) || 
            dynamic_cast<IParamVarSymbol*>(m_VarSymbol)
            )
        {
            return { emitter.GetLocalVarMap().at(m_VarSymbol), {} };
        }
        else if (auto* const varSymbol = dynamic_cast<StaticVarSymbol*>(m_VarSymbol))
        {
            return { emitter.GetStaticVarMap().at(varSymbol), {} };
        }

        ACE_UNREACHABLE();
    }

    auto StaticVarReferenceExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }

    auto StaticVarReferenceExprBoundNode::GetVarSymbol() const -> IVarSymbol*
    {
        return m_VarSymbol;
    }
}
