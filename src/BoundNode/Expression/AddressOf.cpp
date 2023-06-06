#include "BoundNode/Expression//AddressOf.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "ExpressionDropData.hpp"

namespace Ace::BoundNode::Expression
{
    auto AddressOf::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto AddressOf::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::AddressOf>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());
         
        const auto returnValue = std::make_shared<const BoundNode::Expression::AddressOf>(
            m_Expression
        );
        return CreateChanged(returnValue);
    }

    auto AddressOf::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::AddressOf>>
    {
        const auto mchLoweredExpression = m_Expression->GetOrCreateLoweredExpression({});

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::AddressOf>(
            mchLoweredExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto AddressOf::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        const auto expressionEmitResult = m_Expression->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(expressionEmitResult.Temporaries),
            end  (expressionEmitResult.Temporaries)
        );

        auto* const typeSymbol = m_Expression->GetTypeInfo().Symbol;
        auto* const type = llvm::PointerType::get(
            t_emitter.GetIRType(typeSymbol),
            0
        );

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(type);
        temporaries.emplace_back(
            allocaInst, 
            GetCompilation().Natives->Pointer.GetSymbol()
        );

        t_emitter.GetBlockBuilder().Builder.CreateStore(
            expressionEmitResult.Value, 
            allocaInst
        );

        return { allocaInst, temporaries };
    }

    auto AddressOf::GetTypeInfo() const -> TypeInfo
    {
        return 
        { 
            GetCompilation().Natives->Pointer.GetSymbol(), 
            ValueKind::R
        };
    }
}
