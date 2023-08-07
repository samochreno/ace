#include "BoundNodes/Vars/StaticVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/AttributeBoundNode.hpp"
#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/Vars/StaticVarSymbol.hpp"

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

    auto StaticVarBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Attributes);

        return children;
    }

    auto StaticVarBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StaticVarBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const AttributeBoundNode>> checkedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(checkedAttributes),
            [&](const std::shared_ptr<const AttributeBoundNode>& attribute)
            {
                return diagnostics.Collect(attribute->CreateTypeChecked({}));
            }
        );

        if (checkedAttributes == m_Attributes)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const StaticVarBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedAttributes
            ),
            diagnostics,
        };
    }

    auto StaticVarBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StaticVarBoundNode>
    {
        std::vector<std::shared_ptr<const AttributeBoundNode>> loweredAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(loweredAttributes),
            [](const std::shared_ptr<const AttributeBoundNode>& attribute)
            {
                return attribute->CreateLowered({});
            }
        );

        if (loweredAttributes == m_Attributes)
        {
            return shared_from_this();
        }

        return std::make_shared<const StaticVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredAttributes
        )->CreateLowered({});
    }

    auto StaticVarBoundNode::GetSymbol() const -> StaticVarSymbol*
    {
        return m_Symbol;
    }
}
