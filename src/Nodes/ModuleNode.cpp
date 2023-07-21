#include "Nodes/ModuleNode.hpp"

#include <memory>
#include <vector>
#include <string>

#include "SourceLocation.hpp"
#include "Scope.hpp"
#include "Identifier.hpp"
#include "AccessModifier.hpp"
#include "Nodes/Types/TypeNode.hpp"
#include "Nodes/Templates/TypeTemplateNode.hpp"
#include "Nodes/ImplNode.hpp"
#include "Nodes/TemplatedImplNode.hpp"
#include "Nodes/FunctionNode.hpp"
#include "Nodes/Templates/FunctionTemplateNode.hpp"
#include "Nodes/Vars/StaticVarNode.hpp"
#include "Diagnostic.hpp"
#include "BoundNodes/ModuleBoundNode.hpp"
#include "Symbols/Symbol.hpp"
#include "Symbols/ModuleSymbol.hpp"

namespace Ace
{
    ModuleNode::ModuleNode(
        const SourceLocation& sourceLocation,
        const std::shared_ptr<Scope>& scope,
        const std::shared_ptr<Scope>& selfScope,
        const std::vector<Identifier>& name,
        const AccessModifier accessModifier,
        const std::vector<std::shared_ptr<const ModuleNode>>& modules,
        const std::vector<std::shared_ptr<const ITypeNode>>& types,
        const std::vector<std::shared_ptr<const TypeTemplateNode>>& typeTemplates,
        const std::vector<std::shared_ptr<const ImplNode>>& impls,
        const std::vector<std::shared_ptr<const TemplatedImplNode>>& templatedImpls,
        const std::vector<std::shared_ptr<const FunctionNode>>& functions,
        const std::vector<std::shared_ptr<const FunctionTemplateNode>>& functionTemplates,
        const std::vector<std::shared_ptr<const StaticVarNode>>& vars
    ) : m_SourceLocation{ sourceLocation },
        m_Scope{ scope },
        m_SelfScope{ selfScope },
        m_Name{ name },
        m_AccessModifier{ accessModifier },
        m_Modules{ modules },
        m_Types{ types },
        m_TypeTemplates{ typeTemplates },
        m_Impls{ impls },
        m_TemplatedImpls{ templatedImpls },
        m_Functions{ functions },
        m_FunctionTemplates{ functionTemplates },
        m_Vars{ vars }
    {
    }

    auto ModuleNode::GetSourceLocation() const -> const SourceLocation&
    {
        return m_SourceLocation;
    }

    auto ModuleNode::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto ModuleNode::GetChildren() const -> std::vector<const INode*> 
    {
        std::vector<const INode*> children{};

        AddChildren(children, m_Modules);
        AddChildren(children, m_Types);
        AddChildren(children, m_TypeTemplates);
        AddChildren(children, m_Impls);
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

        std::vector<std::shared_ptr<const ImplNode>> clonedImpls{};
        std::transform(
            begin(m_Impls),
            end  (m_Impls),
            back_inserter(clonedImpls),
            [&](const std::shared_ptr<const ImplNode>& impl)
            {
                return impl->CloneInScope(selfScope);
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
            m_SourceLocation,
            scope,
            selfScope,
            m_Name,
            m_AccessModifier,
            clonedModules,
            clonedTypes,
            clonedTypeTemplates,
            clonedImpls,
            clonedTemplatedImpls,
            clonedFunctions,
            clonedFunctionTemplates,
            clonedVars
        );
    }

    auto ModuleNode::CreateBound() const -> Expected<std::shared_ptr<const ModuleBoundNode>>
    {
        ACE_TRY(boundModules, TransformExpectedVector(m_Modules,
        [](const std::shared_ptr<const ModuleNode>& module)
        {
            return module->CreateBound();
        }));

        ACE_TRY(boundTypes, TransformExpectedVector(m_Types,
        [](const std::shared_ptr<const ITypeNode>& type)
        {
            return type->CreateBoundType();
        }));

        ACE_TRY(boundImpls, TransformExpectedVector(m_Impls,
        [](const std::shared_ptr<const ImplNode>& impl)
        {
            return impl->CreateBound();
        }));

        ACE_TRY(boundTemplateImpls, TransformExpectedVector(m_TemplatedImpls,
        [](const std::shared_ptr<const TemplatedImplNode>& templatedImpl)
        {
            return templatedImpl->CreateBound();
        }));

        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const FunctionNode>& function)
        {
            return function->CreateBound();
        }));

        ACE_TRY(boundVars, TransformExpectedVector(m_Vars,
        [](const std::shared_ptr<const StaticVarNode>& var)
        {
            return var->CreateBound();
        }));

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
            begin(boundTemplateImpls),
            end  (boundTemplateImpls)
        );

        return std::make_shared<const ModuleBoundNode>(
            GetSourceLocation(),
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

    auto ModuleNode::CreateSymbol() const -> Expected<std::unique_ptr<ISymbol>>
    {
        if (m_Name.size() > 1)
        {
            std::shared_ptr<Scope> scope =
                GetSymbolScope()->GetParent().value();

            auto nameIt = rbegin(m_Name) + 1;

            for (
                ; 
                nameIt != rend(m_Name); 
                [&](){ scope = scope->GetParent().value(); nameIt++; }()
                )
            {
                ACE_TRY(symbol, scope->ExclusiveResolveSymbol<ModuleSymbol>(
                    *nameIt
                ));
            }
        }

        return std::unique_ptr<ISymbol>
        {
            std::make_unique<ModuleSymbol>(
                m_SelfScope,
                GetName(),
                m_AccessModifier
            )
        };
    }

    auto ModuleNode::ContinueCreatingSymbol(
        ISymbol* const symbol
    ) const -> Expected<void>
    {
        auto* const moduleSymbol = dynamic_cast<ModuleSymbol*>(symbol);
        ACE_ASSERT(moduleSymbol);

        ACE_TRY_ASSERT(moduleSymbol->GetAccessModifier() == m_AccessModifier);

        return Void{}; 
    }

    auto ModuleNode::GetName() const -> const Identifier&
    {
        return m_Name.back();
    }
}
