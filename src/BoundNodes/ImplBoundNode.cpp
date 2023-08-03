#include "BoundNodes/ImplBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    ImplBoundNode::ImplBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Functions{ functions }
    {
    }

    auto ImplBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto ImplBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ImplBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ImplBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Functions);

        return children;
    }

    auto ImplBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ImplBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ImplBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetScope(),
            m_Functions
        );
    }

    auto ImplBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ImplBoundNode>>>
    {
        ACE_TRY(cchCheckedFunctions, TransformExpectedCacheableVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateTypeChecked({});
        }));

        if (!cchCheckedFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ImplBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            cchCheckedFunctions.Value
        ));
    }

    auto ImplBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ImplBoundNode>>
    {
        const auto cchLoweredFunctions = TransformCacheableVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateLowered({});
        });

        if (!cchLoweredFunctions.IsChanged)
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ImplBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            GetScope(),
            cchLoweredFunctions.Value
        )->GetOrCreateLowered({}).Value);
    }
}
