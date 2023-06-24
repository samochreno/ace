#include "BoundNode/Expression/Unbox.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Function.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    Unbox::Unbox(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression
    ) : m_Expression{ t_expression }
    {
    }

    auto Unbox::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expression->GetScope();
    }

    auto Unbox::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);
        
        return children;
    }

    auto Unbox::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Unbox>>>
    {
        ACE_TRY_ASSERT(m_Expression->GetTypeInfo().Symbol->IsStrongPointer());

        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expression::Unbox>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Unbox::GetOrCreateTypeCheckedExpression(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Unbox::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>
    {
        const auto mchLoweredExpression =
            m_Expression->GetOrCreateLoweredExpression({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            GetCompilation(),
            GetCompilation()->Natives->StrongPointer__value.GetSymbol(),
            std::nullopt,
            { mchLoweredExpression.Value->GetTypeInfo().Symbol->GetWithoutStrongPointer() },
            {}
        ).Unwrap();

        auto* const functionSymbol = dynamic_cast<Symbol::Function*>(symbol);
        ACE_ASSERT(functionSymbol);

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpression.Value }
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Unbox::GetOrCreateLoweredExpression(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Unbox::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto Unbox::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            m_Expression->GetTypeInfo().Symbol->GetWithoutStrongPointer(),
            ValueKind::R,
        };
    }
}
