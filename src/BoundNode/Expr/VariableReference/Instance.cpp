#include "BoundNode/Expr/VarReference/Instance.hpp"

#include <memory>
#include <vector>
#include <functional>

#include "BoundNode/Expr/FunctionCall/Instance.hpp"
#include "BoundNode/Expr/Base.hpp"
#include "BoundNode/Expr/DerefAs.hpp"
#include "Symbol/Var/Normal/Instance.hpp"
#include "Symbol/Function.hpp"
#include "Asserts.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExprEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expr::VarReference
{
    Instance::Instance(
        const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr,
        Symbol::Var::Normal::Instance* const t_variableSymbol
    ) : m_Expr{ t_expr },
        m_VarSymbol{ t_variableSymbol }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Expr->GetScope();
    }

    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expr);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>>>
    {
        ACE_TRY(mchCheckedExpr, m_Expr->GetOrCreateTypeCheckedExpr({}));

        if (!mchCheckedExpr.IsChanged)
        {
            CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::VarReference::Instance>(
            mchCheckedExpr.Value,
            m_VarSymbol
        );
        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateTypeCheckedExpr(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Instance::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::VarReference::Instance>>
    {
        const auto mchLoweredExpr =
            m_Expr->GetOrCreateLoweredExpr({});

        if (!mchLoweredExpr.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Expr::VarReference::Instance>(
            mchLoweredExpr.Value,
            m_VarSymbol
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Instance::GetOrCreateLoweredExpr(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Expr::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Instance::Emit(Emitter& t_emitter) const -> ExprEmitResult
    {
        std::vector<ExprDropData> temporaries{};

        auto* const variableSymbol =
            dynamic_cast<Symbol::Var::Normal::Instance*>(m_VarSymbol);
        ACE_ASSERT(variableSymbol);

        const std::function<std::shared_ptr<const BoundNode::Expr::IBase>(const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr)>
        getDereferenced = [&](const std::shared_ptr<const BoundNode::Expr::IBase>& t_expr) -> std::shared_ptr<const BoundNode::Expr::IBase>
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
                    return std::make_shared<const BoundNode::Expr::DerefAs>(
                        t_expr,
                        typeSymbol->GetWithoutReference()
                    );
                }
                else if (isStrongPointer)
                {
                    return std::make_shared<const BoundNode::Expr::DerefAs>(
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

    auto Instance::GetTypeInfo() const -> TypeInfo
    {
        return { m_VarSymbol->GetType(), ValueKind::L };
    }

    auto Instance::GetExpr() const -> std::shared_ptr<const BoundNode::Expr::IBase>
    {
        return m_Expr;
    }

    auto Instance::GetVarSymbol() const -> Symbol::Var::Normal::Instance*
    {
        return m_VarSymbol;
    }
}
