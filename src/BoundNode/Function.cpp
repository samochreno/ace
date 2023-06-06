#include "BoundNode/Function.hpp"

#include <memory>
#include <vector>
#include <optional>

#include "BoundNode/Attribute.hpp"
#include "BoundNode/Variable/Parameter/Self.hpp"
#include "BoundNode/Variable/Parameter/Normal.hpp"
#include "BoundNode/Statement/Block.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    auto Function::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Attributes);

        if (m_OptSelf.has_value())
        {
            AddChildren(children, m_OptSelf.value());
        }

        AddChildren(children, m_Parameters);

        if (m_OptBody.has_value())
        {
            AddChildren(children, m_OptBody.value());
        }

        return children;
    }

    auto Function::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Function>>>
    {
        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptSelf, TransformExpectedMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const BoundNode::Variable::Parameter::Self>& t_self)
        {
            return t_self->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedParameters, TransformExpectedMaybeChangedVector(m_Parameters,
        [](const std::shared_ptr<const BoundNode::Variable::Parameter::Normal>& t_parameter)
        {
            return t_parameter->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedOptBody, TransformExpectedMaybeChangedOptional(m_OptBody,
        [&](const std::shared_ptr<const BoundNode::Statement::Block>& t_body)
        {
            return t_body->GetOrCreateTypeChecked({ m_Symbol->GetType() });
        }));

        if (
            !mchCheckedAttributes.IsChanged &&
            !mchCheckedOptSelf.IsChanged && 
            !mchCheckedParameters.IsChanged && 
            !mchCheckedOptBody.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Function>(
            m_Symbol,
            mchCheckedAttributes.Value,
            mchCheckedOptSelf.Value,
            mchCheckedParameters.Value,
            mchCheckedOptBody.Value
        );
        return CreateChanged(returnValue);
    }

    auto Function::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> MaybeChanged<std::shared_ptr<const BoundNode::Function>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const BoundNode::Attribute>& t_attribute)
        {
            return t_attribute->GetOrCreateLowered({});
        });
        
        const auto mchLoweredOptSelf = TransformMaybeChangedOptional(m_OptSelf,
        [](const std::shared_ptr<const BoundNode::Variable::Parameter::Self>& t_self)
        {
            return t_self->GetOrCreateLowered({});
        });

        const auto mchLoweredParameters = TransformMaybeChangedVector(m_Parameters,
        [](const std::shared_ptr<const BoundNode::Variable::Parameter::Normal>& t_parameter)
        {
            return t_parameter->GetOrCreateLowered({});
        });

        const auto mchLoweredOptBody = TransformMaybeChangedOptional(m_OptBody,
        [](const std::shared_ptr<const BoundNode::Statement::Block>& t_body)
        {
            return t_body->GetOrCreateLowered({});
        });

        if (
            !mchLoweredAttributes.IsChanged && 
            !mchLoweredOptSelf.IsChanged &&
            !mchLoweredParameters.IsChanged && 
            !mchLoweredOptBody.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Function>(
            m_Symbol,
            mchLoweredAttributes.Value,
            mchLoweredOptSelf.Value,
            mchLoweredParameters.Value,
            mchLoweredOptBody.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
