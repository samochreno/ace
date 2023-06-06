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

namespace Ace::Symbol
{
    class IBase;
    class ISelfScoped;

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
    class Scope;
    class ISymbolCreatable;
    class Compilation;

    auto IsSymbolVisibleFromScope(
        Symbol::IBase* const t_symbol,
        const std::shared_ptr<const Scope>& t_scope
    ) -> bool;

    template<typename TSymbol>
    auto IsCorrectSymbolType(const Symbol::IBase* const t_symbol) -> bool
    {
        return dynamic_cast<const TSymbol*>(t_symbol) != nullptr;
    }

    template<typename TSymbol>
    static constexpr auto IsTemplate() -> bool
    {
        return std::is_base_of_v<Symbol::Template::IBase, TSymbol>;
    }

    auto GetSymbolCategory(Symbol::IBase* const t_symbol) -> SymbolCategory;

    struct SymbolResolutionData
    {
        SymbolResolutionData(
            const std::shared_ptr<const Scope>& t_resolvingFromScope,
            const std::vector<SymbolNameSection>::const_iterator& t_nameSectionsBegin,
            const std::vector<SymbolNameSection>::const_iterator& t_nameSectionsEnd,
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes,
            const std::function<bool(const Symbol::IBase* const)>& t_isCorrectSymbolType,
            const std::vector<std::shared_ptr<const Scope>>& t_scopes,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const bool t_isSymbolTemplate
        );

        const std::shared_ptr<const Scope>& ResolvingFromScope{};
        const std::vector<SymbolNameSection>::const_iterator& NameSectionsBegin{};
        const std::vector<SymbolNameSection>::const_iterator& NameSectionsEnd{};
        const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& OptArgumentTypes{};
        const std::function<bool(const Symbol::IBase* const)>& IsCorrectSymbolType{};
        const std::vector<std::shared_ptr<const Scope>>& Scopes{};
        const std::vector<Symbol::Type::IBase*>& ImplTemplateArguments{};
        const bool IsSymbolTemplate{};
        const bool IsLastNameSection{};
        const std::string& Name{};
        const std::string TemplateName{};
    };

    class Scope : public std::enable_shared_from_this<Scope>
    {
    public:
        int ChildCount = 0;

        Scope(const Compilation& t_compilation);
        ~Scope();
        
        auto GetCompilation() const -> const Compilation& { return m_Compilation; }
        auto GetNestLevel() const -> size_t { return m_NestLevel; }
        auto GetParent() const -> const std::optional<std::shared_ptr<Scope>>& { return m_OptParent; }
        auto GetName() const -> const std::string& { return m_Name; }
        auto FindModule() const -> std::optional<Symbol::Module*>;

        auto GetOrCreateChild(
            const std::optional<std::string>& t_optName
        ) -> std::shared_ptr<Scope>;
        auto HasChild(const std::shared_ptr<const Scope>& t_scope) const -> bool;

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
        ) -> Expected<Symbol::IBase*>;
        static auto RemoveSymbol(
            Symbol::IBase* const t_symbol
        ) -> void;

        auto DefineAssociation(const std::shared_ptr<Scope>& t_association) -> void;

