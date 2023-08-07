#include "BoundNodes/Exprs/StructConstructionExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

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

    auto StructConstructionExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StructConstructionExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<StructConstructionExprBoundArg> checkedArgs{};
        std::transform(
            begin(m_Args),
            end  (m_Args),
            back_inserter(checkedArgs),
            [&](const StructConstructionExprBoundArg& arg)
            {
                const auto checkedValue =
                    diagnostics.Collect(arg.Value->CreateTypeCheckedExpr({}));

                return StructConstructionExprBoundArg
                {
                    arg.Symbol,
                    checkedValue,
                };
            }
        );

        if (checkedArgs == m_Args)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const StructConstructionExprBoundNode>(
                GetSrcLocation(),
                GetScope(),
                m_StructSymbol,
                checkedArgs
            ),
            diagnostics,
        };
    }

    auto StructConstructionExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto StructConstructionExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StructConstructionExprBoundNode>
    {
        std::vector<StructConstructionExprBoundArg> loweredArgs{};
        std::transform(begin(m_Args), end(m_Args), back_inserter(loweredArgs),
        [&](const StructConstructionExprBoundArg& arg)
        {
            const auto loweredValue = arg.Value->CreateLoweredExpr(context);

            return StructConstructionExprBoundArg
            {
                arg.Symbol,
                loweredValue,
            };
        });

        if (loweredArgs == m_Args)
        {
            return shared_from_this();
        }

        return std::make_shared<const StructConstructionExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            m_StructSymbol,
            loweredArgs
        )->CreateLowered({});
    }

    auto StructConstructionExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
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
                emitter.GetContext()
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
