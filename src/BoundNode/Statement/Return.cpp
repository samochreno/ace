#include "BoundNode/Statement/Return.hpp"

#include <memory>
#include <vector>

#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Emitter.hpp"

namespace Ace::BoundNode::Statement
{
    auto Return::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        if (m_OptExpression.has_value())
        {
            AddChildren(children, m_OptExpression.value());
        }

        return children;
    }

    auto Return::GetOrCreateTypeChecked(const BoundNode::Statement::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Statement::Return>>>
    {
        const bool isFunctionTypeVoid = 
            t_context.ParentFunctionTypeSymbol == 
            GetCompilation().Natives->Void.GetSymbol();

        ACE_TRY_ASSERT(m_OptExpression.has_value() != isFunctionTypeVoid);

        if (m_OptExpression.has_value())
        {
            auto* const expressionTypeSymbol = 
                m_OptExpression.value()->GetTypeInfo().Symbol->GetUnaliased();

            const bool isExpressionTypeVoid = 
                expressionTypeSymbol ==
                GetCompilation().Natives->Void.GetSymbol();

            ACE_TRY_ASSERT(!isExpressionTypeVoid);
        }

        ACE_TRY(mchCheckedOptExpression, TransformExpectedMaybeChangedOptional(m_OptExpression,
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            return CreateImplicitlyConvertedAndTypeChecked(
                t_expression,
                { t_context.ParentFunctionTypeSymbol, ValueKind::R }
            );
        }));

        if (!mchCheckedOptExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Return>(
            m_Scope,
            mchCheckedOptExpression.Value
        );
        
        return CreateChanged(returnValue);
    }

    auto Return::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Statement::Return>>
    {
        const auto mchLoweredOptExpression = TransformMaybeChangedOptional(m_OptExpression,
        [&](const std::shared_ptr<const BoundNode::Expression::IBase>& t_expression)
        {
            return t_expression->GetOrCreateLoweredExpression({});
        });

        if (!mchLoweredOptExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Statement::Return>(
            m_Scope,
            mchLoweredOptExpression.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered(t_context).Value);
    }

    auto Return::Emit(Emitter& t_emitter) const -> void
    {
        if (m_OptExpression.has_value())
        {
            const auto expressionEmitResult = m_OptExpression.value()->Emit(t_emitter);
            
            auto* const typeSymbol = m_OptExpression.value()->GetTypeInfo().Symbol;
            auto* const type = t_emitter.GetIRType(typeSymbol);

            auto* const allocaInst = t_emitter.GetBlockBuilder().Builder.CreateAlloca(type);

            t_emitter.EmitCopy(
                allocaInst,
                expressionEmitResult.Value,
                typeSymbol
            );

            t_emitter.EmitDropTemporaries(expressionEmitResult.Temporaries);
            t_emitter.EmitDropLocalVariablesBeforeStatement(this);
            
            auto* const loadInst = t_emitter.GetBlockBuilder().Builder.CreateLoad(
                type,
                allocaInst
            );

            t_emitter.GetBlockBuilder().Builder.CreateRet(loadInst);
        }
        else
        {
            t_emitter.EmitDropLocalVariablesBeforeStatement(this);

            t_emitter.GetBlockBuilder().Builder.CreateRetVoid();
        }
    }
}
