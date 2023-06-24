#include "BoundNode/Expression/UserBinary.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Expression
{
    UserBinary::UserBinary(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_lhsExpression,
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_rhsExpression,
        Symbol::Function* const t_operatorSymbol
    ) : m_LHSExpression{ t_lhsExpression },
        m_RHSExpression{ t_rhsExpression },
        m_OperatorSymbol{ t_operatorSymbol }
    {
    }

    auto UserBinary::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_LHSExpression->GetScope();
    }

    auto UserBinary::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_LHSExpression);
        AddChildren(children, m_RHSExpression);

        return children;
    }

    auto UserBinary::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::UserBinary>>>
    {
        const auto argumentTypeInfos =
            m_OperatorSymbol->CollectArgumentTypeInfos();

        ACE_TRY(mchConvertedAndCheckedLHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_LHSExpression,
            argumentTypeInfos.at(0)
        ));

        ACE_TRY(mchConvertedAndCheckedRHSExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_RHSExpression,
            argumentTypeInfos.at(1)
        ));

        if (
            !mchConvertedAndCheckedLHSExpression.IsChanged &&
            !mchConvertedAndCheckedRHSExpression.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::UserBinary>(
            mchConvertedAndCheckedLHSExpression.Value,
            mchConvertedAndCheckedRHSExpression.Value,
            m_OperatorSymbol
        );
        return CreateChanged(returnValue);
    }

    auto UserBinary::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto UserBinary::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>
    {
        const auto mchLoweredLHSExpression =
            m_LHSExpression->GetOrCreateLoweredExpression({});

        const auto mchLoweredRHSExpression =
            m_RHSExpression->GetOrCreateLoweredExpression({});

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            m_OperatorSymbol,
            std::vector
            {
                mchLoweredLHSExpression.Value,
                mchLoweredRHSExpression.Value
            }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto UserBinary::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto UserBinary::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UserBinary::GetTypeInfo() const -> TypeInfo
    {
        return { m_OperatorSymbol->GetType(), ValueKind::R };
    }
}
