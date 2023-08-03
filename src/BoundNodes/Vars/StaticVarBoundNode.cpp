#include "BoundNodes/Vars/StaticVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    StaticVarBoundNode::StaticVarBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        StaticVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
    }

    auto StaticVarBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto StaticVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StaticVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto StaticVarBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto StaticVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const StaticVarBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        ACE_TRY(cchCheckedAttributes, TransformExpectedCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateTypeChecked({});
        }));

        if (!cchCheckedAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedAttributes.Value
        ));
    }

    auto StaticVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StaticVarBoundNode>>
    {
        const auto cchLoweredAttributes = TransformCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateLowered({});
        });

        if (!cchLoweredAttributes.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StaticVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StaticVarBoundNode::GetSymbol() const -> StaticVarSymbol*
    {
        return m_Symbol;
    }
}
