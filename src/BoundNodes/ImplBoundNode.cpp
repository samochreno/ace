#include "BoundNodes/ImplBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ImplBoundNode::ImplBoundNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_Functions{ functions }
    {
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

    auto ImplBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ImplBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const FunctionBoundNode>> checkedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(checkedFunctions),
            [&](const std::shared_ptr<const FunctionBoundNode>& function)
            {
                return diagnostics.Collect(function->CreateTypeChecked({}));
            }
        );

        if (checkedFunctions == m_Functions)
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const ImplBoundNode>(
                GetSrcLocation(),
                GetScope(),
                checkedFunctions
            ),
            diagnostics,
        };
    }

    auto ImplBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ImplBoundNode>
    {
        std::vector<std::shared_ptr<const FunctionBoundNode>> loweredFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(loweredFunctions),
            [](const std::shared_ptr<const FunctionBoundNode>& function)
            {
                return function->CreateLowered({});
            }
        );

        if (loweredFunctions == m_Functions)
        {
            return shared_from_this();
        }

        return std::make_shared<const ImplBoundNode>(
            GetSrcLocation(),
            GetScope(),
            loweredFunctions
        )->CreateLowered({});
    }
}
