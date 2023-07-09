#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <set>
#include <unordered_map>
#include <tuple>

#include "Diagnostics.hpp"
#include "Name.hpp"
#include "SymbolCategory.hpp"

namespace Ace
{
    class ISymbol;
    class ISelfScopedSymbol;
    class ModuleSymbol;
    class TemplatedImplSymbol;
    class ITypeSymbol;
    class ITemplateSymbol;

    class Scope;
    class ISymbolCreatable;
    class Compilation;

    auto IsSymbolVisibleFromScope(
        ISymbol* const t_symbol,
        const std::shared_ptr<const Scope>& t_scope
    ) -> bool;

    template<typename TSymbol>
    auto IsCorrectSymbolType(const ISymbol* const t_symbol) -> bool
    {
        return dynamic_cast<const TSymbol*>(t_symbol) != nullptr;
    }

    template<typename TSymbol>
    static constexpr auto IsTemplate() -> bool
    {
        return std::is_base_of_v<ITemplateSymbol, TSymbol>;
    }

    auto GetSymbolCategory(ISymbol* const t_symbol) -> SymbolCategory;

    struct SymbolResolutionData
    {
        SymbolResolutionData(
            const std::shared_ptr<const Scope>& t_resolvingFromScope,
            const std::vector<SymbolNameSection>::const_iterator t_nameSectionsBegin,
            const std::vector<SymbolNameSection>::const_iterator t_nameSectionsEnd,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes,
            const std::function<bool(const ISymbol* const)>& t_isCorrectSymbolType,
            const std::vector<std::shared_ptr<const Scope>>& t_scopes,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs,
            const bool t_isSymbolTemplate
        );

