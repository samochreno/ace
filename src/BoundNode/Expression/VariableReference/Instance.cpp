#include "BoundNode/Expression/VariableReference/Instance.hpp"

#include <memory>
#include <vector>
#include <functional>

#include "BoundNode/Expression/FunctionCall/Instance.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/DerefAs.hpp"
#include "Symbol/Variable/Normal/Instance.hpp"
#include "Symbol/Function.hpp"
#include "Asserts.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "Emitter.hpp"
#include "ExpressionEmitResult.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"

namespace Ace::BoundNode::Expression::VariableReference
{
    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>>
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
            mchCheckedExpression.Value,
            m_VariableSymbol
            );
        
        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::VariableReference::Instance>>>
    {
        ACE_TRY(mchLoweredExpression, m_Expression->GetOrCreateLoweredExpression({}));

        if (!mchLoweredExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::VariableReference::Instance>(
            mchLoweredExpression.Value,
            m_VariableSymbol
            );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto Instance::Emit(Emitter& t_emitter) const -> ExpressionEmitResult
    {
        std::vector<ExpressionDropData> temporaries{};

        auto* const variableSymbol = dynamic_cast<Symbol::Variable::Normal::Instance*>(m_VariableSymbol);
        ACE_ASSERT(variableSymbol);

        const std::function<std::shared_ptr<const BoundNode::Expression::IBase>(const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)>
        getDereferenced = [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression) -> std::shared_ptr<const BoundNode::Expression::IBase>
        {
            auto* const typeSymbol = t_expression->GetTypeInfo().Symbol;

            const bool isReference = typeSymbol->IsReference();
            const bool isStrongPointer = typeSymbol->IsStrongPointer();

            if (!isReference && !isStrongPointer)
                return t_expression;

            return getDereferenced([&]()
            {
                if (isReference)
                {
                    return std::make_shared<const BoundNode::Expression::DerefAs>(
                        t_expression,
                        typeSymbol->GetWithoutReference()
                    );
                }
                else if (isStrongPointer)
                {
                    return std::make_shared<const BoundNode::Expression::DerefAs>(
                        t_expression,
                        typeSymbol->GetWithoutStrongPointer()
                    );
                }

                ACE_UNREACHABLE();
            }());
        };

        const auto expression = getDereferenced(m_Expression);
        const auto expressionEmitResult = expression->Emit(t_emitter);
        temporaries.insert(
            end  (temporaries), 
            begin(expressionEmitResult.Temporaries), 
            end  (expressionEmitResult.Temporaries)
        );

        auto* const expressionTypeSymbol = expression->GetTypeInfo().Symbol;
        auto* const expressionType = t_emitter.GetIRType(expressionTypeSymbol);

        const auto variableIndex = variableSymbol->GetIndex();

        auto* const int32Type = llvm::Type::getInt32Ty(
            *GetCompilation().LLVMContext
        );

        std::vector<llvm::Value*> indexList{};
        indexList.push_back(llvm::ConstantInt::get(int32Type, 0));
        indexList.push_back(llvm::ConstantInt::get(int32Type, variableIndex));

        auto* const gepInst = t_emitter.GetBlockBuilder().Builder.CreateGEP(
            expressionType,
            expressionEmitResult.Value,
            indexList,
            "",
            true
        );

        return { gepInst, temporaries };
    }

    auto Instance::GetTypeInfo() const -> TypeInfo
    {
        return { m_VariableSymbol->GetType(), ValueKind::L };
    }
}
