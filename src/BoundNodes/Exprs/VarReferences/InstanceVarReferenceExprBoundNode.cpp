#include "BoundNodes/Exprs/VarReferences/InstanceVarReferenceExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceVarReferenceExprBoundNode::InstanceVarReferenceExprBoundNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        InstanceVarSymbol* const varSymbol
    ) : m_SourceLocation{ sourceLocation },
        m_Expr{ expr },
        m_VarSymbol{ varSymbol }
    {
    }

    auto InstanceVarReferenceExprBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarReferenceExprBoundNode>(
            GetSourceLocation(),
            mchCheckedExpr.Value,
            m_VarSymbol
        ));
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceVarReferenceExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarReferenceExprBoundNode>(
            GetSourceLocation(),
            mchLoweredExpr.Value,
            m_VarSymbol
        )->GetOrCreateLowered({}).Value);
    }

    auto InstanceVarReferenceExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    static auto GetOrCreateDereferenced(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isReference = typeSymbol->IsReference();
        const bool isStrongPointer = typeSymbol->IsStrongPointer();

        if (isReference)
        {
            return GetOrCreateDereferenced(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSourceLocation(),
                expr,
                typeSymbol->GetWithoutReference()
            ));
        }

        if (isStrongPointer)
        {
            return GetOrCreateDereferenced(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSourceLocation(),
                expr,
                typeSymbol->GetWithoutStrongPointer()
            ));
        }

        return expr;
    }

    auto InstanceVarReferenceExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const varSymbol = dynamic_cast<InstanceVarSymbol*>(m_VarSymbol);
        ACE_ASSERT(varSymbol);

        const auto expr = GetOrCreateDereferenced(m_Expr);
        const auto exprEmitResult = expr->Emit(emitter);
        temporaries.insert(
            end  (temporaries), 
            begin(exprEmitResult.Temporaries), 
            end  (exprEmitResult.Temporaries)
        );

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const exprType = emitter.GetIRType(exprTypeSymbol);

        const auto varIndex = varSymbol->GetIndex();

        auto* const int32Type = llvm::Type::getInt32Ty(
            *GetCompilation()->LLVMContext
        );

        std::vector<llvm::Value*> indexList{};
        indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
        indexList.push_back(llvm::ConstantInt::get(int32Type, varIndex));

        auto* const gepInst = emitter.GetBlockBuilder().Builder.CreateGEP(
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
