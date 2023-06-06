#include "BoundNode/Expression//Dereference.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Type/Base.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto Dereference::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Dereference::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Dereference>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Dereference>(
            mchCheckedExpression.Value
        );
        return CreateChanged(returnValue);
    }

    auto Dereference::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::Dereference>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Dereference>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Dereference::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        temporaries.insert(end(temporaries), begin(expressionEmitResult.Temporaries), end(expressionEmitResult.Temporaries));
            
        auto* const typeSymbol = m_Expression->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        auto* const type = llvm::PointerType::get(
            t_emitter.GetIRType(typeSymbol), 
            0
        );

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            type,
            expressionEmitResult.Value
        );
        temporaries.emplace_back(loadInst, typeSymbol->GetWithoutReference());

        return { loadInst, temporaries };
    }

    auto Dereference::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expression->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        return { typeSymbol->GetWithoutReference(), typeInfo.ValueKind };
    }
}
