#include "BoundNodes/Exprs/VarRefs/InstanceVarRefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Assert.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceVarRefExprBoundNode::InstanceVarRefExprBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        InstanceVarSymbol* const varSymbol
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_VarSymbol{ varSymbol }
    {
    }

    auto InstanceVarRefExprBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto InstanceVarRefExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InstanceVarRefExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto InstanceVarRefExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto InstanceVarRefExprBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarRefExprBoundNode>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarRefExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            mchCheckedExpr.Value,
            m_VarSymbol
        ));
    }

    auto InstanceVarRefExprBoundNode::GetOrCreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const IExprBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto InstanceVarRefExprBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceVarRefExprBoundNode>>
    {
        const auto mchLoweredExpr = m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarRefExprBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            mchLoweredExpr.Value,
            m_VarSymbol
        )->GetOrCreateLowered({}).Value);
    }

    auto InstanceVarRefExprBoundNode::GetOrCreateLoweredExpr(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const IExprBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    static auto GetOrCreateDerefd(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isRef = typeSymbol->IsRef();
        const bool isStrongPtr = typeSymbol->IsStrongPtr();

        if (isRef)
        {
            return GetOrCreateDerefd(std::make_shared<const DerefAsExprBoundNode>(
                DiagnosticBag{},
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (isStrongPtr)
        {
            return GetOrCreateDerefd(std::make_shared<const DerefAsExprBoundNode>(
                DiagnosticBag{},
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutStrongPtr()
            ));
        }

        return expr;
    }

    auto InstanceVarRefExprBoundNode::Emit(
        Emitter& emitter
    ) const -> ExprEmitResult
    {
        std::vector<ExprDropData> tmps{};

        auto* const varSymbol = dynamic_cast<InstanceVarSymbol*>(m_VarSymbol);
        ACE_ASSERT(varSymbol);

        const auto expr = GetOrCreateDerefd(m_Expr);
        const auto exprEmitResult = expr->Emit(emitter);
        tmps.insert(
            end  (tmps), 
            begin(exprEmitResult.Tmps), 
            end  (exprEmitResult.Tmps)
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

        return { gepInst, tmps };
    }

    auto InstanceVarRefExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }

    auto InstanceVarRefExprBoundNode::GetExpr() const -> std::shared_ptr<const IExprBoundNode>
    {
        return m_Expr;
    }

    auto InstanceVarRefExprBoundNode::GetVarSymbol() const -> InstanceVarSymbol*
    {
        return m_VarSymbol;
    }
}
