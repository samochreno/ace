#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    NormalParamVarBoundNode::NormalParamVarBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        NormalParamVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
    }

    auto NormalParamVarBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto NormalParamVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto NormalParamVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto NormalParamVarBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto NormalParamVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const NormalParamVarBoundNode>>>
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

        return CreateChanged(std::make_shared<const NormalParamVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedAttributes.Value
        ));
    }

    auto NormalParamVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const NormalParamVarBoundNode>>
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

        return CreateChanged(std::make_shared<const NormalParamVarBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredAttributes.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto NormalParamVarBoundNode::GetSymbol() const -> NormalParamVarSymbol*
    {
        return m_Symbol;
    }
}