        std::shared_ptr<const Scope> ResolvingFromScope{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsBegin{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsEnd{};
        std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>> OptArgTypes{};
        std::function<bool(const ISymbol* const)> IsCorrectSymbolType{};
        std::vector<std::shared_ptr<const Scope>> Scopes{};
        std::vector<ITypeSymbol*> ImplTemplateArgs{};
        bool IsSymbolTemplate{};
        bool IsLastNameSection{};
        std::string Name{};
        std::string TemplateName{};
    };

    class GlobalScope
    {
    public:
        GlobalScope();
        GlobalScope(const Compilation* const t_compilation);
        ~GlobalScope();

        auto Unwrap() const -> const std::shared_ptr<Scope>&;

    private:
        std::shared_ptr<Scope> m_Scope{};
    };

    class Scope : public std::enable_shared_from_this<Scope>
    {
        friend GlobalScope;

    public:
        ~Scope();
        
        auto GetCompilation() const -> const Compilation*;
        auto GetNestLevel() const -> size_t;
        auto GetParent() const -> const std::optional<std::shared_ptr<Scope>>&;
        auto GetName() const -> const std::string&;
        auto FindModule() const -> std::optional<ModuleSymbol*>;

        auto GetOrCreateChild(
            const std::optional<std::string>& t_optName
        ) -> std::shared_ptr<Scope>;
        auto HasChild(
            const std::shared_ptr<const Scope>& t_scope
        ) const -> bool;

        template<typename TSymbol>
        static auto DefineSymbol(
            std::unique_ptr<TSymbol>&& t_symbol
        ) -> Expected<TSymbol*>
        {
            auto* const symbol = t_symbol.get();
            const auto scope = symbol->GetScope();
            ACE_TRY_ASSERT(scope->CanDefineSymbol(symbol));
            scope->m_SymbolMap[symbol->GetName()].push_back(std::move(t_symbol));
            return symbol;
        }
        static auto DefineSymbol(
            const ISymbolCreatable* const t_creatable
        ) -> Expected<ISymbol*>;
        static auto RemoveSymbol(
            ISymbol* const t_symbol
        ) -> void;

        auto DefineAssociation(
            const std::shared_ptr<Scope>& t_association
        ) -> void;

        static auto CreateArgTypes(
            ITypeSymbol* const t_argType
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;
        static auto CreateArgTypes(
            const std::vector<ITypeSymbol*>& t_argTypes
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;

        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const std::string& t_name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(t_name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const std::string& t_name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes
        ) const -> Expected<TSymbol*>
        {
            const SymbolName symbolName
            {
                SymbolNameSection{ t_name },
                SymbolNameResolutionScope::Local,
            };

            return ResolveStaticSymbol<TSymbol>(
                symbolName,
                t_optArgTypes
            );
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& t_name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(t_name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& t_name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes
        ) const -> Expected<TSymbol*>
        {
            ACE_TRY(startScope, GetStaticSymbolResolutionStartScope(t_name));

            std::vector<std::shared_ptr<const Scope>> startScopes{};
            startScopes.push_back(startScope);
            startScopes.insert(
                end(startScopes), 
                begin(startScope->m_Associations), 
                end  (startScope->m_Associations)
            );

            ACE_TRY(symbol, ResolveSymbolInScopes(SymbolResolutionData{
                shared_from_this(),
                t_name.Sections.begin(),
                t_name.Sections.end(),
                t_optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                startScopes,
                GetStaticSymbolResolutionImplTemplateArgs(shared_from_this()),
                IsTemplate<TSymbol>()
            }));

            ACE_TRY_ASSERT(
                GetSymbolCategory(symbol) == SymbolCategory::Static
            );

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const t_selfType,
            const SymbolNameSection& t_name
        ) const -> Expected<TSymbol*>
        {
            return ResolveInstanceSymbol<TSymbol>(
                t_selfType,
                t_name,
                {}
            );
        }
        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const t_selfType,
            const SymbolNameSection& t_name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes
        ) const -> Expected<TSymbol*>
        {
            const std::vector nameSections{ t_name };

            const auto scopes = GetInstanceSymbolResolutionScopes(t_selfType);

            ACE_TRY(symbol, ResolveSymbolInScopes({
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                t_optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                CollectInstanceSymbolResolutionImplTemplateArgs(t_selfType),
                IsTemplate<TSymbol>()
            }));

            ACE_TRY_ASSERT(
                GetSymbolCategory(symbol) == SymbolCategory::Instance
            );

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(t_name, {});
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                t_name,
                t_optArgTypes,
                {},
                {}
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                t_name,
                {},
                t_implTemplateArgs,
                t_templateArgs
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) const -> Expected<TSymbol*>
        {
            std::vector<SymbolNameSection> nameSections
            { 
                SymbolNameSection { t_name }
            };

            const std::vector<std::shared_ptr<const Scope>> scopes
            {
                shared_from_this()
            };

            const SymbolResolutionData data
            {
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                t_optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                t_implTemplateArgs,
                IsTemplate<TSymbol>()
            };

            ACE_TRY(symbol, ResolveSymbolInScopes(data, t_templateArgs));

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto CollectSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
            [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<ISymbol>>&>& t_pair)
            {
                std::for_each(begin(t_pair.second), end(t_pair.second),
                [&](const std::unique_ptr<ISymbol>& t_symbol)
                {
                    auto* const symbol = dynamic_cast<TSymbol*>(
                        t_symbol.get()
                    );
                    if (!symbol)
                        return;

                    symbols.push_back(symbol);
                });
            });

            return symbols;
        }
        template<typename TSymbol>
        auto CollectSymbolsRecursive() const -> std::vector<TSymbol*>
        {
            auto symbols = CollectSymbols<TSymbol>();

            std::for_each(begin(m_Children), end(m_Children),
            [&](const std::weak_ptr<Scope>& t_child)
            {
                const auto childSymbols = t_child.lock()->CollectSymbolsRecursive<TSymbol>();
                symbols.insert(
                    end(symbols), 
                    begin(childSymbols), 
                    end  (childSymbols)
                );
            });

            return symbols;
        }

        auto CollectAllDefinedSymbols() const -> std::vector<ISymbol*>;
        auto CollectAllDefinedSymbolsRecursive() const -> std::vector<ISymbol*>;

        static auto ResolveOrInstantiateTemplateInstance(
            const Compilation* const t_compilation,
            ITemplateSymbol* const t_template,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& t_optArgTypes,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) -> Expected<ISymbol*>;

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*>;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>;

        auto DefineTemplateArgAliases(
            const std::vector<std::string>& t_implTemplateParamNames,
            const std::vector<ITypeSymbol*> t_implTemplateArgs,
            const std::vector<std::string>& t_templateParamNames,
            const std::vector<ITypeSymbol*> t_templateArgs
        ) -> Expected<void>;

    private:
        Scope(const Compilation* const t_compilation);
        auto Clear() -> void;

        Scope(
            const Compilation* const t_compilation,
            const std::optional<std::string>& t_optName,
            const std::optional<std::shared_ptr<Scope>>& t_optParent
        );

        auto AddChild(const std::optional<std::string>& t_optName) -> std::shared_ptr<Scope>;

        auto CanDefineSymbol(const ISymbol* const t_symbol) -> bool;
        auto GetDefinedSymbol(
            const std::string& t_name,
            const std::vector<ITypeSymbol*>& t_templateArgs,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs
        ) -> std::optional<ISymbol*>;

        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& t_data
        ) -> Expected<ISymbol*>;
        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& t_data,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) -> Expected<ISymbol*>;

        auto CollectMatchingSymbols(
            const SymbolResolutionData& t_data,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) const -> std::vector<ISymbol*>;
        auto CollectMatchingNormalSymbols(
            const SymbolResolutionData& t_data,
        const std::vector<ITypeSymbol*>& t_templateArgs,
            const std::vector<std::unique_ptr<ISymbol>>& t_matchingNameSymbols
        ) const -> std::vector<ISymbol*>;
        auto CollectMatchingTemplateSymbols(
            const SymbolResolutionData& t_data,
            const std::vector<ITypeSymbol*>& t_templateArgs,
            const std::vector<std::unique_ptr<ISymbol>>& t_matchingNameSymbols
        ) const -> std::vector<ISymbol*>;

        static auto ResolveTemplateInstance(
            const ITemplateSymbol* const t_template,
            const std::vector<ITypeSymbol*>& t_implTemplateArgs,
            const std::vector<ITypeSymbol*>& t_templateArgs
        ) -> Expected<ISymbol*>;

        auto ExclusiveResolveTemplate(
            const std::string& t_name
        ) const -> Expected<ITemplateSymbol*>;

        static auto GetInstanceSymbolResolutionScopes(
            ITypeSymbol* t_selfType
        ) -> std::vector<std::shared_ptr<const Scope>>;
        static auto CollectInstanceSymbolResolutionImplTemplateArgs(
            ITypeSymbol* const t_selfType
        ) -> std::vector<ITypeSymbol*>;

        auto GetStaticSymbolResolutionStartScope(
            const SymbolName& t_name
        ) const -> Expected<std::shared_ptr<const Scope>>;
        static auto GetStaticSymbolResolutionImplTemplateArgs(
            const std::shared_ptr<const Scope>& t_startScope
        ) -> std::vector<ITypeSymbol*>;

        const Compilation* m_Compilation{};
        size_t m_NestLevel{};
        std::string m_Name{};
        std::optional<std::shared_ptr<Scope>> m_OptParent{};
        std::vector<std::weak_ptr<Scope>> m_Children{};
        std::set<std::shared_ptr<Scope>> m_Associations{};
        std::unordered_map<std::string, std::vector<std::unique_ptr<ISymbol>>> m_SymbolMap;
    };
}
