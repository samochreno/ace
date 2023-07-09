#include "BoundNodes/ModuleBoundNode.hpp"

#include <memory>
#include <vector>

#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace
{
    ModuleBoundNode::ModuleBoundNode(
        ModuleSymbol* const t_symbol,
        const std::vector<std::shared_ptr<const ModuleBoundNode>>& t_modules,
        const std::vector<std::shared_ptr<const ITypeBoundNode>>& t_types,
        const std::vector<std::shared_ptr<const ImplBoundNode>>& t_impls,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& t_functions,
        const std::vector<std::shared_ptr<const StaticVarBoundNode>>& t_variables
    ) : m_Symbol{ t_symbol },
        m_Modules{ t_modules },
        m_Types{ t_types },
        m_Impls{ t_impls },
        m_Functions{ t_functions },
        m_Vars{ t_variables }
    {
    }

    auto ModuleBoundNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto ModuleBoundNode::GetChildren() const -> std::vector<const IBoundNode*>
    {
        std::vector<const IBoundNode*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_Impls);
        AddChildren(children, m_Functions);
        AddChildren(children, m_Vars);

        return children;
    }

    auto ModuleBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ModuleBoundNode>>>
    {
        ACE_TRY(mchCheckedModules, TransformExpectedMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& t_module)
        {
            return t_module->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedTypes, TransformExpectedMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& t_type)
        {
            return t_type->GetOrCreateTypeCheckedType({});
        }));

        ACE_TRY(mchCheckedImpls, TransformExpectedMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& t_impl)
        {
            return t_impl->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& t_function)
        {
            return t_function->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedVars, TransformExpectedMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& t_variable)
        {
            return t_variable->GetOrCreateTypeChecked({});
        }));

        if (
            !mchCheckedModules.IsChanged &&
            !mchCheckedTypes.IsChanged &&
            !mchCheckedImpls.IsChanged &&
            !mchCheckedFunctions.IsChanged && 
            !mchCheckedVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ModuleBoundNode>(
            m_Symbol,
            mchCheckedModules.Value,
            mchCheckedTypes.Value,
            mchCheckedImpls.Value,
            mchCheckedFunctions.Value,
            mchCheckedVars.Value
        ));
    }

    auto ModuleBoundNode::GetOrCreateLowered(
        const LoweringContext& t_context
    ) const -> MaybeChanged<std::shared_ptr<const ModuleBoundNode>>
    {
        const auto mchLoweredModules = TransformMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& t_module)
        {
            return t_module->GetOrCreateLowered({});
        });

        const auto mchLoweredTypes = TransformMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& t_type)
        {
            return t_type->GetOrCreateLoweredType({});
        });

        const auto mchLoweredImpls = TransformMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& t_impl)
        {
            return t_impl->GetOrCreateLowered({});
        });

        const auto mchLoweredFunctions = TransformMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& t_function)
        {
            return t_function->GetOrCreateLowered({});
        });

        const auto mchLoweredVars = TransformMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& t_variable)
        {
            return t_variable->GetOrCreateLowered({});
        });

        if (
            !mchLoweredModules.IsChanged &&
            !mchLoweredTypes.IsChanged &&
            !mchLoweredImpls.IsChanged && 
            !mchLoweredFunctions.IsChanged && 
            !mchLoweredVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ModuleBoundNode>(
            m_Symbol,
            mchLoweredModules.Value,
            mchLoweredTypes.Value,
            mchLoweredImpls.Value,
            mchLoweredFunctions.Value,
            mchLoweredVars.Value
        )->GetOrCreateLowered({}).Value);
    }
}
