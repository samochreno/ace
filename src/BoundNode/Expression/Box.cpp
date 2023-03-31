#include "BoundNode/Expression/Box.hpp"

#include <memory>
#include <vector>

#include "TypeInfo.hpp"
#include "ValueKind.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Expression/FunctionCall/Static.hpp"
#include "NativeSymbol.hpp"
#include "Symbol/Function.hpp"
#include "Symbol/Template/Function.hpp"
#include "ExpressionEmitResult.hpp"

namespace Ace::BoundNode::Expression
{
    auto Box::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Expression);

        return children;
    }

    auto Box::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::Box>>> 
    {
        ACE_TRY(mchCheckedExpression, m_Expression->GetOrCreateTypeCheckedExpression({}));

        if (!mchCheckedExpression.IsChanged)
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Expression::Box>(mchCheckedExpression.Value);

        return CreateChanged(returnValue);
    }

    auto Box::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Expression::FunctionCall::Static>>> 
    {
        ACE_TRY(mchLoweredExpression, m_Expression->GetOrCreateLoweredExpression({}));

        ACE_TRY(symbol, Scope::ResolveOrInstantiateTemplateInstance(
            NativeSymbol::StrongPointer__new.GetSymbol(),
            { mchLoweredExpression.Value->GetTypeInfo().Symbol },
            {}
        ));
        
        auto* const functionSymbol = dynamic_cast<Symbol::Function*>(symbol);
        ACE_ASSERT(functionSymbol);

        const auto returnValue = std::make_shared<const BoundNode::Expression::FunctionCall::Static>(
            GetScope(),
            functionSymbol,
            std::vector{ mchLoweredExpression.Value }
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }

    auto Box::GetTypeInfo() const -> TypeInfo
    {
        return { m_Expression->GetTypeInfo().Symbol->GetWithStrongPointer(), ValueKind::R };
    }
}
