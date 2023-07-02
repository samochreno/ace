#include "BoundNodes/ImplBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    ImplBoundNode::ImplBoundNode(
        const std::shared_ptr<Scope>& t_scope,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& t_functions
    ) : m_Scope{ t_scope },
        m_Functions{ t_functions }
    {
    }

    auto ImplBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Functions);

        return children;
    }

    auto ImplBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ImplBoundNode>>>
    {
        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& t_function)
        {
            return t_function->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const ImplBoundNode>(
            m_Scope,
            mchCheckedFunctions.Value
        );
        return CreateChanged(returnValue);
    }

    auto ImplBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ImplBoundNode>>
    {
        const auto mchLoweredFunctions = TransformMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& t_function)
        {
            return t_function->GetOrCreateLowered({});
        });

        if (!mchLoweredFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const ImplBoundNode>(
            m_Scope,
            mchLoweredFunctions.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
