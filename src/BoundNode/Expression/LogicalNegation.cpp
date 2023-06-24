#include "BoundNode/Expression//LogicalNegation.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    LogicalNegation::LogicalNegation(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression
    ) : m_Expression{ t_expression }
    {
    }

    auto LogicalNegation::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expression->GetScope();
    }

    auto LogicalNegation::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto LogicalNegation::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::LogicalNegation>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedExpression, CreateImplicitlyConvertedAndTypeChecked(
            m_Expression,
            typeInfo
        ));

        if (mchConvertedAndCheckedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::LogicalNegation>(
            mchConvertedAndCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto LogicalNegation::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto LogicalNegation::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::LogicalNegation>>
    {
        const auto mchLoweredExpression =
            m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::LogicalNegation>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto LogicalNegation::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto LogicalNegation::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(expressionEmitResult.Temporaries),
            end  (expressionEmitResult.Temporaries)
        );

        auto* const boolType = GetCompilation()->Natives->Bool.GetIRType();

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            boolType,
            expressionEmitResult.Value
        );

        auto* const negatedValue = t_emitter.GetBlockBuilder().Builder.CreateXor(
            loadInst,
            1
        );

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(
            boolType
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            negatedValue,
            allocaInst
        );

        return { allocaInst, expressionEmitResult.Temporaries };
    }

    auto LogicalNegation::GetTypeInfo() const -> TypeInfo
    {
        return { GetCompilation()->Natives->Bool.GetSymbol(), ValueKind::R };
    }
}
