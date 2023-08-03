#include "BoundNodes/ModuleBoundNode.hpp"

#include <memory>
#include <vector>

#include "Diagnostic.hpp"
#include "SrcLocation.hpp"
#include "Symbols/ModuleSymbol.hpp"
#include "BoundNodes/Types/TypeBoundNode.hpp"
#include "BoundNodes/ImplBoundNode.hpp"
#include "BoundNodes/FunctionBoundNode.hpp"
#include "BoundNodes/Vars/StaticVarBoundNode.hpp"
#include "Scope.hpp"
#include "Cacheable.hpp"

namespace Ace
{
    ModuleBoundNode::ModuleBoundNode(
        const DiagnosticBag& diagnostics,
        const SrcLocation& srcLocation,
        ModuleSymbol* const symbol,
        const std::vector<std::shared_ptr<const ModuleBoundNode>>& modules,
        const std::vector<std::shared_ptr<const ITypeBoundNode>>& types,
        const std::vector<std::shared_ptr<const ImplBoundNode>>& impls,
        const std::vector<std::shared_ptr<const FunctionBoundNode>>& functions,
        const std::vector<std::shared_ptr<const StaticVarBoundNode>>& vars
    ) : m_Diagnostics{ diagnostics },
        m_SrcLocation{ srcLocation },
        m_Symbol{ symbol },
        m_Modules{ modules },
        m_Types{ types },
        m_Impls{ impls },
        m_Functions{ functions },
        m_Vars{ vars }
    {
    }

    auto ModuleBoundNode::GetDiagnostics() const -> const DiagnosticBag&
    {
        return m_Diagnostics;
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

    auto ModuleBoundNode::CloneWithDiagnostics(
        DiagnosticBag diagnostics
    ) const -> std::shared_ptr<const ModuleBoundNode>
    {
        if (diagnostics.IsEmpty())
        {
            return shared_from_this();
        }

        return std::make_shared<const ModuleBoundNode>(
            diagnostics.Add(GetDiagnostics()),
            GetSrcLocation(),
            m_Symbol,
            m_Modules,
            m_Types,
            m_Impls,
            m_Functions,
            m_Vars
        );
    }

    auto ModuleBoundNode::GetOrCreateTypeChecked(
        const TypeCheckingContext& context
    ) const -> Expected<Cacheable<std::shared_ptr<const ModuleBoundNode>>>
    {
        ACE_TRY(cchCheckedModules, TransformExpectedCacheableVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& module)
        {
            return module->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedTypes, TransformExpectedCacheableVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& type)
        {
            return type->GetOrCreateTypeCheckedType({});
        }));

        ACE_TRY(cchCheckedImpls, TransformExpectedCacheableVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& impl)
        {
            return impl->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedFunctions, TransformExpectedCacheableVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateTypeChecked({});
        }));

        ACE_TRY(cchCheckedVars, TransformExpectedCacheableVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return var->GetOrCreateTypeChecked({});
        }));

        if (
            !cchCheckedModules.IsChanged &&
            !cchCheckedTypes.IsChanged &&
            !cchCheckedImpls.IsChanged &&
            !cchCheckedFunctions.IsChanged && 
            !cchCheckedVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ModuleBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchCheckedModules.Value,
            cchCheckedTypes.Value,
            cchCheckedImpls.Value,
            cchCheckedFunctions.Value,
            cchCheckedVars.Value
        ));
    }

    auto ModuleBoundNode::GetOrCreateLowered(
        const LoweringContext& context
    ) const -> Cacheable<std::shared_ptr<const ModuleBoundNode>>
    {
        const auto cchLoweredModules = TransformCacheableVector(m_Modules,
        [](const std::shared_ptr<const ModuleBoundNode>& module)
        {
            return module->GetOrCreateLowered({});
        });

        const auto cchLoweredTypes = TransformCacheableVector(m_Types,
        [](const std::shared_ptr<const ITypeBoundNode>& type)
        {
            return type->GetOrCreateLoweredType({});
        });

        const auto cchLoweredImpls = TransformCacheableVector(m_Impls,
        [](const std::shared_ptr<const ImplBoundNode>& impl)
        {
            return impl->GetOrCreateLowered({});
        });

        const auto cchLoweredFunctions = TransformCacheableVector(m_Functions,
        [](const std::shared_ptr<const FunctionBoundNode>& function)
        {
            return function->GetOrCreateLowered({});
        });

        const auto cchLoweredVars = TransformCacheableVector(m_Vars,
        [](const std::shared_ptr<const StaticVarBoundNode>& var)
        {
            return var->GetOrCreateLowered({});
        });

        if (
            !cchLoweredModules.IsChanged &&
            !cchLoweredTypes.IsChanged &&
            !cchLoweredImpls.IsChanged && 
            !cchLoweredFunctions.IsChanged && 
            !cchLoweredVars.IsChanged
            )
        {
            return CreateUnchanged(shared_from_this());
        }

        return CreateChanged(std::make_shared<const ModuleBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            m_Symbol,
            cchLoweredModules.Value,
            cchLoweredTypes.Value,
            cchLoweredImpls.Value,
            cchLoweredFunctions.Value,
            cchLoweredVars.Value
        )->GetOrCreateLowered({}).Value);
    }
}
