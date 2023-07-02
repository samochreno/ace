#include "BoundNodes/Types/StructTypeBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"

namespace Ace
{
    StructTypeBoundNode::StructTypeBoundNode(
        StructTypeSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes,
        const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& t_variables
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes },
        m_Vars{ t_variables }
    {
    }

    auto StructTypeBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto StructTypeBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Vars);

        return children;
    }

    auto StructTypeBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StructTypeBoundNode>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedVars, TransformExpectedMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const InstanceVarBoundNode>& t_variable)
        {
            return t_variable->GetOrCreateTypeChecked({});
        }));

        if (
            !mchCheckedAttributes.IsChanged &&
            !mchCheckedVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const StructTypeBoundNode>(
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedVars.Value
        );
        return CreateChanged(returnValue);
    }

    auto StructTypeBoundNode::GetOrCreateTypeCheckedType(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ITypeBoundNode>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto StructTypeBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const StructTypeBoundNode>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });

        const auto mchLoweredVars = TransformMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const InstanceVarBoundNode>& t_variable)
        {
            return t_variable->GetOrCreateLowered({});
        });

        if (
            !mchLoweredAttributes.IsChanged &&
            !mchLoweredVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const StructTypeBoundNode>(
            m_Symbol,
            mchLoweredAttributes.Value,
            mchLoweredVars.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto StructTypeBoundNode::GetOrCreateLoweredType(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ITypeBoundNode>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto StructTypeBoundNode::GetSymbol() const -> StructTypeSymbol*
    {
        return m_Symbol;
    }
}
