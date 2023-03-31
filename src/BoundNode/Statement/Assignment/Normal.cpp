#include "BoundNode/Statement/Assignment/Normal.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"
#include "ExpressionDropData.hpp"

namespace Ace::BoundNode::Statement::Assignment
{
    auto Normal::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto Normal::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>>>
    {
        auto* const lhsExpressionTypeSymbol = m_LHSExpression->GetTypeInfo().Symbol->GetWithoutReference();

        ACE_TRY(mchConvertedAndCheckedLHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpression,
            TypeInfo{ lhsExpressionTypeSymbol, ValueKind::L }
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpression,
            TypeInfo{ lhsExpressionTypeSymbol, ValueKind::R }
        ));

        if (
            !mchConvertedAndCheckedLHSExpression.IsChanged &&
            !mchConvertedAndCheckedRHSExpression.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Assignment::Normal>(
            mchConvertedAndCheckedLHSExpression.Value,
            mchConvertedAndCheckedRHSExpression.Value
        );

        return CreateChanged(returnValue);
    }

    auto Normal::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Assignment::Normal>>>
    {
        ACE_TRY(mchLoweredRHSExpression, m_RHSExpression->GetOrCreateLoweredExpression({}));
        ACE_TRY(mchLoweredLHSExpression, m_LHSExpression->GetOrCreateLoweredExpression({}));

        if (!mchLoweredLHSExpression.IsChanged && !mchLoweredRHSExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Assignment::Normal>(
            mchLoweredLHSExpression.Value,
            mchLoweredRHSExpression.Value
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto Normal::Emit(Emitter& t_emitter) const -> void
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto rhsEmitResult = m_RHSExpression.get()->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(rhsEmitResult.Temporaries), end(rhsEmitResult.Temporaries));

        const auto lhsEmitResult = m_LHSExpression.get()->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(lhsEmitResult.Temporaries), end(lhsEmitResult.Temporaries));

        t_emitter.EmitCopy(
            lhsEmitResult.Value,
            rhsEmitResult.Value,
            m_LHSExpression->GetTypeInfo().Symbol
        );

        t_emitter.EmitDropTemporaries(temporaries);
    }
}
