#include "BoundNode/Module.hpp"

#include <memory>
#include <vector>

#include "BoundNode/Type/Base.hpp"
#include "BoundNode/Impl.hpp"
#include "BoundNode/Function.hpp"
#include "BoundNode/Variable/Normal/Static.hpp"
#include "Error.hpp"
#include "MaybeChanged.hpp"

namespace Ace::BoundNode
{
    auto Module::GetChildren() const -> std::vector<const BoundNode::IBase*>
    {
        std::vector<const BoundNode::IBase*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_Impls);
        AddChildren(children, m_Functions);
        AddChildren(children, m_Variables);

        return children;
    }

    auto Module::GetOrCreateTypeChecked(const BoundNode::Context::TypeChecking& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Module>>>
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

        ACE_TRY(mchCheckedVariables, TransformExpectedMaybeChangedVector(m_Variables,
        [](const std::shared_ptr<const BoundNode::Variable::Normal::Static>& t_variable)
        {
            return t_variable->GetOrCreateTypeChecked({});
        }));

        if (
            !mchCheckedModules.IsChanged &&
            !mchCheckedTypes.IsChanged &&
            !mchCheckedImpls.IsChanged &&
            !mchCheckedFunctions.IsChanged && 
            !mchCheckedVariables.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Module>(
            m_Symbol,
            mchCheckedModules.Value,
            mchCheckedTypes.Value,
            mchCheckedImpls.Value,
            mchCheckedFunctions.Value,
            mchCheckedVariables.Value
        );

        return CreateChanged(returnValue);
    }

    auto Module::GetOrCreateLowered(const BoundNode::Context::Lowering& t_context) const -> Expected<MaybeChanged<std::shared_ptr<const BoundNode::Module>>>
    {
        ACE_TRY(mchLoweredModules, TransformExpectedMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const BoundNode::Module>& t_module)
        {
            return t_module->GetOrCreateLowered({});
        }));

        ACE_TRY(mchLoweredTypes, TransformExpectedMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const BoundNode::Type::IBase>& t_type)
        {
            return t_type->GetOrCreateLoweredType({});
        }));

        ACE_TRY(mchLoweredImpls, TransformExpectedMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const BoundNode::Impl>& t_impl)
        {
            return t_impl->GetOrCreateLowered({});
        }));

        ACE_TRY(mchLoweredFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const BoundNode::Function>& t_function)
        {
            return t_function->GetOrCreateLowered({});
        }));

        ACE_TRY(mchLoweredVariables, TransformExpectedMaybeChangedVector(m_Variables,
        [](const std::shared_ptr<const BoundNode::Variable::Normal::Static>& t_variable)
        {
            return t_variable->GetOrCreateLowered({});
        }));

        if (
            !mchLoweredModules.IsChanged &&
            !mchLoweredTypes.IsChanged &&
            !mchLoweredImpls.IsChanged && 
            !mchLoweredFunctions.IsChanged && 
            !mchLoweredVariables.IsChanged
            )
            return CreateUnchanged(shared_from_this());

        const auto returnValue = std::make_shared<const BoundNode::Module>(
            m_Symbol,
            mchLoweredModules.Value,
            mchLoweredTypes.Value,
            mchLoweredImpls.Value,
            mchLoweredFunctions.Value,
            mchLoweredVariables.Value
        );

        return CreateChangedLoweredReturn(returnValue->GetOrCreateLowered({}));
    }
}
