#include "BoundNode/Expression/DerefAs.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "NativeSymbol.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "ExpressionDropData.hpp"

namespace Ace::BoundNode::Expression
{
    auto DerefAs::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto DerefAs::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::DerefAs>>>
    {
        ACE_TRY(mchLoweredExpression, m_Expression->GetOrCreateLoweredExpression({}));

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::DerefAs>(
            m_Expression,
            m_TypeSymbol
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto DerefAs::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::DerefAs>>>
    {
        auto* const typeSymbol = m_Expression->GetTypeInfo().Symbol->GetUnaliased();
        ACE_TRY_ASSERT(
            (typeSymbol == NativeSymbol::Pointer.GetSymbol()) ||
            (typeSymbol->IsReference())
        );

        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::DerefAs>(
            m_Expression,
            m_TypeSymbol
        );

        return CreateChanged(returnValue);
    }

    auto DerefAs::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(expressionEmitResult.Temporaries), end(expressionEmitResult.Temporaries));

        auto* const pointerType = NativeSymbol::Pointer.GetIRType(t_emitter);

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            pointerType,
            expressionEmitResult.Value
        );

        return { loadInst, temporaries };
    }

    auto DerefAs::GetTypeInfo() const -> TypeInfo
    {
        return { m_TypeSymbol, ValueKind::R };
    }
}
