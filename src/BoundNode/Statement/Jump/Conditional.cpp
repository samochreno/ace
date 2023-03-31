#include "BoundNode/Statement/Jump/Conditional.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "NativeSymbol.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement::Jump
{
    auto Conditional::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Conditional::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Conditional>>>
    {
        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            TypeInfo{ NativeSymbol::Bool.GetSymbol(), ValueKind::R }
            ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Jump::Conditional>(
            mchConvertedAndCheckedCondition.Value,
            m_LabelSymbol
            );
        
        return CreateChanged(returnValue);
    }

    auto Conditional::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Conditional>>>
    {
        ACE_TRY(mchLoweredCondition, m_Condition->GetOrCreateLoweredExpression({}));

        if (!mchLoweredCondition.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Jump::Conditional>(
            mchLoweredCondition.Value,
            m_LabelSymbol
            );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered(t_context));
    }

    auto Conditional::Emit(Emitter& t_emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            t_emitter.GetContext(),
            t_emitter.GetFunction()
            );

        const auto conditionEmitResult = m_Condition->Emit(t_emitter);

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            NativeSymbol::Bool.GetIRType(t_emitter),
            conditionEmitResult.Value
        );

        t_emitter.EmitDropTemporaries(conditionEmitResult.Temporaries);

        t_emitter.GetBlockBuilder().Builder.CreateCondBr(
            loadInst,
            t_emitter.GetLabelBlockMap().GetOrCreateAt(m_LabelSymbol),
            blockBuilder->Block
        );

        t_emitter.SetBlockBuilder(std::move(blockBuilder));
    }
}
