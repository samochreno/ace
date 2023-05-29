#include "BoundNode/Expression//Reference.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Symbol/Type/Base.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto Reference::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Reference::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Reference>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Reference>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Reference::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::Reference>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Reference>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Reference::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(expressionEmitResult.Temporaries),
            end  (expressionEmitResult.Temporaries)
        ); 

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(expressionEmitResult.Value->getType());
        temporaries.emplace_back(allocaInst, m_Expression->GetTypeInfo().Symbol->GetWithReference());

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            expressionEmitResult.Value,
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto Reference::GetTypeInfo() const -> TypeInfo
    {
        auto* const typeSymbol = m_Expression->GetTypeInfo().Symbol;
        const auto scope = typeSymbol->GetScope();

        return { typeSymbol->GetWithReference(), ValueKind::R };
    }
}
