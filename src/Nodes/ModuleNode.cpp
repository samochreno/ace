#include "Nodes/ModuleNode.hpp"

#include <memory>
#include <vector>
#include <string>

#include "SrcLocation.hpp"
#include "Scope.hpp"
#include "Ident.hpp"
#include "AccessModifier.hpp"
#include "Nodes/Types/TypeNode.hpp"
#include "Nodes/Templates/TypeTemplateNode.hpp"
#include "Nodes/Impls/NormalImplNode.hpp"
#include "Nodes/Impls/TemplatedImplNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/Vars/StaticVarNode.hpp"
#include "Diagnostic.hpp"
#include "Diagnostics/BindingDiagnostics.hpp"
#include "BoundNodes/ModuleBoundNode.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/ModuleSymbol.hpp"

namespace Ace
{
    ModuleNode::ModuleNode(
        const SrcLocation& srcLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& selfScope,
        const std::vector<Ident>& name,
        const AccessModifier accessModifier,
        const std::vector<std::shared_ptr<const ModuleNode>>& modules,
        const std::vector<std::shared_ptr<const ITypeNode>>& types,
        const std::vector<std::shared_ptr<const TypeTemplateNode>>& typeTemplates,
        const std::vector<std::shared_ptr<const NormalImplNode>>& normalImpls,
        const std::vector<std::shared_ptr<const TemplatedImplNode>>& templatedImpls,
        const std::vector<std::shared_ptr<const FunctionNode>>& functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates,
        const std::vector<std::shared_ptr<const StaticVarNode>>& vars
    ) : m_SrcLocation{ srcLocation },
        m_Scope{ scope },
        m_SelfScope{ selfScope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Modules{ modules },
        m_Types{ types },
        m_TypeTemplates{ typeTemplates },
        m_NormalImpls{ normalImpls },
        m_TemplatedImpls{ templatedImpls },
        m_Functions{ functions },
        m_FunctionTemplates{ functionTemplates },
        m_Vars{ vars }
    {
    }

    auto ModuleNode::GetSrcLocation() const -> const SrcLocation&
    {
        return m_SrcLocation;
    }

    auto ModuleNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ModuleNode::CollectChildren() const -> std::vector<const INode*> 
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_TypeTemplates);
        AddChildren(children, m_NormalImpls);
        AddChildren(children, m_TemplatedImpls);
        AddChildren(children, m_Functions);
        AddChildren(children, m_FunctionTemplates);
        AddChildren(children, m_Vars);

