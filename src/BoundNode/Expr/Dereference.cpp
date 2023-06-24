#include "BoundNode/Expr//Dereference.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Type/Base.hpp"
#include "ExprEmitResult.hpp"

namespace Ace::BoundNode::Expr
{
    Dereference::Dereference(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr
    ) : m_Expr{ t_expr }
    {
    }

    auto Dereference::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Dereference::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Dereference::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::Dereference>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (mchCheckedExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::Dereference>(
            mchCheckedExpr.Value
        );
        return CreateChanged(returnValue);
    }
    auto Dereference::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Dereference::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::Dereference>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::Dereference>(
            mchLoweredExpr.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Dereference::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Dereference::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        const auto exprEmitResult = m_Expr->Emit(t_emitter);
        temporaries.insert(
            end(temporaries),
            begin(exprEmitResult.Temporaries),
            end  (exprEmitResult.Temporaries)
        );
            
        auto* const typeSymbol = m_Expr->GetTypeInfo().Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        auto* const type = llvm::PointerType::get(
            t_emitter.GetIRType(typeSymbol), 
            0
        );

        auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
            type,
            exprEmitResult.Value
        );
        temporaries.emplace_back(loadInst, typeSymbol->GetWithoutReference());

        return { loadInst, temporaries };
    }

    auto Dereference::GetTypeInfo() const -> TypeInfo
    {
        const auto typeInfo = m_Expr->GetTypeInfo();

        auto* const typeSymbol = typeInfo.Symbol;
        ACE_ASSERT(typeSymbol->IsReference());

        return { typeSymbol->GetWithoutReference(), typeInfo.ValueKind };
    }

    auto Dereference::GetExpr() const -> std::shared_ptr<const BoundNode::Expr::IBase>
    {
        return m_Expr;
    }
}
