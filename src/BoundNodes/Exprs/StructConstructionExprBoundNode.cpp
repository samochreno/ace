#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Cacheable.hpp"
#include "Emitter.hpp"
#include "Assert.hpp"
#include "ExprEmitResult.hpp"

namespace Ace
{
    StructConstructionExprBoundNode::StructConstructionExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        StructTypeSymbol* const structSymbol,
        const std::vector<StructConstructionExprBoundArg>& args
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_StructSymbol{ structSymbol },
        m_Args{ args }
    {
    }

    auto StructConstructionExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto StructConstructionExprBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const StructConstructionExprBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const StructConstructionExprBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            m_Args
        );
    }

    auto StructConstructionExprBoundNode::CloneWithDiagnosticsExpr(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto StructConstructionExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const StructConstructionExprBoundNode>>>
    {
        ACE_TRY(cchCheckedArgs, TransformExpectedCacheableVector(m_Args,
        [](const StructConstructionExprBoundArg& arg) -> Expected<Cacheable<StructConstructionExprBoundArg>>
        {
            ACE_TRY(cchCheckedValue, arg.Value->GetOrCreateTypeCheckedExpr({}));

            if (!cchCheckedValue.IsChanged)
            {
                return CreateUnchanged(arg);
            }

            return CreateChanged(StructConstructionExprBoundArg{
                arg.Symbol,
                cchCheckedValue.Value,
            });
        }));

        if (!cchCheckedArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructConstructionExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            cchCheckedArgs.Value
        ));
    }

    auto StructConstructionExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StructConstructionExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StructConstructionExprBoundNode>>
    {
        const auto cchLoweredArgs = TransformCacheableVector(m_Args,
        [&](const StructConstructionExprBoundArg& arg) -> Cacheable<StructConstructionExprBoundArg>
        {
            const auto cchLoweredValue =
                arg.Value->GetOrCreateLoweredExpr({});

            if (!cchLoweredValue.IsChanged)
            {
                return CreateUnchanged(arg);
            }

            return CreateChanged(StructConstructionExprBoundArg{
                arg.Symbol,
                cchLoweredValue.Value,
            });
        });

        if (!cchLoweredArgs.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructConstructionExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            cchLoweredArgs.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StructConstructionExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const IExprBoundNode>>
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
                GetCompilation()->GetLLVMContext()
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
