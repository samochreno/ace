#include "BoundNode/Module.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Type/Base.hpp"
#include "BoundNode/Impl.hpp"
#include "BoundNode/Function.hpp"
#include "BoundNode/Var/Normal/Static.hpp"
#include "Diagnostics.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    Module::Module(
        Symbol::Module* const t_symbol,
        const std::vector<std::shared_ptr<const BoundNode::Module>>& t_modules,
        const std::vector<std::shared_ptr<const BoundNode::Type::IBase>>& t_types,
        const std::vector<std::shared_ptr<const BoundNode::Impl>>& t_impls,
        const std::vector<std::shared_ptr<const BoundNode::Function>>& t_functions,
        const std::vector<std::shared_ptr<const BoundNode::Var::Normal::Static>>& t_variables
    ) : m_Symbol{ t_symbol },
        m_Modules{ t_modules },
        m_Types{ t_types },
        m_Impls{ t_impls },
        m_Functions{ t_functions },
        m_Vars{ t_variables }
    {
    }

    auto Module::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Symbol->GetScope();
    }

    auto Module::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_Impls);
        AddChildren(children, m_Functions);
        AddChildren(children, m_Vars);

        return children;
    }

    auto Module::GetOrCreateTypeChecked(
        const BoundNode::Context::TypeChecking& t_context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Module>>>
    {
        ACE_TRY(mchCheckedModules, TransformExpectedMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const BoundNode::Module>& t_module)
        {
            return t_module->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedTypes, TransformExpectedMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const BoundNode::Type::IBase>& t_type)
        {
            return t_type->GetOrCreateTypeCheckedType({});
        }));

        ACE_TRY(mchCheckedImpls, TransformExpectedMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const BoundNode::Impl>& t_impl)
        {
            return t_impl->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const BoundNode::Function>& t_function)
        {
            return t_function->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedVars, TransformExpectedMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const BoundNode::Var::Normal::Static>& t_variable)
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

        const auto returnValue = std::make_shared<const BoundNode::Module>(
            m_Symbol,
            mchCheckedModules.Value,
            mchCheckedTypes.Value,
            mchCheckedImpls.Value,
            mchCheckedFunctions.Value,
            mchCheckedVars.Value
        );
        return CreateChanged(returnValue);
    }

    auto Module::GetOrCreateLowered(
        const BoundNode::Context::Lowering& t_context
    ) const -> MaybeChanged<std::shared_ptr<const BoundNode::Module>>
    {
        const auto mchLoweredModules = TransformMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const BoundNode::Module>& t_module)
        {
            return t_module->GetOrCreateLowered({});
        });

        const auto mchLoweredTypes = TransformMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const BoundNode::Type::IBase>& t_type)
        {
            return t_type->GetOrCreateLoweredType({});
        });

        const auto mchLoweredImpls = TransformMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const BoundNode::Impl>& t_impl)
        {
            return t_impl->GetOrCreateLowered({});
        });

        const auto mchLoweredFunctions = TransformMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const BoundNode::Function>& t_function)
        {
            return t_function->GetOrCreateLowered({});
        });

        const auto mchLoweredVars = TransformMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const BoundNode::Var::Normal::Static>& t_variable)
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

        const auto returnValue = std::make_shared<const BoundNode::Module>(
            m_Symbol,
            mchLoweredModules.Value,
            mchLoweredTypes.Value,
            mchLoweredImpls.Value,
            mchLoweredFunctions.Value,
            mchLoweredVars.Value
        );
        return CreateChanged(returnValue->GetOrCreateLowered({}).Value);
    }
}
