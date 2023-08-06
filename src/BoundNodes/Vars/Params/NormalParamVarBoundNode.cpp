#include "BoundNodes/Vars/Params/NormalParamVarBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/TypeCheckingDiagnostics.hpp"
#include "Symbols/Vars/Params/NormalParamVarSymbol.hpp"

namespace Ace
{
    NormalParamVarBoundNode::NormalParamVarBoundNode(
        const SrcLocation& srcLocation,
        NormalParamVarSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes }
    {
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

    auto NormalParamVarBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const NormalParamVarBoundNode>>
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
            std::make_shared<const NormalParamVarBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedAttributes
            ),
            diagnostics,
        };
    }

    auto NormalParamVarBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const NormalParamVarBoundNode>
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

        return std::make_shared<const NormalParamVarBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredAttributes
        )->CreateLowered({});
    }

    auto NormalParamVarBoundNode::GetSymbol() const -> NormalParamVarSymbol*
    {
        return m_Symbol;
    }
}
