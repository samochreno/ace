#include "BoundNodes/Exprs/VarReferences/InstanceVarReferenceExprBoundNode.hpp"

#include <memory>
#include <vector>
#include <functional>

#include "BoundNodes/Exprs/FunctionCalls/InstanceFunctionCallExprBoundNode.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "BoundNodes/Exprs/DerefAsExprBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Assert.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceVarReferenceExprBoundNode::InstanceVarReferenceExprBoundNode(
        const std::shared_ptr<const IExprBoundNode>& t_expr,
        InstanceVarSymbol* const t_variableSymbol
    ) : m_Expr{ t_expr },
        m_VarSymbol{ t_variableSymbol }
    {
    }

    auto InstanceVarReferenceExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto InstanceVarReferenceExprBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarReferenceExprBoundNode>(
            mchCheckedExpr.Value,
            m_VarSymbol
        ));
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarReferenceExprBoundNode>(
            mchLoweredExpr.Value,
            m_VarSymbol
        )->GetOrCreateLowered({}).Value);
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto InstanceVarReferenceExprBoundNode::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const variableSymbol =
            dynamic_cast<InstanceVarSymbol*>(m_VarSymbol);
        ACE_ASSERT(variableSymbol);

        const std::function<std::shared_ptr<const IExprBoundNode>(const std::shared_ptr<const IExprBoundNode>& t_expr)>
        getDereferenced = [&](const std::shared_ptr<const IExprBoundNode>& t_expr) -> std::shared_ptr<const IExprBoundNode>
        {
            auto* const typeSymbol = t_expr->GetTypeInfo().Symbol;

            const bool isReference = typeSymbol->IsReference();
            const bool isStrongPointer = typeSymbol->IsStrongPointer();

            if (!isReference && !isStrongPointer)
            {
                return t_expr;
            }

            return getDereferenced([&]()
            {
                if (isReference)
                {
                    return std::make_shared<const DerefAsExprBoundNode>(
                        t_expr,
                        typeSymbol->GetWithoutReference()
                    );
                }
                else if (isStrongPointer)
                {
                    return std::make_shared<const DerefAsExprBoundNode>(
                        t_expr,
                        typeSymbol->GetWithoutStrongPointer()
                    );
                }

                ACE_UNREACHABLE();
            }());
        };

        const auto expr = getDereferenced(m_Expr);
        const auto exprEmitResult = expr->Emit(t_emitter);
        temporaries.insert(
            end  (temporaries), 
            begin(exprEmitResult.Temporaries), 
            end  (exprEmitResult.Temporaries)
        );

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const exprType = t_emitter.GetIRType(exprTypeSymbol);

        const auto variableIndex = variableSymbol->GetIndex();

        auto* const int32Type = llvm::Type::getInt32Ty(
            *GetCompilation()->LLVMContext
        );

        std::vector<llvm::Value*> indexList{};
        indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
        indexList.push_back(llvm::ConstantInt::get(int32Type, variableIndex));

        auto* const gepInst = t_emitter.GetBlockBuilder().Builder.CreateGEP(
            exprType,
            exprEmitResult.Value,
            indexList,
            "",
            true
        );

        return { gepInst, temporaries };
    }

    auto InstanceVarReferenceExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }

    auto InstanceVarReferenceExprBoundNode::GetExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return m_Expr;
    }

    auto InstanceVarReferenceExprBoundNode::GetVarSymbol() const -> InstanceVarSymbol*
    {
        return m_VarSymbol;
    }
}
