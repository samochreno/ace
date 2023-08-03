#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    SelfParamVarBoundNode::SelfParamVarBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        SelfParamVarSymbol* const symbol
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol }
    {
    }

    auto SelfParamVarBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto SelfParamVarBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto SelfParamVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto SelfParamVarBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto SelfParamVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const SelfParamVarBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        return CreateUnchanged(shared_from_this());
    }

    auto SelfParamVarBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const SelfParamVarBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SelfParamVarBoundNode::GetSymbol() const -> SelfParamVarSymbol*
    {
        return m_Symbol;
    }
}
