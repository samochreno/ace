#include "BoundNodes/ImplBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    ImplBoundNode::ImplBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Functions{ functions }
    {
    }

    auto ImplBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ImplBoundNode>>>
    {
        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ImplBoundNode>(
            GetSrcLocation(),
            GetScope(),
            mchCheckedFunctions.Value
        ));
    }

    auto ImplBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ImplBoundNode>>
    {
        const auto mchLoweredFunctions = TransformMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateLowered({});
        });

        if (!mchLoweredFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ImplBoundNode>(
            GetSrcLocation(),
            GetScope(),
            mchLoweredFunctions.Value
        )->GetOrCreateLowered({}).Value);
    }
}
