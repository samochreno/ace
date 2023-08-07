#include "BoundNodes/Exprs/UnboxExprBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/Exprs/FunctionCalls/StaticFunctionCallExprBoundNode.hpp"
#include "Symbols/FunctionSymbol.hpp"
#include "Symbols/Templates/FunctionTemplateSymbol.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace
{
    UnboxExprBoundNode::UnboxExprBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<const IExprBoundNode>& expr
    ) : m_SrcLocation{ srcLocation },
        m_Expr{ expr }
    {
    }

    auto UnboxExprBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto UnboxExprBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto UnboxExprBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Expr);
        
        return children;
    }

    auto UnboxExprBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const UnboxExprBoundNode>>
    {
        DiagnosticBag diagnostics{};

        const bool isStrongPtr = m_Expr->GetTypeInfo().Symbol->IsStrongPtr();
        if (!isStrongPtr)
        {
            diagnostics.Add(CreateExpectedStrongPtrExprError(GetSrcLocation()));
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives().StrongPtr__value.GetSymbol(),
            std::nullopt,
            { m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        const TypeInfo typeInfo
        {
            functionSymbol->CollectParams().front()->GetType(),
            ValueKind::R,
        };
        const auto checkedExpr = diagnostics.Collect(CreateImplicitlyConvertedAndTypeChecked(
            m_Expr,
            typeInfo
        ));

        if (checkedExpr == m_Expr)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const UnboxExprBoundNode>(
                GetSrcLocation(),
                checkedExpr
            ),
            diagnostics,
        };
    }

    auto UnboxExprBoundNode::CreateTypeCheckedExpr(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const IExprBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto UnboxExprBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticFunctionCallExprBoundNode>
    {
        const auto loweredExpr = m_Expr->CreateLoweredExpr({});

        auto* const symbol = Scope::ResolveOrInstantiateTemplateInstance(
            SrcLocation{},
            GetCompilation()->GetNatives().StrongPtr__value.GetSymbol(),
            std::nullopt,
            { loweredExpr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr() },
            {}
        ).Unwrap();
        auto* const functionSymbol = dynamic_cast<FunctionSymbol*>(symbol);
        ACE_ASSERT(functionSymbol);

        return std::make_shared<const StaticFunctionCallExprBoundNode>(
            GetSrcLocation(),
            GetScope(),
            functionSymbol,
            std::vector{ loweredExpr }
        )->CreateLowered({});
    }

    auto UnboxExprBoundNode::CreateLoweredExpr(
        const LoweringContext& context
    ) const -> std::shared_ptr<const IExprBoundNode>
    {
        return CreateLowered(context);
    }

    auto UnboxExprBoundNode::Emit(Emitter& emitter) const -> ExprEmitResult
    {
        ACE_UNREACHABLE();
    }

    auto UnboxExprBoundNode::GetTypeInfo() const -> TypeInfo
    {
        return
        {
            m_Expr->GetTypeInfo().Symbol->GetWithoutRef()->GetWithoutStrongPtr(),
            ValueKind::R,
        };
    }
}
