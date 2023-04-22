#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <set>
#include <unordered_map>
#include <tuple>

#include "Token.hpp"
#include "Error.hpp"
#include "Symbol/Base.hpp"
#include "Symbol/SelfScoped.hpp"
#include "SymbolCreatable.hpp"
#include "SpecialIdentifier.hpp"
#include "Name.hpp"

namespace Ace::Symbol
{
    class Module;
    class TemplatedImpl;

    namespace Type
    {
        class IBase;
    }

    namespace Template
    {
        class IBase;
    }
}

namespace Ace
{
    auto IsSymbolVisibleFromScope(Symbol::IBase* const t_symbol, const Scope* const t_scope) -> bool;

    template<typename TSymbol>
    auto IsCorrectSymbolType(const Symbol::IBase* const t_symbol) -> bool
    {
        return dynamic_cast<const TSymbol*>(t_symbol) != nullptr;
    }

    class Scope
    {
    public:
#ifndef NDEBUG
        auto GetDebugSymbolMap() const -> std::vector<std::string>;
#endif

        ~Scope() = default;
        
        static auto GetRoot() -> Scope*;

        auto GetNestLevel() const -> size_t { return m_NestLevel; }
        auto GetParent() const -> Scope* { return m_Parent; }
        auto GetName() const -> const std::string& { return m_Name; }
        
        auto FindModule() const -> std::optional<Symbol::Module*>;

        auto GetOrCreateChild(const std::optional<std::string>& t_optName) -> Scope*;
        auto HasChild(const Scope* const t_child) const -> bool;

        template<typename TSymbol>
        static auto DefineSymbol(std::unique_ptr<TSymbol>&& t_symbol) -> Expected<TSymbol*>
        {
            auto* const symbol = t_symbol.get();
            auto* const scope = symbol->GetScope();
            ACE_TRY_ASSERT(scope->CanDefineSymbol(symbol));
            scope->m_SymbolMap[symbol->GetName()].push_back(std::move(t_symbol));
            return symbol;
        }
        static auto DefineSymbol(const ISymbolCreatable* const t_creatable) -> Expected<Symbol::IBase*>
        {
            auto* const scope = t_creatable->GetSymbolScope();

            if (auto partiallyCreatable = dynamic_cast<const IPartiallySymbolCreatable*>(t_creatable))
            {
                if (auto optDefinedSymbol = scope->GetDefinedSymbol(partiallyCreatable->GetName(), {}, {}))
                {
                    ACE_TRY_VOID(partiallyCreatable->ContinueCreatingSymbol(optDefinedSymbol.value()));
                    return optDefinedSymbol.value();
                }
            }

            ACE_TRY(symbol, t_creatable->CreateSymbol());
            return DefineSymbol(std::move(symbol));
        }

        auto DefineAssociation(Scope* const t_association) -> void
        {
            m_Associations.insert(t_association);
        }

        template<typename TSymbol>
        static auto DoInstantiateTemplate() -> bool
        {
            return !std::is_base_of_v<Symbol::Template::IBase, TSymbol>;
        }