        return children;
    }

    auto ModuleNode::CloneInScope(
        const std::shared_ptr<Scope>& scope
    ) const -> std::shared_ptr<const ModuleNode>
    {
        const auto selfScope = scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const ModuleNode>> clonedModules{};
        std::transform(
            begin(m_Modules),
            end  (m_Modules),
            back_inserter(clonedModules),
            [&](const std::shared_ptr<const ModuleNode>& module)
            {
                return module->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const ITypeNode>> clonedTypes{};
        std::transform(
            begin(m_Types),
            end  (m_Types),
            back_inserter(clonedTypes),
            [&](const std::shared_ptr<const ITypeNode>& type)
            {
                return type->CloneInScopeType(selfScope);
            }
        );

        std::vector<std::shared_ptr<const TypeTemplateNode>> clonedTypeTemplates{};
        std::transform(
            begin(m_TypeTemplates),
            end  (m_TypeTemplates),
            back_inserter(clonedTypeTemplates),
            [&](const std::shared_ptr<const TypeTemplateNode>& typeTemplate)
            {
                return typeTemplate->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const NormalImplNode>> clonedNormalImpls{};
        std::transform(
            begin(m_NormalImpls),
            end  (m_NormalImpls),
            back_inserter(clonedNormalImpls),
            [&](const std::shared_ptr<const NormalImplNode>& normalImpl)
            {
                return normalImpl->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const TemplatedImplNode>> clonedTemplatedImpls{};
        std::transform(
            begin(m_TemplatedImpls),
            end  (m_TemplatedImpls),
            back_inserter(clonedTemplatedImpls),
            [&](const std::shared_ptr<const TemplatedImplNode>& templatedImpl)
            {
                return templatedImpl->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const FunctionNode>> clonedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(clonedFunctions),
            [&](const std::shared_ptr<const FunctionNode>& function)
            {
                return function->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const FunctionTemplateNode>> clonedFunctionTemplates{};
        std::transform(
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates),
            back_inserter(clonedFunctionTemplates),
            [&](const std::shared_ptr<const FunctionTemplateNode>& functionTemplate)
            {
                return functionTemplate->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const StaticVarNode>> clonedVars{};
        std::transform(
            begin(m_Vars),
            end  (m_Vars),
            back_inserter(clonedVars),
            [&](const std::shared_ptr<const StaticVarNode>& var)
            {
                return var->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const ModuleNode>(
            m_SrcLocation,
            scope,
            selfScope,
            m_Name,
            m_AccessModifier,
            clonedModules,
            clonedTypes,
            clonedTypeTemplates,
            clonedNormalImpls,
            clonedTemplatedImpls,
            clonedFunctions,
            clonedFunctionTemplates,
            clonedVars
        );
    }

    auto ModuleNode::CreateBound() const -> std::shared_ptr<const ModuleBoundNode>
    {
        std::vector<std::shared_ptr<const ModuleBoundNode>> boundModules{};
        std::transform(
            begin(m_Modules),
            end  (m_Modules),
            back_inserter(boundModules),
            [&](const std::shared_ptr<const ModuleNode>& module)
            {
                return module->CreateBound();
            }
        );

        std::vector<std::shared_ptr<const ITypeBoundNode>> boundTypes{};
        std::transform(begin(m_Types), end(m_Types), back_inserter(boundTypes),
        [&](const std::shared_ptr<const ITypeNode>& type)
        {
            return type->CreateBoundType();
        });

        std::vector<std::shared_ptr<const ImplBoundNode>> boundImpls{};
        std::transform(
            begin(m_NormalImpls),
            end  (m_NormalImpls),
            back_inserter(boundImpls),
            [&](const std::shared_ptr<const NormalImplNode>& normalImpl)
            {
                return normalImpl->CreateBound();
            }
        );

        std::vector<std::shared_ptr<const ImplBoundNode>> boundTemplatedImpls{};
        std::transform(
            begin(m_TemplatedImpls),
            end  (m_TemplatedImpls),
            back_inserter(boundTemplatedImpls),
            [&](const std::shared_ptr<const TemplatedImplNode>& templatedImpl)
            {
                return templatedImpl->CreateBound();
            }
        );

        std::vector<std::shared_ptr<const FunctionBoundNode>> boundFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(boundFunctions),
            [&](const std::shared_ptr<const FunctionNode>& function)
            {
                return function->CreateBound();
            }
        );

        std::vector<std::shared_ptr<const StaticVarBoundNode>> boundVars{};
        std::transform(
            begin(m_Vars),
            end  (m_Vars),
            back_inserter(boundVars),
            [&](const std::shared_ptr<const StaticVarNode>& var)
            {
                return var->CreateBound();
            }
        );

        auto* const selfSymbol = GetSymbolScope()->ExclusiveResolveSymbol<ModuleSymbol>(
            GetName()
        ).Unwrap();

        std::vector<std::shared_ptr<const ImplBoundNode>> allBoundImpls{};
        allBoundImpls.insert(
            end(allBoundImpls),
            begin(boundImpls),
            end  (boundImpls)
        );
        allBoundImpls.insert(
            end(allBoundImpls),
            begin(boundTemplatedImpls),
            end  (boundTemplatedImpls)
        );

        return std::make_shared<const ModuleBoundNode>(
            DiagnosticBag{},
            GetSrcLocation(),
            selfSymbol,
            boundModules,
            boundTypes,
            allBoundImpls,
            boundFunctions,
            boundVars
        );
    }

    auto ModuleNode::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto ModuleNode::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Module;
    }

    auto ModuleNode::GetSymbolCreationSuborder() const -> size_t
    {
        return m_SelfScope->GetNestLevel();
    }

    auto ModuleNode::CreateSymbol() const -> Diagnosed<std::unique_ptr<ISymbol>>
    {
        return Diagnosed<std::unique_ptr<ISymbol>>
        {
            std::make_unique<ModuleSymbol>(
                m_SelfScope,
                GetName(),
                m_AccessModifier
            ),
            DiagnosticBag{},
        };
    }

    auto ModuleNode::ContinueCreatingSymbol(
        ISymbol* const symbol
    ) const -> Diagnosed<void>
    {
        DiagnosticBag diagnostics{};

        auto* const moduleSymbol = dynamic_cast<ModuleSymbol*>(symbol);
        ACE_ASSERT(moduleSymbol);

        if (moduleSymbol->GetAccessModifier() != m_AccessModifier)
        {
            const SrcLocation nameSrcLocation
            {
                m_Name.begin()->SrcLocation,
                m_Name.back().SrcLocation,
            };

            diagnostics.Add(CreateMismatchedAccessModifierError(
                moduleSymbol,
                nameSrcLocation,
                m_AccessModifier
            ));
        }

        return Diagnosed<void>{ diagnostics };
    }

    auto ModuleNode::GetName() const -> const Ident&
    {
        return m_Name.back();
    }
}
