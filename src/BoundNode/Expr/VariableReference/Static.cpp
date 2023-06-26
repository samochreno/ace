#include "BoundNode/Expr/VarReference/Static.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expr/FunctionCall/Static.hpp"
#include "BoundNode/Expr/Base.hpp"
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

namespace Ace::BoundNode::Expr::VarReference
{
    auto Static::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Static::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Static::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Static>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Static::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        if (
            dynamic_cast<LocalVarSymbol*>(m_VarSymbol) || 
            dynamic_cast<IParamVarSymbol*>(m_VarSymbol)
            )
        {
            return { t_emitter.GetLocalVarMap().at(m_VarSymbol), {} };
        }
        else if (auto variableSymbol = dynamic_cast<StaticVarSymbol*>(m_VarSymbol))
        {
            return { t_emitter.GetStaticVarMap().at(variableSymbol), {} };
        }

        ACE_UNREACHABLE();
    }

    auto Static::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }
}
