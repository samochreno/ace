#include "BoundNodes/Vars/StaticVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/AttributeBoundNode.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    StaticVarBoundNode::StaticVarBoundNode(
        const SrcLocation& srcLocation,
        StaticVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
    }

    auto StaticVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto StaticVarBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto StaticVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const StaticVarBoundNode>>>
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

        return CreateChanged(std::make_shared<const StaticVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchCheckedAttributes.Value
        ));
    }

    auto StaticVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const StaticVarBoundNode>>
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

        return CreateChanged(std::make_shared<const StaticVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            mchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StaticVarBoundNode::GetSymbol() const -> StaticVarSymbol*
    {
        return m_Symbol;
    }
}
