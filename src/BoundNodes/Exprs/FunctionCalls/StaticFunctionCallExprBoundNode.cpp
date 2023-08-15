#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    StaticFunctionCallExprBoundNode::StaticFunctionCallExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        FunctionSymbol* const functionSymbol,
        const std::vector<std::shared_ptr<const IExprBoundNode>>& args
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_FunctionSymbol{ functionSymbol },
        m_Args{ args }
    {
    }

    auto StaticFunctionCallExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticFunctionCallExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto StaticFunctionCallExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Args);

        return children;
    }

    auto StaticFunctionCallExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StaticFunctionCallExprBoundNode>>
    {
        auto diagnostics = DiagnosticBag::Create();

        std::vector<std::shared_ptr<const IExprBoundNode>> convertedArgs = m_Args;
        if (!m_FunctionSymbol->IsError())
        {
            const auto argTypeInfos = m_FunctionSymbol->CollectArgTypeInfos();

            if (m_Args.size() == argTypeInfos.size())
            {
                for (size_t i = 0; i < m_Args.size(); i++)
                {
                    convertedArgs.at(i) = diagnostics.Collect(CreateImplicitlyConverted(
                        m_Args.at(i),
                        argTypeInfos.at(i)
                    ));
                }
            }
            else
            {
                diagnostics.Add(CreateUnexpectedArgCountError(
                    GetSrcLocation(),
                    m_FunctionSymbol,
                    m_Args.size(),
                    argTypeInfos.size()
                ));
            }
        }

        std::vector<std::shared_ptr<const IExprBoundNode>> checkedArgs{};
        std::transform(
            begin(convertedArgs),
            end  (convertedArgs),
            back_inserter(checkedArgs),
            [&](const std::shared_ptr<const IExprBoundNode>& arg)
            {
                return diagnostics.Collect(arg->CreateTypeCheckedExpr({}));
            }
        );

        if (checkedArgs == m_Args)
        {
            return Diagnosed{ shared_from_this(), std::move(diagnostics) };
        }

        return Diagnosed
        {
            std::make_shared<const StaticFunctionCallExprBoundNode>(
                GetSrcLocation(),
                GetScope(),
                m_FunctionSymbol,
                checkedArgs
            ),
            std::move(diagnostics),
        };
    }

    auto StaticFunctionCallExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto StaticFunctionCallExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticFunctionCallExprBoundNode>
    {
        std::vector<std::shared_ptr<const IExprBoundNode>> loweredArgs{};
        if (!m_FunctionSymbol->IsError())
        {
            std::transform(
                begin(m_Args),
                end  (m_Args),
                back_inserter(loweredArgs),
                [&](const std::shared_ptr<const IExprBoundNode>& arg)
                {
                    return arg->CreateLoweredExpr({});
                }
            );
        }

        if (loweredArgs == m_Args)
        {
            return shared_from_this();
        }

        return std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_FunctionSymbol,
            loweredArgs
        )->CreateLowered({});
    }

    auto StaticFunctionCallExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto StaticFunctionCallExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        std::vector<llvm::Value*> args{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(args),
        [&](const std::shared_ptr<const IExprBoundNode>& arg)
        {
            const auto argEmitResult = arg->Emit(emitter);
            tmps.insert(
                end(tmps),
                begin(argEmitResult.Tmps),
                end  (argEmitResult.Tmps)
            );
            return argEmitResult.Value;
        });

        auto* const callInst = emitter.GetBlockBuilder().Builder.CreateCall(
            emitter.GetFunctionMap().at(m_FunctionSymbol),
            args
        );

        if (callInst->getType()->isVoidTy())
        {
            return { nullptr, tmps };
        }

        auto* const allocaInst =
            emitter.GetBlockBuilder().Builder.CreateAlloca(callInst->getType());
        tmps.emplace_back(allocaInst, m_FunctionSymbol->GetType());

        emitter.GetBlockBuilder().Builder.CreateStore(
            callInst,
            allocaInst
        );

        return { allocaInst, tmps };
    }
    
    auto StaticFunctionCallExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_FunctionSymbol->GetType(), ValueKind::R };
    }
}
