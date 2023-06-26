#include "BoundNode/Var/Normal/Instance.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Attribute.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode::Var::Normal
{
    Instance::Instance(
        InstanceVarSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes }
    {
    }

    auto Instance::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Instance::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto Instance::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Instance>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Var::Normal::Instance>(
            m_Symbol,
            mchCheckedAttributes.Value
        );
        return CreateChanged(returnValue);
    }

    auto Instance::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Var::Normal::Instance>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });

        if (!mchLoweredAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Var::Normal::Instance>(
            m_Symbol,
            mchLoweredAttributes.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Instance::GetSymbol() const -> InstanceVarSymbol*
    {
        return m_Symbol;
    }
}
