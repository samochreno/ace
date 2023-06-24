#include "Node/Module.hpp"

#include <memory>
#include <vector>
#include <string>

#include "Scope.hpp"
#include "AccessModifier.hpp"
#include "Node/Type/Base.hpp"
#include "Node/Template/Type.hpp"
#include "Node/Impl.hpp"
#include "Node/TemplatedImpl.hpp"
#include "Node/Function.hpp"
#include "Node/Template/Function.hpp"
#include "Node/Var/Normal/Static.hpp"
#include "Diagnostics.hpp"
#include "BoundNode/Module.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Module.hpp"

namespace Ace::Node
{
    Module::Module(
        const std::shared_ptr<Scope>& t_scope,
        const std::shared_ptr<Scope>& t_selfScope,
        const std::vector<std::string>& t_name,
        const AccessModifier& t_accessModifier,
        const std::vector<std::shared_ptr<const Node::Module>>& t_modules,
        const std::vector<std::shared_ptr<const Node::Type::IBase>>& t_types,
        const std::vector<std::shared_ptr<const Node::Template::Type>>& t_typeTemplates,
        const std::vector<std::shared_ptr<const Node::Impl>>& t_impls,
        const std::vector<std::shared_ptr<const Node::TemplatedImpl>>& t_templatedImpls,
        const std::vector<std::shared_ptr<const Node::Function>>& t_functions,
        const std::vector<std::shared_ptr<const Node::Template::Function>>& t_functionTemplates,
        const std::vector<std::shared_ptr<const Node::Var::Normal::Static>>& t_variables
    ) : m_Scope{ t_scope },
        m_SelfScope{ t_selfScope },
        m_Name{ t_name },
        m_AccessModifier{ t_accessModifier },
        m_Modules{ t_modules },
        m_Types{ t_types },
        m_TypeTemplates{ t_typeTemplates },
        m_Impls{ t_impls },
        m_TemplatedImpls{ t_templatedImpls },
        m_Functions{ t_functions },
        m_FunctionTemplates{ t_functionTemplates },
        m_Vars{ t_variables }
    {
    }

    auto Module::GetScope() const -> std::shared_ptr<Scope>
    {
        return m_Scope;
    }

