#include "BoundNode/Type/Struct.hpp"

#include <memory>
#include <vector>

#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"
#include "BoundNode/Attribute.hpp"
#include "BoundNode/Variable/Normal/Instance.hpp"

namespace Ace::BoundNode::Type
{
    Struct::Struct(
        Symbol::Type::Struct* const t_symbol,
        const std::vector<std::shared_ptr<const BoundNode::Attribute>>& t_attributes,
        const std::vector<std::shared_ptr<const BoundNode::Variable::Normal::Instance>>& t_variables
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes },
        m_Variables{ t_variables }
    {
    }

    auto Struct::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Struct::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Variables);

        return children;
    }

    auto Struct::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedVariables, TransformExpectedMaybeChangedVector(m_Variables,
        [](const std::shared_ptr<const BoundNode::Variable::Normal::Instance>& t_variable)
        {
            return t_variable->GetOrCreateTypeChecked({});
        }));

        if (
            !mchCheckedAttributes.IsChanged &&
            !mchCheckedVariables.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Type::Struct>(
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedVariables.Value
        );
        return CreateChanged(returnValue);
    }

    auto Struct::GetOrCreateTypeCheckedType(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>>>
    {
        return GetOrCreateTypeChecked(t_context);
    }

    auto Struct::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::Struct>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });

        const auto mchLoweredVariables = TransformMaybeChangedVector(m_Variables,
        [](const std::shared_ptr<const BoundNode::Variable::Normal::Instance>& t_variable)
        {
            return t_variable->GetOrCreateLowered({});
        });

        if (
            !mchLoweredAttributes.IsChanged &&
            !mchLoweredVariables.IsChanged
            )
        {
            return CreateUnchanged(this->shared_from_this());
        }

        const auto returnValue = std::make_shared<const BoundNode::Type::Struct>(
            m_Symbol,
            mchLoweredAttributes.Value,
            mchLoweredVariables.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }

    auto Struct::GetOrCreateLoweredType(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Type::IBase>>
    {
        return GetOrCreateLowered(t_context);
    }

    auto Struct::GetSymbol() const -> Symbol::Type::Struct*
    {
        return m_Symbol;
    }
}