        static auto CreateArgumentTypes(
            Symbol::Type::IBase* const t_argumentType
        ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>;
        static auto CreateArgumentTypes(
            const std::vector<Symbol::Type::IBase*>& t_argumentTypes
        ) -> std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>;

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
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes
        ) const -> Expected<TSymbol*>
        {
            const SymbolName symbolName
            {
                SymbolNameSection{ t_name },
                SymbolNameResolutionScope::Local,
            };

            return ResolveStaticSymbol<TSymbol>(
                symbolName,
                t_optArgumentTypes
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
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes
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
                t_optArgumentTypes,
                IsCorrectSymbolType<TSymbol>,
                startScopes,
                GetStaticSymbolResolutionImplTemplateArguments(shared_from_this()),
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
            Symbol::Type::IBase* const t_selfType,
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
            Symbol::Type::IBase* const t_selfType,
            const SymbolNameSection& t_name,
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes
        ) const -> Expected<TSymbol*>
        {
            const std::vector nameSections{ t_name };

            const auto scopes = GetInstanceSymbolResolutionScopes(t_selfType);

            ACE_TRY(symbol, ResolveSymbolInScopes({
                shared_from_this(),
                nameSections.begin(),
                nameSections.end(),
                t_optArgumentTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                CollectInstanceSymbolResolutionImplTemplateArguments(t_selfType),
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
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                t_name,
                t_optArgumentTypes,
                {},
                {}
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) const -> Expected<TSymbol*>
        {
            return ExclusiveResolveSymbol<TSymbol>(
                t_name,
                {},
                t_implTemplateArguments,
                t_templateArguments
            );
        }
        template<typename TSymbol>
        auto ExclusiveResolveSymbol(
            const std::string& t_name,
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
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
                t_optArgumentTypes,
                IsCorrectSymbolType<TSymbol>,
                scopes,
                t_implTemplateArguments,
                IsTemplate<TSymbol>()
            };

            ACE_TRY(symbol, ResolveSymbolInScopes(data, t_templateArguments));

            auto* const castedSymbol = dynamic_cast<TSymbol*>(symbol);
            ACE_TRY_ASSERT(castedSymbol);

            return castedSymbol;
        }

        template<typename TSymbol>
        auto CollectSymbols() const -> std::vector<TSymbol*>
        {
            std::vector<TSymbol*> symbols{};

            std::for_each(begin(m_SymbolMap), end(m_SymbolMap),
            [&](const std::pair<const std::string&, const std::vector<std::unique_ptr<Symbol::IBase>>&>& t_pair)
            {
                std::for_each(begin(t_pair.second), end(t_pair.second),
                [&](const std::unique_ptr<Symbol::IBase>& t_symbol)
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

        auto CollectAllDefinedSymbols() const -> std::vector<Symbol::IBase*>;
        auto CollectAllDefinedSymbolsRecursive() const -> std::vector<Symbol::IBase*>;

        static auto ResolveOrInstantiateTemplateInstance(
            const Compilation& t_compilation,
            Symbol::Template::IBase* const t_template,
            const std::optional<std::reference_wrapper<const std::vector<Symbol::Type::IBase*>>>& t_optArgumentTypes,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) -> Expected<Symbol::IBase*>;

        auto CollectTemplateArguments() const -> std::vector<Symbol::Type::IBase*>;
        auto CollectImplTemplateArguments() const -> std::vector<Symbol::Type::IBase*>;

        auto DefineTemplateArgumentAliases(
            const std::vector<std::string>& t_implTemplateParameterNames,
            const std::vector<Symbol::Type::IBase*> t_implTemplateArguments,
            const std::vector<std::string>& t_templateParameterNames,
            const std::vector<Symbol::Type::IBase*> t_templateArguments
        ) -> Expected<void>;

    private:
        Scope(
            const Compilation& t_compilation,
            const std::optional<std::string>& t_optName,
            const std::optional<std::shared_ptr<Scope>>& t_optParent
        );

        auto AddChild(const std::optional<std::string>& t_optName) -> std::shared_ptr<Scope>;

        auto CanDefineSymbol(const Symbol::IBase* const t_symbol) -> bool;
        auto GetDefinedSymbol(
            const std::string& t_name,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments
        ) -> std::optional<Symbol::IBase*>;

        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& t_data
        ) -> Expected<Symbol::IBase*>;
        static auto ResolveSymbolInScopes(
            const SymbolResolutionData& t_data,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) -> Expected<Symbol::IBase*>;

        auto CollectMatchingSymbols(
            const SymbolResolutionData& t_data,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) const -> std::vector<Symbol::IBase*>;
        auto CollectMatchingNormalSymbols(
            const SymbolResolutionData& t_data,
        const std::vector<Symbol::Type::IBase*>& t_templateArguments,
            const std::vector<std::unique_ptr<Symbol::IBase>>& t_nameMatchedSymbols
        ) const -> std::vector<Symbol::IBase*>;
        auto CollectMatchingTemplateSymbols(
            const SymbolResolutionData& t_data,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments,
            const std::vector<std::unique_ptr<Symbol::IBase>>& t_nameMatchedSymbols
        ) const -> std::vector<Symbol::IBase*>;

        static auto ResolveTemplateInstance(
            const Symbol::Template::IBase* const t_template,
            const std::vector<Symbol::Type::IBase*>& t_implTemplateArguments,
            const std::vector<Symbol::Type::IBase*>& t_templateArguments
        ) -> Expected<Symbol::IBase*>;

        auto ExclusiveResolveTemplate(
            const std::string& t_name
        ) const -> Expected<Symbol::Template::IBase*>;

        static auto GetInstanceSymbolResolutionScopes(
            Symbol::Type::IBase* t_selfType
        ) -> std::vector<std::shared_ptr<const Scope>>;
        static auto CollectInstanceSymbolResolutionImplTemplateArguments(
            Symbol::Type::IBase* const t_selfType
        ) -> std::vector<Symbol::Type::IBase*>;

        auto GetStaticSymbolResolutionStartScope(
            const SymbolName& t_name
        ) const -> Expected<std::shared_ptr<const Scope>>;
        static auto GetStaticSymbolResolutionImplTemplateArguments(
            const std::shared_ptr<const Scope>& t_startScope
        ) -> std::vector<Symbol::Type::IBase*>;

        const Compilation& m_Compilation;
        size_t m_NestLevel{};
        std::string m_Name{};
        std::optional<std::shared_ptr<Scope>> m_OptParent{};
        std::vector<std::weak_ptr<Scope>> m_Children{};
        std::set<std::shared_ptr<Scope>> m_Associations{};
        std::unordered_map<std::string, std::vector<std::unique_ptr<Symbol::IBase>>> m_SymbolMap;
    };
}
