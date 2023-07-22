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
#include "MaybeChanged.hpp"

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
        const TypeCheckingContext& context
    ) const -> Expected<MaybeChanged<std::shared_ptr<const ModuleBoundNode>>>
    {
        ACE_TRY(mchCheckedModules, TransformExpectedMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& module)
        {
            return module->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedTypes, TransformExpectedMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& type)
        {
            return type->GetOrCreateTypeCheckedType({});
        }));

        ACE_TRY(mchCheckedImpls, TransformExpectedMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& impl)
        {
            return impl->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedFunctions, TransformExpectedMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(mchCheckedVars, TransformExpectedMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return var->GetOrCreateTypeChecked({});
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
            GetSrcLocation(),
            m_Symbol,
            mchCheckedModules.Value,
            mchCheckedTypes.Value,
            mchCheckedImpls.Value,
            mchCheckedFunctions.Value,
            mchCheckedVars.Value
        ));
    }

    auto ModuleBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> MaybeChanged<std::shared_ptr<const ModuleBoundNode>>
    {
        const auto mchLoweredModules = TransformMaybeChangedVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& module)
        {
            return module->GetOrCreateLowered({});
        });

        const auto mchLoweredTypes = TransformMaybeChangedVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& type)
        {
            return type->GetOrCreateLoweredType({});
        });

        const auto mchLoweredImpls = TransformMaybeChangedVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& impl)
        {
            return impl->GetOrCreateLowered({});
        });

        const auto mchLoweredFunctions = TransformMaybeChangedVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateLowered({});
        });

        const auto mchLoweredVars = TransformMaybeChangedVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return var->GetOrCreateLowered({});
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
            GetSrcLocation(),
            m_Symbol,
            mchLoweredModules.Value,
            mchLoweredTypes.Value,
            mchLoweredImpls.Value,
            mchLoweredFunctions.Value,
            mchLoweredVars.Value
        )->GetOrCreateLowered({}).Value);
    }
}