        template<typename TSymbol>
        auto ResolveStaticSymbol(const std::string& t_name) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(Name::Symbol::Full{ Name::Symbol::Section{ t_name }, false });
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(const Name::Symbol::Full& t_name) const -> Expected<TSymbol*>
        {
            const auto [startScope, isInTemplatedImplContext] = [&]() -> std::tuple<const Scope*, bool>
            {
                if (t_name.IsGlobal)
                {
                    return { Scope::GetRoot(), false };
                }

                const auto& section = t_name.Sections.front();

                const auto& name = section.Name;
                const auto templateName = SpecialIdentifier::CreateTemplate(name);

                const Scope* scope = this;
                for (; scope; scope = scope->GetParent())
                {
                    if (scope->m_SymbolMap.contains(templateName))
                    {
                        if (section.TemplateArguments.size() != 0)
                        {
                            return { scope, false };
                        }

                        if (IsSameTemplatedImplContext(this, scope))
                        {
                            return { scope, true };
                        }
                    }

                    if (scope->m_SymbolMap.contains(name))
                    {
                        if (section.TemplateArguments.empty())
                        {
                            return { scope, false };
                        }
                    }
                }

                return { nullptr, false };
            }();

            ACE_TRY_ASSERT(startScope);

            std::vector<const Scope*> startScopes{};

            startScopes.push_back(startScope);
            startScopes.insert(end(startScopes), begin(startScope->m_Associations), end(startScope->m_Associations));

            ACE_TRY(symbol, ResolveSymbolInScopes(
                this,
                t_name.Sections.begin(),
                t_name.Sections.end(),
                IsCorrectSymbolType<TSymbol>,
                startScopes,
                GetStaticSymbolResolutionImplTemplateArguments(isInTemplatedImplContext, this),
                DoInstantiateTemplate<TSymbol>()
            ));

            ACE_TRY_ASSERT(symbol->GetSymbolCategory() == SymbolCategory::Static);

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }
        
        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            Symbol::Type::IBase* const t_selfType,
            const Name::Symbol::Section& t_name
        ) const -> Expected<TSymbol*>
        {
            const auto scopes = GetInstanceSymbolResolutionScopes(t_selfType);

            const std::vector nameSections{ t_name };

            ACE_TRY(symbol, ResolveSymbolInScopes(
                this,
                nameSections.begin(),
                nameSections.end(),
                IsCorrectSymbolType<TSymbol>,
                scopes,
                CollectInstanceSymbolImplTemplateArguments(t_selfType),
                DoInstantiateTemplate<TSymbol>()
            ));

            ACE_TRY_ASSERT(symbol->GetSymbolCategory() == SymbolCategory::Instance);

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(t_name, {}, {});
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) const -> Expected<TSymbol*>
        {
            std::vector<Name::Symbol::Section> nameSections{ Name::Symbol::Section{ t_name } };

            ACE_TRY(symbol, ResolveSymbolInScopes(
                this,
                nameSections.begin(),
                nameSections.end(),
                IsCorrectSymbolType<TSymbol>,
                { this },
                t_implTemplateArguments,
                t_templateArguments,
                DoInstantiateTemplate<TSymbol>()
            ));

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto CollectDefinedSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap), [&]
            (const std::pair<const std::string&, const std::vector<std::unique_ptr<Symbol::IBase>>&>& t_pair)
            {
                std::for_each(begin(t_pair.second), end(t_pair.second), [&]
                (const std::unique_ptr<Symbol::IBase>& t_symbol)
                {
                    if (auto* const symbol = dynamic_cast<TSymbol*>(t_symbol.get()))
                    {
                        symbols.push_back(symbol);
                    }
                });
            });

            return symbols;
        }
        template<typename TSymbol>
        auto CollectDefinedSymbolsRecursive() const -> std::vector<TSymbol*>
        {
            auto symbols = CollectDefinedSymbols<TSymbol>();

            std::for_each(begin(m_Children), end(m_Children), [&]
            (const std::unique_ptr<Scope>& t_child)
            {
                auto childSymbols = t_child->CollectDefinedSymbolsRecursive<TSymbol>();
                symbols.insert(end(symbols), begin(childSymbols), end(childSymbols));
            });

            return symbols;
        }

        auto CollectAllDefinedSymbols() const -> std::vector<Symbol::IBase*>;
        auto CollectAllDefinedSymbolsRecursive() const -> std::vector<Symbol::IBase*>;

        static auto ResolveOrInstantiateTemplateInstance(
            Symbol::Template::IBase* const t_template,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) -> Expected<Symbol::IBase*>;

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>;

        auto DefineTemplateArgumentAliases(
            const std::vector<std::string>& t_implTemplateParameters,
            const std::vector<Symbol::Type::IBase*> t_implTemplateArguments,
            const std::vector<std::string>& t_templateParameters,
            const std::vector<Symbol::Type::IBase*> t_templateArguments
        ) -> Expected<void>;

    private:
        Scope(
            const std::optional<std::string>& t_optName,
            Scope* const t_parent
        ) : m_NestLevel{ t_parent ? (t_parent->GetNestLevel() + 1) : 0},
            m_Name{ t_optName ? t_optName.value() : SpecialIdentifier::CreateAnonymous() },
            m_Parent{ t_parent }
        {
        }

        auto AddChild(const std::optional<std::string>& t_optName) -> Scope*;

        static auto ResolveSymbolInScopes(
            const Scope* const t_resolvingFromScope,
            const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsBegin, 
            const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsEnd,
            const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
            const std::vector<const Scope*> t_scopes,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const bool& t_doInstantiateTemplate
        ) -> Expected<Symbol::IBase*>;

        static auto ResolveSymbolInScopes(
            const Scope* const t_resolvingFromScope,
            const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsBegin,
            const std::vector<Name::Symbol::Section>::const_iterator& t_nameSectionsEnd,
            const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
            const std::vector<const Scope*> t_scopes,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments,
            const bool& t_doInstantiateTemplate
        ) -> Expected<Symbol::IBase*>;

        static auto ResolveTemplateInstance(
            const Symbol::Template::IBase* const t_template,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) -> Expected<Symbol::IBase*>;

        auto ExclusiveResolveTemplate(
            const std::string& t_name,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) const -> Expected<Symbol::Template::IBase*>;

        auto CanDefineSymbol(const Symbol::IBase* const t_symbol) -> bool;
        auto GetDefinedSymbol(
            const std::string& t_name,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments
        ) -> std::optional<Symbol::IBase*>;

        static auto GetInstanceSymbolResolutionScopes(Symbol::Type::IBase* t_selfType) -> std::vector<const Scope*>;
        static auto CollectInstanceSymbolImplTemplateArguments(Symbol::Type::IBase* const t_selfType) -> std::vector<Symbol::Type::IBase*>;

        static auto GetStaticSymbolResolutionImplTemplateArguments(const bool& t_isTemplate, const Scope* const t_startScope) -> std::vector<Symbol::Type::IBase*>;

        auto ResolveTemplateArguments(const std::vector<Name::Symbol::Full>& t_templateArgumentNames) const -> Expected<std::vector<Symbol::Type::IBase*>>;

        auto FindTemplatedImplContext() const -> Symbol::TemplatedImpl*;
        static auto IsSameTemplatedImplContext(const Scope* const t_scopeA, const Scope* const t_scopeB) -> bool;

        static auto AreTypesSame(
            const std::vector<Symbol::Type::IBase*>& t_typesA,
            const std::vector<Symbol::Type::IBase*>& t_typesB
        ) -> bool;

        inline static std::unique_ptr<Scope> m_Root{};

        size_t m_NestLevel{};
        std::string m_Name{};
        Scope* m_Parent{};
        std::vector<std::unique_ptr<Scope>> m_Children{};
        std::set<Scope*> m_Associations{};
        std::unordered_map<std::string, std::vector<std::unique_ptr<Symbol::IBase>>> m_SymbolMap{};
    };
}
