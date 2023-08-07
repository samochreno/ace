#include "BoundNodes/Exprs/VarRefs/InstanceVarRefExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "BoundNodes/Exprs/ExprBoundNode.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Assert.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    InstanceVarRefExprBoundNode::InstanceVarRefExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr,
        InstanceVarSymbol* const varSymbol
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr },
        m_VarSymbol{ varSymbol }
    {
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

    auto InstanceVarRefExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const InstanceVarRefExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const auto checkedExpr =
            diagnostics.Collect(m_Expr->CreateTypeCheckedExpr({}));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const InstanceVarRefExprBoundNode>(
                GetSrcLocation(),
                checkedExpr,
                m_VarSymbol
            ),
            diagnostics,
        };
    }

    auto InstanceVarRefExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto InstanceVarRefExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const InstanceVarRefExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        if (loweredExpr == m_Expr)
        {
            return shared_from_this();
        }

        return std::make_shared<const InstanceVarRefExprBoundNode>(
            GetSrcLocation(),
            loweredExpr,
            m_VarSymbol
        )->CreateLowered({});
    }

    auto InstanceVarRefExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    static auto CreateDerefed(
        const std::shared_ptr<const IExprBoundNode>& expr
    ) -> std::shared_ptr<const IExprBoundNode>
    {
        auto* const typeSymbol = expr->GetTypeInfo().Symbol;

        const bool isRef = typeSymbol->IsRef();
        const bool isStrongPtr = typeSymbol->IsStrongPtr();

        if (isRef)
        {
            return CreateDerefed(std::make_shared<const DerefAsExprBoundNode>(
                expr->GetSrcLocation(),
                expr,
                typeSymbol->GetWithoutRef()
            ));
        }

        if (isStrongPtr)
        {
            return CreateDerefed(std::make_shared<const DerefAsExprBoundNode>(
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

        const auto expr = CreateDerefed(m_Expr);
        const auto exprEmitResult = expr->Emit(emitter);
        tmps.insert(
            end  (tmps), 
            begin(exprEmitResult.Tmps), 
            end  (exprEmitResult.Tmps)
        );

        auto* const exprTypeSymbol = expr->GetTypeInfo().Symbol;
        auto* const exprType = emitter.GetIRType(exprTypeSymbol);

        const auto varIndex = varSymbol->GetIndex();

        auto* const int32Type = llvm::Type::getInt32Ty(emitter.GetContext());

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
