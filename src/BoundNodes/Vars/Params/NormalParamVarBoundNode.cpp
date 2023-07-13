#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    ParamVarBoundNode::ParamVarBoundNode(
        NormalParamVarSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& t_attributes
    ) : m_Symbol{ t_symbol },
        m_Attributes{ t_attributes }
    {
    }

    auto ParamVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto ParamVarBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto ParamVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ParamVarBoundNode>>>
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

        return CreateChanged(std::make_shared<const ParamVarBoundNode>(
            m_Symbol,
            mchCheckedAttributes.Value
        ));
    }

    auto ParamVarBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ParamVarBoundNode>>
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

        return CreateChanged(std::make_shared<const ParamVarBoundNode>(
            m_Symbol,
            mchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto ParamVarBoundNode::GetSymbol() const -> NormalParamVarSymbol*
    {
        return m_Symbol;
    }
}
