#include "BoundNode/Expression/VariableReference/Static.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "Symbol/Variable/Normal/Static.hpp"
#include "Symbol/Variable/Local.hpp"
#include "Symbol/Variable/Parameter/Base.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Variable/Base.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    auto Static::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        return {};
    }

    auto Static::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Static::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Static>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto Static::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        if (
            dynamic_cast<Symbol::Variable::Local*>(m_VariableSymbol) || 
            dynamic_cast<Symbol::Variable::Parameter::IBase*>(m_VariableSymbol)
            )
        {
            return { t_emitter.GetLocalVariableMap().at(m_VariableSymbol), {} };
        }
        else if (auto variableSymbol = dynamic_cast<Symbol::Variable::Normal::Static*>(m_VariableSymbol))
        {
            return { t_emitter.GetStaticVariableMap().at(variableSymbol), {} };
        }

        ACE_UNREACHABLE();
    }

    auto Static::GetTypeInfo() const -> TypeInfo
    {
        return { m_VariableSymbol->GetType(), ValueKind::L };
    }
}
