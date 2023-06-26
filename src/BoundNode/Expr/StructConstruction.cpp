#include "BoundNode/Expr/StructConstruction.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "Asserts.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    StructConstruction::StructConstruction(
        const std::shared_ptr<Scope>& t_scope,
        StructTypeSymbol* const t_structSymbol,
        const std::vector<Arg>& t_args
    ) : m_Scope{ t_scope },
        m_StructSymbol{ t_structSymbol },
        m_Args{ t_args }
    {
    }

    auto StructConstruction::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StructConstruction::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        std::for_each(begin(m_Args), end(m_Args),
        [&](const Arg& t_arg)
        {
            AddChildren(children, t_arg.Value);
        });

        return children;
    }

    auto StructConstruction::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::StructConstruction>>>
    {
        ACE_TRY(mchCheckedArgs, TransformExpectedMaybeChangedVector(m_Args,
        [](const Arg& t_arg) -> Expected<MaybeChanged<Arg>>
        {
            ACE_TRY(mchCheckedValue, t_arg.Value->GetOrCreateTypeCheckedExpr({}));

            if (!mchCheckedValue.IsChanged)
            {
                return CreateUnchanged(t_arg);
            }

            return CreateChanged(Arg{
                t_arg.Symbol,
                mchCheckedValue.Value
            });
        }));

        if (!mchCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::StructConstruction>(
            m_Scope,
            m_StructSymbol,
            mchCheckedArgs.Value
        );
        return CreateChanged(returnValue);
    }

    auto StructConstruction::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto StructConstruction::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::StructConstruction>>
    {
        const auto mchLoweredArgs = TransformMaybeChangedVector(m_Args,
        [&](const Arg& t_arg) -> MaybeChanged<Arg>
        {
            const auto mchLoweredValue =
                t_arg.Value->GetOrCreateLoweredExpr({});

            if (!mchLoweredValue.IsChanged)
            {
                return CreateUnchanged(t_arg);
            }

            return CreateChanged(Arg{
                t_arg.Symbol,
                mchLoweredValue.Value
            });
        });

        if (!mchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::StructConstruction>(
            m_Scope,
            m_StructSymbol,
            mchLoweredArgs.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto StructConstruction::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto StructConstruction::Emit(
        Emitter& t_emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const structureType = t_emitter.GetIRType(m_StructSymbol);

        auto* const allocaInst =
            t_emitter.GetBlockBuilder().Builder.CreateAlloca(structureType);
        temporaries.emplace_back(allocaInst, m_StructSymbol);

        std::for_each(begin(m_Args), end(m_Args),
        [&](const BoundNode::Expr::StructConstruction::Arg& t_arg)
        {
            auto* const argTypeSymbol =
                t_arg.Value->GetTypeInfo().Symbol;
            auto* const argType = t_emitter.GetIRType(argTypeSymbol);

            const auto variableIndex = t_arg.Symbol->GetIndex();

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

            const auto argEmitResult = t_arg.Value->Emit(t_emitter);
            temporaries.insert(
                end(temporaries),
                begin(argEmitResult.Temporaries),
                end  (argEmitResult.Temporaries)
            );

            t_emitter.EmitCopy(
                elementPtr,
                argEmitResult.Value, 
                argTypeSymbol
            );
        });
        
        return { allocaInst, temporaries };
    }

    auto StructConstruction::GetTypeInfo() const -> TypeInfo
    {
        return TypeInfo{ m_StructSymbol, ValueKind::R };
    }
}
