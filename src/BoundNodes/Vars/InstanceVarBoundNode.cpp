#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    InstanceVarBoundNode::InstanceVarBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        InstanceVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
    }

    auto InstanceVarBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto InstanceVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto InstanceVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto InstanceVarBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto InstanceVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const InstanceVarBoundNode>>>
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

        return CreateChanged(std::make_shared<const InstanceVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedAttributes.Value
        ));
    }

    auto InstanceVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const InstanceVarBoundNode>>
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

        return CreateChanged(std::make_shared<const InstanceVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto InstanceVarBoundNode::GetSymbol() const -> InstanceVarSymbol*
    {
        return m_Symbol;
    }
}
