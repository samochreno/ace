#include "BoundNodes/Types/StructTypeBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/AttributeBoundNode.hpp"
#include "BoundNodes/Vars/InstanceVarBoundNode.hpp"
#include "Symbols/Types/StructTypeSymbol.hpp"

namespace Ace
{
    StructTypeBoundNode::StructTypeBoundNode(
        const SrcLocation& srcLocation,
        StructTypeSymbol* const symbol,
        const std::vector<std::shared_ptr<const AttributeBoundNode>>& attributes,
        const std::vector<std::shared_ptr<const InstanceVarBoundNode>>& vars
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Attributes{ attributes },
        m_Vars{ vars }
    {
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

    auto StructTypeBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const StructTypeBoundNode>>
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

        std::vector<std::shared_ptr<const InstanceVarBoundNode>> checkedVars{};
        std::transform(begin(m_Vars), end(m_Vars), back_inserter(checkedVars),
        [&](const std::shared_ptr<const InstanceVarBoundNode>& var)
        {
            const auto dgnCheckedVar = var->CreateTypeChecked({});
            diagnostics.Add(dgnCheckedVar);
            return dgnCheckedVar.Unwrap();
        });

        if (
            (checkedAttributes == m_Attributes) &&
            (checkedVars == m_Vars) 
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const StructTypeBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedAttributes,
                checkedVars
            ),
            diagnostics,
        };
    }

    auto StructTypeBoundNode::CreateTypeCheckedType(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ITypeBoundNode>>
    {
        return CreateTypeChecked(context);
    }

    auto StructTypeBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const StructTypeBoundNode>
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

        std::vector<std::shared_ptr<const InstanceVarBoundNode>> loweredVars{};
        std::transform(begin(m_Vars), end(m_Vars), back_inserter(loweredVars),
        [](const std::shared_ptr<const InstanceVarBoundNode>& var)
        {
            return var->CreateLowered({});
        });

        if (
            (loweredAttributes == m_Attributes) &&
            (loweredVars == m_Vars)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const StructTypeBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredAttributes,
            loweredVars
        )->CreateLowered({});
    }

    auto StructTypeBoundNode::CreateLoweredType(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ITypeBoundNode>
    {
        return CreateLowered(context);
    }

    auto StructTypeBoundNode::GetSymbol() const -> StructTypeSymbol*
    {
        return m_Symbol;
    }
}
