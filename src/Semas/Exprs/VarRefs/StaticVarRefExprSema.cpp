#include "Semas/Exprs/VarRefs/StaticVarRefExprSema.hpp"

#include <memory>

#include "Semas/Exprs/Calls/StaticCallExprSema.hpp"
#include "Semas/Exprs/ExprSema.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Symbols/Vars/GlobalVarSymbol.hpp"
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
    StaticVarRefExprSema::StaticVarRefExprSema(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        IVarSymbol* const varSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_VarSymbol{ varSymbol }
    {
    }

    auto StaticVarRefExprSema::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticVarRefExprSema::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticVarRefExprSema::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StaticVarRefExprSema>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto StaticVarRefExprSema::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprSema>>
    {
        return CreateTypeChecked(context);
    }

    auto StaticVarRefExprSema::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticVarRefExprSema>
    {
        return shared_from_this();
    }

    auto StaticVarRefExprSema::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprSema>
    {
        return CreateLowered(context);
    }

    auto StaticVarRefExprSema::CollectMonos() const -> MonoCollector
    {
        return MonoCollector{}.Collect(m_VarSymbol);
    }

    auto StaticVarRefExprSema::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        auto* const globalVarSymbol =
            dynamic_cast<GlobalVarSymbol*>(m_VarSymbol);
        if (globalVarSymbol)
        {
            return { emitter.GetGlobalVar(globalVarSymbol), {} };
        }

        return { emitter.GetLocalVar(m_VarSymbol), {} };
    }

    auto StaticVarRefExprSema::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }
}
