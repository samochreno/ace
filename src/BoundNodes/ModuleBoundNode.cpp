#include "BoundNodes/ModuleBoundNode.hpp"

#include <memory>
#include <vector>

#include "SrcLocation.hpp"
#include "Symbols/ModuleSymbol.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Scope.hpp"
#include "Diagnostic.hpp"

namespace Ace
{
    ModuleBoundNode::ModuleBoundNode(
        const SrcLocation& srcLocation,
        ModuleSymbol* const symbol,
        const std::vector<std::shared_ptr<const ModuleBoundNode>>& modules,
        const std::vector<std::shared_ptr<const ITypeBoundNode>>& types,
        const std::vector<std::shared_ptr<const ImplBoundNode>>& impls,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions,
        const std::vector<std::shared_ptr<const StaticVarBoundNode>>& vars
    ) : m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Modules{ modules },
        m_Types{ types },
        m_Impls{ impls },
        m_Functions{ functions },
        m_Vars{ vars }
    {
    }

    auto ModuleBoundNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ModuleBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto ModuleBoundNode::CollectChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_Impls);
        AddChildren(children, m_Functions);
        AddChildren(children, m_Vars);

        return children;
    }

    auto ModuleBoundNode::CreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Diagnosed<std::shared_ptr<const ModuleBoundNode>>
    {
        DiagnosticBag diagnostics{};

        std::vector<std::shared_ptr<const ModuleBoundNode>> checkedModules{};
        std::transform(
            begin(m_Modules),
            end  (m_Modules),
            back_inserter(checkedModules),
            [&](const std::shared_ptr<const ModuleBoundNode>& module)
            {
                return diagnostics.Collect(module->CreateTypeChecked({}));
            }
        );

        std::vector<std::shared_ptr<const ITypeBoundNode>> checkedTypes{};
        std::transform(
            begin(m_Types),
            end  (m_Types),
            back_inserter(checkedTypes),
            [&](const std::shared_ptr<const ITypeBoundNode>& type)
            {
                return diagnostics.Collect(type->CreateTypeCheckedType({}));
            }
        );

        std::vector<std::shared_ptr<const ImplBoundNode>> checkedImpls{};
        std::transform(
            begin(m_Impls),
            end  (m_Impls),
            back_inserter(checkedImpls),
            [&](const std::shared_ptr<const ImplBoundNode>& impl)
            {
                return diagnostics.Collect(impl->CreateTypeChecked({}));
            }
        );

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

        std::vector<std::shared_ptr<const StaticVarBoundNode>> checkedVars{};
        std::transform(begin(m_Vars), end(m_Vars), back_inserter(checkedVars),
        [&](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return diagnostics.Collect(var->CreateTypeChecked({}));
        });

        if (
            (checkedModules == m_Modules) &&
            (checkedTypes == m_Types) &&
            (checkedImpls == m_Impls) &&
            (checkedFunctions == m_Functions) &&
            (checkedVars == m_Vars)
            )
        {
            return Diagnosed{ shared_from_this(), diagnostics };
        }

        return Diagnosed
        {
            std::make_shared<const ModuleBoundNode>(
                GetSrcLocation(),
                m_Symbol,
                checkedModules,
                checkedTypes,
                checkedImpls,
                checkedFunctions,
                checkedVars
            ),
            diagnostics,
        };
    }

    auto ModuleBoundNode::CreateLowered(
        const LoweringContext& context
    ) const -> std::shared_ptr<const ModuleBoundNode>
    {
        std::vector<std::shared_ptr<const ModuleBoundNode>> loweredModules{};
        std::transform(
            begin(m_Modules),
            end  (m_Modules),
            back_inserter(loweredModules),
            [](const std::shared_ptr<const ModuleBoundNode>& module)
            {
                return module->CreateLowered({});
            }
        );

        std::vector<std::shared_ptr<const ITypeBoundNode>> loweredTypes{};
        std::transform(
            begin(m_Types),
            end  (m_Types),
            back_inserter(loweredTypes),
            [](const std::shared_ptr<const ITypeBoundNode>& type)
            {
                return type->CreateLoweredType({});
            }
        );

        std::vector<std::shared_ptr<const ImplBoundNode>> loweredImpls{};
        std::transform(
            begin(m_Impls),
            end  (m_Impls),
            back_inserter(loweredImpls),
            [](const std::shared_ptr<const ImplBoundNode>& impl)
            {
                return impl->CreateLowered({});
            }
        );

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

        std::vector<std::shared_ptr<const StaticVarBoundNode>> loweredVars{};
        std::transform(begin(m_Vars), end(m_Vars), back_inserter(loweredVars),
        [](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return var->CreateLowered({});
        });

        if (
            (loweredModules == m_Modules) &&
            (loweredTypes == m_Types) &&
            (loweredImpls == m_Impls) &&
            (loweredFunctions == m_Functions) &&
            (loweredVars == m_Vars)
            )
        {
            return shared_from_this();
        }

        return std::make_shared<const ModuleBoundNode>(
            GetSrcLocation(),
            m_Symbol,
            loweredModules,
            loweredTypes,
            loweredImpls,
            loweredFunctions,
            loweredVars
        )->CreateLowered({});
    }
}
