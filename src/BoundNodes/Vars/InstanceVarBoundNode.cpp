#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    InstanceVarBoundNode::InstanceVarBoundNode(
        InstanceVarSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes }
    {
    }

    auto InstanceVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto InstanceVarBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto InstanceVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const InstanceVarBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarBoundNode>(
            m_Symbol,
            mchCheckedAttributes.Value
        ));
    }

    auto InstanceVarBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const InstanceVarBoundNode>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });

        if (!mchLoweredAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const InstanceVarBoundNode>(
            m_Symbol,
            mchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto InstanceVarBoundNode::GetSymbol() const -> InstanceVarSymbol*
    {
        return m_Symbol;
    }
}
