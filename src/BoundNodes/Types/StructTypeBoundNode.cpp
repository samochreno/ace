#include "BoundNodes/Types/StructTypeBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    StructTypeBoundNode::StructTypeBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        StructTypeSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
        const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& vars
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes },
        m_Vars{ vars }
    {
    }

    auto StructTypeBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
    }

    auto StructTypeBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto StructTypeBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto StructTypeBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);
        AddChildren(children, m_Vars);

        return children;
    }

    auto StructTypeBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const StructTypeBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const StructTypeBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            GetSymbol(),
            m_Attributes,
            m_Vars
        );
    }

    auto StructTypeBoundNode::CloneWithDiagnosticsType(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ITypeBoundNode>
    {
        return CloneWithDiagnostics(std::move(diagnostics));
    }

    auto StructTypeBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const StructTypeBoundNode>>>
    {
        ACE_TRY(cchCheckedAttributes, TransformExpectedCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedVars, TransformExpectedCacheableVector(m_Vars,
        [](const std::shared_ptr<const InstanceVarBoundNode>& var)
        {
            return var->GetOrCreateTypeChecked({});
        }));

        if (
            !cchCheckedAttributes.IsChanged &&
            !cchCheckedVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructTypeBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedAttributes.Value,
            cchCheckedVars.Value
        ));
    }

    auto StructTypeBoundNode::GetOrCreateTypeCheckedType(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ITypeBoundNode>>>
    {
        return GetOrCreateTypeChecked(context);
    }

    auto StructTypeBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const StructTypeBoundNode>>
    {
        const auto cchLoweredAttributes = TransformCacheableVector(m_Attributes,
        [](const std::shared_ptr<const AttributeBoundNode>& attribute)
        {
            return attribute->GetOrCreateLowered({});
        });

        const auto cchLoweredVars = TransformCacheableVector(m_Vars,
        [](const std::shared_ptr<const InstanceVarBoundNode>& var)
        {
            return var->GetOrCreateLowered({});
        });

        if (
            !cchLoweredAttributes.IsChanged &&
            !cchLoweredVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const StructTypeBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredAttributes.Value,
            cchLoweredVars.Value
        )->GetOrCreateLowered({}).Value);
    }

    auto StructTypeBoundNode::GetOrCreateLoweredType(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ITypeBoundNode>>
    {
        return GetOrCreateLowered(context);
    }

    auto StructTypeBoundNode::GetSymbol() const -> StructTypeSymbol*
    {
        return m_Symbol;
    }
}
