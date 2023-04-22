#include "Scope.hpp"

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>

#include "Asserts.hpp"
#include "Error.hpp"
#include "SpecialIdentifier.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/Type/Base.hpp"
#include "Symbol/Module.hpp"
#include "Symbol/Typed.hpp"
#include "Symbol/Templatable.hpp"
#include "Symbol/Template/Base.hpp"
#include "Symbol/Template/Type.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Impl.hpp"
#include "Symbol/Type/Alias/TemplateArgument/Normal.hpp"
#include "SymbolCreatable.hpp"
#include "Symbol/TemplatedImpl.hpp"
#include "BoundNode/Expression/Base.hpp"
#include "BoundNode/Expression/ConversionPlaceholder.hpp"

namespace Ace
{
    auto IsSymbolVisibleFromScope(Symbol::IBase* const t_symbol, const Scope* const t_scope) -> bool
    {
        switch (t_symbol->GetAccessModifier())
        {
            case AccessModifier::Public:
            {
                return true;
            }

            case AccessModifier::Private:
            {
                const auto optSymbolModule = t_symbol->GetScope()->FindModule();
                if (!optSymbolModule.has_value())
                    return true; 

                auto* const symbolModule = optSymbolModule.value();

                const auto optScopeModule = t_scope->FindModule();
                if (!optScopeModule.has_value())
                    return false;

                auto* const scopeModule = optScopeModule.value();

                if (symbolModule == scopeModule)
                    return true;

                if (symbolModule->GetSelfScope()->HasChild(scopeModule->GetSelfScope()))
                    return true;

                return false;
            }

            default:
            {
                ACE_UNREACHABLE();
            }
        }
    }

#ifndef NDEBUG
    auto Scope::GetDebugSymbolMap() const -> std::vector<std::string>
    {
        std::vector<std::string> vec{};
        for (const auto& kv : m_SymbolMap)
        {
            vec.push_back(kv.first);
        }
        return vec;
    }
#endif

    auto Scope::GetRoot() -> Scope*
    {
        if (m_Root)
            return m_Root.get();

        m_Root = std::unique_ptr<Scope>
        { 
            new Scope(
                std::string{ SpecialIdentifier::Global }, 
                std::optional<Scope*>{}
            )
        };

        return m_Root.get();
    }

    auto Scope::FindModule() const -> std::optional<Symbol::Module*>
    {
        auto* child = this;
        auto optParent = child->GetParent();

        const auto moveUp = [&]() -> void
        {
            child = optParent.value(); 
            optParent = optParent.value()->GetParent();
        };

        for (; optParent.has_value(); moveUp())
        {
            auto* const parent = optParent.value();

            const auto foundIt = parent->m_SymbolMap.find(child->GetName());

            if (foundIt == end(parent->m_SymbolMap))
                continue;

            if (foundIt->second.size() != 1)
                continue;

            auto* const moduleSymbol = dynamic_cast<Symbol::Module*>(foundIt->second.front().get());
            if (!moduleSymbol)
                continue;

            return moduleSymbol;
        }

        return std::nullopt;
    }

    auto Scope::GetOrCreateChild(const std::optional<std::string>& t_optName) -> Scope*
    {
        if (t_optName)
        {
            auto findIt = std::find_if(begin(m_Children), end(m_Children), [&]
            (const std::unique_ptr<Scope>& t_child)
            {
                return t_child->m_Name == t_optName.value();
            });

            if (findIt != end(m_Children))
                return findIt->get();
        }

        return AddChild(t_optName);
    }

    auto Scope::HasChild(const Scope* const t_child) const -> bool
    {
        const auto foundIt = std::find_if(begin(m_Children), end(m_Children), [&]
        (const std::unique_ptr<Scope>& t_scope)
        {
            if (t_scope.get() == t_child)
                return true;

            if (t_scope->HasChild(t_child))
                return true;

            return false;
        });

        return foundIt != end(m_Children);
    }

    auto Scope::CollectAllDefinedSymbols() const -> std::vector<Symbol::IBase*>
    {
        std::vector<Symbol::IBase*> symbols{};

        std::for_each(begin(m_SymbolMap), end(m_SymbolMap), [&]
        (const std::pair<const std::string&, const std::vector<std::unique_ptr<Symbol::IBase>>&>& t_pair)
        {
            std::transform(begin(t_pair.second), end(t_pair.second), back_inserter(symbols), []
            (const std::unique_ptr<Symbol::IBase>& t_symbol)
            {
                return t_symbol.get();
            });
        });

        return symbols;
    }

