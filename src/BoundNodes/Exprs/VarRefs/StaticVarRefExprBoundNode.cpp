#include "BoundNodes/Exprs/VarRefs/StaticVarRefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Symbols/Vars/LocalVarSymbol.hpp"
#include "Symbols/Vars/Params/ParamVarSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Vars/VarSymbol.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticVarRefExprBoundNode::StaticVarRefExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        IVarSymbol* const varSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_VarSymbol{ varSymbol }
    {
    }

    auto StaticVarRefExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticVarRefExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarRefExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto StaticVarRefExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StaticVarRefExprBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto StaticVarRefExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto StaticVarRefExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticVarRefExprBoundNode>
    {
        return shared_from_this();
    }

    auto StaticVarRefExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto StaticVarRefExprBoundNode::Emit(
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

    auto StaticVarRefExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }

    auto StaticVarRefExprBoundNode::GetVarSymbol() const -> IVarSymbol*
    {
        return m_VarSymbol;
    }
}
