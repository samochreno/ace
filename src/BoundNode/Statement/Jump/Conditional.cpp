#include "BoundNode/Statement/Jump/Conditional.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Expression/Base.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement::Jump
{
    Conditional::Conditional(
        const std::shared_ptr<const BoundNode::Expression::IBase>& t_condition,
        Symbol::Label* const t_labelSymbol
    ) : m_Condition{ t_condition },
        m_LabelSymbol{ t_labelSymbol }
    {
    }

    auto Conditional::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Condition->GetScope();
    }

    auto Conditional::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Condition);

        return children;
    }

    auto Conditional::GetOrCreateTypeChecked(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Conditional>>>
    {
        const TypeInfo typeInfo
        {
            GetCompilation()->Natives->Bool.GetSymbol(),
            ValueKind::R,
        };

        ACE_TRY(mchConvertedAndCheckedCondition, CreateImplicitlyConvertedAndTypeChecked(
            m_Condition,
            typeInfo
        ));

        if (!mchConvertedAndCheckedCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Jump::Conditional>(
            mchConvertedAndCheckedCondition.Value,
            m_LabelSymbol
        );
        return CreateChanged(returnValue);
    }

    auto Conditional::GetOrCreateTypeCheckedStatement(
        const BoundNode::Statement::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Conditional::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Jump::Conditional>>
    {
        const auto mchLoweredCondition =
            m_Condition->GetOrCreateLoweredExpression({});

        if (!mchLoweredCondition.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Statement::Jump::Conditional>(
            mchLoweredCondition.Value,
            m_LabelSymbol
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Conditional::GetOrCreateLoweredStatement(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Conditional::Emit(Emitter& t_emitter) const -> void
    {
        auto blockBuilder = std::make_unique<BlockBuilder>(
            *GetCompilation()->LLVMContext,
            t_emitter.GetFunction()
        );

        const auto conditionEmitResult = m_Condition->Emit(t_emitter);

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            GetCompilation()->Natives->Bool.GetIRType(),
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

    auto Conditional::GetLabelSymbol() const -> Symbol::Label*
    {
        return m_LabelSymbol;
    }
}