    auto Scope::CollectAllDefinedSymbolsRecursive() const -> std::vector<Symbol::IBase*>
    {
        auto symbols = CollectAllDefinedSymbols();

        std::for_each(begin(m_Children), end(m_Children), [&]
        (const std::unique_ptr<Scope>& t_child)
        {
            auto childSymbols = t_child->CollectAllDefinedSymbolsRecursive();
            symbols.insert(end(symbols), begin(childSymbols), end(childSymbols));
        });

        return symbols;
    }
    
    auto Scope::ResolveOrInstantiateTemplateInstance(
        Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) -> Expected<Symbol::IBase*>
    {
        if (auto expResolvedInstance = ResolveTemplateInstance(
            t_template,
            t_implTemplateArguments,
            t_templateArguments
        ))
        {
            return expResolvedInstance.Unwrap();
        }

        return t_template->InstantiateSymbols(
            t_implTemplateArguments,
            t_templateArguments
        );
    }

    auto Scope::CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases = CollectDefinedSymbols<Symbol::Type::Alias::TemplateArgument::Normal>();
        std::sort(begin(aliases), end(aliases), []
        (
            const Symbol::Type::Alias::TemplateArgument::Normal* const t_lhs,
            const Symbol::Type::Alias::TemplateArgument::Normal* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> arguments{};
        std::transform(begin(aliases), end(aliases), back_inserter(arguments), []
        (const Symbol::Type::Alias::TemplateArgument::Normal* const t_alias)
        {
            return t_alias->GetAliasedType();
        });

        return arguments;
    }

    auto Scope::CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>
    {
        auto aliases = CollectDefinedSymbols<Symbol::Type::Alias::TemplateArgument::Impl>();
        std::sort(begin(aliases), end(aliases), []
        (
            const Symbol::Type::Alias::TemplateArgument::Impl* const t_lhs,
            const Symbol::Type::Alias::TemplateArgument::Impl* const t_rhs
            )
        {
            return t_lhs->GetIndex() < t_rhs->GetIndex();
        });

        std::vector<Symbol::Type::IBase*> arguments{};
        std::transform(begin(aliases), end(aliases), back_inserter(arguments), []
        (const Symbol::Type::Alias::TemplateArgument::Impl* const t_alias)
        {
            return t_alias->GetAliasedType();
        });

        return arguments;
    }

    auto Scope::DefineTemplateArgumentAliases(
        const std::vector<std::string>& t_implTemplateParameters, 
        const std::vector<Symbol::Type::IBase*> t_implTemplateArguments, 
        const std::vector<std::string>& t_templateParameters, 
        const std::vector<Symbol::Type::IBase*> t_templateArguments
    ) -> Expected<void>
    {
        ACE_TRY_ASSERT(t_implTemplateParameters.size() == t_implTemplateArguments.size());
        ACE_TRY_ASSERT(t_templateParameters.size() == t_templateArguments.size());

        for (size_t i = 0; i < t_implTemplateParameters.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArgument::Impl>(
                this,
                t_implTemplateParameters.at(i),
                t_implTemplateArguments.at(i),
                i
                );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        for (size_t i = 0; i < t_templateParameters.size(); i++)
        {
            auto aliasSymbol = std::make_unique<Symbol::Type::Alias::TemplateArgument::Normal>(
                this,
                t_templateParameters.at(i),
                t_templateArguments.at(i),
                i
                );

            ACE_TRY(symbol, DefineSymbol(std::move(aliasSymbol)));
        }

        return ExpectedVoid;
    }

    Scope::Scope(
        const std::optional<std::string>& t_optName,
        std::optional<Scope*> const t_optParent
    ) : m_OptParent{ t_optParent }
    {
        m_NestLevel = m_OptParent.has_value() ?
            m_OptParent.value()->GetNestLevel() + 1 :
            0;

        m_Name = t_optName.has_value() ? 
            t_optName.value() :
            SpecialIdentifier::CreateAnonymous();
    }

    auto Scope::AddChild(const std::optional<std::string>& t_optName) -> Scope*
    {
        m_Children.emplace_back(new Scope(t_optName, this));
        return m_Children.back().get();
    }

    auto Scope::ResolveSymbolInScopes(
        const Scope* const t_resolvingFromScope,
        const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsBegin,
        const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsEnd,
        const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
        const std::vector<const Scope*> t_scopes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const bool& t_doInstantiateTemplate
    ) -> Expected<Symbol::IBase*>
    {
        ACE_TRY(templateArguments, t_resolvingFromScope->ResolveTemplateArguments(t_nameSectionsBegin->TemplateArguments));
        return ResolveSymbolInScopes(
            t_resolvingFromScope,
            t_nameSectionsBegin,
            t_nameSectionsEnd,
            t_isCorrectSymbolType,
            t_scopes,
            t_implTemplateArguments,
            templateArguments,
            t_doInstantiateTemplate
        );
    }

    auto Scope::ResolveSymbolInScopes(
        const Scope* const t_resolvingFromScope,
        const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsBegin,
        const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsEnd,
        const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
        const std::vector<const Scope*> t_scopes,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const bool& t_doInstantiateTemplate
    ) -> Expected<Symbol::IBase*>
    {
        const bool isLastNameSection = std::distance(t_nameSectionsBegin, t_nameSectionsEnd) == 1;

        const auto& name = t_nameSectionsBegin->Name;
        const auto templateName = SpecialIdentifier::CreateTemplate(name);

        std::vector<Symbol::IBase*> symbols{};
        std::for_each(begin(t_scopes), end(t_scopes), [&]
        (const Scope* const t_scope)
        {
            auto& symbolMap = t_scope->m_SymbolMap;

            const bool isTemplate = symbolMap.find(templateName) != end(symbolMap);
            const auto foundIt = symbolMap.find(isTemplate ? templateName : name);

            const bool isInstanceVariable = [&]() -> bool
            {
                if (foundIt == end(symbolMap))
                    return false;

                if (!isLastNameSection)
                    return false;

                if (foundIt->second.size() != 1)
                    return false;
                
                if (!dynamic_cast<Symbol::Variable::Normal::Instance*>(foundIt->second.front().get()))
                    return false;

                return true;
            }();

            if (isTemplate && isLastNameSection && !isInstanceVariable)
            {
                const auto expTemplate = t_scope->ExclusiveResolveTemplate(
                    templateName,
                    t_implTemplateArguments,
                    t_templateArguments
                );
                if (!expTemplate)
                    return;
                
                if (!t_doInstantiateTemplate)
                {
                    symbols.push_back(expTemplate.Unwrap());
                    return;
                }

                const auto expTemplateInstance = ResolveOrInstantiateTemplateInstance(
                    expTemplate.Unwrap(),
                    t_implTemplateArguments,
                    t_templateArguments
                );
                if (!expTemplateInstance)
                    return;
                
                symbols.push_back(expTemplateInstance.Unwrap());
            }
            else
            {
                if (foundIt == end(symbolMap))
                    return;

                std::transform(begin(foundIt->second), end(foundIt->second), back_inserter(symbols), [&]
                (const std::unique_ptr<Symbol::IBase>& t_symbol)
                {
                    return t_symbol.get();
                });
            }
        });

        if (isLastNameSection)
        {
            std::vector<Symbol::IBase*> correctSymbols{};
            std::copy_if(begin(symbols), end(symbols), back_inserter(correctSymbols), [&]
            (Symbol::IBase* const t_symbol)
            {
                return t_isCorrectSymbolType(t_symbol);
            });

            ACE_TRY_ASSERT(correctSymbols.size() > 0);
            ACE_ASSERT(correctSymbols.size() == 1);

            auto* const symbol = correctSymbols.front();
            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(symbol, t_resolvingFromScope));
            return symbol;
        }
        else
        {
            ACE_TRY_ASSERT(symbols.size() == 1);

            auto* const selfScopedSymbol = dynamic_cast<Symbol::ISelfScoped*>(symbols.front());
            ACE_TRY_ASSERT(selfScopedSymbol);

            ACE_TRY_ASSERT(IsSymbolVisibleFromScope(selfScopedSymbol, t_resolvingFromScope));

            std::vector<const Scope*> scopes{};

            scopes.push_back(selfScopedSymbol->GetSelfScope());

            const auto& selfScopeAssociations = selfScopedSymbol->GetSelfScope()->m_Associations;
            scopes.insert(end(scopes), begin(selfScopeAssociations), end(selfScopeAssociations));

            return ResolveSymbolInScopes(
                t_resolvingFromScope,
                t_nameSectionsBegin + 1,
                t_nameSectionsEnd,
                t_isCorrectSymbolType,
                scopes,
                t_templateArguments,
                t_doInstantiateTemplate
            );
        }
    }

    auto Scope::ResolveTemplateInstance(
        const Symbol::Template::IBase* const t_template,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) -> Expected<Symbol::IBase*>
    {
        auto* const scope = t_template->GetScope();

        const auto foundIt = scope->m_SymbolMap.find(t_template->GetASTName());
        ACE_TRY_ASSERT(foundIt != end(scope->m_SymbolMap));

        auto& symbols = foundIt->second;

        const auto perfectCandidateIt = std::find_if(begin(symbols), end(symbols), [&]
        (const std::unique_ptr<Symbol::IBase>& t_symbol)
        {
            auto* const templatableSymbol = dynamic_cast<Symbol::ITemplatable*>(t_symbol.get());
            ACE_ASSERT(templatableSymbol);

            if (!AreTypesSame(t_templateArguments, templatableSymbol->CollectTemplateArguments()))
                return false;

            if (!AreTypesSame(t_implTemplateArguments, templatableSymbol->CollectImplTemplateArguments()))
                return false;

            return true;
        });

        ACE_TRY_ASSERT(perfectCandidateIt != end(symbols));
        return perfectCandidateIt->get();
    }

    auto Scope::ExclusiveResolveTemplate(
        const std::string& t_name,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments
    ) const -> Expected<Symbol::Template::IBase*>
    {
        auto foundIt = m_SymbolMap.find(t_name);
        ACE_TRY_ASSERT(foundIt != end(m_SymbolMap));

        std::vector<Symbol::IBase*> symbols{};
        std::transform(begin(foundIt->second), end(foundIt->second), back_inserter(symbols), []
        (const std::unique_ptr<Symbol::IBase>& t_symbol)
        {
            return t_symbol.get();
        });

        ACE_TRY_ASSERT(symbols.size() == 1);

        auto* const castedSymbol = dynamic_cast<Symbol::Template::IBase*>(symbols.front());
        ACE_ASSERT(castedSymbol);

        return castedSymbol;
    }

    auto Scope::CanDefineSymbol(const Symbol::IBase* const t_symbol) -> bool
    {
        // TODO: Dont allow private types to leak in public interface.

#if 0 // This doesnt work for associated functions parameters, needs rework.
        if (auto typedSymbol = dynamic_cast<const Symbol::IBase*>(t_symbol))
        {
            if (
                (typedSymbol->GetType()->GetAccessModifier() == AccessModifier::Private) && 
                (typedSymbol->GetAccessModifier() != AccessModifier::Private)
                )
                return false;
        }
#endif

        const auto [templateArguments, implTemplateArguments] = [&]() -> std::tuple<std::vector<Symbol::Type::IBase*>, std::vector<Symbol::Type::IBase*>>
        {
            const auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(t_symbol);
            if (!templatableSymbol)
                return {};

            return 
            { 
                templatableSymbol->CollectTemplateArguments(), 
                templatableSymbol->CollectImplTemplateArguments() 
            };
        }();

        return !GetDefinedSymbol(t_symbol->GetName(), templateArguments, implTemplateArguments).has_value();
    }

    auto Scope::GetDefinedSymbol(
        const std::string& t_name,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
        const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments
    ) -> std::optional<Symbol::IBase*>
    {
        auto foundIt = m_SymbolMap.find(t_name);
        ACE_TRY_ASSERT(foundIt != end(m_SymbolMap));

        const bool isTemplateInstance = !t_templateArguments.empty() || !t_implTemplateArguments.empty();
        if (isTemplateInstance)
        {
            const auto perfectMatchIt = std::find_if(begin(foundIt->second), end(foundIt->second), [&]
            (const std::unique_ptr<Symbol::IBase>& t_symbol)
            {
                auto* const templatableSymbol = dynamic_cast<const Symbol::ITemplatable*>(t_symbol.get());
                ACE_ASSERT(templatableSymbol);

                const auto templateArguments = templatableSymbol->CollectTemplateArguments();
                const auto implTemplateArguments = templatableSymbol->CollectImplTemplateArguments();

                if (!AreTypesSame(templateArguments, t_templateArguments))
                    return false;

                if (!AreTypesSame(implTemplateArguments, t_implTemplateArguments))
                    return false;

                return true;
            });

            return (perfectMatchIt == end(foundIt->second)) ?
                std::optional<Symbol::IBase*>{} :
                perfectMatchIt->get();
        }
        else
        {
            ACE_ASSERT(foundIt->second.size() == 1);
            return foundIt->second.front().get();
        }
    }

    auto Scope::GetInstanceSymbolResolutionScopes(Symbol::Type::IBase* t_selfType) -> std::vector<const Scope*>
    {
        t_selfType = t_selfType->GetUnaliased();

        if (t_selfType->IsReference())
        {
            return GetInstanceSymbolResolutionScopes(t_selfType->GetWithoutReference());
        }

        if (t_selfType->IsStrongPointer())
        {
            return GetInstanceSymbolResolutionScopes(t_selfType->GetWithoutStrongPointer());
        }
        
        std::vector<const Scope*> scopes{};
        if (auto optTemplate = t_selfType->GetTemplate())
        {
            auto* const templateSelfScope = optTemplate.value()->GetSelfScope();

            scopes.push_back(t_selfType->GetSelfScope());
            scopes.insert(end(scopes), begin(templateSelfScope->m_Associations), end(templateSelfScope->m_Associations));
        }
        else
        {
            auto* const typeSelfScope = t_selfType->GetSelfScope();

            scopes.push_back(typeSelfScope);
            scopes.insert(end(scopes), begin(typeSelfScope->m_Associations), end(typeSelfScope->m_Associations));
        }

        return scopes;
    }

    auto Scope::CollectInstanceSymbolImplTemplateArguments(Symbol::Type::IBase* const t_selfType) -> std::vector<Symbol::Type::IBase*>
    {
        return t_selfType->CollectTemplateArguments();
    }

    auto Scope::ResolveTemplateArguments(const std::vector<Name::Symbol::Full>& t_templateArgumentNames) const -> Expected<std::vector<Symbol::Type::IBase*>>
    {
        ACE_TRY(templateArguments, TransformExpectedVector(t_templateArgumentNames, [&]
        (const Name::Symbol::Full& t_typeName)
        {
            return ResolveStaticSymbol<Symbol::Type::IBase>(t_typeName);
        }));

        return templateArguments;
    }

    auto Scope::FindTemplatedImplContext() const -> Symbol::TemplatedImpl*
    {
        const auto* scope = this;
        for (
            ; 
            scope->GetParent().has_value(); 
            scope = scope->GetParent().value()
            )
        {
            auto* const parentScope = scope->GetParent().value();

            const auto templatedImplSymbols = parentScope->CollectDefinedSymbols<Symbol::TemplatedImpl>();
            auto foundIt = std::find_if(begin(templatedImplSymbols), end(templatedImplSymbols), [&]
            (Symbol::TemplatedImpl* const t_templatedImpl)
            {
                return t_templatedImpl->GetSelfScope() == scope;
            });

            if (foundIt == end(templatedImplSymbols))
                continue;

            return *foundIt;
        }

        return nullptr;
    }

    auto Scope::IsSameTemplatedImplContext(const Scope* const t_scopeA, const Scope* const t_scopeB) -> bool
    {
        return 
            t_scopeA->FindTemplatedImplContext()->GetImplementedTypeTemplate() == 
            t_scopeB->FindTemplatedImplContext()->GetImplementedTypeTemplate();
    }

    auto Scope::GetStaticSymbolResolutionImplTemplateArguments(const bool& t_isInTemplatedImplContext, const Scope* const t_startScope) -> std::vector<Symbol::Type::IBase*>
    {
        if (!t_isInTemplatedImplContext)
            return {};

        for (
            std::optional<const Scope*> optScope = t_startScope;
            optScope.has_value(); 
            optScope = optScope.value()->GetParent()
            )
        {
            auto* const scope = optScope.value();

            const auto implTemplateArguments = scope->CollectImplTemplateArguments();
            if (!implTemplateArguments.empty())
            {
                return implTemplateArguments;
            }
        }

        ACE_UNREACHABLE();
    }

    auto Scope::AreTypesSame(
        const std::vector<Symbol::Type::IBase*>& t_typesA,
        const std::vector<Symbol::Type::IBase*>& t_typesB
    ) -> bool
    {
        if (t_typesA.size() != t_typesB.size())
            return false;

        for (size_t i = 0; i < t_typesA.size(); i++)
        {
            if (
                t_typesA.at(i)->GetUnaliased() !=
                t_typesB.at(i)->GetUnaliased()
                )
                return false;
        }

        return true;
    }
}
