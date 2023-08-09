#include "BoundNodes/Vars/Params/SelfParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Symbols/Vars/Params/SelfParamVarSymbol.hpp"

namespace Ace
{
    SelfParamVarBoundNode::SelfParamVarBoundNode(
        const SrcLocation& srcLocation,
        SelfParamVarSymbol* const symbol
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol }
    {
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

    auto SelfParamVarBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const SelfParamVarBoundNode>>
    {
        return Diagnosed{ shared_from_this(), DiagnosticBag::Create() };
    }

    auto SelfParamVarBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const SelfParamVarBoundNode>
    {
        return shared_from_this();
    }

    auto SelfParamVarBoundNode::GetSymbol() const -> SelfParamVarSymbol*
    {
        return m_Symbol;
    }
}
