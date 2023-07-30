#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Assert.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    StructConstructionExprBoundNode::StructConstructionExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        StructTypeSymbol* const structSymbol,
        const std::vector<StructConstructionExprBoundArg>& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_StructSymbol{ structSymbol },
        m_Args{ args }
    {
    }

    auto StructConstructionExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructConstructionExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstructionExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprBoundArg& arg)
        {
            AddChildren(children, arg.Value);
        });

        return children;
    }

    auto StructConstructionExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>>>
    {
        ACE_TRY(mchCheckedArgs, TransformExpectedMaybeChangedVector(m_Args,
        [](const StructConstructionExprBoundArg& arg) -> Expected<MaybeChanged<StructConstructionExprBoundArg>>
        {
            ACE_TRY(mchCheckedValue, arg.Value->GetOrCreateTypeCheckedExpr({}));

            if (!mchCheckedValue.IsChanged)
            {
                return CreateUnchanged(arg);
            }

            return CreateChanged(StructConstructionExprBoundArg{
                arg.Symbol,
                mchCheckedValue.Value,
            });
        }));

        if (!mchCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructConstructionExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            mchCheckedArgs.Value
        ));
    }

    auto StructConstructionExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StructConstructionExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StructConstructionExprBoundNode>>
    {
        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const StructConstructionExprBoundArg& arg) -> MaybeChanged<StructConstructionExprBoundArg>
        {
            const auto mchLoweredValue =
                arg.Value->GetOrCreateLoweredExpr({});

            if (!mchLoweredValue.IsChanged)
            {
                return CreateUnchanged(arg);
            }

            return CreateChanged(StructConstructionExprBoundArg{
                arg.Symbol,
                mchLoweredValue.Value,
            });
        });

        if (!mchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructConstructionExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            mchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StructConstructionExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto StructConstructionExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const structureType = emitter.GetIRType(m_StructSymbol);

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(structureType);
        tmps.emplace_back(allocaInst, m_StructSymbol);

        std::for_each(begin(m_Args), end(m_Args),
        [&](const StructConstructionExprBoundArg& arg)
        {
            auto* const argTypeSymbol =
                arg.Value->GetTypeInfo().Symbol;
            auto* const argType = emitter.GetIRType(argTypeSymbol);

            const auto varIndex = arg.Symbol->GetIndex();

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
                varIndex
            ));

            auto* const elementPtr = emitter.GetBlockBuilder().Builder.CreateGEP(
                structureType,
                allocaInst,
                indexList,
                "",
                true
            );

            const auto argEmitResult = arg.Value->Emit(emitter);
            tmps.insert(
                end(tmps),
                begin(argEmitResult.Tmps),
                end  (argEmitResult.Tmps)
            );

            emitter.EmitCopy(
                elementPtr,
                argEmitResult.Value, 
                argTypeSymbol
            );
        });
        
        return { allocaInst, tmps };
    }

    auto StructConstructionExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return TypeInfo{ m_StructSymbol, ValueKind::R };
    }
}