    auto Module::GetChildren() const -> std::vector<const Node::IBase*> 
    {
        std::vector<const Node::IBase*> children{};

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

    auto Module::CloneInScope(
        const std::shared_ptr<Scope>& t_scope
    ) const -> std::shared_ptr<const Node::Module>
    {
        const auto selfScope = t_scope->GetOrCreateChild({});

        std::vector<std::shared_ptr<const Node::Module>> clonedModules{};
        std::transform(
            begin(m_Modules),
            end  (m_Modules),
            back_inserter(clonedModules),
            [&](const std::shared_ptr<const Node::Module>& t_module)
            {
                return t_module->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Type::IBase>> clonedTypes{};
        std::transform(
            begin(m_Types),
            end  (m_Types),
            back_inserter(clonedTypes),
            [&](const std::shared_ptr<const Node::Type::IBase>& t_type)
            {
                return t_type->CloneInScopeType(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Template::Type>> clonedTypeTemplates{};
        std::transform(
            begin(m_TypeTemplates),
            end  (m_TypeTemplates),
            back_inserter(clonedTypeTemplates),
            [&](const std::shared_ptr<const Node::Template::Type>& t_typeTemplate)
            {
                return t_typeTemplate->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Impl>> clonedImpls{};
        std::transform(
            begin(m_Impls),
            end  (m_Impls),
            back_inserter(clonedImpls),
            [&](const std::shared_ptr<const Node::Impl>& t_impl)
            {
                return t_impl->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::TemplatedImpl>> clonedTemplatedImpls{};
        std::transform(
            begin(m_TemplatedImpls),
            end  (m_TemplatedImpls),
            back_inserter(clonedTemplatedImpls),
            [&](const std::shared_ptr<const Node::TemplatedImpl>& t_templatedImpl)
            {
                return t_templatedImpl->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Function>> clonedFunctions{};
        std::transform(
            begin(m_Functions),
            end  (m_Functions),
            back_inserter(clonedFunctions),
            [&](const std::shared_ptr<const Node::Function>& t_function)
            {
                return t_function->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Template::Function>> clonedFunctionTemplates{};
        std::transform(
            begin(m_FunctionTemplates),
            end  (m_FunctionTemplates),
            back_inserter(clonedFunctionTemplates),
            [&](const std::shared_ptr<const Node::Template::Function>& t_functionTemplate)
            {
                return t_functionTemplate->CloneInScope(selfScope);
            }
        );

        std::vector<std::shared_ptr<const Node::Var::Normal::Static>> clonedVars{};
        std::transform(
            begin(m_Vars),
            end  (m_Vars),
            back_inserter(clonedVars),
            [&](const std::shared_ptr<const Node::Var::Normal::Static>& t_variable)
            {
                return t_variable->CloneInScope(selfScope);
            }
        );

        return std::make_shared<const Node::Module>(
            t_scope,
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

    auto Module::CreateBound() const -> Expected<std::shared_ptr<const BoundNode::Module>>
    {
        ACE_TRY(boundModules, TransformExpectedVector(m_Modules,
        [](const std::shared_ptr<const Node::Module>& t_module)
        {
            return t_module->CreateBound();
        }));

        ACE_TRY(boundTypes, TransformExpectedVector(m_Types,
        [](const std::shared_ptr<const Node::Type::IBase>& t_type)
        {
            return t_type->CreateBoundType();
        }));

        ACE_TRY(boundImpls, TransformExpectedVector(m_Impls,
        [](const std::shared_ptr<const Node::Impl>& t_impl)
        {
            return t_impl->CreateBound();
        }));

        ACE_TRY(boundTemplateImpls, TransformExpectedVector(m_TemplatedImpls,
        [](const std::shared_ptr<const Node::TemplatedImpl>& t_templatedImpl)
        {
            return t_templatedImpl->CreateBound();
        }));

        ACE_TRY(boundFunctions, TransformExpectedVector(m_Functions,
        [](const std::shared_ptr<const Node::Function>& t_function)
        {
            return t_function->CreateBound();
        }));

        ACE_TRY(boundVars, TransformExpectedVector(m_Vars,
        [](const std::shared_ptr<const Node::Var::Normal::Static>& t_variable)
        {
            return t_variable->CreateBound();
        }));

        auto* const selfSymbol =
            GetSymbolScope()->ExclusiveResolveSymbol<Symbol::Module>(GetName()).Unwrap();

        std::vector<std::shared_ptr<const BoundNode::Impl>> allBoundImpls{};
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

        return std::make_shared<const BoundNode::Module>(
            selfSymbol,
            boundModules,
            boundTypes,
            allBoundImpls,
            boundFunctions,
            boundVars
        );
    }

    auto Module::GetSymbolScope() const -> std::shared_ptr<Scope>
    {
        return m_SelfScope->GetParent().value();
    }

    auto Module::GetSymbolKind() const -> SymbolKind
    {
        return SymbolKind::Module;
    }

    auto Module::GetSymbolCreationSuborder() const -> size_t
    {
        return m_SelfScope->GetNestLevel();
    }

    auto Module::CreateSymbol() const -> Expected<std::unique_ptr<Symbol::IBase>>
    {
        if (m_Name.size() > 1)
        {
            std::shared_ptr<Scope> scope = GetSymbolScope()->GetParent().value();
            auto nameIt = rbegin(m_Name) + 1;

            for (
                ; 
                nameIt != rend(m_Name); 
                [&](){ scope = scope->GetParent().value(); nameIt++; }()
                )
            {
                ACE_TRY(symbol, scope->ExclusiveResolveSymbol<Symbol::Module>(*nameIt));
            }
        }

        return std::unique_ptr<Symbol::IBase>
        {
            std::make_unique<Symbol::Module>(
                m_SelfScope,
                GetName(),
                m_AccessModifier
            )
        };
    }

    auto Module::ContinueCreatingSymbol(
        Symbol::IBase* const t_symbol
    ) const -> Expected<void>
    {
        auto* const moduleSymbol = dynamic_cast<Symbol::Module*>(t_symbol);
        ACE_ASSERT(moduleSymbol);

        ACE_TRY_ASSERT(moduleSymbol->GetAccessModifier() == m_AccessModifier);

        return Void;
    }

    auto Module::GetName() const -> const std::string&
    {
        return m_Name.back();
    }
}
