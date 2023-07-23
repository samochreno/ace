#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <set>
#include <unordered_map>
#include <tuple>

#include "Diagnostic.hpp"
#include "Name.hpp"
#include "SymbolCategory.hpp"
#include "Ident.hpp"
#include "SpecialIdent.hpp"

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
        ISymbol* const symbol,
        const std::shared_ptr<const Scope>& scope
    ) -> bool;

    template<typename TSymbol>
    auto IsCorrectSymbolType(const ISymbol* const symbol) -> bool
    {
        return dynamic_cast<const TSymbol*>(symbol) != nullptr;
    }

    template<typename TSymbol>
    static constexpr auto IsTemplate() -> bool
    {
        return std::is_base_of_v<ITemplateSymbol, TSymbol>;
    }

    auto GetSymbolCategory(ISymbol* const symbol) -> SymbolCategory;

    struct SymbolResolutionData
    {
        SymbolResolutionData(
            const std::shared_ptr<const Scope>& resolvingFromScope,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsBegin,
            const std::vector<SymbolNameSection>::const_iterator nameSectionsEnd,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::function<bool(const ISymbol* const)>& isCorrectSymbolType,
            const std::vector<std::shared_ptr<const Scope>>& scopes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const bool isTemplate
        ) : ResolvingFromScope{ resolvingFromScope },
            NameSectionsBegin{ nameSectionsBegin },
            NameSectionsEnd{ nameSectionsEnd },
            OptArgTypes{ optArgTypes },
            IsCorrectSymbolType{ isCorrectSymbolType },
            Scopes{ scopes },
            ImplTemplateArgs{ implTemplateArgs },
            IsTemplate{ isTemplate },
            IsLastNameSection
            {
                std::distance(nameSectionsBegin, nameSectionsEnd) == 1
            },
            Name{ nameSectionsBegin->Name.String },
            TemplateName{ SpecialIdent::CreateTemplate(Name) }
        {
        }

        std::shared_ptr<const Scope> ResolvingFromScope{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsBegin{};
        std::vector<SymbolNameSection>::const_iterator NameSectionsEnd{};
        std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>> OptArgTypes{};
        std::function<bool(const ISymbol* const)> IsCorrectSymbolType{};
        std::vector<std::shared_ptr<const Scope>> Scopes{};
        std::vector<ITypeSymbol*> ImplTemplateArgs{};
        bool IsTemplate{};
        bool IsLastNameSection{};
        std::string Name{};
        std::string TemplateName{};
    };

    class GlobalScope
    {
    public:
        GlobalScope();
        GlobalScope(const Compilation* const compilation);
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
            const std::optional<std::string>& optName
        ) -> std::shared_ptr<Scope>;
        auto HasChild(
            const std::shared_ptr<const Scope>& scope
        ) const -> bool;

        template<typename TSymbol>
        static auto DefineSymbol(
            std::unique_ptr<TSymbol>&& ownedSymbol
        ) -> Expected<TSymbol*>
        {
            auto* const symbol = ownedSymbol.get();
            const auto scope = symbol->GetScope();
            ACE_TRY_ASSERT(scope->CanDefineSymbol(symbol));
            scope->m_SymbolMap[symbol->GetName().String].push_back(
                std::move(ownedSymbol)
            );
            return symbol;
        }
        static auto DefineSymbol(
            const ISymbolCreatable* const creatable
        ) -> Expected<ISymbol*>;
        static auto RemoveSymbol(
            ISymbol* const symbol
        ) -> void;

        auto DefineAssociation(
            const std::shared_ptr<Scope>& association
        ) -> void;

        static auto CreateArgTypes(
            ITypeSymbol* const argType
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;
        static auto CreateArgTypes(
            const std::vector<ITypeSymbol*>& argTypes
        ) -> std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>;

        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const Ident& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            const SymbolName symbolName
            {
                SymbolNameSection{ name },
                SymbolNameResolutionScope::Local,
            };

            return ResolveStaticSymbol<TSymbol>(
                symbolName,
                optArgTypes
            );
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveStaticSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ResolveStaticSymbol(
            const SymbolName& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            ACE_TRY(startScope, GetStaticSymbolResolutionStartScope(name));

            std::vector<std::shared_ptr<const Scope>> startScopes{};
            startScopes.push_back(startScope);
            startScopes.insert(
                end(startScopes), 
                begin(startScope->m_Associations), 
                end  (startScope->m_Associations)
            );

            ACE_TRY(symbol, ResolveSymbolInScopes(SymbolResolutionData{
                shared_from_this(),
                name.Sections.begin(),
                name.Sections.end(),
                optArgTypes,
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
            ITypeSymbol* const selfType,
            const SymbolNameSection& name
        ) const -> Expected<TSymbol*>
        {
            return ResolveInstanceSymbol<TSymbol>(
                selfType,
                name,
                {}
            );
        }
        template<typename TSymbol>
        auto ResolveInstanceSymbol(
            ITypeSymbol* const selfType,
            const SymbolNameSection& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            const std::vector nameSections{ name };

            const auto scopes = GetInstanceSymbolResolutionScopes(selfType);

            ACE_TRY(symbol, ResolveSymbolInScopes({
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                CollectInstanceSymbolResolutionImplTemplateArgs(selfType),
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
            const Ident& name
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(name, {});
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                name,
                optArgTypes,
                {},
                {}
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                name,
                {},
                implTemplateArgs,
                templateArgs
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const Ident& name,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) const -> Expected<TSymbol*>
        {
            std::vector<SymbolNameSection> nameSections
            { 
                SymbolNameSection{ name }
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
                optArgTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                implTemplateArgs,
                IsTemplate<TSymbol>()
            };

            ACE_TRY(symbol, ResolveSymbolInScopes(data, templateArgs));

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto CollectSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
            [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<ISymbol>>&>& pair)
            {
                std::for_each(begin(pair.second), end(pair.second),
                [&](const std::unique_ptr<ISymbol>& ownedSymbol)
                {
                    auto* const symbol = dynamic_cast<TSymbol*>(
                        ownedSymbol.get()
                    );
                    if (!symbol)
                    {
                        return;
                    }

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
            [&](const std::weak_ptr<Scope>& child)
            {
                const auto childSymbols = child.lock()->CollectSymbolsRecursive<TSymbol>();
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
            const Compilation* const compilation,
            ITemplateSymbol* const t3mplate,
            const std::optional<std::reference_wrapper<const std::vector<ITypeSymbol*>>>& optArgTypes,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) -> Expected<ISymbol*>;

        auto CollectTemplateArgs() const -> std::vector<ITypeSymbol*>;
        auto CollectImplTemplateArgs() const -> std::vector<ITypeSymbol*>;

        auto DefineTemplateArgAliases(
            const std::vector<Ident>& implTemplateParamNames,
            const std::vector<ITypeSymbol*> implTemplateArgs,
            const std::vector<Ident>& templateParamNames,
            const std::vector<ITypeSymbol*> templateArgs
        ) -> Expected<void>;

    private:
        Scope(const Compilation* const compilation);
        auto Clear() -> void;

        Scope(
            const Compilation* const compilation,
            const std::optional<std::string>& optName,
            const std::optional<std::shared_ptr<Scope>>& optParent
        );

        auto AddChild(const std::optional<std::string>& optName) -> std::shared_ptr<Scope>;

        auto CanDefineSymbol(const ISymbol* const symbol) -> bool;
        auto GetDefinedSymbol(
            const std::string& name,
            const std::vector<ITypeSymbol*>& templateArgs,
            const std::vector<ITypeSymbol*>& implTemplateArgs
        ) -> std::optional<ISymbol*>;

        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& data
        ) -> Expected<ISymbol*>;
        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& data,
            const std::vector<ITypeSymbol*>& templateArgs
        ) -> Expected<ISymbol*>;

        auto CollectMatchingSymbols(
            const SymbolResolutionData& data,
            const std::vector<ITypeSymbol*>& templateArgs
        ) const -> std::vector<ISymbol*>;
        auto CollectMatchingNormalSymbols(
            const SymbolResolutionData& data,
        const std::vector<ITypeSymbol*>& templateArgs,
            const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
        ) const -> std::vector<ISymbol*>;
        auto CollectMatchingTemplateSymbols(
            const SymbolResolutionData& data,
            const std::vector<ITypeSymbol*>& templateArgs,
            const std::vector<std::unique_ptr<ISymbol>>& matchingNameSymbols
        ) const -> std::vector<ISymbol*>;

        static auto ResolveTemplateInstance(
            const ITemplateSymbol* const t3mplate,
            const std::vector<ITypeSymbol*>& implTemplateArgs,
            const std::vector<ITypeSymbol*>& templateArgs
        ) -> Expected<ISymbol*>;

        auto ExclusiveResolveTemplate(
            const std::string& name
        ) const -> Expected<ITemplateSymbol*>;

        static auto GetInstanceSymbolResolutionScopes(
            ITypeSymbol* selfType
        ) -> std::vector<std::shared_ptr<const Scope>>;
        static auto CollectInstanceSymbolResolutionImplTemplateArgs(
            ITypeSymbol* const selfType
        ) -> std::vector<ITypeSymbol*>;

        auto GetStaticSymbolResolutionStartScope(
            const SymbolName& name
        ) const -> Expected<std::shared_ptr<const Scope>>;
        static auto GetStaticSymbolResolutionImplTemplateArgs(
            const std::shared_ptr<const Scope>& startScope
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
