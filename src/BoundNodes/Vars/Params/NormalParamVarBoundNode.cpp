#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    ParamVarBoundNode::ParamVarBoundNode(
        const SrcLocation& srcLocation,
        NormalParamVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
    }

    auto ParamVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ParamVarBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(mchCheckedAttributes, TransformExpectedMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateTypeChecked({});
        }));

        if (!mchCheckedAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ParamVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchCheckedAttributes.Value
        ));
    }

    auto ParamVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ParamVarBoundNode>>
    {
        const auto mchLoweredAttributes = TransformMaybeChangedVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateLowered({});
        });

        if (!mchLoweredAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ParamVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto ParamVarBoundNode::GetSymbol() const -> NormalParamVarSymbol*
    {
        return m_Symbol;
    }
}
