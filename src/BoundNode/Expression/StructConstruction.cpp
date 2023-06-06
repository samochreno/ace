#include "BoundNode/Expression/StructConstruction.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Asserts.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto StructConstruction::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        std::for_each(begin(m_Arguments), end(m_Arguments),
        [&](const Argument& t_argument)
        {
            AddChildren(children, t_argument.Value);
        });

        return children;
    }
    
    auto StructConstruction::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::StructConstruction>>>
    {
        ACE_TRY(mchCheckedArguments, TransformExpectedMaybeChangedVector(m_Arguments,
        [](const Argument& t_argument) -> Expected<MaybeChanged<Argument>>
        {
            ACE_TRY(mchCheckedValue, t_argument.Value->GetOrCreateTypeCheckedExpression({}));

            if (!mchCheckedValue.IsChanged)
                return CreateUnchanged(t_argument);

            return CreateChanged(Argument{ t_argument.Symbol, mchCheckedValue.Value });
        }));

        if (!mchCheckedArguments.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::StructConstruction>(
            m_Scope,
            m_StructSymbol,
            mchCheckedArguments.Value
        );
        return CreateChanged(returnValue);
    }

    auto StructConstruction::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expression::StructConstruction>>
    {
        const auto mchLoweredArguments = TransformMaybeChangedVector(m_Arguments,
        [&](const Argument& t_argument) -> MaybeChanged<Argument>
        {
            const auto mchLoweredValue = t_argument.Value->GetOrCreateLoweredExpression({});

            if (!mchLoweredValue.IsChanged)
                return CreateUnchanged(t_argument);

            return CreateChanged(Argument{ t_argument.Symbol, mchLoweredValue.Value });
        });

        if (!mchLoweredArguments.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::StructConstruction>(
            m_Scope,
            m_StructSymbol,
            mchLoweredArguments.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto StructConstruction::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        auto* const structureType = t_emitter.GetIRType(m_StructSymbol);

        auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(structureType);
        temporaries.emplace_back(allocaInst, m_StructSymbol);

        std::for_each(begin(m_Arguments), end(m_Arguments),
        [&](const BoundNode::Expression::StructConstruction::Argument& t_argument)
        {
            auto* const argumentTypeSymbol = t_argument.Value->GetTypeInfo().Symbol;
            auto* const argumentType = t_emitter.GetIRType(argumentTypeSymbol);

            const auto variableIndex = t_argument.Symbol->GetIndex();

            auto* const int32Type = llvm::Type::getInt32Ty(
                *GetCompilation()->LLVMContext
            );

            std::vector<llvm::Value*> indexList{};
            indexList.push_back(llvm::ConstantInt::get(
                int32Type,
                0
            ));
            indexList.push_back(llvm::ConstantInt::get(
                int32Type,
                variableIndex
            ));

            auto* const elementPtr = t_emitter.GetBlockBuilder().Builder.CreateGEP(
                structureType,
                allocaInst,
                indexList,
                "",
                true
            );

            const auto argumentEmitResult = t_argument.Value->Emit(t_emitter);
            temporaries.insert(end(temporaries), begin(argumentEmitResult.Temporaries), end(argumentEmitResult.Temporaries));

            t_emitter.EmitCopy(
                elementPtr,
                argumentEmitResult.Value, 
                argumentTypeSymbol
            );
        });
        
        return { allocaInst, temporaries };
    }

    auto StructConstruction::GetTypeInfo() const -> TypeInfo
    {
        return TypeInfo{ m_StructSymbol, ValueKind::R };
    }
}
