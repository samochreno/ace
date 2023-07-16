#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "SourceLocation.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    SelfParamVarBoundNode::SelfParamVarBoundNode(
        const SourceLocation& t_sourceLocation,
        SelfParamVarSymbol* const t_symbol
    ) : m_SourceLocation{ t_sourceLocation },
        m_Symbol{ t_symbol }
    {
    }

    auto SelfParamVarBoundNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto SelfParamVarBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto SelfParamVarBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        return {};
    }

    auto SelfParamVarBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const SelfParamVarBoundNode>>>
    {
        ACE_TRY(sizeKind, m_Symbol->GetType()->GetSizeKind());
        ACE_TRY_ASSERT(sizeKind == TypeSizeKind::Sized);

        return CreateUnchanged(shared_from_this());
    }

    auto SelfParamVarBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const SelfParamVarBoundNode>>
    {
        return CreateUnchanged(shared_from_this());
    }

    auto SelfParamVarBoundNode::GetSymbol() const -> SelfParamVarSymbol*
    {
        return m_Symbol;
    }
}
