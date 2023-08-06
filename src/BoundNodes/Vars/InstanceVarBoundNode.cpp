#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/Vars/InstanceVarSymbol.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    InstanceVarBoundNode::InstanceVarBoundNode(
        const SrcLocation& srcLocation,
        InstanceVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
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

    auto InstanceVarBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const InstanceVarBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const AttributeBoundNode>> checkedAttributes{};
        std::transform(
            begin(m_Attributes),
            end  (m_Attributes),
            back_inserter(checkedAttributes),
            [&](const std::shared_ptr<const AttributeBoundNode>& attribute)
            {
                const auto dgnCheckedAttribute =
                    attribute->CreateTypeChecked({});
                diagnostics.Add(dgnCheckedAttribute);
                return dgnCheckedAttribute.Unwrap();
            }
        );

        if (checkedAttributes == m_Attributes)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const InstanceVarBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedAttributes
            ),
            diagnostics,
        };
    }

    auto InstanceVarBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const InstanceVarBoundNode>
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

        return std::make_shared<const InstanceVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredAttributes
        )->CreateLowered({});
    }

    auto InstanceVarBoundNode::GetSymbol() const -> InstanceVarSymbol*
    {
        return m_Symbol;
    }
}
